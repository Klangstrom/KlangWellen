#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "FilterLowPassMoogLadder.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable               lfo(256, 48000);
Wavetable               wavetable(256, 48000);
FilterLowPassMoogLadder low_pass_filter(48000);

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("------");
    console_println("09.LFO");
    console_println("------");

    lfo.set_waveform(KlangWellen::WAVEFORM_SINE);
    lfo.set_oscillation_range(55, 1000);
    lfo.set_frequency(0.5);

    wavetable.set_waveform(KlangWellen::WAVEFORM_SQUARE);
    wavetable.set_frequency(27.5);

    low_pass_filter.set_resonance(0.85f);
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    wavetable.set_frequency(27.5 + 27.5 * ((beat_counter / 8) % 2));
}

void audioblock(const AudioBlock* audio_block) {
    for (int i = 0; i < audio_block->block_size; ++i) {
        low_pass_filter.set_frequency(lfo.process());
        audio_block->output[0][i] = low_pass_filter.process(wavetable.process());
    }
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
