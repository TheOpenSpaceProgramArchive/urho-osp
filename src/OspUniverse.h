#pragma once

#include <Urho3D/Container/RefCounted.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "LongVector3.h"
#include "Resource/GLTFFile.h"
#include "Satellites/AstronomicalBody.h"
#include "Terrain/PlanetWrenderer.h"

namespace osp
{

/**
 * @brief A (singleton) class that handles many in-game functions
 */
class OspUniverse : public Object
{
    // Scene to store nodes like Part Prototypes. Has no physics, rendering
    // updating, etc... It is completely frozen in time
    Urho3D::SharedPtr<Scene> m_hiddenScene;

    // A Node full of Part Prototypes, stored in the hidden scene
    Urho3D::WeakPtr<Node> m_parts;

    // Root node of the entire solar system/universe
    Urho3D::SharedPtr<Satellite> m_bigUniverse;

    // List of ActiveAreas
    Urho3D::Vector<Urho3D::WeakPtr<ActiveArea>> m_activeAreas;

    //HashMap<StringHash, SharedPtr<ObjectFactory> > m_machines;

    //Vector<OspPart>
    // list of countried, manufacturers, and other stuff

    URHO3D_OBJECT(OspUniverse, Urho3D::Object)

public:

    OspUniverse(Urho3D::Context* context);

    Urho3D::Scene* get_hidden_scene() { return m_hiddenScene; }

    /**
     * Various debug functions that can be called from AngelScript
     * @param which [in] Hash for which function to call
     */
    void debug_function(const StringHash which);

    /**
     * @brief Adds an OSP resource directory
     * @param path [in] Absolute File Path to directory
     *
     * The directory contents will be scanned for parts and scripts.
     * Contents of a subdirectory named "Parts" will be scanned recursively for
     * .gltf.sturdy files
     *
     */
    void process_directory(const String& path);

    /**
     * @brief Include a .gltf.sturdy's contents into the categories of parts
     * @param gltf [in] GLTF file to process
     */
    void register_parts(const GLTFFile* gltf);

    /**
     * This function shouldn't exist
     * @param node Yes
     */
    void make_craft(Node* node);
    
private:

    /**
     * Recursive function used to scan through sturdy files
     * @param partRoot glTF root that should be passed on unchanged
     * @param node Current node
     */
    void part_node_recurse(Node* partRoot, Node* node);
};

} // namespace osp
