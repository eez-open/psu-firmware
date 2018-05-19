/**
* EEZ Middleware
* Copyright (C) 2015-present, Envox d.o.o.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "beeper.h"

#include <SDL.h>
#include <SDL_audio.h>

#include <queue>
#include <cmath>

#undef min

namespace eez {
namespace platform {
namespace simulator {

const int AMPLITUDE = 28000;
const int FREQUENCY = 44100;

struct BeepData {
    double freq;
    int samplesLeft;
    double v;
};

void audio_callback(void *userdata, Uint8 *_stream, int _length) {
    BeepData &beep_data = *((BeepData *)userdata);
    Sint16* stream = (Sint16*)_stream;
    int length = _length / 2;

    for (int i = 0; i < length; ++i) {
        if (beep_data.samplesLeft > 0) {
            stream[i] = (Sint16)(AMPLITUDE * std::sin(beep_data.v * 2 * M_PI / FREQUENCY));
            beep_data.v += beep_data.freq;
            --beep_data.samplesLeft;
        } else {
            stream[i] = 0;
        }
    }
}

void beep(double freq, int duration) {
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    BeepData beep_data;

    beep_data.freq = freq;
    beep_data.samplesLeft = duration * FREQUENCY / 1000;
    beep_data.v = 0;

    SDL_AudioSpec desiredSpec;

    SDL_memset(&desiredSpec, 0, sizeof(desiredSpec));

    desiredSpec.freq = FREQUENCY;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 2048;
    desiredSpec.callback = audio_callback;
    desiredSpec.userdata = &beep_data;

    SDL_AudioSpec obtainedSpec;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &obtainedSpec, 0);
    if (dev == 0) {
        printf("Failed to open audio: %s\n", SDL_GetError());
    } else {
        // start play audio
        SDL_PauseAudioDevice(dev, 0);

        // wait
        do {
            SDL_Delay(20);
        } while (beep_data.samplesLeft > 0);

        SDL_CloseAudioDevice(dev);
    }
}

}
}
} // namespace eez::platform::simulator
