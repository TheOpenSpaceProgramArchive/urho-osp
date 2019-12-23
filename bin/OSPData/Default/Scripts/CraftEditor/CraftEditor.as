#include "Default/Scripts/UIController.as"
#include "Default/Scripts/HotkeyHandler.as"
#include "PartsList.as"
#include "Navigation.as"
#include "Utility.as"

// @ = handle
// & = reference (functions only)

// Different "Feature Operations" that can be called into features.
// Pass into features through args["FeatureOp"] = FEATUREOP_SOMETHING;
// Some features ignore FeatureOp entirely
// eg. Calling START into MoveFree starts dragging the selected parts
const int FEATUREOP_NONE = 0;
const int FEATUREOP_START = 1;

// STAYON is used on Features in the FixedUpdate loop that have m_stayOn = true
const int FEATUREOP_STAYON = 2;

// Features can "lock the cursor" by setting m_cursorLock to itself. Used by
// features that drag things. CANCEL and CONFIRM are called into  m_cursorLock
// when the cancel or confirm features are activated.
// eg. clicking while MoveFree is locking the cursor, confirms and places the
//     parts being dragged
const int FEATUREOP_CONFIRM = 3;
const int FEATUREOP_CANCEL = 4;

funcdef int EditorFunction_t(CraftEditor@, EditorFeature@, VariantMap&);

// The Feature class, It has a few identifiers and a function pointer that can
// be called to add functionality to the editor. The plan is to make Features
// modular af, so that addons can easily extend and modify the editor.
class EditorFeature
{
    // Some identifiers
    //String m_name;
    String m_desc;

    // Completely ignore when false
    bool m_enabled;
    // Ignore ActivateFeature if true
    bool m_blockActivate;
    // Call every FixedUpdate
    bool m_stayOn;

    // Data stored that can be accessed by the ActivateFunction
    Dictionary m_data;

    // Function pointer
    EditorFunction_t@ Activate;
}

class AttachmentPair
{
    Node@ m_selected;
    Node@ m_target;
    float distance;
    float distanceScreen;
}

// Add this to the editor scene using a ScriptInstance. That turns the scene
// into a Craft Editor
class CraftEditor : UIController
{
    // Main list of features
    Array<EditorFeature@> m_features;
    // Maps feature names to indices to features
    Dictionary m_featureMap;
    
    // Handles all the controls, and calls ActivateFeature according to defined
    // button presses. Can bind to mouse, keyboard, and maybe controllers.
    HotkeyHandler@ m_hotkeys;
    
    // A feature sets itself to this when it wants to do a "cursor lock," such
    // as dragging a part.
    EditorFeature@ m_cursorLock;
    
    // Cursor on screen, maybe control using a controller some day
    Vector2 m_cursor; 

    // Ray in global space, not normalized, m_cursor position going directly
    // out of the camera 
    Vector3 m_cursorRay;

    // Main list of selected parts
    Array<Node@> m_selection;

    Node@ m_camera; // Node that contains the Camera component
    Camera@ m_cameraComponent;
    Node@ m_cameraCenter; // Node the camera node is parented to, orbits this

    // Root node of the craft being edited. Every part is parented to this.
    Node@ m_subject; 
    
    // Play all sort of sound effects through here, like that metal clang
    SoundSource@ m_sfx;

    // Suppose to be different modes the editor can be put in
    // barely implemented, and needs work
    // construction - Modify the craft and add parts
    // wiring       - Edit wiring
    // crew         - Change who gets to sit in the seats
    // testing      - Test the craft like a wind tunnel
    // More can be added by addons
    Array<UIElement@> m_workspaces;
    int m_currentWorkspace;
    
    // A UIElement that follows the cursor
    UIElement@ m_uiCursor;

    // Set to true when exiting the editor
    bool m_isClosed;

    
    //VariantMap m_vars;

