#include "Default/Scripts/UIController.as"
#include "Default/Scripts/HotkeyHandler.as"

int bint(bool b)
{
    return b ? 1 : 0;
}

// Should be added to the editor scene
class FlightUI : UIController
{

    Node@ m_node;
    Node@ m_camera;
    Node@ m_cameraCenter;
    Node@ m_subject;

    HotkeyHandler@ m_hotkeys;
    
    bool m_isClosed;
    
    /**
     * Activate a feature right away
     * @param name [in] Technical name used to address the feature
     * @param args [in] Arguments to pass to the Activate function
     * @return whatever is returned by the EditorFunction
     */
    int ActivateFeature(const String& name, VariantMap& args)
    {
        // put an actual feature thing later
        if (name == "backToEditor")
        {
            // Call HandleResetPostUpdate in PostUpdate to avoid a cash
            // Components of m_node cannot be removed here, as this function
            // is often called in the FixedUpdate of this component. The array
            // of components (or any array) will mess up when elements are
            // removed while the array is being looped through, and it's
            // generally bad practice.
            SubscribeToEvent("PostUpdate", "HandleResetPostUpdate");
            
            Close();

        }
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
        
        m_node = node;
        m_cameraCenter = node.GetChild("CameraCenter");
        m_camera = m_cameraCenter.GetChild("Camera");
        m_subject = node.GetChild("Subject");
        
        @m_hotkeys = HotkeyHandler(this);
        
        Hotkey@ backHotkey = m_hotkeys.AddHotkey("backToEditor");
        m_hotkeys.BindToKey(backHotkey, KEY_R, INPUT_RISING);
        
        SubscribeToEvent("RenderUpdate", "HandleRenderUpdate");
    }

    // Part of ScriptObject
    void FixedUpdate(float timeStep)
    {
        m_hotkeys.Update();
 

        
 
        //if (m_isClosed && self !is null)
        //{
        //    self.Remove();
        //}
    }
    
    Node@ GetNode()
    {
        return node;
    }
    
    // Only called when going back to editor
    void HandleResetPostUpdate(StringHash eventType, VariantMap& eventData)
    {
        // Kill everything
        m_node.RemoveAllChildren();
        //m_node.RemoveAllComponents(); 
        
        // Load scene and add editor script
        cast<Scene@>(m_node).LoadJSON(cache.GetFile("Default/Scenes/CraftEditor.json"));
        m_node.CreateScriptObject("Default/Scripts/CraftEditor/CraftEditor.as", "CraftEditor");
    }
    
    void HandleRenderUpdate(StringHash eventType, VariantMap& eventData)
    {
        // Called each frame
        float delta = eventData["TimeStep"].GetFloat();

        cast<PhysicsWorld>(m_node.GetComponent("PhysicsWorld")).DrawDebugGeometry(true);   

        m_cameraCenter.position = m_subject.position;

        // Handle camera movement by arrow keys
        Vector2 arrowKeys(bint(input.keyDown[KEY_RIGHT]) - bint(input.keyDown[KEY_LEFT]), bint(input.keyDown[KEY_UP]) - bint(input.keyDown[KEY_DOWN]));
        
        // Roll the camera to level with the horizon
        Quaternion toHorizon;
        Node@ terrain = m_node.GetChild("PlanetTerrain");
        if (terrain !is null)
        {
            toHorizon.FromLookRotation(m_camera.worldRotation * Vector3(0, 0, 1), (m_cameraCenter.position - terrain.position).Normalized());
            
        }
        else
        {
            toHorizon.FromLookRotation(m_camera.worldRotation * Vector3(0, 0, 1), Vector3(0, 1, 0));
        }
        
        m_cameraCenter.rotation = toHorizon;
        m_cameraCenter.rotation *= Quaternion(arrowKeys.y * delta * 90, -arrowKeys.x * delta * 90, 0.0);

        m_camera.position = Vector3(0, 0, m_camera.position.z + delta * 10.0 * (bint(input.keyDown[KEY_Z]) - bint(input.keyDown[KEY_X])));
    }
}




