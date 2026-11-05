/*---------------------------------------------------------*\
| ViaAudioCapture.h                                         |
|                                                           |
|   WASAPI loopback capture of the default output device,   |
|   reduced to two normalized levels per call: bass (below  |
|   ~150 Hz) and treble (the rest). Must be opened, used    |
|   and closed on the same thread (COM).                    |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

struct IAudioClient;
struct IAudioCaptureClient;

class ViaAudioCapture
{
public:
    ViaAudioCapture();
    ~ViaAudioCapture();

    bool    Open();
    void    Close();
    bool    IsOpen();

    /*-----------------------------------------------------*\
    | Consumes everything captured since the last call.    |
    | Levels are 0..1 relative to the recent peak. Returns  |
    | false if the device went away (reopen later).         |
    \*-----------------------------------------------------*/
    bool    Process(float elapsed_s, float* bass, float* treble);

private:
    IAudioClient*           audio_client;
    IAudioCaptureClient*    capture_client;
    bool                    com_initialized;

    unsigned int            channels;
    bool                    is_float;
    unsigned int            bytes_per_sample;
    float                   lowpass_coeff;

    float                   lowpass_state;
    float                   bass_envelope;
    float                   treble_envelope;
    float                   peak;
};
