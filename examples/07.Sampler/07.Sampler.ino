#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "Sampler.h"

using namespace klangwellen;

Sampler sampler(24000, 48000);
Beat    beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("----------");
    console_println("07.Sampler");
    console_println("----------");

    for (int32_t i = 0; i < sampler.get_buffer_length(); i++) {
        float ratio             = 1.0 - (float) i / sampler.get_buffer_length();
        sampler.get_buffer()[i] = KlangWellen::random() * 0.2f * ratio;
    }

    beat_timer.init();
    beat_timer.set_bpm(120 * 4);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    if (beat_counter % 4 == 0) {
        sampler.rewind();
        sampler.play();
        console_println("beat: %i", beat_counter);
    } else {
        sampler.stop();
    }
}

void audioblock(const AudioBlock* audio_block) {
    for (uint16_t i = 0; i < audio_block->block_size; i++) {
        audio_block->output[0][i] = sampler.process();
    }
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
