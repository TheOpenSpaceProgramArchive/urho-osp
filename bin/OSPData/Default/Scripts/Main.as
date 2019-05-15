Scene@ g_scene;

void loaded()
{
    Print("This is a print from the menu script " + osp.typeName);

    g_scene = renderer.viewports[0].scene;

    // Load the scene with the earth and some cubes
    //scene.LoadXML(cache.GetFile("Scenes/Menu.xml"));

    // Load the menu interface
    UIElement@ menuUI = ui.LoadLayout(cache.GetResource("XMLFile", "UI/MenuUI.xml"));

    // Get the play button, and start listening to when it's pressed
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

    // Load the editor scene, which has the platform and the 2 balls
    g_scene.LoadJSON(cache.GetFile("Default/Scenes/CraftEditor.json"));
    
    // Create a script instance that has the CraftEditor class
    g_scene.CreateScriptObject("Default/Scripts/CraftEditor/CraftEditor.as", "CraftEditor");
    //ScriptInstance@ editorScript = cast<ScriptInstance>(g_scene.CreateComponent("ScriptInstance"));
    //editorScript.className = "CraftEditor";
    //editorScript.scriptFile = cache.GetResource("ScriptFile", "Default/Scripts/CraftEditor/CraftEditor.as");
}
