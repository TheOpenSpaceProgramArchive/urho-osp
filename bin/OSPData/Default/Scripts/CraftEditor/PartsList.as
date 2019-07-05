namespace PartsList
{



int PartInsert(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    // This is an EditorFunction that inserts parts, known as "partInsert"

    // Get prototype part from argument
    Node@ prototype = args["Prototype"].GetPtr();
    
    if (prototype is null)
    {
        return 1;
    }
    
    Print("Display Name : " + prototype.vars["name"].GetString());
    Print("Description  : " + prototype.vars["description"].GetString());
    Print("Manufacturer : " + prototype.vars["manufacturer"].GetString());
    Print("Country      : " + prototype.vars["country"].GetString());
    
    // Clone the prototype into the editor's subject node

    Node@ clone = prototype.Clone();
    clone.enabled = true;
   
    //Array<Node@> models = prototype.GetChildrenWithComponent("StaticModel", true);
    
    editor.m_subject.AddChild(clone);
    
    clone.worldPosition = args["Position"].GetVector3();
    
    return clone.id;
}

void SetupPartsList(CraftEditor@ editor, UIElement@ panelPartsList)
{
    
    // Add features
    
    EditorFeature@ partInsert = editor.AddFeature("partInsert", "Insert a part", @PartInsert);
    
    ListView@ partList = panelPartsList.GetChild("Content").GetChild("ListView");
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
            butt.SetMinSize(64, 48);
            butt.SetMaxSize(64, 48);
            butt.vars["Prototype"] = parts[j];
            butt.vars["Scene"] = editor.GetNode();
            
            BorderImage@ rocketPicture = BorderImage("Thumbnail");
            rocketPicture.SetSize(40, 40);
            rocketPicture.SetPosition(4, 4);
            rocketPicture.texture = cache.GetResource("Texture2D", "Textures/NoThumbnail.png");
            butt.AddChild(rocketPicture);
                
            Text@ someIndicator = Text("TypeIndicator");
            someIndicator.text = "42";
            someIndicator.SetFont(cache.GetResource("Font", "Fonts/BlueHighway.ttf"), 8);
            someIndicator.SetPosition(47, 4);
            butt.AddChild(someIndicator);
            
            itemContainer.SetSize(itemContainer.size.x, itemContainer.size.y + 70);
            itemContainer.AddChild(butt);
            
            SubscribeToEvent(butt, "Pressed", "PartsList::HandlePartButtonPressed");
        }
    }
}

void HandlePartButtonPressed(StringHash eventType, VariantMap& eventData)
{
    UIElement@ butt = cast<UIElement@>(eventData["Element"].GetPtr());
    Scene@ scn = cast<Scene@>(butt.vars["Scene"].GetPtr());
    CraftEditor@ editor = cast<CraftEditor@>(scn.GetScriptObject("CraftEditor"));

    // currently dragging another part
    if (editor.m_cursorLock != null)
    {
        // maybe call cancel?
        return;
    }
    
    VariantMap args;// = {{"s", asd}};
    
    // Insert, Select, then Drag the part
    
    // Activate PartInsert Feature
    // Position the new part under the cursor, 6 meters into the screen
    args["Position"] = editor.m_camera.worldPosition + editor.ScreenPosToRay(editor.m_cursor) * 6;
    args["Prototype"] = butt.vars["Prototype"];
    
    // Actually insert the part, returns the ID
    int newPartId = editor.ActivateFeature("partInsert", args);
    Node@ part = scn.GetNode(newPartId);

    if (part is null)
    {
        return;
    }

    Print("New part ID: " + newPartId + " Name: " + part.name);

    // Make a list of attachments
    //Node@[]@ attachments = part.GetChildrenWithTag("Attachment");
    //Print("Attachment count: " + attachments.length);
    Variant[] attachments;
    Node@[]@ partChildren = part.GetChildren();
    for (int i = 0; i < partChildren.length; i ++)
    {
        if (partChildren[i].HasTag("Attachment"))
        {
            attachments.Push(Variant(partChildren[i]));
        }
    }
    part.vars["Attachments"] = attachments;

    // Activate Select Feature
    args.Clear();
    Array<Variant> selection = {part};
    args["Parts"] = selection;
    editor.ActivateFeature("select", args);
    
    // Activate the drag feature
    args.Clear();
    args["FeatureOp"] = FEATUREOP_START;
    args["EnableDeleteOnCancel"] = true;
    editor.ActivateFeature("moveFree", args);
    
    //for (uint i = 0; i < models.length; i ++) 
    //{
        //cast<StaticModel>(models[i].GetComponent("StaticModel")).material.scene = scene;
    //}
    
    //Print(g_editor.m_features[int(g_editor.m_featureMap["partInsert"])].m_desc); 
    //grabbed = clone;
    //subject.AddChild(clone);
}

}
