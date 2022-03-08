#include "TonePlayer.h"

#include <cmath>
#include <numeric>
#include <cstdint>

namespace
{
    static constexpr float SAMPLE_RATE = 44100.0f;
    void audio_callback(void *userdata, Uint8 *stream, int len)
    {
        auto tone_player = (TonePlayer *)userdata;
        tone_player->GenerateSamples(stream, len);
    }
}

void TonePlayer::initAudioDevice()
{
    SDL_AudioSpec have, want;
    samples_played_ = 0;
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_F32SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;
    want.userdata = this;

    device_ = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    SDL_Log("Audio device opened (%d)", device_);
    SDL_Log("Frequency: %d", have.freq);
    SDL_Log("Format: 0x%x", have.format);
    SDL_Log("Channels: %d", have.channels);
    SDL_Log("Samples: %d", have.samples);
}

TonePlayer::~TonePlayer()
{
    SDL_CloseAudioDevice(device_);
}

void TonePlayer::Play()
{
    SDL_PauseAudioDevice(device_, 0);
}

void TonePlayer::Pause()
{
    SDL_PauseAudioDevice(device_, 1);
}

void TonePlayer::GenerateSamples(void *stream, int len)
{
    float *fstream = (float *)(stream);

    for (int sid = 0; sid < (len / sizeof(float)); ++sid)
    {
        double time = (samples_played_ + sid) / SAMPLE_RATE;
        double x = 2.0 * M_PI * time * frequency_;
        fstream[sid] = volume_ * std::sin(x);
    }

    samples_played_ += (len / sizeof(float));
}
