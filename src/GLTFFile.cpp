#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Math/Sphere.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "GLTFFile.h"

GLTFFile::GLTFFile(Context* context) :
    JSONFile(context), meshs_()
{

}

GLTFFile::~GLTFFile()
{

}

void GLTFFile::RegisterObject(Context* context)
{
    context->RegisterFactory<GLTFFile>("GLTFFile");
}

unsigned GLTFFile::TypeComponentCount(const String& type)
{
    // no type is less than 4 characters
    if (type.Length() < 4)
    {
        return 0;
    }
    // only SCALAR starts with S
    if (type.StartsWith("S"))
    {
        return 1;
    }
    // VEC
    else if (type.StartsWith("V"))
    {
        // either VEC2, VEC3, VEC4
        // Use the ASCII code of the number
        unsigned length = type[3] - 48;
        // Check if in range
        if (length <= 4)
        {
            return length;
        } else {
            return 0;
        }
    }
    // MAT
    else if (type.StartsWith("M"))
    {

    }
}

/**
 * Called by Urho3D. Real loading begins here
 */
bool GLTFFile::BeginLoad(Deserializer& source)
{
    // Load the JSON
    JSONFile::BeginLoad(source);

    // See this for more information, very important to understanding this
    // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0

    // Get root object
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

            // Push the binFile into the buffers_ array
            buffers_.Push(binFile);

            //FileSystem* fileSystem = GetSubsystem<FileSystem>();

            // Loading the the same way as Urho3D source code

            // Allocate
            //SharedArrayPtr<unsigned char> binData(new unsigned char[binFile->GetSize()]);

            // Then put binary file data into it
            //source.Read(binData.Get(), binFile->GetSize());

            // Buffer has been loaded!
            //buffers_.Push(binData);

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

        // Put accessors and bufferViews into their own variables for ParseAccessor to access later
        if (rootObj.Contains("accessors") && rootObj.Contains("bufferViews"))
        {
            if (!rootObj["accessors"]->IsArray())
            {
                URHO3D_LOGERROR("accessors must be an array");
                return false;
            }
            if (!rootObj["bufferViews"]->IsArray())
            {
                URHO3D_LOGERROR("bufferViews must be an array");
                return false;
            }

            accessors_ = *(rootObj["accessors"]);
            views_ = *(rootObj["bufferViews"]);
        }

        // Meshs section in the GLTF file
        const JSONArray& meshs = rootObj["meshes"]->GetArray();

        // Loop through meshs array
        for (int i = 0; i < meshs.Size(); i ++)
        {
            URHO3D_LOGINFO("now loading a mesh...");

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

            Vector<SharedPtr<VertexBuffer> > vertList;
            Vector<SharedPtr<IndexBuffer> > indList;

            for (int j = 0; j < primitives.Size(); j ++)
            {
                if (!primitives[j].IsObject())
                {
                    URHO3D_LOGERROR("Primitives must be an object");
                    return false;
                }

                // Call ParsePrimitive because there are too many code blocks
                if (!ParsePrimitive(primitives[j].GetObject(), *model, vertList, indList))
                {
                    return false;
                }

            }

            model->SetBoundingBox(BoundingBox(Sphere(Vector3(0, 0, 0), 4)));

            PODVector<unsigned> morphRangeStarts;
            PODVector<unsigned> morphRangeCounts;
            morphRangeStarts.Push(0);
            morphRangeCounts.Push(0);
            model->SetVertexBuffers(vertList, morphRangeStarts, morphRangeCounts);
            model->SetIndexBuffers(indList);


            URHO3D_LOGINFO("AAAAAASDSADASDASd");
            GetSubsystem<ResourceCache>()->AddManualResource(model);
            meshs_.Push(model);
        }

    }

    return true;

}