    /**
     * Add a new feature to the editor
     * @param name [in] The technical name this feature will be indexed as
     * @param desc [in] Nice text for a human to read
     * @param func [in] Function pointer to the function
     * @return A handle to the EditorFeature just added
     */
    EditorFeature@ AddFeature(const String& name, const String& desc,
                              EditorFunction_t@ func)
    {
        EditorFeature@ feature = EditorFeature();
        //feature.m_name = name;
        feature.m_desc = desc;
        feature.Activate = func;
        feature.m_enabled = true;
        feature.m_blockActivate = false;
        feature.m_stayOn = false;
        
        m_featureMap[name] = m_features.length;
        m_features.Push(feature);
        return feature;
    }

    /**
     * Activate a feature right away
     * @param name [in] Technical name used to address the feature
     * @param args [in] Arguments to pass to the Activate function
     * @return whatever is returned by the EditorFunction
     */
    int ActivateFeature(const String& name, VariantMap& args)
    {
        if (!m_featureMap.Exists(name))
        {
            return -1;
        }
        
        EditorFeature@ feature = m_features[int(m_featureMap[name])];

        // feature should never be null. m_featureMap should keep track

        // Don't call disabled or activate-blocking features
        if (!feature.m_enabled || feature.m_blockActivate)
        {
            return -2;
        }

        return feature.Activate(this, feature, args);
    }

    /**
     * Calculate a list of possible snaps between the selected parts and other
     * placed parts. Two thresholds are used to consider if two parts are close
     * enough for a possible snap.
     * @param pairs [out] Array for pairs of possible attachments
     * @param screenDistance [in] threshold in screen pixels for how far apart
                                  attachments can be on-screen, before counting
                                  as a snap 
     * @param distance [in] threshold in physical meters for how far apart
                            attachments can physically be, before counting as a
                            snap
     * @param angleLimit [in] Max angle pair of attachments can vary
     */
    void CalculatePossibleSnaps(Array<AttachmentPair>& pairs,
                                float screenDistance, float distance,
                                float angleLimit)
    {
        // TODO: add any sort of filtering for different kinds of attachments
        // TODO: optimize all of this, it isn't expected to perform that well
        
        float screenDistSqr = screenDistance * screenDistance;
        float angleCos = Cos(angleLimit);
        
        // Get a list of non-selected parts
        Array<Node@> nonSelected;
        const Array<Node@> parts = m_subject.GetChildren();
        for (int i = 0; i < parts.length; i ++)
        {
            if (!parts[i].vars["Selected"].GetBool())
            {
                nonSelected.Push(parts[i]);
            }
        }
        
        //Print("not selected " + nonSelected.length);
        
        Variant[]@ attachmentsA;
        Variant[]@ attachmentsB;
        
        // Loop through all selected parts
        for (int i = 0; i < m_selection.length; i ++)
        {
            @attachmentsA = m_selection[i].vars["Attachments"]
                                .GetVariantVector();
            
            // Loop through all attachments of i'th selected part
            for (int j = 0; j < attachmentsA.length; j ++)
            {
                
                Node@ attachA = cast<Node@>(attachmentsA[j].GetPtr());
                
                Vector2 screenPosA = m_cameraComponent.WorldToScreenPoint(
                                                        attachA.worldPosition);
                screenPosA.x *= graphics.width;
                screenPosA.y *= graphics.height;
                
                // Loop through all non-selected parts
                for (int k = 0; k < nonSelected.length; k ++)
                {
                    @attachmentsB = nonSelected[k].vars["Attachments"]
                                        .GetVariantVector();

                    // Loop through attachments of k'th non-selected part
                    for (int l = 0; l < attachmentsB.length; l ++)
                    {
                    
                        Node@ attachB = cast<Node@>(attachmentsB[l].GetPtr());
                        
                        Vector2 screenPosB = m_cameraComponent
                                                .WorldToScreenPoint(
                                                        attachB.worldPosition);
                        screenPosB.x *= graphics.width;
                        screenPosB.y *= graphics.height;
                        
                        bool snapped = false;
  
                        // Check distance
                        if ((attachA.worldPosition
                             - attachB.worldPosition).length < distance)
                        {
                            // maybe do something else in here
                            snapped = true;
                        }
                        // Check on-screen distance
                        else if ((screenPosB - screenPosA)
                                            .lengthSquared < screenDistSqr)
                        {
                            // maybe do other stuff in here
                            snapped = true;
                        }
                        
                        // Limit angle
                        if (snapped)
                        {
                            // Use dot product
                            // One is negative, since the two attachment nodes
                            // face each other
                            float dot = attachA.worldDirection.DotProduct(
                                            -attachB.worldDirection);
                            
                            if (dot < angleCos)
                            {
                                snapped = false;
                            }
                        }
                        
                        if (snapped)
                        {
                            pairs.Resize(pairs.length + 1);
                            AttachmentPair@ pair = pairs[pairs.length - 1];
                            pair.m_selected = attachA;
                            pair.m_target = attachB;
                            
                        }
                        //Print(distance);
                    }
                }
            }
            
        }
        
    }

