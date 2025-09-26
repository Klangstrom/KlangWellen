/*
 * KlangWellen
 *
 * This file is part of the *KlangWellen* library (https://github.com/dennisppaul/klangwellen).
 * Copyright (c) 2025 Dennis P Paul
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * PROCESSOR INTERFACE
 *
 * - [x] float process()
 * - [ ] float process(float)
 * - [ ] void process(AudioSignal&)
 * - [x] void process(float*, uint32_t) *overwrite*
 * - [ ] void process(float*, float*, uint32_t)
 */

/*
 * TODO
 * - LINE 325: check for memory leak and change recording mechanism in a way that the maximum length can be specified beforehand as well as a pre-allocated buffer
 * - LINE 153: "huuui, this is not nice and might cause some trouble somewhere"
 * - LINE 220: "evaluate direction?"
 */

#pragma once

#include <vector>

#include "KlangWellen.h"

namespace klangwellen {
    class SamplerListener {
    public:
        virtual ~SamplerListener() = default;
        virtual void is_done()     = 0;
    };

    /**
     * plays back an array of samples at different speeds.
     */
    template<class BUFFER_TYPE>
    class SamplerT {
    public:
        static constexpr int8_t NO_LOOP_POINT = -1;

        explicit SamplerT(uint32_t sample_rate) : SamplerT(nullptr, 0, sample_rate) {}

        explicit SamplerT(uint32_t sample_rate, int32_t buffer_length) : SamplerT(new BUFFER_TYPE[buffer_length], buffer_length, sample_rate) {
            _allocated_buffer = true;
        }

        SamplerT(BUFFER_TYPE*   buffer,
                 const int32_t  buffer_length,
                 const uint32_t sample_rate) : _sample_rate(sample_rate),
                                               _direction_forward(true),
                                               _in_point(0),
                                               _out_point(0),
                                               _speed(0) {
            set_buffer(buffer, buffer_length);
            _buffer_index       = 0;
            _interpolate_samples = false;
            _edge_fade_padding    = 0;
            _is_playing          = false;
            set_in(0);
            set_out(_buffer_length - 1);
            _frequency_scale = 1.0f;
            set_speed(1.0f);
            set_amplitude(1.0f);
            _is_recording      = false;
            _allocated_buffer = false;
        }

        ~SamplerT() {
            if (_allocated_buffer) {
                delete[] _buffer;
            }
        }

        void add_listener(SamplerListener* sampler_listener) {
            _sampler_listeners.push_back(sampler_listener);
        }

        bool remove_listener(const SamplerListener* sampler_listener) {
            for (auto it = _sampler_listeners.begin(); it != _sampler_listeners.end(); ++it) {
                if (*it == sampler_listener) {
                    _sampler_listeners.erase(it);
                    return true;
                }
            }
            return false;
        }

        int32_t get_in() const {
            return _in_point;
        }

        void set_in(int32_t in_point) {
            if (in_point > _out_point) {
                in_point = _out_point;
            }
            _in_point = in_point;
        }

        int32_t get_out() const {
            return _out_point;
        }

        void set_out(const int32_t out_point) {
            _out_point = out_point > last_index() ? last_index() : (out_point < _in_point ? _in_point : out_point);
        }

        float get_speed() const {
            return _speed;
        }

        void set_speed(const float speed) {
            _speed             = speed;
            _direction_forward = speed > 0;
            set_frequency(KlangWellen::abs(speed) * _sample_rate / _buffer_length); /* aka `step_size = speed` */
        }

        void set_frequency(const float frequency) {
            _frequency = frequency;
            _step_size  = _frequency / _frequency_scale * (static_cast<float>(_buffer_length) / _sample_rate);
        }

        float get_frequency() const {
            return _frequency;
        }

        void set_amplitude(const float amplitude) {
            _amplitude = amplitude;
        }

        float get_amplitude() const {
            return _amplitude;
        }

        BUFFER_TYPE* get_buffer() {
            return _buffer;
        }

        int32_t get_buffer_length() const {
            return _buffer_length;
        }

