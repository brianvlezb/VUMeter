#include "VUMeter.h"

// Umbral mínimo para evitar log(0)
static const float RMS_FLOOR = 0.000001f;

// ---------------------------------------------------------------------------
CVUMeter::CVUMeter()  {}
CVUMeter::~CVUMeter() {}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnLoad()
{
    cb->DeclareParameter(&m_luDisplay, VDJPARAM_SLIDER, ID_LU, "LU", "LU", 1.0f);
    cb->DeclareParameter(&m_dbDisplay, VDJPARAM_SLIDER, ID_DB, "dB", "dB", 1.0f);
    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
    infos->PluginName   = "VU Meter";
    infos->Author       = "Brian";
    infos->Description  = "L | R - RMS +3dB (AES-17), ballistica VU 300ms";
    infos->Version      = "1.9";
    infos->Flags        = 0x00;
    infos->Bitmap       = NULL;
    return S_OK;
}

// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnProcessSamples(float *buffer, int nb)
{
    if (!buffer || nb <= 0) return S_OK;

    // Acumular energía RMS y detectar peak por bloque
    double sumL = 0.0, sumR = 0.0;
    float  blockPeak = 0.0f;

    for (int i = 0; i < nb * 2; i += 2)
    {
        double sL = buffer[i];
        double sR = buffer[i + 1];

        sumL += sL * sL;
        sumR += sR * sR;

        // Peak: máximo absoluto de ambos canales (mejor precisión)
        float absL = fabsf(buffer[i]);
        float absR = fabsf(buffer[i + 1]);
        if (absL > blockPeak) blockPeak = absL;
        if (absR > blockPeak) blockPeak = absR;   // ← más directo y fiable
    }

    // RMS instantáneo por canal (para LU, izquierdo)
    float rmsInstL = (float)sqrt(sumL / nb);
    float rmsInstR = (float)sqrt(sumR / nb);

    // Suavizado exponencial RMS — ballística VU ~300ms
    float blockDuration = (float)nb / (float)m_sampleRate;
    float alpha         = 1.0f - expf(-blockDuration / 0.3f);
    m_rmsL = m_rmsL * (1.0f - alpha) + rmsInstL * alpha;
    m_rmsR = m_rmsR * (1.0f - alpha) + rmsInstR * alpha;

    // Peak hold con caída suave (1.5 de decay)
    float decayAlpha = 1.0f - expf(-blockDuration / 1.5f);
    if (blockPeak > m_peakHold)
        m_peakHold = blockPeak;                             // ataque instantáneo
    else
        m_peakHold = m_peakHold * (1.0f - decayAlpha);     // caída lenta

    // Izquierdo: RMS +3dB AES-17 (igual que Klanghelm RMS mode)
    float dbL   = (m_rmsL > RMS_FLOOR) ? 20.0f * log10f(m_rmsL) + 3.0f : -60.0f;

    // Derecho: Peak dBFS (sin corrección +3dB, igual que Klanghelm peak readout)
    float dbPeak = (m_peakHold > RMS_FLOOR) ? 20.0f * log10f(m_peakHold) : -60.0f;

    // Actualizar display ~3 veces por segundo (legible)
    m_counter++;
    if (m_counter >= (m_sampleRate / nb / 1))
    {
        m_luDisplay = dbL;      // Izquierdo: RMS
        m_dbDisplay = dbPeak;   // Derecho:   Peak dBFS
        m_counter   = 0;
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
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
