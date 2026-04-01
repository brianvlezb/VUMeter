#include "VUMeter.h"

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static const float DB_FLOOR = -60.0f;
static const float DB_CEIL  = 0.0f;
static const float DB_RANGE = DB_CEIL - DB_FLOOR;
static const float RMS_FLOOR = 0.000001f;

static inline float toSlider(float db)
{
    float v = (db - DB_FLOOR) / DB_RANGE;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    return v;
}

// ---------------------------------------------------------------------------
CVUMeter::CVUMeter()  {}
CVUMeter::~CVUMeter() {}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnLoad()
{
    // Forma correcta en SDK público: usar el host
    m_pVdjHost->DeclareParameter(&m_levelL, VDJPARAM_SLIDER, ID_METER_L, "Level L", "L", 1.0f);
    m_pVdjHost->DeclareParameter(&m_levelR, VDJPARAM_SLIDER, ID_METER_R, "Level R", "R", 1.0f);

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
    infos->PluginName   = "VU Meter";
    infos->Author       = "Brian";
    infos->Description  = "Stereo RMS / dBFS VU Meter for VirtualDJ";
    infos->Version      = "1.0";
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
    if (!buffer || nb <= 0)
        return S_OK;

    double sumL = 0.0, sumR = 0.0;

    for (int i = 0; i < nb * 2; i += 2)
    {
        double l = buffer[i];
        double r = buffer[i + 1];
        sumL += l * l;
        sumR += r * r;
    }

    float instantL = (float)sqrt(sumL / nb);
    float instantR = (float)sqrt(sumR / nb);

    const float ATTACK  = 0.5f;
    const float RELEASE = 0.05f;

    m_rmsL = m_rmsL + (instantL > m_rmsL ? ATTACK : RELEASE) * (instantL - m_rmsL);
    m_rmsR = m_rmsR + (instantR > m_rmsR ? ATTACK : RELEASE) * (instantR - m_rmsR);

    m_dbL = (m_rmsL > RMS_FLOOR) ? 20.0f * log10f(m_rmsL) : DB_FLOOR;
    m_dbR = (m_rmsR > RMS_FLOOR) ? 20.0f * log10f(m_rmsR) : DB_FLOOR;

    m_levelL = toSlider(m_dbL);
    m_levelR = toSlider(m_dbR);

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetParameterString(int id, char *outParam, int outParamSize)
{
    switch (id)
    {
        case ID_METER_L:
            sprintf(outParam, "L: %.1f dB", m_dbL);
            break;
        case ID_METER_R:
            sprintf(outParam, "R: %.1f dB", m_dbR);
            break;
        default:
            if (outParamSize > 0) outParam[0] = '\0';
            break;
    }
    return S_OK;
}
