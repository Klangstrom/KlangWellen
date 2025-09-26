#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "KlangWellen.h"
#include "SAM.h"
#include "Vocoder.h"
#include "Wavetable.h"

using namespace klangwellen;

SAM       sam(48000);
Wavetable wavetable(256, 48000);
Vocoder   vocoder(48000, 13, 3); // NOTE this still works on KLST_SHEEP but only with `fastest` compiler option
float     modulator_buffer[128];
float     carrier_buffer[128];
Beat      beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("----------");
    console_println("13.Vocoder");
    console_println("----------");

    sam.speak("hello world");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SAWTOOTH);
    wavetable.set_frequency(55);
    wavetable.set_amplitude(1.0);

    vocoder.set_reaction_time(0.03);
    vocoder.set_formant_shift(1.0);
    vocoder.set_volume(4);

    beat_timer.init();
    beat_timer.set_bpm(80);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    wavetable.set_frequency(27.5 * KlangWellen::pow(2, beat_counter % 4));
    sam.speak_from_buffer();
}

void audioblock(const AudioBlock* audio_block) {
    sam.process(modulator_buffer, audio_block->block_size);
    wavetable.process(carrier_buffer, audio_block->block_size);
    vocoder.process(carrier_buffer, modulator_buffer, audio_block->output[0], audio_block->block_size);
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
