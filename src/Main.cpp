#include "VUMeter.h"

// Standard COM-style DLL entry point required by VirtualDJ.
// VirtualDJ calls this to get an instance of the plugin.

HRESULT VDJ_API DllGetClassObject(const GUID &rclsid, const GUID &riid, void **ppObject)
{
    if (memcmp(&rclsid, &CLSID_VdjPlugin8,       sizeof(GUID)) == 0 &&
        memcmp(&riid,   &IID_IVdjPluginBasic8,    sizeof(GUID)) == 0)
    {
        *ppObject = new CVUMeter();
        return NO_ERROR;
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}
