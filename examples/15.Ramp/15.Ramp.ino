#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "FilterLowPassMoogLadder.h"
#include "Ramp.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable               wavetable(256, 48000);
FilterLowPassMoogLadder low_pass_filter(48000);
Ramp                    ramp_filter_resonance(48000);
Ramp                    ramp_filter_frequency(48000);
Ramp                    ramp_frequency(48000);
Beat                    beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("-------");
    console_println("15.Ramp");
    console_println("-------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SAWTOOTH);
    wavetable.set_frequency(27.5);
    low_pass_filter.set_resonance(0.3);
    low_pass_filter.set_frequency(1);

    ramp_filter_resonance.set_start(0.1);
    ramp_filter_resonance.set_end(0.85);
    ramp_filter_resonance.set_duration(0.5);

    ramp_filter_frequency.set(1, 2000, 1.0);

    ramp_frequency.set(27.5, 55, 2.0);

    beat_timer.init();
    beat_timer.set_bpm(120 * 2);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    if (beat_counter % 4 == 0) {
        ramp_filter_resonance.start();
        ramp_filter_frequency.start();
    }
    if (beat_counter % 8 == 0) {
        ramp_frequency.start();
    }
}

void audioblock(const AudioBlock* audio_block) {
    for (uint16_t i = 0; i < audio_block->block_size; i++) {
        wavetable.set_frequency(ramp_frequency.process());
        low_pass_filter.set_resonance(ramp_filter_resonance.process());
        low_pass_filter.set_frequency(ramp_filter_frequency.process());
        audio_block->output[0][i] = low_pass_filter.process(wavetable.process());
    }
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
