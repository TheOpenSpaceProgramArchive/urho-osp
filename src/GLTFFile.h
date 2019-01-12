#pragma once

#include <Urho3D/Container/Vector.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Math/Matrix3x4.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/Image.h>

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
    unsigned bufferStride;

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
    static const Vector3 ParseVector3(const JSONValue* value);
    static const Matrix3x4 ParseMatrix(const JSONValue* value);
    static const Quaternion ParseQuaternion(const JSONValue* value);
    static const String StringValue(const JSONValue* value);

    bool BeginLoad(Deserializer& source) override;
    bool EndLoad() override;

    SharedPtr<Node> GetNode(unsigned index) const;
    SharedPtr<Scene> GetScene(unsigned index) const;
    void GetScene(unsigned index, Node* addTo) const;
    inline String UriToResourcePath(const String& in) const;
    const Vector<SharedPtr<Material>>& GetMaterials() const { return materials_; }
    const Vector<SharedPtr<Model>>& GetMeshes() const { return meshes_; }

private:

    JSONValue accessors_;
    JSONValue nodes_;
    JSONValue scenes_;
    JSONValue views_;

    //Vector<SharedArrayPtr<unsigned char>> buffers_;
    Vector<SharedPtr<File>> buffers_;
    Vector<SharedPtr<Model>> meshes_;
    Vector<SharedPtr<Material>> materials_;
    Vector<SharedPtr<Image>> images_;
    Vector<SharedPtr<Texture2D>> textures_;
    Vector<PODVector<int>> meshMaterialIndices_;
    Vector<AsyncBufferData> asyncLoading_;

    //Vector<WeakPtr<Texture>> textures_;
    //Vector<Vector<Pair<SharedArrayPtr<unsigned char>, SharedArrayPtr<unsigned char>>>> bufData;

    bool ParsePrimitive(const JSONObject& object, int modelIndex, Model& model, Vector<SharedPtr<VertexBuffer> >& vertList, Vector<SharedPtr<IndexBuffer> >& indList);
    bool ParseMaterial(const JSONObject& object);
    BufferAccessor ParseAccessor(unsigned index);
};

