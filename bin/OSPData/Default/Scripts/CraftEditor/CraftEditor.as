#include "Default/Scripts/UIController.as"
#include "Default/Scripts/HotkeyHandler.as"
#include "PartsList.as"
#include "Navigation.as"
#include "Utility.as"

// @ = handle
// & = reference (functions only)

funcdef int EditorFunction_t(CraftEditor@, EditorFeature@, VariantMap&);

CraftEditor@ g_editor;

int bint(bool b)
{
    return b ? 1 : 0;
}

class EditorFeature
{
    // Some identifiers
    //String m_name;
    String m_desc;
    // Data stored that can be accessed by the ActivateFunction
    VariantMap m_data;
    // Function pointer
    EditorFunction_t@ Activate;
}

// Should be added to the editor scene
class CraftEditor : UIController
{
    Array<EditorFeature@> m_features;
    // Maps feature names to indices to features
    Dictionary m_featureMap;
    
    HotkeyHandler@ m_hotkeys;
    
    Array<Node@> m_selection;

    Node@ m_camera;
    Node@ m_cameraCenter;
    Node@ m_subject;

    Array<UIElement@> m_workspaces;
    int m_currentWorkspace;
    
    bool m_isClosed;

    /**
     * Add a new feature to the editor
     * @param name [in] The technical name this feature will be indexed as
     * @param desc [in] Nice text for a human to read
     * @param func [in] Function pointer to the function
     * @return A handle to the EditorFeature just added
     */
    EditorFeature@ AddFeature(const String& name, const String& desc, EditorFunction_t@ func)
    {
        EditorFeature@ feature = EditorFeature();
        //feature.m_name = name;
        feature.m_desc = desc;
        feature.Activate = func;
        
        m_featureMap[name] = m_features.length;
        m_features.Push(feature);
        return feature;
    }

    /**
     * Activate a feature right away
     * @param name [in] Technical name used to address the feature
     * @param args [in] Arguments to pass to the Activate function
     * @return whatever is returned by the EditorFunction
     */
    int ActivateFeature(const String& name, VariantMap& args)
    {
        EditorFeature@ feature = m_features[int(m_featureMap[name])];
        return feature.Activate(this, feature, args);
    }

    void Close()
    {
        m_isClosed = true;
    }

    // Part of ScriptObject
    void Start()
    {
        Print("Hey there");
        
        @g_editor = this;
        m_isClosed = false;
        
        // Create two nodes for the camera
        
        // CameraCenter is the point being looked at and orbited around
        m_cameraCenter = node.CreateChild("CameraCenter");
        
        // Camera contains the camera node
        m_camera = m_cameraCenter.CreateChild("Camera");
        // Move the camera 8m backwards from the center, so that CameraCenter
        // is visible at the center of the viewport
        m_camera.position = Vector3(0, 0, -8);
        
        // Create a camera node and put it into Camera
        Camera@ cam = m_camera.CreateComponent("Camera");
        cam.farClip = 65536;
        renderer.viewports[0].camera = cam;

        m_subject = node.CreateChild("Subject");

        // UI

        // Load the construction workspace. (Toolbars and blank panels)
        UIElement@ wkConstruction = ui.LoadLayout(cache.GetResource("XMLFile", "Default/UI/WorkspaceConstruction.xml"));
        
        // Load the parts list panel (Just the parts list)
        UIElement@ panelPartsList = ui.LoadLayout(cache.GetResource("XMLFile", "Default/UI/PanelPartsList.xml")); 

        // Make the parts list stretch to the size of its parent
        SetUIAnchors(panelPartsList);
        
        // Setup stuff in PartsList.as
        PartsList::SetupPartsList(this, panelPartsList);
        
        // Add the parts 
        wkConstruction.GetChild("ToolbarLeft").GetChild("Panel0").AddChild(panelPartsList);

        // Add the construction UI to the list of workspaces.
        // This does nothing right now, but exists for future use where there
        // can be multiple workspaces (eg. construction, wiring, testing, crew)
        m_workspaces.Push(wkConstruction);
        m_currentWorkspace = 0;
               
        // Make it show on the screen
        ui.root.AddChild(m_workspaces[0]);

        // Add the HotkeyHandler
        @m_hotkeys = HotkeyHandler(this);

        // Add Mouse and keyboard binds
        // + some simple boolean algebra

        // Add Launch feature
        EditorFeature@ launch = AddFeature("lunch", "Start eating the meal", @LunchTime);

        // Add Orbit View and Undo features
        EditorFeature@ viewOrbit = AddFeature("vorbit", "Orbit View", @Navigation::ViewOrbit);
        EditorFeature@ undo = AddFeature("uundo", "Orbit View", @Utility::Undo);

        // Create blank hotkeys for them all
        Hotkey@ launchHotkey = m_hotkeys.AddHotkey("lunch");
        Hotkey@ viewOrbitHotkey = m_hotkeys.AddHotkey("vorbit");
        Hotkey@ undoHotkey = m_hotkeys.AddHotkey("uundo");

        // For Lunch...
        // Activate is HIGH when (SPACE is RISING)
        m_hotkeys.BindToKey(launchHotkey, KEY_SPACE, INPUT_RISING);

        // For Orbit...
        // Activate is HIGH when (RightMouse is HIGH) OR (KeyQ is HIGH)
        m_hotkeys.BindToMouseButton(viewOrbitHotkey, MOUSEB_RIGHT, INPUT_HIGH);
        m_hotkeys.BindAddOr(viewOrbitHotkey);
        m_hotkeys.BindToKey(viewOrbitHotkey, KEY_E, INPUT_HIGH);

        // For Undo...
        // Activate is HIGH when (Ctrl is HIGH) AND (Shift is LOW) AND (Alt is LOW) AND (KeyZ is RISING)
        m_hotkeys.BindToKeyScancode(undoHotkey, SCANCODE_CTRL, INPUT_HIGH); 
        m_hotkeys.BindToKeyScancode(undoHotkey, SCANCODE_SHIFT, INPUT_LOW);
        m_hotkeys.BindToKeyScancode(undoHotkey, SCANCODE_ALT, INPUT_LOW);
        m_hotkeys.BindToKey(undoHotkey, KEY_Z, INPUT_RISING);
    }