    void Close()
    {
        m_isClosed = true;
    }

    Node@ GetNode()
    {
        return node;
    }

    /**
     * Unproject a point on the screen into a ray going out of the camera
     * Returned vector will not be normalized, instead is a point on a plane
     * 1m from the camera. Camera rotation is dealt with, translation ignored
     * @param input [in] Position on screen in pixels
     * @return Vector3 camera ray
     */
    Vector3 ScreenPosToRay(const Vector2& input)
    {
        Vector2 mouseCenter(input.x - (graphics.width - graphics.width / 2),
                          -input.y + (graphics.height - graphics.height / 2));
        float scaleFactor = (graphics.height - graphics.height / 2)
                                / Tan(float(m_cameraComponent.fov) / 2.0f);
        mouseCenter /= scaleFactor;
        return m_camera.worldRotation
                    * (Vector3(mouseCenter.x, mouseCenter.y, 1));
    }

    // Functions below are used by ScriptObject

    void Start()
    {
        Print("Hey there");
        
        m_isClosed = false;
        
        // Create two nodes for the camera
        
        // CameraCenter is the point being looked at and orbited around
        @m_cameraCenter = node.CreateChild("CameraCenter");
        
        // Camera contains the camera node
        @m_camera = m_cameraCenter.CreateChild("Camera");
        // Move the camera 8m backwards from the center, so that CameraCenter
        // is visible at the center of the viewport
        m_camera.position = Vector3(0, 0, -8);
        
        // Create a camera component and put it into Camera
        @m_cameraComponent = m_camera.CreateComponent("Camera");
        m_cameraComponent.farClip = 65536;
        renderer.viewports[0].camera = m_cameraComponent;

        // Create the node the user will be adding stuff to
        @m_subject = node.CreateChild("Subject");

        // Create sound making thing
        @m_sfx = node.CreateComponent("SoundSource");

        // UI

        // Load the construction workspace. (Toolbars and blank panels)
        // Note: workspaces are an arrangement of empty panels
        UIElement@ wkConstruction = ui.LoadLayout(cache.GetResource("XMLFile",
                                    "Default/UI/WorkspaceConstruction.xml"));
        wkConstruction.vars["Scene"] = node;
        
        // Load the parts list panel (Just the parts list)
        UIElement@ panelPartsList = ui.LoadLayout(cache.GetResource("XMLFile",
                                            "Default/UI/PanelPartsList.xml")); 

        // Make the parts list stretch to the size of its parent
        SetUIAnchors(panelPartsList);
        
        // Setup stuff in PartsList.as
        PartsList::SetupPartsList(this, panelPartsList);
        
        // Add the parts list
        wkConstruction.GetChild("ToolbarLeft").GetChild("Panel0")
                                                    .AddChild(panelPartsList);

        // Add the construction UI to the list of workspaces.
        // This does nothing right now, but exists for future use where there
        // can be multiple workspaces (eg. construction, wiring, testing, crew)
        m_workspaces.Push(wkConstruction);
        m_currentWorkspace = 0;
               
        // Make it show on the screen
        ui.root.AddChild(m_workspaces[0]);
        
        // Add the UIElement that is suppose to follow the mouse cursor
        m_uiCursor = ui.root.CreateChild("Cursor");
        
        Text@ t = Text("CursorText");
        t.SetFont(cache.GetResource("Font", "Fonts/BlueHighway.ttf"), 12);
        t.text = "";
        t.SetPosition(15, 0);
        
        m_uiCursor.AddChild(t);
        

        // Add the HotkeyHandler
        // Note: this is a constructor, maybe snake_case functions might have
        //       been a good idea.
        @m_hotkeys = HotkeyHandler(this);

        // Start adding features. This registers the functions, so they can be
        // called using ActivateFeature
        // eg. ActivateFeature("confirm", ...);

        // Add basic features
        AddFeature("confirm", "Confirm", @CursorLockConfirm);
        AddFeature("cancel", "Cancel", @CursorLockCancel);
        AddFeature("select", "Select Parts", @Select);
        AddFeature("moveFree", "Select Parts", @MoveFree);
        AddFeature("lunch", "Start eating the meal", @LunchTime);

        // Add Navigation features
        AddFeature("vorbit", "Orbit View", @Navigation::ViewOrbit);
        AddFeature("vzoomIn", "Zoom In", @Navigation::ZoomIn);
        AddFeature("vzoomOut", "Zoom Out", @Navigation::ZoomOut);
        
        // Add utility features
        AddFeature("uundo", "Orbit View", @Utility::Undo);
        AddFeature("uclear", "Clear All", @Utility::ClearAll);

        AddFeature("rotateUp", "Rotate part Up", @Utility::RotateUp);
        AddFeature("rotateDown", "Rotate part Down", @Utility::RotateDown);
        AddFeature("rotateLeft", "Rotate part Left", @Utility::RotateLeft);
        AddFeature("rotateRight", "Rotate part Right", @Utility::RotateRight);

        // Bind features to hotkeys.
        // HotkeyHandler calls this craft editor's features through the
        // ActivateFeature function.
        
        // TODO: use some sort of config file later, instead of this spaghetti

        // Create and bind hotkeys for confirm and cancel
        {
            Hotkey@ confirmHotkey = m_hotkeys.AddHotkey("confirm");
            Hotkey@ cancelHotkey = m_hotkeys.AddHotkey("cancel");
            
            // Left mouse to confirm, backspace to cancel
            m_hotkeys.BindToMouseButton(confirmHotkey, MOUSEB_LEFT,
                                        INPUT_RISING);
            m_hotkeys.BindToKey(cancelHotkey, KEY_BACKSPACE, INPUT_RISING);
        }
        
        // Create and bind hotkeys for basic functions
        {
            Hotkey@ undoHotkey = m_hotkeys.AddHotkey("uundo");
            Hotkey@ clearHotkey = m_hotkeys.AddHotkey("uclear");
            Hotkey@ launchHotkey = m_hotkeys.AddHotkey("lunch");
            
            // For Lunch...
            // Activate is HIGH when (SPACE is RISING)
            m_hotkeys.BindToKey(launchHotkey, KEY_SPACE, INPUT_RISING);
            
            // For Undo... (Not implemented, it just prints a message)
            // Activate is HIGH when (Ctrl is HIGH) AND (Shift is LOW)
            //                       AND (Alt is LOW) AND (KeyZ is RISING)
            m_hotkeys.BindToKeyScancode(undoHotkey, SCANCODE_CTRL, INPUT_HIGH); 
            m_hotkeys.BindToKeyScancode(undoHotkey, SCANCODE_SHIFT, INPUT_LOW);
            m_hotkeys.BindToKeyScancode(undoHotkey, SCANCODE_ALT, INPUT_LOW);
            m_hotkeys.BindToKey(undoHotkey, KEY_Z, INPUT_RISING);

            // For Clear
            // Activate is HIGH when (R is rising)
            m_hotkeys.BindToKey(clearHotkey, KEY_R, INPUT_RISING);
        }
        
        // Create and bind hotkeys for messing with the craft
        {
                
            // these keys will allow orientation of spacecraft parts
            Hotkey@ rotateUp = m_hotkeys.AddHotkey("rotateUp");
            Hotkey@ rotateDown = m_hotkeys.AddHotkey("rotateDown");
            Hotkey@ rotateLeft = m_hotkeys.AddHotkey("rotateLeft");
            Hotkey@ rotateRight = m_hotkeys.AddHotkey("rotateRight");
            
            Hotkey@ moveFree = m_hotkeys.AddHotkey("moveFree");
            
            // WASD to rotate the whole thing
            // ideally this should be a single rotate function with arguments
            m_hotkeys.BindToKey(rotateUp, KEY_W, INPUT_RISING);
            m_hotkeys.BindToKey(rotateDown, KEY_S, INPUT_RISING);
            m_hotkeys.BindToKey(rotateLeft, KEY_A, INPUT_RISING);
            m_hotkeys.BindToKey(rotateRight, KEY_D, INPUT_RISING);
            
            // Press G to drag, but there isn't a select function to know what
            // to drag, so be careful :)
            m_hotkeys.BindToKey(moveFree, KEY_G, INPUT_RISING);
            // Make the bind above include an argument, see MoveFree function
            moveFree.m_arguments["FeatureOp"] = FEATUREOP_START;
        }
        
        // Create and bind hotkeys for camera navigation
        {
            Hotkey@ viewOrbitHotkey = m_hotkeys.AddHotkey("vorbit");
            Hotkey@ viewZoomIn = m_hotkeys.AddHotkey("vzoomIn");
            Hotkey@ viewZoomOut = m_hotkeys.AddHotkey("vzoomOut");

            // For Orbit...
            // Activate is HIGH when (RightMouse is HIGH) OR (KeyQ is HIGH)
            m_hotkeys.BindToMouseButton(viewOrbitHotkey, MOUSEB_RIGHT,
                                        INPUT_HIGH);
            m_hotkeys.BindAddOr(viewOrbitHotkey);
            m_hotkeys.BindToKey(viewOrbitHotkey, KEY_E, INPUT_HIGH);
            
            m_hotkeys.BindToMouseWheel(viewZoomIn, INPUT_POSITIVE);
            m_hotkeys.BindToMouseWheel(viewZoomOut, INPUT_NEGATIVE);
            
            
        }
        
        



    }

