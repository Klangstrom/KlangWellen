#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Beat.h"

#include "FilterVowelFormant.h"
#include "Wavetable.h"

using namespace klangwellen;

Wavetable          wavetable(256, 48000);
FilterVowelFormant vowel_filter;
Beat               beat_timer;

void setup() {
    system_init();
    system_init_audiocodec();

    console_println("----------------");
    console_println("05.FormantFilter");
    console_println("----------------");

    wavetable.set_waveform(KlangWellen::WAVEFORM_SAWTOOTH);
    wavetable.set_frequency(35);
    wavetable.set_amplitude(0.125f);
    vowel_filter.set_vowel(FilterVowelFormant::VOWEL_U);

    beat_timer.init();
    beat_timer.set_bpm(120 * 10);
    beat_timer.start();
}

void loop() {}

void beat_event(const uint8_t beat_id, const uint16_t beat_counter) {
    static constexpr int mLerpDuration = 10;
    const uint8_t        mVowel        = (beat_counter / mLerpDuration) % (FilterVowelFormant::NUM_OF_VOWELS - 1);
    const uint8_t        mVowelOffset  = beat_counter / (mLerpDuration * 5);
    const uint8_t        mNextVowel    = (mVowel + mVowelOffset) % (FilterVowelFormant::NUM_OF_VOWELS - 1);
    const float          lerp          = (beat_counter % mLerpDuration) / (float) mLerpDuration;
    vowel_filter.lerp_vowel(mVowel, mNextVowel, lerp);
    wavetable.set_frequency(35 + ((beat_counter / 3) % 5) * 4);
    wavetable.set_amplitude(((beat_counter / 2) % 13) / 13.0 * 0.1 + 0.05);
}

// void audioblock(float** input_signal, float** output_signal) {
//     wavetable.process(output_signal[LEFT]);
//     vowel_filter.process(output_signal[LEFT]);
//     KlangWellen::copy(output_signal[LEFT], output_signal[RIGHT]);
// }

void audioblock(const AudioBlock* audio_block) {
    wavetable.process(audio_block->output[0], audio_block->block_size);
    vowel_filter.process(audio_block->output[0], audio_block->block_size);
    if (audio_block->output_channels == 2) {
        KlangWellen::copy(audio_block->output[0], audio_block->output[1], audio_block->block_size);
    }
}
