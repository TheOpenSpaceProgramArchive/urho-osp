#include <Urho3D/IO/Log.h>

#include "GLTFFile.h"

GLTFFile::GLTFFile(Context* context) :
    JSONFile(context)
{

}

GLTFFile::~GLTFFile()
{

}

void GLTFFile::RegisterObject(Context* context)
{
    context->RegisterFactory<GLTFFile>("GLTFFile");
}

bool GLTFFile::BeginLoad(Deserializer& source)
{
    JSONFile::BeginLoad(source);
    URHO3D_LOGINFO("A file that might be a GLTF has been loaded.");


}