        void set_buffer(BUFFER_TYPE* buffer, const int32_t buffer_length) {
            if (_buffer != nullptr && _allocated_buffer) {
                delete[] _buffer;
            }
            _allocated_buffer = false; // TODO huuui, this is not nice and might cause some trouble somewhere
            _buffer           = buffer;
            _buffer_length    = buffer_length;
            rewind();
            set_speed(_speed);
            set_in(0);
            set_out(_buffer_length - 1);
            _loop_in  = NO_LOOP_POINT;
            _loop_out = NO_LOOP_POINT;
        }

        void interpolate_samples(bool const interpolate_samples) {
            _interpolate_samples = interpolate_samples;
        }

        bool interpolate_samples() const {
            return _interpolate_samples;
        }

        int32_t get_position() const {
            return static_cast<int32_t>(_buffer_index);
        }

        float get_position_normalized() const {
            return _buffer_length > 0 ? _buffer_index / _buffer_length : 0.0f;
        }

        float get_position_fractional_part() const {
            return _buffer_index - get_position();
        }

        bool is_playing() const {
            return _is_playing;
        }

        float process() {
            if (_buffer_length == 0) {
                notifyListeners(); // "buffer is empty"
                return 0.0f;
            }

            if (!_is_playing) {
                notifyListeners(); // "not playing"
                return 0.0f;
            }

            validateInOutPoints();

            _buffer_index += _direction_forward ? _step_size : -_step_size;
            const int32_t mRoundedIndex = static_cast<int32_t>(_buffer_index);

            const float   mFrac         = _buffer_index - mRoundedIndex;
            const int32_t mCurrentIndex = wrapIndex(mRoundedIndex);
            _buffer_index               = mCurrentIndex + mFrac;

            if (_direction_forward ? (mCurrentIndex >= _out_point) : (mCurrentIndex <= _in_point)) {
                notifyListeners(); // "reached end"
                return 0.0f;
            } else {
                _is_flagged_done = false;
            }

            float mSample = convert_sample(_buffer[mCurrentIndex]);

            /* interpolate */
            if (_interpolate_samples) {
                // TODO evaluate direction?
                const int32_t mNextIndex  = wrapIndex(mCurrentIndex + 1);
                const float   mNextSample = convert_sample(_buffer[mNextIndex]);
                mSample                   = mSample * (1.0f - mFrac) + mNextSample * mFrac;
                // mSample = interpolate_samples_linear(_buffer, _buffer_length, _buffer_index);
                // mSample = interpolate_samples_cubic(_buffer, _buffer_length, _buffer_index);
            }
            mSample *= _amplitude;

            /* fade edges */
            if (_edge_fade_padding > 0) {
                const int32_t mRelativeIndex = _buffer_length - mCurrentIndex;
                if (mCurrentIndex < _edge_fade_padding) {
                    const float mFadeInAmount = static_cast<float>(mCurrentIndex) / _edge_fade_padding;
                    mSample *= mFadeInAmount;
                } else if (mRelativeIndex < _edge_fade_padding) {
                    const float mFadeOutAmount = static_cast<float>(mRelativeIndex) / _edge_fade_padding;
                    mSample *= mFadeOutAmount;
                }
            }
            return mSample;
        }

        void process(float* signal_buffer, const uint32_t buffer_length) {
            for (uint16_t i = 0; i < buffer_length; i++) {
                signal_buffer[i] = process();
            }
        }

        int32_t get_edge_fading() const {
            return _edge_fade_padding;
        }

        void set_edge_fading(int32_t edge_fade_padding) {
            _edge_fade_padding = edge_fade_padding;
        }

        void rewind() {
            _buffer_index = _direction_forward ? _in_point : _out_point;
        }

        void forward() {
            _buffer_index = _direction_forward ? _out_point : _in_point;
        }

        bool is_looping() const {
            return _evaluate_loop;
        }

        void enable_loop(bool loop) {
            _evaluate_loop = loop;
        }

        void set_looping() {
            _evaluate_loop = true;
            _loop_in       = 0;
            _loop_out      = _buffer_length > 0 ? (_buffer_length - 1) : 0;
        }

        void play() {
            _is_playing = true;
            _recording.clear();
        }

