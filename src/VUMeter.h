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
    float m_lu  = -60.0f;     // Valor izquierdo: LU estimado (para mostrar)
    float m_db  = -60.0f;     // Valor derecho: dB RMS (para mostrar)

    float m_rmsL = 0.0f;      // RMS interno (se actualiza rápido)
    float m_rmsR = 0.0f;

    // Para actualización lenta (como Klanghelm)
    float m_displayLU = -60.0f;
    float m_displayDB = -60.0f;
    int   m_counter   = 0;

    typedef enum _ID_Interface
    {
        ID_LU = 0,
        ID_DB = 1
    } ID_Interface;
};

#endif // VUMETER_H
