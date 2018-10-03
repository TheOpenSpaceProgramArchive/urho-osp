#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/Resource.h>

using namespace Urho3D;

class GLTFFile : public Resource
{

    URHO3D_OBJECT(GLTFFile, Resource)

public:

    GLTFFile(Context* context);
    virtual ~GLTFFile();

    static void RegisterObject(Context* context);

    virtual bool BeginLoad(Deserializer& source);

};

