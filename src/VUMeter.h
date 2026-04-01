#ifndef VUMETER_H
#define VUMETER_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>
#include "vdjDsp8.h"     // ← Correcto para plugins DSP

class CVUMeter : public IVdjPluginDsp8     // ← Cambiado de IVdjPlugin8Dsp
{
public:
    CVUMeter();
    ~CVUMeter();

    HRESULT VDJ_API OnLoad();
    HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *infos);
    HRESULT VDJ_API OnStart();
    HRESULT VDJ_API OnStop();
    HRESULT VDJ_API OnProcess(float *buffer, int numsamples, int samplerate, int numchannels);

    HRESULT VDJ_API OnGetParameterString(int id, char *outParam, int outParamSize);

    // Release ya viene implementado en la clase base IVdjPluginDsp8
    // No es necesario redeclararlo a menos que quieras sobrescribirlo

private:
    // Slider values (0.0–1.0) used by VDJ to draw the meter bars
    float m_levelL = 0.0f;
    float m_levelR = 0.0f;

    // Internal RMS accumulators (smoothed)
    float m_rmsL = 0.0f;
    float m_rmsR = 0.0f;

    // Current dB values
    float m_dbL = -60.0f;
    float m_dbR = -60.0f;

    // IDs para los parámetros (medidores)
    typedef enum _ID_Interface
    {
        ID_METER_L = 0,
        ID_METER_R = 1
    } ID_Interface;
};

#endif // VUMETER_H
