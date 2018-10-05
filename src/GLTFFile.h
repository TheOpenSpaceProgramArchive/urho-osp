#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Graphics/Texture.h>
#include <Urho3D/Resource/Resource.h>
#include <Urho3D/Resource/JSONFile.h>


using namespace Urho3D;

class GLTFFile : public JSONFile
{

    URHO3D_OBJECT(GLTFFile, JSONFile)

public:

    GLTFFile(Context* context);
    virtual ~GLTFFile();

    static void RegisterObject(Context* context);

    virtual bool BeginLoad(Deserializer& source);

private:
    Vector<SharedArrayPtr<unsigned char>> buffers_;
    Vector<WeakPtr<Texture>> textures_;
};