    void FixedUpdate(float timeStep)
    {
        //Print(m_inputs[0] - m_inputsPrevious[0]);
        
        //UIElement@ f = ui.GetElementAt(input.mousePosition, true);
        //if (f !is null)
        //{
        //    //Print("Under mouse: " + f.name);
        //}

        // the cursor may not just be the mouse in the future considering controller support;
        m_cursor = Vector2(input.mousePosition);
        m_uiCursor.position = input.mousePosition;

        // Enable mouse features only if there is no buttons under the mouse
        m_hotkeys.m_enableMouseDown = (ui.GetElementAt(input.mousePosition, true) is null);

        // Call any stayOn features
        VariantMap stayOnArgs;
        stayOnArgs["FeatureOp"] = FEATUREOP_STAYON;
        
        for (uint i = 0; i < m_features.length; i ++)
        {
            if (m_features[i].m_stayOn)
            {
                m_features[i].Activate(this, m_features[i], stayOnArgs);
            }
        }

        // Update HotKeys, this calls features
        m_hotkeys.Update();
        
        // Kill self once close is requested
        if (m_isClosed && self !is null)
        {
            // self is the ScriptInstance component
            self.Remove();
            
            // Remove all UI, Change this when more workspaces are added
            m_workspaces[0].Remove();
        }
    }
}


