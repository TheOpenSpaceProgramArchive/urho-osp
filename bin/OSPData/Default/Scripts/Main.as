//class MainMenu : ScriptObject
//{
//    void Start()
//    {
//        
//    }
//}

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
    construct_apparatus();
}


// Yeah, it's called MainMenu but the rocket building code is here too.

Node@ camera;
Node@ cameraCenter;
Node@ cursor;
Node@ grabbed;
Node@ subject;

Vector2 prevMouseRay;

int bint(bool b)
{
    return b ? 1 : 0;
}

void construct_apparatus()
{

    // Called when "Do something with rockets" is pressed

    g_scene.LoadJSON(cache.GetFile("Scenes/VAB.json"));

    // Create the camera, consisting of two nodes
    // Node the camera orbits around
    cameraCenter = g_scene.CreateChild("CameraCenter");
    // Node that contains the camera component
    camera = cameraCenter.CreateChild("Camera");
    // Distanced from the center a bit
    camera.position = Vector3(0, 0, -8);
    
    // Create component
    Camera@ cam = camera.CreateComponent("Camera");
    cam.farClip = 65536;
    renderer.viewports[0].camera = cam;

    // Add a light to make it easier to see
    
    Light@ mothBait = cameraCenter.CreateComponent("Light");
    mothBait.usePhysicalValues = true;
    mothBait.range = 500;
    mothBait.brightness = 7000;
    mothBait.temperature = 6700;
    

    // Locate the cursor
    // Cursor has the drop sound in it
    // Cursor will soon have the move tool handles
    cursor = g_scene.GetChild("Cursor");

    // This is the root for the craft being built
    subject = g_scene.CreateChild("Subject");
    subject.position = Vector3(0, 0, 0);

    // UI things 
    //Button@ bottomBlank = Button();
    // Load the Parts List from a file
    UIElement@ assemblyUI = ui.LoadLayout(cache.GetResource("XMLFile", "UI/AssemblyUI.xml"));
    ListView@ partList = assemblyUI.GetChild("Content").GetChild("ListView");
    UIElement@ itemContainer = partList.GetChild("SV_ScrollPanel").GetChild("LV_ItemContainer");

    // Access the OSP hidden scene to get list of parts
    Array<Node@> categories = osp.hiddenScene.GetChild("Parts").GetChildren();

    for (uint i = 0; i < categories.length; i ++)
    { 
        Array<Node@> parts = categories[i].GetChildren();
        for (uint j = 0; j < parts.length; j ++)
        {
            // Make buttons for each part
            Print("Making button for: " + parts[j].name);
            Button@ butt = Button();
            butt.SetStyleAuto();
            butt.SetSize(100, 50);
            butt.SetPosition(0, 50 * j);
            butt.vars["N"] = parts[j];
            itemContainer.AddChild(butt);
            itemContainer.SetSize(itemContainer.size.x, 2000);
            SubscribeToEvent(butt, "Pressed", "clickpart");
        }
    }
    ui.root.AddChild(assemblyUI);
   
    Text@ help = Text();
    help.text = "Controls: [Arrow Keys] Orbit Camera  [Z/X] Move Camera In/Out\n[Space] Launch  [R] Restart";
    help.SetFont(cache.GetResource("Font", "Fonts/BlueHighway.ttf"), 18);
    help.SetPosition(0, 0);
    ui.root.AddChild(help);
    
    //bottomBlank.SetPosition(0, 0);
    //bottomBlank.SetSize(graphics.width, graphics.height);
    //bottomBlank.opacity = 0;
    
    //ui.root.AddChild(bottomBlank);



    SubscribeToEvent("UIMouseClick", "click_drop");
    SubscribeToEvent("KeyDown", "construct_keydown");
    
    SubscribeToEvent("RenderUpdate", "test_update");
    SubscribeToEvent(g_scene, "SceneUpdate", "construct_update");

}

void click_drop(StringHash eventType, VariantMap& eventData)
{
    if (eventData["Element"].GetPtr() is null)
    {
        Print("Nothing was clicked");
        //cast<RigidBody>(grabbed.GetComponent("RigidBody")).mass = 1;
        if (grabbed != null)
        {
            grabbed = null;
            SoundSource@ dropSound = cast<SoundSource>(cursor.GetComponent("SoundSource"));
            dropSound.frequency = 44100.0f * Random(0.9, 1.1);
            dropSound.Play(dropSound.sound);
        }
    }
}

void clickpart(StringHash eventType, VariantMap& eventData)
{
    Node@ part = cast<Node@>(cast<UIElement@>(eventData["Element"].GetPtr()).vars["N"].GetPtr());
    Print("Part clicked");
    
    Print("Display Name : " + part.vars["name"].GetString());
    Print("Description  : " + part.vars["description"].GetString());
    Print("Manufacturer : " + part.vars["manufacturer"].GetString());
    Print("Country      : " + part.vars["country"].GetString());
    
    Node@ clone = part.Clone();
    clone.enabled = true;
    
    grabbed = clone;
    subject.AddChild(clone);
}

