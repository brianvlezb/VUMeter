#include "VUMeter.h"

static const float RMS_FLOOR = 0.000001f;

// ---------------------------------------------------------------------------
CVUMeter::CVUMeter()  {}
CVUMeter::~CVUMeter() {}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnLoad()
{
    // Solo declaramos para que VirtualDJ muestre el texto
    cb->DeclareParameter(&m_displayLU, VDJPARAM_SLIDER, ID_LU, "LU", "LU", 1.0f);
    cb->DeclareParameter(&m_displayDB, VDJPARAM_SLIDER, ID_DB, "dB", "dB", 1.0f);
    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
    infos->PluginName   = "VU Meter";
    infos->Author       = "Brian";
    infos->Description  = "LU (izq) | dB (der) - actualización lenta";
    infos->Version      = "1.3";
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

    // Suavizado rápido interno
    const float alpha = 0.4f;
    m_rmsL = m_rmsL * (1.0f - alpha) + rmsL * alpha;
    m_rmsR = m_rmsR * (1.0f - alpha) + rmsR * alpha;

    // Cálculo real
    float currentLU = (m_rmsL > RMS_FLOOR) ? 20.0f * log10f(m_rmsL) + 3.0f : -60.0f;
    float rmsAvg = (m_rmsL + m_rmsR) * 0.5f;
    float currentDB = (rmsAvg > RMS_FLOOR) ? 20.0f * log10f(rmsAvg) : -60.0f;

    // Actualizar display solo cada 8 buffers (~120-150ms)
    m_counter++;
    if (m_counter >= 8)
    {
        m_displayLU = currentLU;
        m_displayDB = currentDB;
        m_counter = 0;
    }

    // Guardamos los valores reales para cálculos internos
    m_lu = currentLU;
    m_db = currentDB;

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetParameterString(int id, char *outParam, int outParamSize)
{
    switch (id)
    {
        case ID_LU:
            sprintf(outParam, "%.1f", m_displayLU);   // Izquierda: LU
            break;
        case ID_DB:
            sprintf(outParam, "%.1f", m_displayDB);   // Derecha: dB
            break;
        default:
            if (outParamSize > 0) outParam[0] = '\0';
            break;
    }
    return S_OK;
}