/**
 * Convert a physicsless blueprint into a working craft with physics
 * @param blueprint [in] The mess of parts just built by the user.
 */
void SolidifyBlueprint(Node@ blueprint)
{
    // Step 1: Calculate the center of mass.
    // Add up all the vectors and divide by total mass
    
    Vector3 centerOfMass(0, 0, 0);
    float totalMass = 0;
    
    // Get the list of all the parts
    const Array<Node@> parts = blueprint.GetChildren();
    
    // Move the blueprint to (0,0,0) for convinience
    blueprint.position = Vector3::ZERO;

    // Loop through all the parts and add up their masses
    for (uint i = 0; i < parts.length; i ++) 
    {
        //RigidBody@ shape = cast<RigidBody>(childrenColliders[i].GetComponent("RigidBody"));
        // TODO: account for mass-changing Machine classes when fuel tanks are
        //       added; perhaps a CalculateMass function?
        float mass = parts[i].vars["massdry"].GetFloat();
        centerOfMass += (parts[i].worldPosition) * mass;
        totalMass += mass;
    }

    centerOfMass /= totalMass;
    Print("total mass: " + totalMass);
  
    // Step 2: Combine the CollisionShapes into a single node
  
    // TODO: support crafts with moving parts
  
    // Get a list of every node with a collider
    const Array<Node@> childrenColliders = blueprint.GetChildrenWithComponent("CollisionShape", true);
    Array<Variant> colliders;
    Array<Component@> shapes;
    
    // Loop through every node that has a collider
    for (uint i = 0; i < childrenColliders.length; i ++) 
    {
        // Move the node since the center of mass is now the origin
        childrenColliders[i].worldPosition -= centerOfMass;
        
        // Get a list of all the CollisionShapes in the current node
        shapes = childrenColliders[i].GetComponents("CollisionShape");
        colliders.Clear();

        for (uint j = 0; j < shapes.length; j ++) 
        {
            // shapeA is new collision shape, shapeB is existing
            // shapeB's information will be copied into shapeA
            CollisionShape@ shapeA = cast<CollisionShape>(blueprint.CreateComponent("CollisionShape"));
            CollisionShape@ shapeB = cast<CollisionShape>(shapes[j]);
            // Keep track of which CollisionShapes are associated with this node
            colliders.Push(Variant(shapeA));
            shapeA.SetBox(Vector3(1, 1, 1)); // this avoids a weird glitch

            // Correctly position the collision shape, as it's position property
            // is affected by the node's position, scale, and rotation
            shapeA.position = childrenColliders[i].position + childrenColliders[i].rotation * shapeB.position * childrenColliders[i].scale;
            shapeA.rotation = shapeB.rotation * childrenColliders[i].rotation;
            shapeA.size = childrenColliders[i].scale * shapeB.size * 1.01f;
            shapeA.shapeType = shapeB.shapeType;
            //Print("shape added " + shapeA.position.x);
        }
        // Make sure the old node knows which CollisionShapes it's associated with
        childrenColliders[i].vars["colliders"] = colliders;
    }

    // Call the OspUniverse make_craft function
    osp.make_craft(blueprint);
    RigidBody@ body = blueprint.CreateComponent("RigidBody");
    // Set the mass
    body.mass = totalMass;

    // Make sure the craft doesn't slip off the platform so easily
    body.friction = 3;
}

