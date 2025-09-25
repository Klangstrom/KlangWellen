#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "ADSR.h"
#include "Trigger.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable wavetable(256, 48000);
Wavetable wavetable_dsp(256, 48000);
Wavetable lfo(256, 48000);
ADSR      adsr(48000);
ADSR      adsr_dsp(48000);
Trigger   trigger;
void      trigger_callback(uint8_t event);
Beat      beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("----------");
    console_println("11.Trigger");
    console_println("----------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SINE);
    wavetable.set_frequency(220.0);
    wavetable_dsp.set_waveform(KlangWellen::WAVEFORM_TRIANGLE);
    wavetable_dsp.set_frequency(110.0);

    static constexpr uint8_t pulses_per_beat = 4;

    lfo.set_waveform(KlangWellen::WAVEFORM_SINE);
    lfo.set_frequency(pulses_per_beat);

    trigger.trigger_rising_edge(true);
    trigger.trigger_falling_edge(true);
    trigger.set_callback(trigger_callback);

    beat_timer.init();
    beat_timer.set_bpm(120 * pulses_per_beat);
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

void trigger_callback(uint8_t event) {
    if (event == Trigger::EVENT_RISING_EDGE) {
        adsr_dsp.start();
    } else {
        adsr_dsp.stop();
    }
}

void audioblock(const AudioBlock* audio_block) {
    for (int i = 0; i < audio_block->block_size; ++i) {
        trigger.process(lfo.process());
        audio_block->output[0][i] = wavetable.process() * adsr.process();
        if (audio_block->output_channels == 2) {
            audio_block->output[1][i] = wavetable_dsp.process() * adsr_dsp.process();
        }
    }
}
