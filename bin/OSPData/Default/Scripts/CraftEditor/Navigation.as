namespace Navigation
{


int ViewOrbit(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    Print("Orbit");
    Vector2 sensitivity = Vector2(0.4, 0.4);
    Quaternion rotX = Quaternion(0, editor.m_hotkeys.m_joysticks[0].x
                                    * sensitivity.x, 0);
    Quaternion rotY = Quaternion(editor.m_hotkeys.m_joysticks[0].y
                                 * sensitivity.y, 0, 0);
    editor.m_cameraCenter.Rotate(rotX, TS_WORLD);
    editor.m_cameraCenter.Rotate(rotY, TS_LOCAL);
    return 0;
}

// Zoom function from the_DemonGod
// some day a zoom function that uses arguments instead of separate in/out

int ZoomIn(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    Print("Zoom In");
    editor.m_camera.position += editor.m_camera.direction.Normalized();
    return 0;
}

int ZoomOut(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    Print("Zoom Out");
    editor.m_camera.position -= editor.m_camera.direction.Normalized();
    return 0;
}

}