    // Part of ScriptObject
    void FixedUpdate(float timeStep)
    {
        //Print(m_inputs[0] - m_inputsPrevious[0]);
        
        //UIElement@ f = ui.GetElementAt(input.mousePosition, false);
        //if (f !is null)
        //{
        //    Print("Under mouse: " + f.name);
        //}
        
        // considering supporting devices that don't use a mouse later on


        m_cameraCenter.position = m_subject.position;

        m_hotkeys.Update();
        
        if (m_isClosed)
        {
            // self is the ScriptInstance
            self.Remove();
            m_workspaces[0].Remove();
        }
    }
}


/**
 * Convert a physicsless blueprint into a working craft with physics
 * @param blueprint [in] The mess of parts just built by the user.
 */
void SolidifyBlueprint(Node@ blueprint)
{
    // Calculate center of mass
    Array<Node@> parts = blueprint.GetChildren();
    blueprint.position = Vector3::ZERO;

    Vector3 centerOfMass(0, 0, 0);
    float totalMass = 0;

    for (uint i = 0; i < parts.length; i ++) 
    {
        //RigidBody@ shape = cast<RigidBody>(childrenColliders[i].GetComponent("RigidBody"));
        float mass = parts[i].vars["massdry"].GetFloat();
        centerOfMass += (parts[i].worldPosition) * mass;
        totalMass += mass;
    }

    centerOfMass /= totalMass;
    Print("total mass: " + totalMass);
  
    // Create a single rigid body with all the colliders
    Array<Node@> childrenColliders = blueprint.GetChildrenWithComponent("CollisionShape", true);

    //subject.position += centerOfMass;
    for (uint i = 0; i < childrenColliders.length; i ++) 
    {
        childrenColliders[i].worldPosition -= centerOfMass;
        Array<Variant> colliders;
        Array<Component@> shapes = childrenColliders[i].GetComponents("CollisionShape");
        for (uint j = 0; j < shapes.length; j ++) 
        {
            CollisionShape@ shapeA = cast<CollisionShape>(blueprint.CreateComponent("CollisionShape"));
            CollisionShape@ shapeB = cast<CollisionShape>(shapes[j]);
            colliders.Push(Variant(shapeA));
            shapeA.SetBox(Vector3(1, 1, 1)); // this is too avoid a weird glitch
            shapeA.position = childrenColliders[i].worldPosition + childrenColliders[i].rotation * shapeB.position * childrenColliders[i].scale;
            shapeA.rotation = shapeB.rotation * childrenColliders[i].worldRotation;
            shapeA.size = childrenColliders[i].scale * shapeB.size * 1.01f;
            shapeA.shapeType = shapeB.shapeType;
            Print("shape added " + shapeA.position.x);
        }
        childrenColliders[i].vars["colliders"] = colliders;
    }

    osp.make_craft(blueprint);
    RigidBody@ body = blueprint.CreateComponent("RigidBody");
    body.mass = totalMass;
    body.friction = 3;
    

}

void SetUIAnchors(UIElement@ panel)
{
    panel.enableAnchor = true;
    panel.SetMinAnchor(0.0f, 0.0f);
    panel.SetMaxAnchor(1.0f, 1.0f);
}

// Important features

int LunchTime(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    //ui.root.RemoveAllChildren();
    SolidifyBlueprint(editor.m_subject);
    //cast<SoundSource>(scene.GetComponent("SoundSource")).Stop();
    
    osp.debug_function(StringHash("create_universe"));
    
    editor.Close();
    return 0;
}
