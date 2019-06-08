namespace Utility
{

int ClearAll(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{

    editor.m_subject.RemoveAllChildren();
    editor.m_selection.Clear();
    
    return 0;
}

int Undo(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    Print("UNDO!");
    return 0;   
}

int RotateUp(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{

    Print("Rotating Up");   
    editor.m_subject.Pitch(90);
    return 0;
}
int RotateDown(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{

    Print("Rotating Down");  
        editor.m_subject.Pitch(-90);

    return 0;
}
int RotateLeft(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{

    Print("Rotating Left");  
    editor.m_subject.Yaw(-90);
    return 0;
}
int RotateRight(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{

    Print("Rotating Right");  
    editor.m_subject.Yaw(90);
    return 0;
}


}
