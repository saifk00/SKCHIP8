#ifndef TONE_PLAYER_H_
#define TONE_PLAYER_H_

#include "SDL.h"
#include <iostream>

// plays a single tone at the given frequency and volume
class TonePlayer
{
public:
    TonePlayer(double freq, double vol) : frequency_(freq), volume_(vol), samples_played_(0)
    {
        initAudioDevice();
        if (device_ == 0)
        {
            throw std::runtime_error("Failed to initialize audio device");
        }
    }

    ~TonePlayer();

    void Play();
    void Pause();
    void GenerateSamples(void *stream, int len);

protected:
    void initAudioDevice();

protected:
    int device_;
    uint64_t samples_played_;
    double frequency_;
    double volume_;
};

#endif