void SetUIAnchors(UIElement@ panel)
{
    panel.enableAnchor = true;
    panel.SetMinAnchor(0.0f, 0.0f);
    panel.SetMaxAnchor(1.0f, 1.0f);
}

// Important features

int Select(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    // TODO: different select types: add to selection, invert selecition, etc..
    
    // if (arg to replace) then
    
    // Make it easier to know if a part is selected
    for (int i = 0; i < editor.m_selection.length; i ++)
    {
        editor.m_selection[i].vars["Selected"] = false;
    }
    editor.m_selection.Clear();
    
    Array<Variant> parts = args["Parts"].GetVariantVector();
    for (uint i = 0; i < parts.length; i ++)
    {
        Node@ part = cast<Node@>(parts[i].GetPtr());
        Print("Selecting: " + part.name);
        part.vars["Selected"] = true;
        editor.m_selection.Push(part);
    }
    
    
    return 0;
}

/**
 * Call Cancel on the feature that is locking the cursor (m_cursorLock)
 * eg. Cancel Move, selected part goes back to its previous position
 * @return Whatever m_cursorLock returns, -1 if cursor not locked.
 */
int CursorLockCancel(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    Print("Cancel!");
    if (editor.m_cursorLock is null)
    {
        return -1;
    }
    else
    {
        VariantMap lockArgs;
        lockArgs["FeatureOp"] = FEATUREOP_CANCEL;
        return editor.m_cursorLock.Activate(editor, editor.m_cursorLock, lockArgs);
    }
}

