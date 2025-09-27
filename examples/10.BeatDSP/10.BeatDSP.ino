#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "ADSR.h"
#include "BeatDSP.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable wavetable(256, 48000);
Wavetable wavetable_dsp(256, 48000);
ADSR      adsr(48000);
ADSR      adsr_dsp(48000);
BeatDSP   beat_dsp(48000);
void      beat_dsp_callback(uint32_t beat_counter);
Beat      beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("----------");
    console_println("10.BeatDSP");
    console_println("----------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SINE);
    wavetable.set_frequency(220.0);
    wavetable_dsp.set_waveform(KlangWellen::WAVEFORM_TRIANGLE);
    wavetable_dsp.set_frequency(110.0);

    beat_dsp.set_callback(beat_dsp_callback);
    beat_dsp.set_bpm(120 * 2);

    beat_timer.init();
    beat_timer.set_bpm(120 * 2);
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

void beat_dsp_callback(const uint32_t beat_counter) {
    if (beat_counter % 2 == 0) {
        adsr_dsp.start();
    } else {
        adsr_dsp.stop();
    }
}

void audioblock(const AudioBlock* audio_block) {
    for (uint16_t i = 0; i < audio_block->block_size; i++) {
        beat_dsp.process();
        audio_block->output[0][i] = wavetable.process() * adsr.process();
        if (audio_block->output_channels == 2) {
            audio_block->output[1][i] = wavetable_dsp.process() * adsr_dsp.process();
        }
    }
}
