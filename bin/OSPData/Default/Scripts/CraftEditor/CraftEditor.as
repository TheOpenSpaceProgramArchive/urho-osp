#include "PartsList.as"
#include "Navigation.as"
#include "Utility.as"

// @ = handle
// & = reference (functions only)

funcdef int EditorFunction_t(CraftEditor@, EditorFeature@);

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

// Activation condition
// if that is changing and this is changing then that

// Keep 2 boolean arrays of only all the buttons that are bound
// K

class EditorFeature
{
    String m_name;
    String m_desc;
    
    bool m_active;
    
    // [Index, [0/1/rising/falling], next operation [and/or], repeat...]
    VariantMap m_data;
    Array<int> m_conditions;
    EditorFunction_t@ Activate;
}


// Should be added to the editor scene
class CraftEditor : ScriptObject
{
    
    Array<EditorFeature@> m_features;
    
    Array<int> m_inputs;
    Array<int> m_inputsPrevious;
    // Accessed by event handlers to set values in m_inputs
    // will only hold int indices
    VariantMap m_binds;

    Array<Node@> m_selection;

    Node@ m_camera;
    Node@ m_cameraCenter;

    Array<UIElement@> m_workspaces;
    int m_currentWorkspace;

    EditorFeature@ AddFeature(const String& name, const String& desc, EditorFunction_t@ func)
    {
        EditorFeature@ feature = EditorFeature();
        feature.m_name = name;
        feature.m_desc = desc;
        feature.Activate = func;
        m_features.Push(feature);
        return feature;
    }

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
    
    void BindToMouseButton(EditorFeature@ feature, int button, int press)
    {
        StringHash hash = StringHash("MB" + button);
        
        int inputIndex = ControlIndex(hash);
        
        feature.m_conditions.Push(inputIndex);
        feature.m_conditions.Push(press);
        feature.m_conditions.Push(INPUTOP_AND);
    }

    void BindToKey(EditorFeature@ feature, int key, int press)
    {
        StringHash hash = StringHash("KB" + key);
        
        int inputIndex = ControlIndex(hash);
        
        feature.m_conditions.Push(inputIndex);
        feature.m_conditions.Push(press);
        feature.m_conditions.Push(INPUTOP_AND);
    }

    void BindToKeyScancode(EditorFeature@ feature, int key, int press)
    {
        StringHash hash = StringHash("KS" + key);
        
        int inputIndex = ControlIndex(hash);
        
        feature.m_conditions.Push(inputIndex);
        feature.m_conditions.Push(press);
        feature.m_conditions.Push(INPUTOP_AND);
    }

    void BindAddOr(EditorFeature@ feature)
    {
        feature.m_conditions[feature.m_conditions.length - 1] = INPUTOP_OR;
    }

    void Start()
    {
        Print("Hey there");
        
        //@g_editor = this;
        
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

        // Load the construction workspace. (Toolbars and blank panels)
        UIElement@ wkConstruction = ui.LoadLayout(cache.GetResource("XMLFile", "Default/UI/WorkspaceConstruction.xml"));
        
        // Load the parts list panel (Just the parts list)
        UIElement@ panelPartsList = ui.LoadLayout(cache.GetResource("XMLFile", "Default/UI/PanelPartsList.xml")); 

        // Make the parts list stretch to the size of its parent
        SetUIAnchors(panelPartsList);
        
        // Add the parts 
        wkConstruction.GetChild("ToolbarLeft").GetChild("Panel0").AddChild(panelPartsList);

        // Add the construction UI to the list of workspaces.
        // This does nothing right now, but exists for future use where there
        // can be multiple workspaces (eg. construction, wiring, testing, crew)
        m_workspaces.Push(wkConstruction);
        m_currentWorkspace = 0;
               
        // Make it show on the screen
        ui.root.AddChild(m_workspaces[0]);

        // Subscribe to user input events
        SubscribeToEvent("MouseButtonDown", "HandleMouseDown");
        SubscribeToEvent("MouseButtonUp", "HandleMouseUp");
        SubscribeToEvent("KeyDown", "HandleKeyDown");
        SubscribeToEvent("KeyUp", "HandleKeyUp");

        // Add Mouse and keyboard binds
        // + some simple boolean algebra

        // Add Orbit View feature
        EditorFeature@ viewOrbit = AddFeature("vorbit", "Orbit View", @Navigation::ViewOrbit);

        // Activate is HIGH when (RightMouse is HIGH) OR (KeyQ is HIGH)
        BindToMouseButton(viewOrbit, MOUSEB_RIGHT, INPUT_HIGH);
        BindAddOr(viewOrbit);
        BindToKey(viewOrbit, KEY_Q, INPUT_HIGH);


        // Add Undo feature
        EditorFeature@ undo = AddFeature("vorbit", "Orbit View", @Utility::Undo);

        // Activate is HIGH when (Ctrl is HIGH) AND (Shift is LOW) AND (Alt is LOW) AND (KeyZ is RISING)
        BindToKeyScancode(undo, SCANCODE_CTRL, INPUT_HIGH);
        BindToKeyScancode(undo, SCANCODE_SHIFT, INPUT_LOW);
        BindToKeyScancode(undo, SCANCODE_ALT, INPUT_LOW);
        BindToKey(undo, KEY_Z, INPUT_RISING);
    }
    
    void FixedUpdate(float timeStep)
    {
        //Print(m_inputs[0] - m_inputsPrevious[0]);
        
        // Loop through all the features
        EditorFeature@ feature;
        for (uint i = 0; i < m_features.length; i ++)
        {
            @feature = m_features[i];

            // Read through the conditions array from left to right
            // It's a boolean sum of products with user inputs as inputs

            Array<int>@ conditions = feature.m_conditions;

            int lastOperator = INPUTOP_OR; // Set to OR because the first condition is going to be compared by total, which is false
            bool total = false;
            bool currentValue;
            
            for (uint j = 0; j < conditions.length; j += 3)
            {
                // Index to mouse button or key in m_inputs
                int index = conditions[j];
                
                // Set currentValue to true if input matches the condition
                switch (conditions[j + 1])
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
                lastOperator = conditions[j + 2];
            }
            
            // Now activate the feature
            if (total)
            {
                feature.Activate(this, feature);
            }
        }
        
        m_inputsPrevious = m_inputs;
    }

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
    
    void HandleMouseMove(StringHash eventType, VariantMap& eventData)
    {
        
    }
}



void SetUIAnchors(UIElement@ panel)
{
    panel.enableAnchor = true;
    panel.SetMinAnchor(0.0f, 0.0f);
    panel.SetMaxAnchor(1.0f, 1.0f);
}
