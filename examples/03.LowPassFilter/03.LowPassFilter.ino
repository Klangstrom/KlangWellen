#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "FilterLowPassMoogLadder.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable               wavetable(256, 48000);
FilterLowPassMoogLadder low_pass_filter(48000);
Beat                    beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("----------------");
    console_println("03.LowPassFilter");
    console_println("----------------");

    wavetable.set_waveform(Klangwellen::WAVEFORM_SQUARE);
    wavetable.set_frequency(55);
    low_pass_filter.set_resonance(0.85f);

    beat_timer.init();
    beat_timer.set_bpm(120 * 4);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    low_pass_filter.set_frequency(1.0f + 100.0f * ((beat_counter) % 32));
    wavetable.set_frequency(55 + 55 * ((beat_counter / 4) % 2));
}

void audioblock(const AudioBlock* audio_block) {
    wavetable.process(audio_block->output[0], audio_block->block_size);
    low_pass_filter.process(audio_block->output[0], audio_block->block_size);

    // for (int i = 0; i < audio_block->block_size; ++i) {
    //     const float sample        = low_pass_filter.process(wavetable.process());
    //     audio_block->output[0][i] = sample;
    // }

    if (audio_block->output_channels == 2) {
        Klangwellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
