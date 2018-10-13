#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Texture.h>
#include <Urho3D/Resource/Resource.h>
#include <Urho3D/Resource/JSONFile.h>


using namespace Urho3D;

// Refer to:
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0

// combined accessor and bufferView
struct BufferAccessor
{
    unsigned count; // How many elements
    unsigned components; // How many in each element

    unsigned buffer; // Index to buffers_
    unsigned bufferLength; // in bytes
    unsigned bufferOffset; // also in bytes
    unsigned bufferTarget; // not sure if this is needed
};

class GLTFFile : public JSONFile
{

    URHO3D_OBJECT(GLTFFile, JSONFile)

public:

    GLTFFile(Context* context);
    virtual ~GLTFFile();

    static void RegisterObject(Context* context);

    virtual bool BeginLoad(Deserializer& source);

private:

    JSONArray accessors_;

    Vector<SharedArrayPtr<unsigned char>> buffers_;
    Vector<SharedPtr<Model>> meshs_;
    //Vector<WeakPtr<Texture>> textures_;

    bool ParsePrimitive(const JSONObject& object, const Model& model);
    BufferAccessor ParseAccessor();
};

