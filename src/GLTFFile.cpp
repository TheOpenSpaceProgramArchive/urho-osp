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

    // Start loading meshes
    if (rootObj.Contains("meshes"))
    {
        if (!rootObj["meshes"]->IsArray())
        {
            URHO3D_LOGERROR("Mesh section must be an array");
            return false;
        }

        // Put accessors into it's own variable for ParsePrimitive later
        if (rootObj.Contains("accessors"))
        {
            if (!rootObj["accessors"]->IsArray())
            {
                URHO3D_LOGERROR("Mesh section must be an array");
                return false;
            }

            accessors_ = rootObj["accessors"]->GetArray();
        }


        const JSONArray& meshs = rootObj["meshes"]->GetArray();

        // Loop through meshs array
        for (int i = 0; i < meshs.Size(); i ++)
        {
            URHO3D_LOGINFO("now loading a mesh...");

            //
            SharedPtr<Model> model(new Model(context_));

            if (!meshs[i].IsObject())
            {
                URHO3D_LOGERROR("Incorrect definition of mesh");
                return false;
            }

            const JSONObject& mesh = meshs[i].GetObject();

            if (mesh.Contains("name"))
            {
                model->SetName(mesh["name"]->GetString());
            }

            if (!mesh.Contains("primitives"))
            {
                URHO3D_LOGERROR("Mesh has no primetives");
                return false;
            }

            if (!mesh["primitives"]->IsArray())
            {
                URHO3D_LOGERROR("Primitives section must be an array");
                return false;
            }

            const JSONArray& primitives = mesh["primitives"]->GetArray();

            for (int j = 0; j < primitives.Size(); j ++)
            {
                if (!primitives[j].IsObject())
                {
                    URHO3D_LOGERROR("Primitives must be an object");
                    return false;
                }

                // Call ParsePrimitive because there are too many code blocks
                if (!ParsePrimitive(primitives[j].GetObject(), *model))
                {
                    return false;
                }

            }

            meshs_.Push(model);
        }

    }

}


bool GLTFFile::ParsePrimitive(const JSONObject &object, const Model &model)
{
    // Start parsing attributes
    if (!object.Contains("attributes"))
    {
        URHO3D_LOGERROR("Missing attributes");
        return false;
    }

    if (!object["attributes"]->IsObject())
    {
        URHO3D_LOGERROR("Attributes must be an object");
        return false;
    }

    const JSONObject& attributes = object["attributes"]->GetObject();

    if (attributes.Contains("POSITION"))
    {
        if (attributes["POSITION"]->IsNumber())
        {
            URHO3D_LOGINFO("This part of the code has been reached!");
        } else {
            URHO3D_LOGERROR("Positions attribute must be a number. Model will be messed up, this is not fatal.");
        }
    }
}
