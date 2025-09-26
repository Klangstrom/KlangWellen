#include "Arduino.h"
#include "System.h"
#include "Console.h"

#include "AudioDevice.h"
#include "Beat.h"

#include "Waveshaper.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable  wavetable(256, 48000);
Waveshaper waveshaper;
uint8_t    waveshaper_type = Waveshaper::SIN;
Beat       beat_timer;

void setup() {
    system_init();
    console_init();
    system_init_audiocodec();

    console_println("-------------");
    console_println("17.Waveshaper");
    console_println("-------------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SINE);
    wavetable.set_amplitude(4.0);
    wavetable.set_frequency(55);

    waveshaper.set_type(Waveshaper::ATAN);
    waveshaper.set_output_gain(0.2);

    beat_timer.init();
    beat_timer.set_bpm(120 * 4);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    wavetable.set_frequency(55 + 55 * ((beat_counter / 4) % 2));
    if (beat_counter % 8 == 0) {
        waveshaper_type++;
        waveshaper_type %= Waveshaper::NUM_WAVESHAPER_TYPES;
        waveshaper.set_type(waveshaper_type);
    }
}

void audioblock(const AudioBlock* audio_block) {
    wavetable.process(audio_block->output[0], audio_block->block_size);
    waveshaper.process(audio_block->output[0], audio_block->block_size);
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