bool GLTFFile::ParsePrimitive(const JSONObject &object, Model &model, Vector<SharedPtr<VertexBuffer> >& vertList, Vector<SharedPtr<IndexBuffer> >& indList)
{
    // Index of new geometry to add
    unsigned index = model.GetNumGeometries();

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

    PODVector<VertexElement> elements;
    PODVector<BufferAccessor> bufferAccessors;
    BufferAccessor indBuffAcc;
    Geometry* geometry = new Geometry(context_);
    unsigned vertexByteSize = 0;
    unsigned vertexCount = 0;

    // Gather information on vertex buffer

    // Load position data
    if (attributes.Contains("POSITION"))
    {
        if (attributes["POSITION"]->IsNumber())
        {
            // Read from JSON and into this BufferAccessor
            BufferAccessor bufAcc(ParseAccessor(attributes["POSITION"]->GetUInt()));

            // Check if ParseAccessor was succesful
            // See the line that reads "result.buffer = buffers_.Size();"
            if (bufAcc.buffer == buffers_.Size())
            {
                return false;
            }

            bufAcc.vertexElement = VertexElement(TYPE_VECTOR3, SEM_POSITION);
            elements.Push(bufAcc.vertexElement);

            bufAcc.vertexOffset = vertexByteSize;
            vertexByteSize += bufAcc.componentType * bufAcc.components;
            vertexCount = bufAcc.count;

            bufferAccessors.Push(bufAcc);

        } else {
            URHO3D_LOGERROR("Positions attribute must be a number");
            return false;
        }
    }

    // Load data for other attributes
    if (attributes.Contains("SOMETHINGELSE"))
    {
        if (attributes["POSITION"]->IsNumber())
        {
            URHO3D_LOGINFO("Loading whatever data");
        }
    }

    // Get Index buffer

    if (object.Contains("indices"))
    {
        if (object["indices"]->IsNumber())
        {

            indBuffAcc = ParseAccessor(object["indices"]->GetUInt());

            if (indBuffAcc.buffer == buffers_.Size())
            {
                // Index buffer loading error
                return false;
            }

        }
    }

    VertexBuffer* vertBuff = new VertexBuffer(context_);
    IndexBuffer* indBuff = new IndexBuffer(context_);
    vertBuff->SetShadowed(true);
    indBuff->SetShadowed(true);

    unsigned totalByteSize = vertexCount * vertexByteSize;

    // Create an Urho-compatible Vertex buffer

    {
        // Allocate buffer that's large enough to fit all the vertex data
        SharedArrayPtr<unsigned char> vertData(new unsigned char[totalByteSize]);

        // This loop adds each vertex element
        for (unsigned i = 0; i < vertexCount; i ++)
        {
            for (unsigned j = 0; j < bufferAccessors.Size(); j ++)
            {
                const BufferAccessor& bufAcc = bufferAccessors[j];
                File& file = *(buffers_[bufAcc.buffer]);
                file.Seek(bufAcc.bufferOffset + i * bufAcc.components * bufAcc.componentType);
                file.Read(vertData.Get() + i * vertexByteSize + bufAcc.vertexOffset, bufAcc.components * bufAcc.componentType);
                URHO3D_LOGINFOF("Somevertex: %s", reinterpret_cast<const Vector3&>(vertData[i * vertexByteSize + bufAcc.vertexOffset]).ToString().CString());
            }
        }
        vertBuff->SetSize(vertexCount, elements);
        vertBuff->SetData(vertData.Get());
        vertList.Push(SharedPtr<VertexBuffer>(vertBuff));
    }

    // Load the index buffer

    {
        // maybe try converting to CW from CCW triangles
        // for now, just load directly

        // Allocate large enough buffer
        SharedArrayPtr<unsigned char> indData(new unsigned char[indBuffAcc.bufferLength]);

        File& file = *(buffers_[indBuffAcc.buffer]);
        file.Seek(indBuffAcc.bufferOffset);
        file.Read(indData.Get(), indBuffAcc.bufferLength);

        indBuff->SetSize(indBuffAcc.count, indBuffAcc.componentType == 4);
        indBuff->SetData(indData.Get());
        indList.Push(SharedPtr<IndexBuffer>(indBuff));
    }

    model.SetNumGeometries(index + 1);
    model.SetGeometry(index, 0, geometry);

    geometry->SetNumVertexBuffers(1);
    geometry->SetVertexBuffer(0, vertBuff);
    geometry->SetIndexBuffer(indBuff);
    geometry->SetDrawRange(TRIANGLE_LIST, 0, indBuffAcc.count);

    //vertList.Push(SharedPtr<VertexBuffer>(vertBuff));

    return true; // success!
}


BufferAccessor GLTFFile::ParseAccessor(unsigned index)
{
    BufferAccessor result;
    // Let's just say that an out of range buffer means invalid
    result.buffer = buffers_.Size();

    //const JSONArray& accessors = accessors_.GetArray();

    // An error check
    if (accessors_.Size() <= index)
    {
        URHO3D_LOGERROR("Accessor index out of range");
        return result;
    }

    if (!accessors_[index].IsObject())
    {
        URHO3D_LOGERROR("Accessor must be an object");
        return result;
    }

    const JSONObject& accessor = accessors_[index].GetObject();

    // TODO: do checks on these
    result.components = TypeComponentCount(accessor["type"]->GetString());
    result.count = accessor["count"]->GetUInt();

    // Set result.componentType to the byte size of what componentType describes
    switch(accessor["componentType"]->GetUInt())
    {
        case 5102: // BYTE
        case 5121: // UNSIGNED_BYTE
            result.componentType = 1; // 1 byte, duh
            break;
        case 5122: // SHORT
        case 5123: // UNSIGNED_SHORT
            result.componentType = 2; // 2 byte
            break;
        case 5125: // UNSIGNED_INT
        case 5126: // FLOAT
            result.componentType = 4; // 4 byte
        break;
        default:
            // this means error
            return result;
    }

    // Parse the buffer view

    if (!accessor.Contains("bufferView"))
    {
        URHO3D_LOGERROR("Missing bufferView");
        return result;
    }

    if (!accessor["bufferView"]->IsNumber())
    {
        URHO3D_LOGERROR("Accessor's bufferView must be a number");
        return result;
    }

    unsigned viewIndex = accessor["bufferView"]->GetUInt();

    if (views_.Size() <= viewIndex)
    {
        URHO3D_LOGERROR("bufferView index out of range");
        return result;
    }

    const JSONObject& bufferView = views_[viewIndex].GetObject();

    // TODO: got even more lazy, do checks for all these
    result.bufferLength = bufferView["byteLength"]->GetUInt();
    result.bufferOffset = bufferView["byteOffset"]->GetUInt();
    result.buffer = bufferView["buffer"]->GetUInt();

    return result;
}