        void stop() {
            _is_playing = false;
        }

        void start_recording() {
            _is_recording = true;
        }

        void resume_recording() {
            _is_recording = true;
        }

        void pause_recording() {
            _is_recording = false;
        }

        void delete_recording() {
            _recording.clear();
        }

        void record(float sample) {
            if (_is_recording) {
                _recording.push_back(sample);
            }
        }

        void record(const float* samples, int32_t num_samples) {
            if (_is_recording) {
                for (int32_t i = 0; i < num_samples; i++) {
                    const float sample = samples[i];
                    _recording.push_back(sample);
                }
            }
        }

        bool is_recording() const {
            return _is_recording;
        }

        int get_length_recording() {
            return _recording.size();
        }

        uint32_t end_recording() {
            _is_recording                = false;
            const int32_t mBufferLength = _recording.size();
            float*        mBuffer       = new float[mBufferLength];
            for (int32_t i = 0; i < mBufferLength; i++) {
                mBuffer[i] = _recording[i];
            }
            _recording.clear();
            if (_allocated_buffer) {
                delete[] _buffer;
            }
            set_buffer(mBuffer, mBufferLength);
            _allocated_buffer = true;
            return mBufferLength;
        }

        int32_t get_loop_in() const {
            return _loop_in;
        }

        void set_loop_in(const int32_t loop_in_point) {
            _loop_in = KlangWellen::clamp(loop_in_point, NO_LOOP_POINT, _buffer_length - 1);
        }

        float get_loop_in_normalized() const {
            if (_buffer_length < 2) {
                return 0.0f;
            }
            return static_cast<float>(_loop_in) / (_buffer_length - 1);
        }

        void set_loop_in_normalized(const float loop_in_point_normalized) {
            set_loop_in(static_cast<int32_t>(loop_in_point_normalized * _buffer_length - 1));
        }

        int32_t get_loop_out() const {
            return _loop_out;
        }

        void set_loop_out(const int32_t loop_out_point) {
            _loop_out = KlangWellen::clamp(loop_out_point, NO_LOOP_POINT, _buffer_length - 1);
        }

        float get_loop_out_normalized() const {
            if (_buffer_length < 2) {
                return 0.0f;
            }
            return static_cast<float>(_loop_out) / (_buffer_length - 1);
        }

        void set_loop_out_normalized(const float loop_out_point_normalized) {
            set_loop_out(static_cast<int32_t>(loop_out_point_normalized * _buffer_length - 1));
        }

        void note_on() {
            rewind();
            play();
            enable_loop(true);
        }

        void note_on(const uint8_t note, const uint8_t velocity) {
            _is_playing = true;
            set_frequency(KlangWellen::midi_note_to_frequency(note));
            set_amplitude(KlangWellen::clamp127(velocity) / 127.0f);
            note_on();
        }

        void note_off() {
            enable_loop(false);
        }

        /**
         * this function can be used to tune a loaded sample to a specific frequency. after the sampler has been tuned the
         * method <code>set_frequency(float)</code> can be used to play the sample at a desired frequency.
         *
         * @param tune_frequency the assumed frequency of the sampler buffer in Hz
         */
        void tune_frequency_to(const float tune_frequency) {
            _frequency_scale = tune_frequency;
        }

        void set_duration(const float seconds) {
            if (_buffer_length == 0 || seconds == 0.0f) {
                return;
            }
            const float mNormDurationSec = (static_cast<float>(_buffer_length) / static_cast<float>(_sample_rate));
            const float mSpeed           = mNormDurationSec / seconds;
            set_speed(mSpeed);
        }

        float get_duration() const {
            if (_buffer_length == 0 || _speed == 0.0f) {
                return 0;
            }
            const float mNormDurationSec = (static_cast<float>(_buffer_length) / static_cast<float>(_sample_rate));
            return mNormDurationSec / _speed;
        }

