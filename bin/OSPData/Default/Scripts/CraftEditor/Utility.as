namespace Utility
{

int ClearAll(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{

    editor.m_subject.RemoveAllChildren();
    
    return 0;
}

int Undo(CraftEditor@ editor, EditorFeature@ feature, VariantMap& args)
{
    Print("UNDO!");
    return 0;   
}


}
