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
    float m_luDisplay  = -60.0f;   // Izquierda: LU estimado (muestra lento)
    float m_dbDisplay  = -60.0f;   // Derecha: dB (muestra lento)

    float m_rmsL = 0.0f;
    float m_rmsR = 0.0f;

    int   m_updateCounter = 0;

    typedef enum _ID_Interface
    {
        ID_LU = 0,   // Izquierda
        ID_DB = 1    // Derecha
    } ID_Interface;
};

#endif // VUMETER_H
