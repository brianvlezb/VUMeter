#include "VUMeter.h"

// Umbral mínimo para evitar log(0)
static const float RMS_FLOOR = 0.000001f;

// Tiempo de integración VU estándar: ~300 ms
// alpha se recalcula en OnStart cuando conocemos el sampleRate y block size
static const float VU_INTEGRATION_MS = 300.0f;

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
    // VirtualDJ opera a 44100 Hz internamente
    m_sampleRate = 44100;

    m_rmsL    = 0.0f;
    m_rmsR    = 0.0f;
    m_counter = 0;
    return S_OK;
}

HRESULT VDJ_API CVUMeter::OnStop() { return S_OK; }

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnProcessSamples(float *buffer, int nb)
{
    if (!buffer || nb <= 0) return S_OK;

    // Calcular alpha según duración real del bloque
    // tau = VU_INTEGRATION_MS / 1000
    // alpha = 1 - exp(-blockDuration / tau)
    float blockDuration = (float)nb / (float)m_sampleRate;
    float tau           = VU_INTEGRATION_MS / 1000.0f;
    float alpha         = 1.0f - expf(-blockDuration / tau);

    // Acumular energía por canal (interleaved: L, R, L, R...)
    double sumL = 0.0, sumR = 0.0;
    for (int i = 0; i < nb * 2; i += 2)
    {
        double sL = buffer[i];
        double sR = buffer[i + 1];
        sumL += sL * sL;
        sumR += sR * sR;
    }

    // RMS instantáneo por canal
    float rmsInstL = (float)sqrt(sumL / nb);
    float rmsInstR = (float)sqrt(sumR / nb);

    // Suavizado exponencial independiente por canal (ballística VU)
    m_rmsL = m_rmsL * (1.0f - alpha) + rmsInstL * alpha;
    m_rmsR = m_rmsR * (1.0f - alpha) + rmsInstR * alpha;

    // Convertir a dBFS con corrección +3dB AES-17
    float dbL = (m_rmsL > RMS_FLOOR) ? 20.0f * log10f(m_rmsL) + 3.0f : -60.0f;
    float dbR = (m_rmsR > RMS_FLOOR) ? 20.0f * log10f(m_rmsR) + 3.0f : -60.0f;

    // Actualizar display ~15 veces por segundo (legible, sin parpadeo)
    m_counter++;
    if (m_counter >= (m_sampleRate / nb / 15))
    {
        m_luDisplay = dbL;  // Izquierdo
        m_dbDisplay = dbR;  // Derecho
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
