#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Display.h"
#include "Draw.h"
#include "KLSTFont_5x8.h"

#include "KlangWellen.h"
#include "Wavetable.h"
#include "ADSR.h"
#include "Beat.h"
#include "Scale.h"
#include "Note.h"
#include "Distortion.h"

#include "Encoder.h"

using namespace klangwellen;

Wavetable wavetable(1024, 48000);
ADSR adsr(48000);
Distortion distortion;

Beat beat_timer;
Encoder* encoder_LEFT;
Encoder* encoder_RIGHT;

int16_t x = display_get_width() / 2;
int16_t y = display_get_height() / 2;

const int NUM_OFFSET = 4;
int offset_id = 0;
int offset_value[NUM_OFFSET] = { 0, -5, 3, -2 };
float clip = 0.2f;
float pre_amplification = 1.0f;
bool toggle_mode = false;
float master_volume = 1.0f;
int face_counter = 0;

void setup() {
  system_init();
  console_init();
  system_init_audiocodec();

  display_init(true);
  display_enable_automatic_update(true);
  display_set_backlight(0.9f);
  draw_set_text(&Font_5x8);

  encoder_LEFT = encoder_create(ENCODER_LEFT);
  encoder_RIGHT = encoder_create(ENCODER_RIGHT);

  wavetable.set_waveform(KlangWellen::WAVEFORM_SAWTOOTH, 5);
  wavetable.set_frequency(55);
  wavetable.set_amplitude(0.4);

  adsr.set_attack(0.001);
  adsr.set_decay(0.2);
  adsr.set_sustain(0.1);
  adsr.set_release(0.1);

  distortion.set_type(DistortionTypes::DISTORTION_FOLDBACK);
  distortion.set_clip(0.5f);
  distortion.set_amplification(2.0f);

  beat_timer.init();
  beat_timer.set_bpm(80 * 4);
  beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
  face_counter = beat_counter;

  const int counter = beat_counter % 8;
  const int midi_note = Scale::note(Scale::MAJOR_PENTATONIC, Note::C_2, counter + offset_value[offset_id]);
  const float freq = KlangWellen::midi_note_to_frequency(midi_note);
  wavetable.set_frequency(freq);
  adsr.start();

  if (beat_counter % 8 == 0) {
    offset_id++;
    offset_id %= NUM_OFFSET;
  }
}

void audioblock(const AudioBlock* audio_block) {
  for (int i = 0; i < audio_block->block_size; ++i) {
    float sample = wavetable.process();
    sample *= adsr.process();
    sample = distortion.process(sample);
    sample *= master_volume;
    audio_block->output[0][i] = sample;
  }
  if (audio_block->output_channels == 2) {
    KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
  }
}

void encoder_event(const Encoder* encoder, const uint8_t event) {
  if (event == ENCODER_EVENT_BUTTON) {
    if (encoder->device_type == ENCODER_LEFT) {
      toggle_mode = !toggle_mode;
    }
  }
  if (event == ENCODER_EVENT_ROTATION) {
    if (encoder->device_type == ENCODER_LEFT) {
      int clamped_rotation = encoder->rotation;
      if (toggle_mode) {
        clamped_rotation = clamped_rotation > 100 ? 100 : clamped_rotation;
        clamped_rotation = clamped_rotation < -100 ? -100 : clamped_rotation;
        master_volume = 1.0f + (float)clamped_rotation / 100.0f;
      } else {
        clamped_rotation = clamped_rotation > 100 ? 100 : clamped_rotation;
        clamped_rotation = clamped_rotation < 0 ? 0 : clamped_rotation;
        pre_amplification = 10 * (float)clamped_rotation / 100.0f;
        distortion.set_amplification(pre_amplification);
      }
    }
    if (encoder->device_type == ENCODER_RIGHT) {
      int clamped_rotation = encoder->rotation;
      clamped_rotation = clamped_rotation > 100 ? 100 : clamped_rotation;
      clamped_rotation = clamped_rotation < 0 ? 0 : clamped_rotation;
      clip = (float)clamped_rotation / 100.0f;
      distortion.set_clip(clip);
    }
  }
}

void display_update_event() {
  draw_clear(color_from_gray(0.0f));
  draw_fill(color_from_gray(1.0f));
  draw_set_background_color(color_from_gray_alpha(0.0f, 0.0f));
  bool sing = face_counter % 8 < 4;
  draw_text(x - 40, y - 32, sing ? ":)" : ":o", 8);
  draw_text(10, y, std::to_string((int)(pre_amplification * 100)).c_str());
  draw_text(display_get_width() - 30, y, std::to_string((int)(clip * 100)).c_str());
}
