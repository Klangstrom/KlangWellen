#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"

#include "KlangWellen.h"
#include "Wavetable.h"
#include "ADSR.h"
#include "Beat.h"
#include "Scale.h"
#include "Note.h"

using namespace klangwellen;

Wavetable wavetable(1024, 48000);
ADSR      adsr(48000);
Beat      beat_timer;

void setup() {
    system_init();
    console_init();
    system_init_audiocodec();

    console_println("-----------------");
    console_println("19.NotesAndScales");
    console_println("-----------------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SAWTOOTH, 4);
    wavetable.set_frequency(55);
    wavetable.set_amplitude(0.3);

    adsr.set_attack(0.03);
    adsr.set_decay(0.3);
    adsr.set_sustain(0.0);
    adsr.set_release(0.0);

    beat_timer.init();
    beat_timer.set_bpm(120 * 4);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    const int   counter   = beat_counter % 8;
    const int   midi_note = Scale::note(Scale::MINOR_PENTATONIC, Note::C_2, counter);
    const float freq      = KlangWellen::midi_note_to_frequency(midi_note);
    wavetable.set_frequency(freq);
    adsr.start();
}

void audioblock(const AudioBlock* audio_block) {
    for (int i = 0; i < audio_block->block_size; ++i) {
        float sample = wavetable.process();
        sample *= adsr.process();
        audio_block->output[0][i] = sample;
    }
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
