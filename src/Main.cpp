#include "VUMeter.h"

// Standard COM-style DLL entry point required by VirtualDJ.
HRESULT VDJ_API DllGetClassObject(const GUID &rclsid, const GUID &riid, void **ppObject)
{
    // Usamos el CLSID y IID correctos para plugins DSP con el SDK público
    if (IsEqualGUID(rclsid, CLSID_VdjPlugin8) &&
        (IsEqualGUID(riid, IID_IVdjPlugin8) || IsEqualGUID(riid, IID_IVdjPluginDsp8)))
    {
        *ppObject = new CVUMeter();
        return S_OK;                    // NO_ERROR es lo mismo que S_OK
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}
