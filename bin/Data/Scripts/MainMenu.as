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
    scene.LoadXML(cache.GetFile("Scenes/Menu.xml"));

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
}

void construct_apparatus()
{


}
