#pragma once

#include <Urho3D/Container/Vector.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Texture.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/JSONFile.h>

using namespace Urho3D;

// Refer to:
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0

/// Combined accessor and bufferView
struct BufferAccessor
{
    unsigned count; // How many elements
    unsigned components; // How many in each element
    unsigned componentType; // Size of each component

    unsigned buffer; // Index to buffers_
    unsigned bufferLength; // in bytes
    unsigned bufferOffset; // also in bytes
    unsigned bufferTarget; // not sure if this is needed

    unsigned vertexOffset; // Used when assembling VertexBuffer

    VertexElement vertexElement;
    //VertexElementSemantic vertexSemantic;
    //VertexElementType vertexType;
};

/// Used for async loading
struct AsyncBufferData
{
    IndexBuffer* indBuff_;
    SharedArrayPtr<unsigned char> indData_;
    unsigned indCount_;
    bool indLarge_;

    VertexBuffer* vertBuff_;
    SharedArrayPtr<unsigned char> vertData_;
    PODVector<VertexElement> vertexElements_;
    unsigned vertCount_;

    Geometry* geometry_;
};

class GLTFFile : public JSONFile
{

    URHO3D_OBJECT(GLTFFile, JSONFile)

public:

    GLTFFile(Context* context);
    virtual ~GLTFFile();

    static void RegisterObject(Context* context);
    static void ComponentTypeByteSize(unsigned componentType);
    static unsigned TypeComponentCount(const String& type);

    bool BeginLoad(Deserializer& source) override;
    bool EndLoad() override;

    SharedPtr<Node> GetNode(unsigned index) const;
    SharedPtr<Scene> GetScene(unsigned index) const;
    void GetScene(unsigned index, Node* addTo) const;
    const Vector<SharedPtr<Model>>& GetMeshs() const { return meshs_; }

private:

    JSONValue accessors_;
    JSONValue nodes_;
    JSONValue scenes_;
    JSONValue views_;

    //Vector<SharedArrayPtr<unsigned char>> buffers_;
    Vector<SharedPtr<File>> buffers_;
    Vector<SharedPtr<Model>> meshs_;
    Vector<AsyncBufferData> asyncLoading_;
    //Vector<WeakPtr<Texture>> textures_;
    //Vector<Vector<Pair<SharedArrayPtr<unsigned char>, SharedArrayPtr<unsigned char>>>> bufData;

    bool ParsePrimitive(const JSONObject& object, Model& model, Vector<SharedPtr<VertexBuffer> >& vertList, Vector<SharedPtr<IndexBuffer> >& indList);
    BufferAccessor ParseAccessor(unsigned index);
};

