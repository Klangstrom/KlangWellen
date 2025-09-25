#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "KlangWellen.h"
#include "SAM.h"

using namespace klangwellen;

int8_t buffer[48000];
SAM    sam_left(buffer, 48000);
SAM    sam_right(48000);
Beat   beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("------");
    console_println("06.SAM");
    console_println("------");

    sam_right.set_speed(120);
    sam_right.set_throat(100);

    beat_timer.init();
    beat_timer.set_bpm(60);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    sam_left.speak(to_string(beat_counter).c_str());
    if (beat_counter % 2 == 0) {
        sam_right.speak_to_buffer(to_string(beat_counter).c_str());
    } else {
        sam_right.speak_from_buffer();
    }
}

void audioblock(const AudioBlock* audio_block) {
    sam_right.process(audio_block->output[0], audio_block->block_size);
    KlangWellen::mult(audio_block->output[0], 0.25f, audio_block->block_size);
    if (audio_block->output_channels == 2) {
        sam_left.process(audio_block->output[1], audio_block->block_size);
        KlangWellen::mult(audio_block->output[1], 0.25, audio_block->block_size);
    }
}
