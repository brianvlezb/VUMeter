#include "VUMeter.h"

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
    infos->Description  = "LU (izq) | dB (der) - lento";
    infos->Version      = "1.4";
    infos->Flags        = 0x00;
    infos->Bitmap       = NULL;
    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnStart()  { return S_OK; }
HRESULT VDJ_API CVUMeter::OnStop()   { return S_OK; }

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnProcessSamples(float *buffer, int nb)
{
    if (!buffer || nb <= 0) return S_OK;

    double sumL = 0.0, sumR = 0.0;

    for (int i = 0; i < nb * 2; i += 2)
    {
        sumL += buffer[i]   * buffer[i];
        sumR += buffer[i+1] * buffer[i+1];
    }

    float rmsL = (float)sqrt(sumL / nb);
    float rmsR = (float)sqrt(sumR / nb);

    // Suavizado interno rápido
    const float alpha = 0.35f;
    m_rmsL = m_rmsL * (1.0f - alpha) + rmsL * alpha;
    m_rmsR = m_rmsR * (1.0f - alpha) + rmsR * alpha;

    // Cálculos reales
    float currentLU = (m_rmsL > RMS_FLOOR) ? 20.0f * log10f(m_rmsL) + 3.0f : -60.0f;
    float currentDB = (m_rmsR > RMS_FLOOR) ? 20.0f * log10f(m_rmsR) : -60.0f;   // usamos el canal derecho como referencia para peak

    // Actualizamos el display solo cada 12 buffers ≈ 200ms
    m_updateCounter++;
    if (m_updateCounter >= 12)
    {
        m_luDisplay = currentLU;
        m_dbDisplay = currentDB;
        m_updateCounter = 0;
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetParameterString(int id, char *outParam, int outParamSize)
{
    switch (id)
    {
        case ID_LU:
            sprintf(outParam, "%.1f", m_luDisplay);   // Izquierda: LU
            break;
        case ID_DB:
            sprintf(outParam, "%.1f", m_dbDisplay);   // Derecha: dB
            break;
        default:
            if (outParamSize > 0) outParam[0] = '\0';
            break;
    }
    return S_OK;
}
