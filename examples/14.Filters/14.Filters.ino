#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "Filter.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable wavetable(256, 48000);
Filter    filter(48000, true);
Beat      beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("----------");
    console_println("14.Filters");
    console_println("----------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SQUARE);
    wavetable.set_frequency(55);
    wavetable.set_amplitude(0.3);

    beat_timer.init();
    beat_timer.set_bpm(120 * 4);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    wavetable.set_frequency(55 + 55 * ((beat_counter / 4) % 2));
    if (beat_counter % 16 == 0) {
        const uint8_t type = (beat_counter / 8) % Filter::NUM_FILTER_TYPES;
        filter.set(type, 0.0, 1000, 2);
    }
}

void audioblock(const AudioBlock* audio_block) {
    wavetable.process(audio_block->output[0], audio_block->block_size);
    filter.process(audio_block->output[0], audio_block->block_size);
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
