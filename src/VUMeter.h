#ifndef VUMETER_H
#define VUMETER_H

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include "vdjDsp8.h"

class CVUMeter : public IVdjPlugin8Dsp
{
public:
    HRESULT VDJ_API OnLoad();
    HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *infos);
    ULONG   VDJ_API Release();
    HRESULT VDJ_API OnGetUserInterface(TVdjPluginInterface8 *pluginInterface);
    HRESULT VDJ_API OnProcess(float *buffer, int numsamples, int samplerate, int numchannels);
    HRESULT VDJ_API OnGetParameterString(int id, char *outParam, int outParamSize);

private:
    // Slider values (0.0–1.0) used by VDJ to draw the meter bars
    float m_levelL;
    float m_levelR;

    // Internal RMS accumulators (smoothed)
    float m_rmsL;
    float m_rmsR;

    // Current dB values (updated every buffer)
    float m_dbL;
    float m_dbR;

    typedef enum _ID_Interface
    {
        ID_METER_L = 0,
        ID_METER_R = 1
    } ID_Interface;
};

#endif // VUMETER_H
