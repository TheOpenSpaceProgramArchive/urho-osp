#include "Default/Scripts/UIController.as"
#include "Default/Scripts/HotkeyHandler.as"


// Should be added to the editor scene
class FlightUI : UIController
{

    HotkeyHandler@ m_hotkeys;
    
    /**
     * Activate a feature right away
     * @param name [in] Technical name used to address the feature
     * @param args [in] Arguments to pass to the Activate function
     * @return whatever is returned by the EditorFunction
     */
    int ActivateFeature(const String& name, VariantMap& args)
    {
        return 0;
    }

    void Close()
    {
        m_isClosed = true;
    }

    // Part of ScriptObject
    void Start()
    {
        Print("why world");
    }

    // Part of ScriptObject
    void FixedUpdate(float timeStep)
    {

    }
}



