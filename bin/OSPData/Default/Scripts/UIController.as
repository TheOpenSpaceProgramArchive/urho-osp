interface UIController : ScriptObject
{
    int ActivateFeature(const String& name, VariantMap& args);
    void Close();
}
