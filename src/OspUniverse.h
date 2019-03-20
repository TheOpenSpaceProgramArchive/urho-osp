#pragma once


#include <Urho3D/Container/RefCounted.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "AstronomicalBody.h"
#include "GLTFFile.h"
#include "LongVector3.h"
#include "PlanetWrenderer.h"

using namespace Urho3D;

namespace osp {




/**
 * @brief A (singleton) class that handles many in-game functions
 */
class OspUniverse : public Object
{
    SharedPtr<Scene> m_hiddenScene;
    SharedPtr<Node> m_parts;
    SharedPtr<Node> m_solarSystem;
    Vector<SharedPtr<Scene>> m_activeScenes;

    //HashMap<StringHash, SharedPtr<ObjectFactory> > m_machines;

    //Vector<OspPart>
    // list of countried, manufacturers, and other stuff

    URHO3D_OBJECT(OspUniverse, Object)
public:
    OspUniverse(Context* context);

    Scene* get_hidden_scene() { return m_hiddenScene; }

    void debug_function(const StringHash which);
    void process_directory(const String& path);
    void register_parts(const GLTFFile* gltf);
    //void register_machine(ObjectFactory* factory);
    void make_craft(Node* node);
    
private:
    void part_node_recurse(Node* partRoot, Node* node);
};

}
