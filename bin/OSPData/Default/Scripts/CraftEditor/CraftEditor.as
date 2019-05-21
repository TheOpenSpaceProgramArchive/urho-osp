#include "Default/Scripts/UIController.as"
#include "Default/Scripts/HotkeyHandler.as"
#include "PartsList.as"
#include "Navigation.as"
#include "Utility.as"

// @ = handle
// & = reference (functions only)

funcdef int EditorFunction_t(CraftEditor@, EditorFeature@, VariantMap&);


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

    Node@ GetNode()
    {
        return node;
    }

    void Close()
    {
        m_isClosed = true;
    }

    // Part of ScriptObject
    void Start()
    {
        Print("Hey there");
        
        m_isClosed = false;
        
        // Create two nodes for the camera
        
        // CameraCenter is the point being looked at and orbited around
        m_cameraCenter = node.CreateChild("CameraCenter");
        
        // Camera contains the camera node
        m_camera = m_cameraCenter.CreateChild("Camera");
        // Move the camera 8m backwards from the center, so that CameraCenter
        // is visible at the center of the viewport
        m_camera.position = Vector3(0, 0, -8);
        
        // Create a camera component and put it into Camera
        Camera@ cam = m_camera.CreateComponent("Camera");
        cam.farClip = 65536;
        renderer.viewports[0].camera = cam;

        // Create the node the user will be adding stuff to
        m_subject = node.CreateChild("Subject");

        // UI

        // Load the construction workspace. (Toolbars and blank panels)
        // Note: workspaces are an arrangement of empty panels
        UIElement@ wkConstruction = ui.LoadLayout(cache.GetResource("XMLFile", "Default/UI/WorkspaceConstruction.xml"));
        wkConstruction.vars["Scene"] = node;
        
        // Load the parts list panel (Just the parts list)
        UIElement@ panelPartsList = ui.LoadLayout(cache.GetResource("XMLFile", "Default/UI/PanelPartsList.xml")); 

        // Make the parts list stretch to the size of its parent
        SetUIAnchors(panelPartsList);
        
        // Setup stuff in PartsList.as
        PartsList::SetupPartsList(this, panelPartsList);
        
        // Add the parts list
        wkConstruction.GetChild("ToolbarLeft").GetChild("Panel0").AddChild(panelPartsList);

        // Add the construction UI to the list of workspaces.
        // This does nothing right now, but exists for future use where there
        // can be multiple workspaces (eg. construction, wiring, testing, crew)
        m_workspaces.Push(wkConstruction);
        m_currentWorkspace = 0;
               
        // Make it show on the screen
        ui.root.AddChild(m_workspaces[0]);

        // Add the HotkeyHandler
        // Note: this is a constructor, maybe snake_case functions might have
        //       been a good idea.
        @m_hotkeys = HotkeyHandler(this);

        // Add Mouse and keyboard binds
        // + some simple boolean algebra

        // Add Launch feature
        EditorFeature@ launch = AddFeature("lunch", "Start eating the meal", @LunchTime);

        // Add Orbit View and Undo features
        AddFeature("vorbit", "Orbit View", @Navigation::ViewOrbit);
        AddFeature("uundo", "Orbit View", @Utility::Undo);
        AddFeature("uclear", "Clear All", @Utility::ClearAll);

        // Create blank hotkeys for them all
        Hotkey@ launchHotkey = m_hotkeys.AddHotkey("lunch");
        Hotkey@ viewOrbitHotkey = m_hotkeys.AddHotkey("vorbit");
        Hotkey@ undoHotkey = m_hotkeys.AddHotkey("uundo");
        Hotkey@ clearHotkey = m_hotkeys.AddHotkey("uclear");

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
        
        // For Clear
        // Activate is HIGH when (R is rising)
        m_hotkeys.BindToKey(clearHotkey, KEY_R, INPUT_RISING);
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

        m_hotkeys.Update();
        
        if (m_isClosed && self !is null)
        {
            // Kill everything related to the editor
            
            // self is the ScriptInstance component
            self.Remove();
            
            // Remove all UI, Change this when more workspaces are added
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
    // Step 1: Calculate the center of mass.
    // Add up all the vectors and divide by total mass
    
    Vector3 centerOfMass(0, 0, 0);
    float totalMass = 0;
    
    // Get the list of all the parts
    const Array<Node@> parts = blueprint.GetChildren();
    
    // Move the blueprint to (0,0,0) for convinience
    blueprint.position = Vector3::ZERO;

    // Loop through all the parts and add up their masses
    for (uint i = 0; i < parts.length; i ++) 
    {
        //RigidBody@ shape = cast<RigidBody>(childrenColliders[i].GetComponent("RigidBody"));
        // TODO: account for mass-changing Machine classes when fuel tanks are
        //       added; perhaps a CalculateMass function?
        float mass = parts[i].vars["massdry"].GetFloat();
        centerOfMass += (parts[i].worldPosition) * mass;
        totalMass += mass;
    }

    centerOfMass /= totalMass;
    Print("total mass: " + totalMass);
  
    // Step 2: Combine the CollisionShapes into a single node
  
    // TODO: support crafts with moving parts
  
    // Get a list of every node with a collider
    const Array<Node@> childrenColliders = blueprint.GetChildrenWithComponent("CollisionShape", true);
    Array<Variant> colliders;
    Array<Component@> shapes;
    
    // Loop through every node that has a collider
    for (uint i = 0; i < childrenColliders.length; i ++) 
    {
        // Move the node since the center of mass is now the origin
        childrenColliders[i].worldPosition -= centerOfMass;
        
        // Get a list of all the CollisionShapes in the current node
        shapes = childrenColliders[i].GetComponents("CollisionShape");
        colliders.Clear();

        for (uint j = 0; j < shapes.length; j ++) 
        {
            // shapeA is new collision shape, shapeB is existing
            // shapeB's information will be copied into shapeA
            CollisionShape@ shapeA = cast<CollisionShape>(blueprint.CreateComponent("CollisionShape"));
            CollisionShape@ shapeB = cast<CollisionShape>(shapes[j]);
            // Keep track of which CollisionShapes are associated with this node
            colliders.Push(Variant(shapeA));
            shapeA.SetBox(Vector3(1, 1, 1)); // this avoids a weird glitch

            // Correctly position the collision shape, as it's position property
            // is affected by the node's position, scale, and rotation
            shapeA.position = childrenColliders[i].worldPosition + childrenColliders[i].rotation * shapeB.position * childrenColliders[i].scale;
            shapeA.rotation = shapeB.rotation * childrenColliders[i].worldRotation;
            shapeA.size = childrenColliders[i].scale * shapeB.size * 1.01f;
            shapeA.shapeType = shapeB.shapeType;
            //Print("shape added " + shapeA.position.x);
        }
        // Make sure the old node knows which CollisionShapes it's associated with
        childrenColliders[i].vars["colliders"] = colliders;
    }

    // Call the OspUniverse make_craft function
    osp.make_craft(blueprint);
    RigidBody@ body = blueprint.CreateComponent("RigidBody");
    // Set the mass
    body.mass = totalMass;

    // Make sure the craft doesn't slip off the platform so easily
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
    cast<SoundSource>(editor.GetNode().GetComponent("SoundSource")).Stop();
    
    osp.debug_function(StringHash("create_universe"));
    
    editor.Close();
    
    // Add the FlightUI
    Node@ sceneA = editor.GetNode();
    sceneA.CreateScriptObject("Default/Scripts/FlightUI/FlightUI.as", "FlightUI");
    
    return 0;
}
