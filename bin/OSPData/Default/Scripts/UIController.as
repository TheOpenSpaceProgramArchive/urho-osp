interface UIController : ScriptObject
{
    int ActivateFeature(const String& name, VariantMap& args);
    Node@ GetNode();
    void Close();
}
