#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "SDCard.h"
#include "Beat.h"

#include "WAV.h"
#include "Sampler.h"

using namespace klangwellen;

Sampler sampler(48000);
Beat    beat_timer;

void setup() {
    system_init();
    console_init();
    system_init_audiocodec();
    sdcard_init();

    console_println("-------------");
    console_println("18.LoadSample");
    console_println("-------------");

    while (!sdcard_detected()) {
        console_println("SD card not detected");
        delay(1000);
    }

    sdcard_status();
    sdcard_mount();

    const std::string filename = "LINSE.WAV"; // NOTE make sure this file exists on the SD card
    wav_load_header(filename);
    const int sample_buffer_size = wav_num_sample_frames();
    float*    sample_buffer      = system_external_memory_allocate_float_array(sample_buffer_size);
    wav_load_sample_frames(sample_buffer, WAV_ALL_SAMPLES);

    sampler.set_buffer(sample_buffer, sample_buffer_size);
    sampler.play();

    beat_timer.init();
    beat_timer.set_bpm(120 * 4);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    if (beat_counter % 16 == 0) {
        sampler.set_speed(beat_counter / 16 % 4 * 0.25 + 0.25f);
        sampler.rewind();
        sampler.play();
    }
}

void audioblock(const AudioBlock* audio_block) {
    for (int i = 0; i < audio_block->block_size; ++i) {
        audio_block->output[0][i] = sampler.process();
    }
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
