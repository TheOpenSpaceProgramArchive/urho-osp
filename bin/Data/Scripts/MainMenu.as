class MainMenu : ScriptObject
{
    void Start()
    {
        
    }
}

Scene@ g_scene;

void main(Scene& scene)
{
    Print("This is a print from the menu script " + osp.typeName);

    g_scene = scene;

    // Load the scene with the earth and some cubes
    //scene.LoadXML(cache.GetFile("Scenes/Menu.xml"));

    // Load the menu interface
    UIElement@ menuUI = ui.LoadLayout(cache.GetResource("XMLFile", "UI/MenuUI.xml"));

    // Get the play button, and start listening to it
    Button@ play = menuUI.GetChild("Play", true);
    SubscribeToEvent(play, "Released", "play_pressed");

    ui.root.RemoveAllChildren();
    ui.root.AddChild(menuUI);
}

void play_pressed()
{
    Print("Button pressed");
    ui.root.RemoveAllChildren();
    g_scene.RemoveAllChildren();
    construct_apparatus();
}




Node@ cameraCenter;

int bint(bool b)
{
    return b ? 1 : 0;
}

void construct_apparatus()
{
    
    g_scene.LoadJSON(cache.GetFile("Scenes/VAB.json"));
    UIElement@ assemblyUI = ui.LoadLayout(cache.GetResource("XMLFile", "UI/AssemblyUI.xml"));
    ListView@ partList = assemblyUI.GetChild("Content").GetChild("ListView");
    UIElement@ itemContainer = partList.GetChild("SV_ScrollPanel").GetChild("LV_ItemContainer");
    
    cameraCenter = g_scene.CreateChild("CameraCenter");
    Node@ camera = cameraCenter.CreateChild("Camera");
    camera.position = Vector3(0, 0, -8);
    renderer.viewports[0].camera = camera.CreateComponent("Camera");
    
    Array<Node@> categories = osp.hiddenScene.GetChild("Parts").GetChildren();
    
    for (uint i = 0; i < categories.length; i ++)
    {
        Array<Node@> parts = categories[i].GetChildren();
        for (uint j = 0; j < parts.length; j ++)
        {
            Print("Making button for: " + parts[j].name);
            Button@ butt = Button();
            butt.SetStyleAuto();
            butt.SetSize(100, 50);
            butt.SetPosition(0, 50 * j);
            butt.vars["N"] = parts[j];
            itemContainer.AddChild(butt);
            itemContainer.SetSize(itemContainer.size.x, 2000);
            SubscribeToEvent(butt, "Released", "clickpart");
        }
    }
   
    ui.root.AddChild(assemblyUI);

    SubscribeToEvent(g_scene, "SceneUpdate", "construct_update");

}

void clickpart(StringHash eventType, VariantMap& eventData)
{
    Node@ part = cast<Node@>(cast<UIElement@>(eventData["Element"].GetPtr()).vars["N"].GetPtr());
    Print("Part clicked");
    
    Print("Display Name : " + part.vars["DisplayName"].GetString());
    Print("Description  : " + part.vars["Description"].GetString());
    Print("Manufacturer : " + part.vars["Manufacturer"].GetString());
    Print("Country      : " + part.vars["Country"].GetString());
    
    Node@ clone = part.Clone();
    g_scene.AddChild(clone);
}

void construct_update(StringHash eventType, VariantMap& eventData)
{
    float delta = eventData["TimeStep"].GetFloat();
    Vector2 arrowKeys(bint(input.keyDown[KEY_RIGHT]) - bint(input.keyDown[KEY_LEFT]), bint(input.keyDown[KEY_UP]) - bint(input.keyDown[KEY_DOWN]));
    //cameraCenter.rotation.FromEulerAngles(cameraCenter.rotation.yaw, cameraCenter.rotation.pitch, 0.0);
    Print("update! " + Clamp(cameraCenter.rotation.pitch + arrowKeys.y * delta * 90, -100, 100));
    cameraCenter.rotation = Quaternion(Clamp(cameraCenter.rotation.pitch + arrowKeys.y * delta * 90, -80.0f, 80.0f), cameraCenter.rotation.yaw - arrowKeys.x * delta * 90, 0.0);
}

void grid_arrange(UIElement@ p)
{
    
    
}
