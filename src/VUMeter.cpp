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
    infos->Description  = "LU | dB - lento como Klanghelm";
    infos->Version      = "1.6";
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

    // Suavizado muy lento (como aguja analógica)
    const float alpha = 0.08f;
    m_rmsL = m_rmsL * (1.0f - alpha) + rmsL * alpha;
    m_rmsR = m_rmsR * (1.0f - alpha) + rmsR * alpha;

    // Cálculos
    float currentLU = (m_rmsL > RMS_FLOOR) ? 20.0f * log10f(m_rmsL) + 3.0f : -60.0f;  // +3dB AES-17
    float currentDB = (m_rmsR > m_rmsL ? m_rmsR : m_rmsL);                           // canal más fuerte
    currentDB = (currentDB > RMS_FLOOR) ? 20.0f * log10f(currentDB) : -60.0f;

    // Actualizar display solo cada 24 buffers ≈ 400 ms
    m_counter++;
    if (m_counter >= 24)
    {
        m_luDisplay = currentLU;
        m_dbDisplay = currentDB;
        m_counter = 0;
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetParameterString(int id, char *outParam, int outParamSize)
{
    switch (id)
    {
        case ID_LU:
            sprintf(outParam, "%.1f", m_luDisplay);
            break;
        case ID_DB:
            sprintf(outParam, "%.1f", m_dbDisplay);
            break;
        default:
            if (outParamSize > 0) outParam[0] = '\0';
            break;
    }
    return S_OK;
}
