/*---------------------------------------------------------*\
| ViaAudioCapture.cpp                                       |
|                                                           |
|   WASAPI loopback capture reduced to bass/treble levels   |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cmath>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>
#include "ViaAudioCapture.h"
#include "ViaLog.h"

#define AUDIO_BUFFER_DURATION_100NS     2000000     /* 200 ms, far more than one tick       */
#define AUDIO_BASS_CUTOFF_HZ            150.0f
#define AUDIO_ENVELOPE_RELEASE_PER_S    6.0f        /* level falls to 1/e in ~170 ms        */
#define AUDIO_PEAK_RELEASE_PER_S        0.15f       /* auto gain adapts over several seconds*/
#define AUDIO_PEAK_FLOOR                0.0005f     /* silence is not amplified to full     */
#define VIA_PI                          3.14159265f

template<typename T> static void release(T** ptr)
{
    if(*ptr != NULL)
    {
        (*ptr)->Release();
        *ptr = NULL;
    }
}

ViaAudioCapture::ViaAudioCapture()
{
    audio_client        = NULL;
    capture_client      = NULL;
    com_initialized     = false;
    channels            = 0;
    is_float            = false;
    bytes_per_sample    = 0;
    lowpass_coeff       = 0.0f;
    lowpass_state       = 0.0f;
    bass_envelope       = 0.0f;
    treble_envelope     = 0.0f;
    peak                = AUDIO_PEAK_FLOOR;
}

ViaAudioCapture::~ViaAudioCapture()
{
    Close();
}

bool ViaAudioCapture::IsOpen()
{
    return(capture_client != NULL);
}

bool ViaAudioCapture::Open()
{
    if(!com_initialized)
    {
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

        com_initialized = SUCCEEDED(hr);
    }

    IMMDeviceEnumerator*    enumerator  = NULL;
    IMMDevice*              device      = NULL;
    WAVEFORMATEX*           format      = NULL;
    bool                    ok          = false;

    if(FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator))
    || FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))
    || FAILED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&audio_client))
    || FAILED(audio_client->GetMixFormat(&format)))
    {
        goto done;
    }

    channels         = format->nChannels;
    bytes_per_sample = format->wBitsPerSample / 8;
    is_float         = (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
                    || (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE && ((WAVEFORMATEXTENSIBLE*)format)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);

    if(channels == 0 || !((is_float && bytes_per_sample == 4) || (!is_float && bytes_per_sample == 2)))
    {
        VIA_LOG(LL_WARNING, "unsupported audio mix format (%u bit, %u channels)", format->wBitsPerSample, format->nChannels);
        goto done;
    }

    lowpass_coeff = 1.0f - expf(-2.0f * VIA_PI * AUDIO_BASS_CUTOFF_HZ / (float)format->nSamplesPerSec);

    if(FAILED(audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, AUDIO_BUFFER_DURATION_100NS, 0, format, NULL))
    || FAILED(audio_client->GetService(__uuidof(IAudioCaptureClient), (void**)&capture_client))
    || FAILED(audio_client->Start()))
    {
        goto done;
    }

    ok = true;

done:
    if(format != NULL)
    {
        CoTaskMemFree(format);
    }

    release(&device);
    release(&enumerator);

    if(!ok)
    {
        release(&capture_client);
        release(&audio_client);
    }

    return(ok);
}

void ViaAudioCapture::Close()
{
    if(audio_client != NULL)
    {
        audio_client->Stop();
    }

    release(&capture_client);
    release(&audio_client);

    if(com_initialized)
    {
        CoUninitialize();
        com_initialized = false;
    }
}

bool ViaAudioCapture::Process(float elapsed_s, float* bass, float* treble)
{
    double          bass_energy     = 0.0;
    double          treble_energy   = 0.0;
    unsigned int    frame_total     = 0;
    UINT32          packet_frames   = 0;

    if(capture_client == NULL)
    {
        return(false);
    }

    while(true)
    {
        HRESULT hr = capture_client->GetNextPacketSize(&packet_frames);

        if(FAILED(hr))
        {
            Close();
            return(false);
        }

        if(packet_frames == 0)
        {
            break;
        }

        BYTE*   data;
        UINT32  frames;
        DWORD   flags;

        if(FAILED(capture_client->GetBuffer(&data, &frames, &flags, NULL, NULL)))
        {
            Close();
            return(false);
        }

        bool silent = (flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0;

        for(UINT32 frame_idx = 0; frame_idx < frames; frame_idx++)
        {
            float mono = 0.0f;

            if(!silent)
            {
                for(unsigned int channel_idx = 0; channel_idx < channels; channel_idx++)
                {
                    unsigned int sample_idx = frame_idx * channels + channel_idx;

                    mono += is_float ? ((float*)data)[sample_idx] : (((short*)data)[sample_idx] / 32768.0f);
                }

                mono /= channels;
            }

            lowpass_state  += lowpass_coeff * (mono - lowpass_state);

            float high      = mono - lowpass_state;

            bass_energy    += lowpass_state * lowpass_state;
            treble_energy  += high * high;
        }

        frame_total += frames;
        capture_client->ReleaseBuffer(frames);
    }

    /*-----------------------------------------------------*\
    | Fast attack, exponential release, slow auto gain      |
    \*-----------------------------------------------------*/
    float bass_rms      = frame_total ? sqrtf((float)(bass_energy   / frame_total)) : 0.0f;
    float treble_rms    = frame_total ? sqrtf((float)(treble_energy / frame_total)) : 0.0f;
    float release_env   = expf(-AUDIO_ENVELOPE_RELEASE_PER_S * elapsed_s);
    float release_peak  = expf(-AUDIO_PEAK_RELEASE_PER_S     * elapsed_s);

    bass_envelope       = fmaxf(bass_rms,   bass_envelope   * release_env);
    treble_envelope     = fmaxf(treble_rms, treble_envelope * release_env);
    peak                = fmaxf(fmaxf(bass_envelope, treble_envelope), fmaxf(peak * release_peak, AUDIO_PEAK_FLOOR));

    *bass               = bass_envelope   / peak;
    *treble             = treble_envelope / peak;

    return(true);
}
