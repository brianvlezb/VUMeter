#include "VUMeter.h"

HRESULT VDJ_API DllGetClassObject(const GUID &rclsid, const GUID &riid, void **ppObject)
{
    if (IsEqualGUID(rclsid, CLSID_VdjPlugin8) &&
        IsEqualGUID(riid, IID_IVdjPluginDsp8))
    {
        *ppObject = new CVUMeter();
        return S_OK;
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}
