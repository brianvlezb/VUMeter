#include "VUMeter.h"

// Standard COM-style DLL entry point required by VirtualDJ.

HRESULT VDJ_API DllGetClassObject(const GUID &rclsid, const GUID &riid, void **ppObject)
{
    // Accept both the DSP-specific IID and the generic basic IID for compatibility
    bool clsidOk = (memcmp(&rclsid, &CLSID_VdjPlugin8, sizeof(GUID)) == 0);
    bool riidOk  = (memcmp(&riid, &IID_IVdjPluginBasic8,   sizeof(GUID)) == 0)
                || (memcmp(&riid, &IID_IVdjPlugin8Dsp,      sizeof(GUID)) == 0);

    if (clsidOk && riidOk)
    {
        *ppObject = new CVUMeter();
        return NO_ERROR;
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}
