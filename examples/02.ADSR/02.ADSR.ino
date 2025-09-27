#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "ADSR.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable wavetable(256, 48000);
ADSR      adsr(48000);
Beat      beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("-------");
    console_println("02.ADSR");
    console_println("-------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SINE);

    adsr.set_attack(0.25f);
    adsr.set_decay(0.125f);
    adsr.set_sustain(0.5f);
    adsr.set_release(0.5f);

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
        const float sample        = wavetable.process() * adsr.process();
        audio_block->output[0][i] = sample;
    }
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
