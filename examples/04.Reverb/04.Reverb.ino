#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "ADSR.h"
#include "Reverb.h"
#include "Wavetable.h"
#include "AudioSignal.h"

using namespace klangwellen;

Wavetable wavetable(256, 48000);
ADSR      adsr(48000);
Reverb    reverb;
Beat      beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("---------");
    console_println("04.Reverb");
    console_println("---------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SINE);
    reverb.set_roomsize(0.9);

    beat_timer.init();
    beat_timer.set_bpm(60);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    if (beat_counter % 2 == 0) {
        adsr.start();
    } else {
        adsr.stop();
    }
}

void audioblock(const AudioBlock* audio_block) {
    for (int i = 0; i < audio_block->block_size; ++i) {
        float sample = wavetable.process();
        sample *= adsr.process();
        /* mono */
        if (audio_block->output_channels == 1) {
            audio_block->output[0][i] = reverb.process(sample);
        }
        /* stereo */
        if (audio_block->output_channels == 2) {
            float left  = sample;
            float right = sample;
            reverb.process(left, right);
            audio_block->output[0][i] = left;
            audio_block->output[1][i] = right;
            // "same stereo signal using 'AudioSignal'"
            // AudioSignal stereo_signal(sample);
            // reverb.process(stereo_signal);
            // audio_block->output[0][i] = stereo_signal.left;
            // audio_block->output[1][i] = stereo_signal.right;
        }
    }
}