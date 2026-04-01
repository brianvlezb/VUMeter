#ifndef VUMETER_H
#define VUMETER_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>
#include "vdjDsp8.h"

class CVUMeter : public IVdjPluginDsp8
{
public:
    CVUMeter();
    ~CVUMeter();

    HRESULT VDJ_API OnLoad();
    HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *infos);
    HRESULT VDJ_API OnStart();
    HRESULT VDJ_API OnStop();
    HRESULT VDJ_API OnProcessSamples(float *buffer, int nb);
    HRESULT VDJ_API OnGetParameterString(int id, char *outParam, int outParamSize);

private:
    float m_lufsL = -60.0f;   // Valor izquierdo: LUFS estimado
    float m_dbR   = -60.0f;   // Valor derecho: dB RMS

    float m_rmsL = 0.0f;
    float m_rmsR = 0.0f;

    typedef enum _ID_Interface
    {
        ID_LEFT  = 0,   // LUFS
        ID_RIGHT = 1    // dB
    } ID_Interface;
};

#endif // VUMETER_H
