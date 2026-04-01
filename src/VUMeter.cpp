#include "VUMeter.h"

static const float RMS_FLOOR = 0.000001f;

// ---------------------------------------------------------------------------
CVUMeter::CVUMeter()  {}
CVUMeter::~CVUMeter() {}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnLoad()
{
    cb->DeclareParameter(&m_lu, VDJPARAM_SLIDER, ID_LU, "LU", "LU", 1.0f);
    cb->DeclareParameter(&m_db, VDJPARAM_SLIDER, ID_DB, "dB", "dB", 1.0f);
    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
    infos->PluginName   = "VU Meter";
    infos->Author       = "Brian";
    infos->Description  = "LU (izq) | dB (der)";
    infos->Version      = "1.2";
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

    for (int i = 0; i < nb*2; i += 2)
    {
        sumL += buffer[i]   * buffer[i];
        sumR += buffer[i+1] * buffer[i+1];
    }

    float rmsL = (float)sqrt(sumL / nb);
    float rmsR = (float)sqrt(sumR / nb);

    const float alpha = 0.35f;
    m_rmsL = m_rmsL * (1.0f - alpha) + rmsL * alpha;
    m_rmsR = m_rmsR * (1.0f - alpha) + rmsR * alpha;

    // Izquierda: LUFS estimado (RMS + offset aproximado)
    m_lu = (m_rmsL > RMS_FLOOR) ? 20.0f * log10f(m_rmsL) + 3.0f : -60.0f;

    // Derecha: dB RMS (promedio de ambos canales)
    float rmsAvg = (m_rmsL + m_rmsR) * 0.5f;
    m_db = (rmsAvg > RMS_FLOOR) ? 20.0f * log10f(rmsAvg) : -60.0f;

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetParameterString(int id, char *outParam, int outParamSize)
{
    switch (id)
    {
        case ID_LU:   // Valor izquierdo
            sprintf(outParam, "%.1f", m_lu);
            break;
        case ID_DB:   // Valor derecho
            sprintf(outParam, "%.1f", m_db);
            break;
        default:
            if (outParamSize > 0) outParam[0] = '\0';
            break;
    }
    return S_OK;
}
