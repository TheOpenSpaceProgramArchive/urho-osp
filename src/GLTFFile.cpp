#include "GLTFFile.h"

GLTFFile::GLTFFile(Context* context) :
    Resource(context)
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

}
