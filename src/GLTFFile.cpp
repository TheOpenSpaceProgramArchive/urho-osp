#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>


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

    //URHO3D_LOGINFO("A file that might be a GLTF has been loaded.");

    // See this for more information
    // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0

    //root_.GetObject().operator []()
    const JSONObject& rootObj = GetRoot().GetObject();

    // Check if the GLTF file defines any buffers
    // Some of them don't have any
    if (rootObj.Contains("buffers"))
    {
        // Load buffers

        if (!rootObj["buffers"]->IsArray())
        {
            URHO3D_LOGERROR("Buffers section must be an array");
            return false;
        }

        const JSONArray& buffers = rootObj["buffers"]->GetArray();

        // Loop through buffers array
        for (int i = 0; i < buffers.Size(); i ++)
        {
            URHO3D_LOGINFO("now loading a buffer...");
            if (!buffers[i].IsObject())
            {
                URHO3D_LOGERROR("Incorrect definition of buffer");
                return false;
            }

            const JSONObject& buffer = buffers[i].GetObject();

            JSONValue* uri = buffer["uri"];

            if (uri == NULL)
            {
                URHO3D_LOGERROR("Buffer has no uri");
                return false;
            }

            if (!uri->IsString())
            {
                URHO3D_LOGERROR("Buffer uri must be a string");
                return false;
            }

            //URHO3D_LOGINFO(uri->GetString());
            //URHO3D_LOGINFO(source.GetName());

            // Set binPath to the path to the GLTF resource, ("path/to/file.gltf")
            String binPath(source.GetName());

            // Trim off the filename to get directory, and add the .bin filename ("path/to/" + "binary.bin")
            binPath = binPath.Substring(0, binPath.FindLast('/') + 1) + uri->GetString();
            URHO3D_LOGINFO("Binpath: " + binPath);

            // The GLTF binary is just raw vertex/index/texCoord/... data, no headers or anything else
            SharedPtr<File> binFile = GetSubsystem<ResourceCache>()->GetFile(binPath, false);

            if (!binFile)
                {
                    URHO3D_LOGERROR("Failed to load GLTF binary file: " + binPath);
                }

            URHO3D_LOGINFOF("Binary Data Size: %u bytes", binFile->GetSize());
            //FileSystem* fileSystem = GetSubsystem<FileSystem>();

            SharedArrayPtr<unsigned char> binData(new unsigned char[binFile->GetSize()]);
            source.Read(binData.Get(), binFile->GetSize());

            // Buffer has been loaded!
            buffers_.Push(binData);

        }
    }


}
