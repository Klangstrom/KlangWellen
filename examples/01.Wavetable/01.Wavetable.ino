#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"

#include "KlangWellen.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable wavetable(256, 48000);

void setup() {
    system_init();
    console_init();
    system_init_audiocodec();

    console_println("------------");
    console_println("01.Wavetable");
    console_println("------------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SAWTOOTH);
    wavetable.set_frequency(55);
    wavetable.set_amplitude(0.5);
}

void loop() {}

void audioblock(const AudioBlock* audio_block) {
    for (int i = 0; i < audio_block->block_size; ++i) {
        const float sample        = wavetable.process();
        audio_block->output[0][i] = sample;
    }
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}