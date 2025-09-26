#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "Envelope.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable wavetable(256, 48000);
Envelope  fFrequencyEnvelope(48000);
Envelope  fAmplitudeEnvelope(48000);
Beat      beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("-----------");
    console_println("16.Envelope");
    console_println("-----------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SAWTOOTH);
    wavetable.set_frequency(55.0);
    wavetable.set_amplitude(0.0);

    fFrequencyEnvelope.add_stage(55.0, 0.5);
    fFrequencyEnvelope.add_stage(110.0);

    fAmplitudeEnvelope.add_stage(0.0, 0.25);
    fAmplitudeEnvelope.add_stage(0.3, 0.125);
    fAmplitudeEnvelope.add_stage(0.0, 0.125);
    fAmplitudeEnvelope.add_stage(0.6, 0.1);
    fAmplitudeEnvelope.add_stage(0.0);
    fAmplitudeEnvelope.enable_loop(true);
    fAmplitudeEnvelope.start();

    beat_timer.init();
    beat_timer.set_bpm(120 * 2);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    if (beat_counter % 4 == 0) {
        fFrequencyEnvelope.start();
    }
}

void audioblock(const AudioBlock* audio_block) {
    for (uint16_t i = 0; i < audio_block->block_size; i++) {
        wavetable.set_frequency(fFrequencyEnvelope.process());
        wavetable.set_amplitude(fAmplitudeEnvelope.process());
        audio_block->output[0][i] = wavetable.process();
    }
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
