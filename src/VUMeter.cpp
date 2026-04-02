#include "VUMeter.h"

static const float RMS_FLOOR = 0.000001f;

CVUMeter::CVUMeter()  {}
CVUMeter::~CVUMeter() {}

HRESULT VDJ_API CVUMeter::OnLoad()
{
    cb->DeclareParameter(&m_luDisplay, VDJPARAM_SLIDER, ID_LU, "LU", "LU", 1.0f);
    cb->DeclareParameter(&m_dbDisplay, VDJPARAM_SLIDER, ID_DB, "dB", "dB", 1.0f);
    return S_OK;
}

HRESULT VDJ_API CVUMeter::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
    infos->PluginName   = "VU Meter";
    infos->Author       = "Brian";
    infos->Description  = "L | R - RMS +3dB (AES-17), ballistica VU 300ms";
    infos->Version      = "2.0";
    infos->Flags        = 0x00;
    infos->Bitmap       = NULL;
    return S_OK;
}

HRESULT VDJ_API CVUMeter::OnStart()
{
    m_sampleRate = 44100;
    m_rmsL       = 0.0f;
    m_rmsR       = 0.0f;
    m_peakHold   = 0.0f;
    m_counter    = 0;
    return S_OK;
}

HRESULT VDJ_API CVUMeter::OnStop() { return S_OK; }

HRESULT VDJ_API CVUMeter::OnProcessSamples(float *buffer, int nb)
{
    if (!buffer || nb <= 0) return S_OK;

    double sumL = 0.0, sumR = 0.0;
    float  blockPeak = 0.0f;

    for (int i = 0; i < nb * 2; i += 2)
    {
        double sL = buffer[i];
        double sR = buffer[i + 1];
        sumL += sL * sL;
        sumR += sR * sR;

        float absL = fabsf(buffer[i]);
        float absR = fabsf(buffer[i + 1]);
        if (absL > blockPeak) blockPeak = absL;
        if (absR > blockPeak) blockPeak = absR;
    }

    float rmsInstL = (float)sqrt(sumL / nb);
    float rmsInstR = (float)sqrt(sumR / nb);

    // VU ballistics IEC 60268-17: tau=65ms → 99% en 300ms (igual que VUMT)
    float blockDuration = (float)nb / (float)m_sampleRate;
    float alpha         = 1.0f - expf(-blockDuration / 0.065f);
    m_rmsL = m_rmsL * (1.0f - alpha) + rmsInstL * alpha;
    m_rmsR = m_rmsR * (1.0f - alpha) + rmsInstR * alpha;

    // Peak hold: ataque instantáneo, caída 1.5s
    float decayAlpha = 1.0f - expf(-blockDuration / 1.5f);
    if (blockPeak > m_peakHold)
        m_peakHold = blockPeak;
    else
        m_peakHold = m_peakHold * (1.0f - decayAlpha);

    // RMS stereo promediado L²+R² (igual que VUMT RMS mode) + AES-17 +3.01dB
    float rmsAvg = sqrtf((m_rmsL * m_rmsL + m_rmsR * m_rmsR) * 0.5f);
    float dbLU   = (rmsAvg    > RMS_FLOOR) ? 20.0f * log10f(rmsAvg)    + 3.01f : -60.0f;
    float dbPeak = (m_peakHold > RMS_FLOOR) ? 20.0f * log10f(m_peakHold)        : -60.0f;

    // ~3 actualizaciones por segundo
    m_counter++;
    if (m_counter >= (int)(m_sampleRate / nb / 3))
    {
        m_luDisplay = dbLU;
        m_dbDisplay = dbPeak;
        m_counter   = 0;
    }

    return S_OK;
}

HRESULT VDJ_API CVUMeter::OnGetParameterString(int id, char *outParam, int outParamSize)
{
    switch (id)
    {
        case ID_LU:
            snprintf(outParam, outParamSize, "%.1f", m_luDisplay);
            break;
        case ID_DB:
            snprintf(outParam, outParamSize, "%.1f", m_dbDisplay);
            break;
        default:
            if (outParamSize > 0) outParam[0] = '\0';
            break;
    }
    return S_OK;
}
