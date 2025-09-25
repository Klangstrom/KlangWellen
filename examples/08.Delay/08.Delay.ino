// @TODO Delay is broken, fix it!

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "ADSR.h"
#include "Delay.h"
#include "KlangWellen.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable wavetable(256, 48000);
ADSR      adsr(48000);
Delay     m_delay(48000);
Beat      beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("--------");
    console_println("08.Delay");
    console_println("--------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_TRIANGLE);
    wavetable.set_amplitude(0.4);

    m_delay.set_echo_length(0.5);
    m_delay.set_decay_rate(0.1);
    m_delay.set_wet(0.5f);

    beat_timer.init();
    beat_timer.set_bpm(120 * 2);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    if (beat_counter % 2 == 0) {
        wavetable.set_frequency((55.0 * (1 + (beat_counter / 2) % 4)));
        adsr.start();
    } else {
        adsr.stop();
    }
    if (beat_counter % 8 == 0) {
        m_delay.set_echo_length(0.05f + 0.045f * ((beat_counter / 8) % 10));
    }
}

void audioblock(const AudioBlock* audio_block) {
    wavetable.process(audio_block->output[0], audio_block->block_size);
    adsr.process(audio_block->output[0], audio_block->block_size);
    m_delay.process(audio_block->output[0], audio_block->block_size);

    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
