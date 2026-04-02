#pragma once

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
    float m_luDisplay = -60.0f;   // Izquierdo: RMS +3dB (AES-17) → valor VU
    float m_dbDisplay = -60.0f;   // Derecho:   Peak dBFS (pico instantáneo)

    float m_rmsL      = 0.0f;     // Energía suavizada canal L
    float m_rmsR      = 0.0f;     // Energía suavizada canal R
    float m_peakHold  = 0.0f;     // Peak hold para suavizar la lectura derecha

    int   m_counter    = 0;
    int   m_sampleRate = 44100;

    typedef enum _ID_Interface
    {
        ID_LU = 0,
        ID_DB = 1
    } ID_Interface;
};
