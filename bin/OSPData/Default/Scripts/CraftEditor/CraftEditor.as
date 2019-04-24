#include "PartsList.as"
#include "Navigation.as"

// @ = handle
// & = reference

funcdef int EditorFunction_t(CraftEditor@, EditorFeature@, VariantMap&);

CraftEditor@ g_editor;

int bint(bool b)
{
    return b ? 1 : 0;
}

class EditorFeature
{
    String m_name;
    String m_desc;
    bool active;
    EditorFunction_t@ Activate;
}

class CraftEditor : ScriptObject
{
    
    Array<EditorFeature@> m_features;
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

    void BindToMouseButton(EditorFeature@ feature, int button)
    {
    
    }

    void Start()
    {
        Print("Hey there");
        
        //@g_editor = this;
        
        m_cameraCenter = node.CreateChild("CameraCenter");
        m_camera = m_cameraCenter.CreateChild("Camera");
        m_camera.position = Vector3(0, 0, -8);
        
        Camera@ cam = m_camera.CreateComponent("Camera");
        cam.farClip = 65536;
        renderer.viewports[0].camera = cam;
        
        UIElement@ wkConstruction = ui.LoadLayout(cache.GetResource("XMLFile", "Default/UI/WorkspaceConstruction.xml"));
        UIElement@ panelPartsList = ui.LoadLayout(cache.GetResource("XMLFile", "Default/UI/PanelPartsList.xml")); 

        SetUIAnchors(panelPartsList);
        wkConstruction.GetChild("ToolbarLeft").GetChild("Panel0").AddChild(panelPartsList);

        m_workspaces.Push(wkConstruction);
        m_currentWorkspace = 0;
               
        ui.root.AddChild(m_workspaces[0]);

        SubscribeToEvent("MouseButtonDown", "HandleMouseDown");
        SubscribeToEvent("MouseButtonUp", "HandleMouseUp");

        EditorFeature@ viewOrbit = AddFeature("vorbit", "Orbit View", @Navigation::ViewOrbit);
        
    }
    
    void HandleMouseDown(StringHash eventType, VariantMap& eventData)
    {
        // Mouse button is pressed
    }

    void HandleMouseUp(StringHash eventType, VariantMap& eventData)
    {
        // Mouse button released
    }
        
    UIElement@ CreateWorkspace()
    {
        UIElement@ workspace = UIElement();
        workspace.enableAnchor = true;
        workspace.maxAnchor = Vector2(1, 1);
        m_workspaces.Push(workspace);
        return workspace;
    }
    

}



void SetUIAnchors(UIElement@ panel)
{
    panel.enableAnchor = true;
    panel.SetMinAnchor(0.0f, 0.0f);
    panel.SetMaxAnchor(1.0f, 1.0f);
}
