#include "VUMeter.h"

// ---------------------------------------------------------------------------
// Range:  -60 dB  →  0 dB  mapped to slider  0.0 → 1.0
// ---------------------------------------------------------------------------
static const float DB_FLOOR  = -60.0f;
static const float DB_CEIL   =   0.0f;
static const float DB_RANGE  = DB_CEIL - DB_FLOOR;   // 60.0f
static const float RMS_FLOOR = 0.000001f;            // -120 dBFS, avoids log(0)

static inline float toSlider(float db)
{
    float v = (db - DB_FLOOR) / DB_RANGE;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    return v;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnLoad()
{
    m_levelL = 0.0f;
    m_levelR = 0.0f;
    m_rmsL   = 0.0f;
    m_rmsR   = 0.0f;
    m_dbL    = DB_FLOOR;
    m_dbR    = DB_FLOOR;

    // Declare sliders so VirtualDJ draws the meter bars
    DeclareParameterSlider(&m_levelL, ID_METER_L, "Level L", "L", 1.0f);
    DeclareParameterSlider(&m_levelR, ID_METER_R, "Level R", "R", 1.0f);

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
ULONG VDJ_API CVUMeter::Release()
{
    delete this;
    return 0;
}

// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetUserInterface(TVdjPluginInterface8 *pluginInterface)
{
    pluginInterface->Type = VDJINTERFACE_DEFAULT;
    return S_OK;
}

// ---------------------------------------------------------------------------
// Core DSP: measure only, audio passes through unchanged
// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnProcess(float *buffer, int numsamples, int samplerate, int numchannels)
{
    if (!buffer || numsamples <= 0)
        return S_OK;

    double sumL = 0.0, sumR = 0.0;

    if (numchannels >= 2)
    {
        // Interleaved stereo: L0 R0 L1 R1 ...
        for (int i = 0; i < numsamples * 2; i += 2)
        {
            double l = buffer[i];
            double r = buffer[i + 1];
            sumL += l * l;
            sumR += r * r;
        }
        float instantL = (float)sqrt(sumL / numsamples);
        float instantR = (float)sqrt(sumR / numsamples);

        // Exponential smoothing — attack fast, release slower
        const float ATTACK  = 0.5f;
        const float RELEASE = 0.05f;
        m_rmsL = m_rmsL + (instantL > m_rmsL ? ATTACK : RELEASE) * (instantL - m_rmsL);
        m_rmsR = m_rmsR + (instantR > m_rmsR ? ATTACK : RELEASE) * (instantR - m_rmsR);
    }
    else  // mono
    {
        for (int i = 0; i < numsamples; i++)
        {
            double s = buffer[i];
            sumL += s * s;
        }
        float instantL = (float)sqrt(sumL / numsamples);
        const float ATTACK  = 0.5f;
        const float RELEASE = 0.05f;
        m_rmsL = m_rmsL + (instantL > m_rmsL ? ATTACK : RELEASE) * (instantL - m_rmsL);
        m_rmsR = m_rmsL;
    }

    // RMS → dBFS
    m_dbL = (m_rmsL > RMS_FLOOR) ? 20.0f * log10f(m_rmsL) : DB_FLOOR;
    m_dbR = (m_rmsR > RMS_FLOOR) ? 20.0f * log10f(m_rmsR) : DB_FLOOR;

    // Map to slider (0.0–1.0) — VirtualDJ reads these for the bar display
    m_levelL = toSlider(m_dbL);
    m_levelR = toSlider(m_dbR);

    return S_OK;  // unmodified audio
}

// ---------------------------------------------------------------------------
// VirtualDJ calls this to show the value string next to each slider
// ---------------------------------------------------------------------------
HRESULT VDJ_API CVUMeter::OnGetParameterString(int id, char *outParam, int outParamSize)
{
    switch (id)
    {
        case ID_METER_L:
            sprintf(outParam, "L: %.1f dB  (RMS %.4f)", m_dbL, m_rmsL);
            break;
        case ID_METER_R:
            sprintf(outParam, "R: %.1f dB  (RMS %.4f)", m_dbR, m_rmsR);
            break;
        default:
            outParam[0] = '\0';
            break;
    }
    return S_OK;
}