/**
 * Call Confirm on the feature that is locking the cursor (m_cursorLock)
 * eg. Confirm Move, selected part's position is modified
 * @return Whatever m_cursorLock returns. -1 if cursor not locked.
 */
int CursorLockConfirm(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    Print("Confirm!");
    if (editor.m_cursorLock is null)
    {
        return -1;
    }
    else
    {
        VariantMap lockArgs;
        lockArgs["FeatureOp"] = FEATUREOP_CONFIRM;
        return editor.m_cursorLock.Activate(editor, editor.m_cursorLock, lockArgs);
    }
}

int MoveFree(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{

    // TODO: support symmetries, support attachments
    Text@ cursorText = editor.m_uiCursor.GetChild("CursorText");
    
    
    switch (args["FeatureOp"].GetInt())
    {
    case FEATUREOP_START:
        // Start Moving
        
        if (editor.m_selection.length == 0)
        {
            Print("No objects selected!");
            return -2;
        }
        else
        {

            feature.m_stayOn = true;
            @(editor.m_cursorLock) = feature;
            
            // Used to save original transformations of all selected parts
            Matrix3x4[] originalTransforms;
            originalTransforms.Resize(editor.m_selection.length);

            // First selected part will be used as a reference point for "Offset"
            // which is how far away the selected part is from the cursor
            //Vector3 firstPos(editor.m_selection[0].worldPosition);

            // Get average position of all selected parts, and record their original transformations
            Vector3 averagePos();
            for (int i = 0; i < editor.m_selection.length; i ++)
            {
                originalTransforms[i] = editor.m_selection[i].transform;
                averagePos += editor.m_selection[i].worldPosition;
            }
            averagePos /= editor.m_selection.length;
            
            // Calculate current distance from camera plane
            // This mostly affects how much of the selection moves for each 
            // pixel of the cursor moved.
            float cameraDepth = editor.m_camera.WorldToLocal(averagePos).z;
            
            // Get cursor position
            Vector3 cursorPos = editor.ScreenPosToRay(editor.m_cursor) * cameraDepth + editor.m_camera.worldPosition;

            // Get the offset
            //Vector3 offset = firstPos - cursorPos;
            
            // Store required data for next use
            
            //feature.m_data["Offset"] = offset;
            feature.m_data["CameraDepth"] = cameraDepth;
            feature.m_data["CursorPrevious"] = cursorPos;
            feature.m_data["OriginalTransforms"] = @originalTransforms;
            feature.m_data["AttachmentPairs"] = @(Array<AttachmentPair>());

            // Store some specific options
            
            // Self explanatory
            feature.m_data["DeleteOnCancel"] = args["EnableDeleteOnCancel"].GetBool();
            // Record attachments on confirm
            feature.m_data["EditStructure"] = args["EnableEditStructure"].GetBool();
            // Snap to attachment nodes
            feature.m_data["AttachmentSnap"] = args["EnableAttachmentSnap"].GetBool();
            // Snap to collision boxes
            feature.m_data["ColliderSnap"] = args["EnableColliderSnap"].GetBool();
        }

        return 0;

    case FEATUREOP_STAYON:
        // Currently Moving
        // Block is here because variables cannot be declared here
        {
            // Move all selected parts by adding cursor movement to all parts
            // For preview purposes only, due to loss of precision
            // On confirm,

            // Get stuff from above
            float cameraDepth = float(feature.m_data["CameraDepth"]);
            Vector3 cursorPrevious = Vector3(feature.m_data["CursorPrevious"]);

            // Calculate new cursor position and delta
            Vector3 cursorPosition = editor.ScreenPosToRay(editor.m_cursor)
                                        * cameraDepth
                                        + editor.m_camera.worldPosition;
            Vector3 cursorDelta = cursorPosition - cursorPrevious;

            // Add to all positions
            for (int i = 0; i < editor.m_selection.length; i ++)
            {
                editor.m_selection[i].worldPosition += cursorDelta;
            }
            
            // Calculate possible snapping points
            // TODO: somehow let the player decide which attachment to connect to
            AttachmentPair[]@ pairs = cast<AttachmentPair[]@>(
                                            feature.m_data["AttachmentPairs"]);
            pairs.Clear();
            editor.CalculatePossibleSnaps(pairs, 10, 0.1f, 10.0f);

            //for (int i = 0; i < pairs.length; i ++)
            //{
            //    Print("Possible attachment for: " + pairs[i].m_selected.name);
            //}

            
            
            if (pairs.length == 0)
            {
                cursorText.text = "";
            }
            else
            {
                cursorText.text = "Possible Connections: " + pairs.length;
            }

            feature.m_data["CursorPrevious"] = cursorPosition;
            
            return 0;
        }
    
    case FEATUREOP_CONFIRM:
        // Confirm and apply changes.
        
        // TODO: Move all selected parts back to their original positions, then
        //       calculate new positions using the position of the first part
        {
            
            
            AttachmentPair[]@ pairs = cast<AttachmentPair[]@>(
                                            feature.m_data["AttachmentPairs"]);
            if (pairs.length != 0) 
            {
                // there is a snap!
                
                // TODO: record attachments instead of just moving stuff
                
                // assume the player chooses the first possible snap
                AttachmentPair@ pair = pairs[0];
                
                // Used to revert all parts to their original positions
                //Matrix3x4[]@ originalTransforms=
                //cast<Matrix3x4[]@>(feature.m_data["OriginalTransforms"]);
                
                // Don't do any complicated vector math just yet
                // just add the difference between the two attachments
                Vector3 displacement = pair.m_target.worldPosition
                                        - pair.m_selected.worldPosition;
                
                for (int i = 0; i < editor.m_selection.length; i ++)
                {
                    editor.m_selection[i].worldPosition += displacement;
                }
                
                //Node@ partToAttach = m_selected.parent;
                //Node@ partToAttachTo = m_selected
                
                
                // Play annoying metal hit sound
                Sound@ hit = cache.GetResource("Sound",
                                               "Default/Sfx/MetalHit.ogg");
                editor.m_sfx.Play(hit);
                editor.m_sfx.frequency = hit.frequency * Random(0.9f, 1.1f);
            }
            
            cursorText.text = "";
            feature.m_stayOn = false;
            @(editor.m_cursorLock) = null;
            return 0;
        }
    case FEATUREOP_CANCEL:
        // Cancel and move selection back to original positions
        {
            if (bool(feature.m_data["DeleteOnCancel"]))
            {
                // Delete selection
                for (int i = 0; i < editor.m_selection.length; i ++)
                {
                    //editor.m_subject.RemoveChild(editor.m_selection[i]);
                    
                    editor.m_selection[i].Remove();
                }
                editor.m_selection.Clear();
            }
            else
            {
                // TODO: move back to original positions
            }
            
            cursorText.text = "";
            feature.m_stayOn = false;
            @(editor.m_cursorLock) = null;
        }
    
    }
    
   
    
    return 0;
}

int LunchTime(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    //ui.root.RemoveAllChildren();
    cast<SoundSource>(editor.GetNode().GetComponent("SoundSource")).Stop();
    
    osp.debug_function(StringHash("create_universe"));
    SolidifyBlueprint(editor.m_subject);
    
    editor.Close();
    
    // Add the FlightUI
    Node@ sceneA = editor.GetNode();
    sceneA.CreateScriptObject("Default/Scripts/FlightUI/FlightUI.as",
                              "FlightUI");
    
    return 0;
}
