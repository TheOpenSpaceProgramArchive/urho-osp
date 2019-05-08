#include "PartsList.as"
#include "Navigation.as"
#include "Utility.as"

// @ = handle
// & = reference (functions only)

funcdef int EditorFunction_t(CraftEditor@, EditorFeature@, VariantMap&);

const int INPUT_LOW = 0;
const int INPUT_HIGH = 1;
const int INPUT_RISING = 2;
const int INPUT_FALLING = 3;

const int INPUTOP_AND = 0;
const int INPUTOP_OR = 1;

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

// Calls an EditorFeature
class Hotkey
{
    // If the right inputs triggered an activation
    bool m_active;
    // Arguments that will be passed on to m_feature's Activate function
    VariantMap m_arguments;
    // Feature that will be called
    EditorFeature@ m_feature;
    // A boolean expression describing the input conditions needed to activate
    // [Index to m_inputs, [0/1/rising/falling], next operation [and/or], repeat...]
    Array<int> m_expression;
}

// Should be added to the editor scene
class CraftEditor : ScriptObject
{
    Array<EditorFeature@> m_features;
    // Maps feature names to indices to features
    Dictionary m_featureMap;
    
    Array<int> m_inputs;
    Array<int> m_inputsPrevious;
    Array<Hotkey@> m_hotkeys; 
    Vector2 m_cursor;
    Array<Vector2> m_joysticks;
    // Accessed by event handlers to set values in m_inputs
    // will only hold int indices
    VariantMap m_binds;
    
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
     * Add a new hotkey to the editor
     * @param feature [in] Feature the hotkey activates
     * @return A handle to the new hotkey. Bind inputs to this.
     */
    Hotkey@ AddHotkey(EditorFeature@ feature)
    {
        Hotkey@ hotkey = Hotkey();
        @(hotkey.m_feature) = feature;
        m_hotkeys.Push(hotkey);
        return hotkey;
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

    /**
     * Get an m_inputs index from a hashed string to a physical input device button
     * eg. m_inputs[ControlIndex(StringHash("MB" + MOUSEB_RIGHT))] is the input
     *     that responds to the right mouse button
     * A new entry in m_index will be created, if one is not found
     * @param control [in] StringHash to input device
     * @return An index to m_inputs
     */
    int ControlIndex(StringHash control)
    {
        // index to m_inputs for new control to listen ot/
        int inputIndex;
        
        /// Check if already added
        if (m_binds[control].empty)
        {
            // Set input index to where a new element will be
            inputIndex = m_inputs.length;
            
            // Add the new element
            m_inputs.Push(0);
            m_inputsPrevious.Push(0);
            
            // Set m_binds[control] to point to it
            m_binds[control] = inputIndex;
        } else {
            
            // The bind has already been set
            inputIndex = m_binds[control].GetInt();
        }
        
        return inputIndex;
    }
    
    /**
     * Bind a mouse button to a Hotkey
     * @param hotkey [ref] Hotkey to write binds to
     * @param button [in] Mouse button
     * @param press [in] INPUT_HIGH/LOW/RISING/FALLING
     */
    void BindToMouseButton(Hotkey@ hotkey, int button, int press)
    {
        StringHash hash = StringHash("MB" + button);
        
        int inputIndex = ControlIndex(hash);

        hotkey.m_expression.Push(inputIndex);
        hotkey.m_expression.Push(press);
        hotkey.m_expression.Push(INPUTOP_AND);
    }

    /**
     * Bind a keyboard key to a Hotkey
     * @param hotkey [ref] Hotkey to write binds to
     * @param key [in] Keyboard key
     * @param press [in] INPUT_HIGH/LOW/RISING/FALLING
     */
    void BindToKey(Hotkey@ hotkey, int key, int press)
    {
        StringHash hash = StringHash("KB" + key);
        
        int inputIndex = ControlIndex(hash);
        
        hotkey.m_expression.Push(inputIndex);
        hotkey.m_expression.Push(press);
        hotkey.m_expression.Push(INPUTOP_AND);
    }

    /**
     * Bind a keyboard scancode to a Hotkey
     * @param hotkey [ref] Hotkey to write binds to
     * @param key [in] Keyboard scancode
     * @param press [in] INPUT_HIGH/LOW/RISING/FALLING
     */
    void BindToKeyScancode(Hotkey@ hotkey, int key, int press)
    {
        StringHash hash = StringHash("KS" + key);
        
        int inputIndex = ControlIndex(hash);
        
        hotkey.m_expression.Push(inputIndex);
        hotkey.m_expression.Push(press);
        hotkey.m_expression.Push(INPUTOP_AND);
    }

    /**
     * Set the last expression in m_expression to an OR instead of an AND
     * @param hotkey [ref] Hotkey to write binds to
     */
    void BindAddOr(Hotkey@ hotkey)
    {
        hotkey.m_expression[hotkey.m_expression.length - 1] = INPUTOP_OR;
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

        // Input

        // Subscribe to user input events
        SubscribeToEvent("MouseButtonDown", "HandleMouseDown");
        SubscribeToEvent("MouseButtonUp", "HandleMouseUp");
        SubscribeToEvent("KeyDown", "HandleKeyDown");
        SubscribeToEvent("KeyUp", "HandleKeyUp");

        SubscribeToEvent("MouseMove", "HandleMouseMove");

        // m_joysticks[0] will be used for mouse movement
        m_joysticks.Resize(1);  

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

        // Add Mouse and keyboard binds
        // + some simple boolean algebra

        // Add Launch feature
        EditorFeature@ launch = AddFeature("lunch", "Start eating the meal", @LunchTime);

        // Add Orbit View and Undo features
        EditorFeature@ viewOrbit = AddFeature("vorbit", "Orbit View", @Navigation::ViewOrbit);
        EditorFeature@ undo = AddFeature("vorbit", "Orbit View", @Utility::Undo);

        // Create blank hotkeys for them all
        Hotkey@ launchHotkey = AddHotkey(launch);
        Hotkey@ viewOrbitHotkey = AddHotkey(viewOrbit);
        Hotkey@ undoHotkey = AddHotkey(undo);

        // For Lunch...
        // Activate is HIGH when (SPACE is RISING)
        BindToKey(launchHotkey, KEY_SPACE, INPUT_RISING);

        // For Orbit...
        // Activate is HIGH when (RightMouse is HIGH) OR (KeyQ is HIGH)
        BindToMouseButton(viewOrbitHotkey, MOUSEB_RIGHT, INPUT_HIGH);
        BindAddOr(viewOrbitHotkey);
        BindToKey(viewOrbitHotkey, KEY_E, INPUT_HIGH);

        // For Undo...
        // Activate is HIGH when (Ctrl is HIGH) AND (Shift is LOW) AND (Alt is LOW) AND (KeyZ is RISING)
        BindToKeyScancode(undoHotkey, SCANCODE_CTRL, INPUT_HIGH); 
        BindToKeyScancode(undoHotkey, SCANCODE_SHIFT, INPUT_LOW);
        BindToKeyScancode(undoHotkey, SCANCODE_ALT, INPUT_LOW);
        BindToKey(undoHotkey, KEY_Z, INPUT_RISING);
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
        m_cursor = Vector2(input.mousePosition);

        m_cameraCenter.position = m_subject.position;

        // Loop through all the hotkeys
        Hotkey@ hotkey;
        for (uint i = 0; i < m_hotkeys.length; i ++)
        {
            @hotkey = m_hotkeys[i];

            // Read through the expression array from left to right
            // It's a boolean sum of products with user inputs as inputs

            Array<int>@ expression = hotkey.m_expression;

            int lastOperator = INPUTOP_OR; // Set to OR because the first condition is going to be compared by total, which is false
            bool total = false;
            bool currentValue;
            
            for (uint j = 0; j < expression.length; j += 3)
            {
                // Index to mouse button or key in m_inputs
                int index = expression[j];
                
                // Set currentValue to true if input matches the condition
                switch (expression[j + 1])
                {
                case INPUT_LOW:
                    currentValue = (m_inputs[index] <= 0);
                    break;
                case INPUT_HIGH:
                    currentValue = (m_inputs[index] > 0);
                    break;
                case INPUT_RISING:
                    currentValue = (m_inputs[index] - m_inputsPrevious[index] > 0);
                    break;
                case INPUT_FALLING:
                    currentValue = (m_inputsPrevious[index] - m_inputs[index] > 0);
                    break;
                }
                
                // Combine the currentValue with the calculated total using the specified operator
                switch (lastOperator)
                {
                case INPUTOP_AND:
                    total = currentValue && total;
                    break;
                case INPUTOP_OR:
                    total = currentValue || total;
                    break;
                }
                
                // Savew the operator for the next condition
                lastOperator = expression[j + 2];
            }
            
            // Now activate the feature
            if (total)
            {
                if (!hotkey.m_active)
                {
                    // Set First to true on first activation
                    hotkey.m_arguments["First"] = true;
                }
                hotkey.m_active = true;
                hotkey.m_feature.Activate(this, hotkey.m_feature, hotkey.m_arguments);
                
                hotkey.m_arguments["First"] = false;
            } else {
                hotkey.m_active = false;
            }
        }
        
        // Set mouse displacement back to zero
        m_joysticks[0] *= 0;
        
        m_inputsPrevious = m_inputs;
        
        if (m_isClosed)
        {
            // self is the ScriptInstance
            self.Remove();
            m_workspaces[0].Remove();
        }
    }

    // Handle KeyDown events, see the SubscribeToEvent functions above
    void HandleKeyDown(StringHash eventType, VariantMap& eventData)
    {
        int key = eventData["Key"].GetInt();
        StringHash hash = StringHash("KB" + key);

        Variant input = m_binds[hash];
        
        if (!input.empty)
        {
            m_inputs[input.GetInt()] = 1;
        }
        
        // Do the same but for scancodes
        key = eventData["Scancode"].GetInt();
        hash = StringHash("KS" + key);

        input = m_binds[hash];
        
        if (!input.empty)
        {
            m_inputs[input.GetInt()] = 1;
        }
        
        // else no binds use this key
    }

    // Handle KeyUp events, see the SubscribeToEvent functions above
    void HandleKeyUp(StringHash eventType, VariantMap& eventData)
    {
        int key = eventData["Key"].GetInt();
        StringHash hash = StringHash("KB" + key);
        
        Variant input = m_binds[hash];
        
        if (!input.empty)
        {
            m_inputs[input.GetInt()] = 0;
        }
        
        // Do the same but for scancodes
        key = eventData["Scancode"].GetInt();
        hash = StringHash("KS" + key);

        input = m_binds[hash];
        
        if (!input.empty)
        {
            m_inputs[input.GetInt()] = 0;
        }
        // else no binds use this key
    }

    // Handle MouseButtonDown events, see the SubscribeToEvent functions above
    void HandleMouseDown(StringHash eventType, VariantMap& eventData)
    {
        // Mouse button is pressed
        int button = eventData["Button"].GetInt();
        StringHash hash = StringHash("MB" + button);
        
        Variant input = m_binds[hash];
        
        if (!input.empty)
        {
            m_inputs[input.GetInt()] = 1;
        }
        // else no binds use this mouse button
    }

    // Handle MouseButtonUp events, see the SubscribeToEvent functions above
    void HandleMouseUp(StringHash eventType, VariantMap& eventData)
    {
        // Mouse button is released, same as above but sets to 0 instead
        int button = eventData["Button"].GetInt();
        StringHash hash = StringHash("MB" + button);
        
        Variant input = m_binds[hash];
        
        if (!input.empty)
        {
            m_inputs[input.GetInt()] = 0;
        }
        // else no binds use this mouse button
    }
    
    // Handle MouseMove events, see the SubscribeToEvent functions above
    void HandleMouseMove(StringHash eventType, VariantMap& eventData)
    {
        // in the future, make m_joysticks respond to actual controllers
        // for now, this is hard-coded
        m_joysticks[0] = m_joysticks[0] + Vector2(eventData["DX"].GetInt(), eventData["DY"].GetInt());
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