void construct_keydown(StringHash eventType, VariantMap& eventData)
{
    if (eventData["Key"].GetInt() == KEY_SPACE && !subject.HasComponent("RigidBody"))
    {

        solidify_prototype(subject);
        
        ui.root.RemoveAllChildren();
        
        cast<SoundSource>(g_scene.GetComponent("SoundSource")).Stop();
        
        osp.debug_function(StringHash("create_universe"));
        
        Text@ help = Text();
        help.text = "Controls: [Arrow Keys] Orbit Camera  [Z/X] Move Camera In/Out\n[WASD] Rotate  [F/G] Throttle rockets Up/Down  [Q] Toggle Planet Wireframe  [R] Restart";
        help.SetFont(cache.GetResource("Font", "Fonts/BlueHighway.ttf"), 18);
        help.SetPosition(0, 0);
        ui.root.AddChild(help);
    }
    else if (eventData["Key"].GetInt() == KEY_R)
    {
        ui.root.RemoveAllChildren();
        g_scene.RemoveAllChildren();
        construct_apparatus();
    }
}

void solidify_prototype(Node@ prototype)
{
    // Calculate center of mass
    Array<Node@> parts = prototype.GetChildren();
    prototype.position = Vector3::ZERO;

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
    Array<Node@> childrenColliders = prototype.GetChildrenWithComponent("CollisionShape", true);

    //subject.position += centerOfMass;
    for (uint i = 0; i < childrenColliders.length; i ++) 
    {
        childrenColliders[i].worldPosition -= centerOfMass;
        Array<Variant> colliders;
        Array<Component@> shapes = childrenColliders[i].GetComponents("CollisionShape");
        for (uint j = 0; j < shapes.length; j ++) 
        {
            CollisionShape@ shapeA = cast<CollisionShape>(prototype.CreateComponent("CollisionShape"));
            CollisionShape@ shapeB = cast<CollisionShape>(shapes[j]);
            colliders.Push(Variant(shapeA));
            shapeA.SetBox(Vector3(1, 1, 1)); // this is too avoid a weird glitch
            shapeA.position = childrenColliders[i].worldPosition + childrenColliders[i].rotation * shapeB.position * childrenColliders[i].scale;
            shapeA.size = childrenColliders[i].scale * shapeB.size * 1.01f;
            shapeA.shapeType = shapeB.shapeType;
            Print("shape added " + shapeA.position.x);
        }
        childrenColliders[i].vars["colliders"] = colliders;
    }

    osp.make_craft(prototype);
    RigidBody@ body = prototype.CreateComponent("RigidBody");
    body.mass = totalMass;
    body.friction = 3;
}

Node@ get_part_from_node(Node@ node)
{
    // only parts contain "prototype" in vars
    while (!node.vars.Contains("prototype"))
    {
        // keep reassigning node to its parent until it's a part
        node = node.parent;
    }
    return node;
}

void construct_update(StringHash eventType, VariantMap& eventData)
{
    // Called each frame
    float delta = eventData["TimeStep"].GetFloat();

    // Handle camera movement by arrow keys
    Vector2 arrowKeys(bint(input.keyDown[KEY_RIGHT]) - bint(input.keyDown[KEY_LEFT]), bint(input.keyDown[KEY_UP]) - bint(input.keyDown[KEY_DOWN]));
    
    // Roll the camera to level with the horizon
    Quaternion toHorizon;
    Node@ terrain = g_scene.GetChild("PlanetTerrain");
    if (terrain !is null)
    {
        toHorizon.FromLookRotation(camera.worldRotation * Vector3(0, 0, 1), (cameraCenter.position - terrain.position).Normalized());
        
    }
    else
    {
        toHorizon.FromLookRotation(camera.worldRotation * Vector3(0, 0, 1), Vector3(0, 1, 0));
    }
    
    cameraCenter.rotation = toHorizon;
    //cameraCenter.rotation *= toHorizon.Inverse();
    //cameraCenter.rotation = Quaternion(Clamp(cameraCenter.rotation.pitch + arrowKeys.y * delta * 90, -80.0f, 80.0f), cameraCenter.rotation.yaw - arrowKeys.x * delta * 90, 0.0);
    //cameraCenter.rotation *= toHorizon;
    cameraCenter.rotation *= Quaternion(arrowKeys.y * delta * 90, -arrowKeys.x * delta * 90, 0.0);

    camera.position = Vector3(0, 0, camera.position.z + delta * 10.0 * (bint(input.keyDown[KEY_Z]) - bint(input.keyDown[KEY_X])));
    //cameraCenter.rotation.FromEulerAngles(cameraCenter.rotation.yaw, cameraCenter.rotation.pitch, 0.0);
    //Print("update! " + Clamp(cameraCenter.rotation.pitch + arrowKeys.y * delta * 90, -100, 100));
    //prevMouseRay
      
    // do kind of an unproject
    float dist = 8;
    Vector2 mouseCenter(input.mousePosition.x - (graphics.width - graphics.width / 2), -input.mousePosition.y + (graphics.height - graphics.height / 2));
    float scaleFactor = (graphics.height - graphics.height / 2) / Tan(float(renderer.viewports[0].camera.fov) / 2.0f);
    mouseCenter /= scaleFactor;

    //Print("Floating position: " + mouseCenter.x + ", " + mouseCenter.y);
    
    if (grabbed !is null)
    {
        grabbed.position = renderer.viewports[0].camera.node.LocalToWorld(Vector3(mouseCenter.x * dist, mouseCenter.y * dist, dist));
    }

}

void test_update(StringHash eventType, VariantMap& eventData)
{
    
    cameraCenter.position = subject.position;
}

void grid_arrange(UIElement@ p)
{
    
    
}