    private:
        std::vector<SamplerListener*> _sampler_listeners;
        std::vector<BUFFER_TYPE>      _recording;
        const uint32_t                _sample_rate;
        float                         _amplitude;
        BUFFER_TYPE*                  _buffer;
        int32_t                       _buffer_length;
        float                         _buffer_index;
        bool                          _direction_forward;
        int32_t                       _edge_fade_padding;
        bool                          _evaluate_loop;
        float                         _frequency;
        float                         _frequency_scale;
        int32_t                       _in_point;
        int32_t                       _out_point;
        int32_t                       _loop_in;
        int32_t                       _loop_out;
        bool                          _interpolate_samples;
        bool                          _is_playing;
        float                         _speed;
        float                         _step_size;
        bool                          _is_flagged_done;
        bool                          _is_recording;
        bool                          _allocated_buffer;

        int32_t last_index() const {
            return _buffer_length - 1;
        }

        void notifyListeners() {
            if (!_is_flagged_done) {
                for (SamplerListener* l: _sampler_listeners) {
                    l->is_done();
                }
            }
            _is_flagged_done = true;
        }

        void validateInOutPoints() {
            if (_in_point < 0) {
                _in_point = 0;
            } else if (_in_point > _buffer_length - 1) {
                _in_point = _buffer_length - 1;
            }
            if (_out_point < 0) {
                _out_point = 0;
            } else if (_out_point > _buffer_length - 1) {
                _out_point = _buffer_length - 1;
            }
            if (_out_point < _in_point) {
                _out_point = _in_point;
            }
            if (_loop_in < _in_point) {
                _loop_in = _in_point;
            }
            if (_loop_out > _out_point) {
                _loop_out = _out_point;
            }
        }

        int32_t wrapIndex(int32_t i) const {
            /* check if in loop concept viable i.e loop in- and output points are set */
            if (_evaluate_loop) {
                if (_loop_in != NO_LOOP_POINT && _loop_out != NO_LOOP_POINT) {
                    if (_direction_forward) {
                        if (i > _loop_out) {
                            i = _loop_in;
                        }
                    } else {
                        if (i < _loop_in) {
                            i = _loop_out;
                        }
                    }
                }
            }

            /* check if within bounds */
            if (i > _out_point) {
                i = _out_point;
            } else if (i < _in_point) {
                i = _in_point;
            }
            return i;
        }

        static float convert_sample(const BUFFER_TYPE pRawSample) {
            return pRawSample;
        }
    };

    template<>
    inline float klangwellen::SamplerT<uint8_t>::convert_sample(const uint8_t pRawSample) {
        constexpr static float mScale = 1.0 / ((1 << 8) - 1);
        const float            mRange = pRawSample * mScale;
        return mRange * 2.0 - 1.0;
    }

    template<>
    inline float klangwellen::SamplerT<int8_t>::convert_sample(const int8_t pRawSample) {
        constexpr static float mScale  = 1.0 / ((1 << 8) - 1);
        const float            mOffset = pRawSample + (1 << 7);
        const float            mRange  = mOffset * mScale;
        return mRange * 2.0 - 1.0;
    }

    template<>
    inline float klangwellen::SamplerT<uint16_t>::convert_sample(const uint16_t pRawSample) {
        constexpr static float mScale = 1.0 / ((1 << 16) - 1);
        const float            mRange = pRawSample * mScale;
        return mRange * 2.0 - 1.0;
        // @note(below: less precise but faster)
        // const float s      = pRawSample;
        // static const float mScale = 1.0 / (1 << 15);
        // const float a      = s * mScale - 1.0;
        // return a;
    }

    template<>
    inline float klangwellen::SamplerT<int16_t>::convert_sample(const int16_t pRawSample) {
        constexpr static float mScale  = 1.0 / ((1 << 16) - 1);
        const float            mOffset = pRawSample + (1 << 15);
        const float            mRange  = mOffset * mScale;
        return mRange * 2.0 - 1.0;
    }

    using SamplerUI8  = SamplerT<uint8_t>;
    using SamplerI8   = SamplerT<int8_t>;
    using SamplerUI16 = SamplerT<uint16_t>;
    using SamplerI16  = SamplerT<int16_t>;
    using SamplerF32  = SamplerT<float>;
    using Sampler     = SamplerT<float>;
} // namespace klangwellen
