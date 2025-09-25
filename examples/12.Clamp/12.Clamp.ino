#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "ADSR.h"
#include "Clamp.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable wavetable(256, 48000);
ADSR      adsr(48000);
Clamp     clamp;
Beat      beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("--------");
    console_println("12.Clamp");
    console_println("--------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SINE);
    clamp.set_min(-0.1);
    clamp.set_max(0.1);

    beat_timer.init();
    beat_timer.set_bpm(120);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    if (beat_counter % 2) {
        adsr.start();
    } else {
        adsr.stop();
    }
}

void audioblock(const AudioBlock* audio_block) {
    wavetable.process(audio_block->output[0], audio_block->block_size);
    adsr.process(audio_block->output[0], audio_block->block_size);
    clamp.process(audio_block->output[0], audio_block->block_size);
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
