#include "Default/Scripts/UIController.as"

const int INPUT_LOW = 0;
const int INPUT_HIGH = 1;
const int INPUT_RISING = 2;
const int INPUT_FALLING = 3;

const int INPUTOP_AND = 0;
const int INPUTOP_OR = 1;
  
// Calls an EditorFeature
class Hotkey
{
    // If the right inputs triggered an activation
    bool m_active;
    // Arguments that will be passed on to m_feature's Activate function
    VariantMap m_arguments;
    // Feature that will be called
    String m_feature;
    // A boolean expression describing the input conditions needed to activate
    // [Index to m_inputs, [0/1/rising/falling], next operation [and/or], repeat...]
    Array<int> m_expression;
}


class HotkeyHandler
{
    Array<int> m_inputs;
    Array<int> m_inputsPrevious;
    Array<Hotkey@> m_hotkeys;
    Array<Vector2> m_joysticks;
    Vector2 m_cursor;
    
    bool m_enabled;

    UIController@ m_controller;
    
    // Accessed by event handlers to set values in m_inputs
    // will only hold int indices
    VariantMap m_binds;
    
    HotkeyHandler(UIController@ controller)
    {
        m_controller = controller;
 
        // m_joysticks[0] will be used for mouse movement
        m_joysticks.Resize(1);
    
        // Subscribe to user input events
        SubscribeToEvent("MouseButtonDown", "HandleMouseDown");
        SubscribeToEvent("MouseButtonUp", "HandleMouseUp");
        SubscribeToEvent("KeyDown", "HandleKeyDown");
        SubscribeToEvent("KeyUp", "HandleKeyUp");

        SubscribeToEvent("MouseMove", "HandleMouseMove");
    }

    /**
     * Add a new hotkey to the editor
     * @param feature [in] Feature the hotkey activates
     * @return A handle to the new hotkey. Bind inputs to this.
     */
    Hotkey@ AddHotkey(const String& feature)
    {
        Hotkey@ hotkey = Hotkey();
        hotkey.m_feature = feature;
        m_hotkeys.Push(hotkey);
        return hotkey;
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

    void Update()
    {
        m_cursor = Vector2(input.mousePosition);
        
        // Loop through all the hotkeys
        Hotkey@ hotkey;
        for (uint i = 0; i < m_hotkeys.length; i ++)
        {
            //if (m_isClosed)
            //{
            //    break;
            //}
            
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
                m_controller.ActivateFeature(hotkey.m_feature, hotkey.m_arguments);
                //hotkey.m_feature.Activate(this, hotkey.m_feature, hotkey.m_arguments);
                
                hotkey.m_arguments["First"] = false;
            } else {
                hotkey.m_active = false;
            }
        }
        
        // Set mouse displacement back to zero
        m_joysticks[0] *= 0;
        
        // Save previous inputs state
        m_inputsPrevious = m_inputs;
    }
    
    
}
