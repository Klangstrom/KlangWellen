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
 * - [*] void process(float*, uint32_t)
 * - [ ] void process(float*, float*, uint32_t)
 */

#pragma once

#include <stdint.h>
#include <iostream>

namespace klangwellen {
    class StreamDataProvider {
    public:
        virtual ~StreamDataProvider() = default;

        virtual void fill_buffer(float* buffer, const uint32_t length) {
            std::fill_n(buffer, length, 0.0f);
        }
    };

    class Stream final {
    public:
        Stream(StreamDataProvider* stream_data_provider,
               const uint32_t      sample_rate,
               const uint32_t      stream_buffer_size,
               const uint8_t       stream_buffer_division      = 4,
               const uint8_t       stream_buffer_update_offset = 1)
            : _stream_data_provider(stream_data_provider),
              _buffer_length(stream_buffer_size),
              _buffer(new float[stream_buffer_size]),
              _buffer_division(stream_buffer_division),
              _buffer_segment_offset(stream_buffer_update_offset % stream_buffer_division),
              _sample_rate(sample_rate),
              _amplitude(1.0f),
              _step_size(1.0f),
              _interpolate_samples(true),
              _buffer_index(0.0f),
              _buffer_index_prev(0.0f),
              _complete_event(NO_EVENT) {
            _stream_data_provider->fill_buffer(_buffer, _buffer_length);
        }

        ~Stream() {
            delete[] _buffer;
        }

        float process() {
            _buffer_index += _step_size;
            const int32_t mRoundedIndex = static_cast<int32_t>(_buffer_index);
            const float   mFrac         = _buffer_index - mRoundedIndex;
            const int32_t mCurrentIndex = wrapIndex(mRoundedIndex);
            _buffer_index               = mCurrentIndex + mFrac;

            float mSample = convert_sample(_buffer[mCurrentIndex]);

            /* linear interpolation */
            if (_interpolate_samples) {
                const int32_t mNextIndex  = wrapIndex(mCurrentIndex + 1);
                const float   mNextSample = convert_sample(_buffer[mNextIndex]);
                const float   a           = mSample * (1.0f - mFrac);
                const float   b           = mNextSample * mFrac;
                mSample                   = a + b;
            }
            mSample *= _amplitude;

            /* load next block */
            int8_t mCompleteEvent = checkCompleteEvent(_buffer_division);
            if (mCompleteEvent > NO_EVENT) {
                _complete_event = mCompleteEvent;
                mCompleteEvent -= _buffer_segment_offset;
                mCompleteEvent += _buffer_division;
                mCompleteEvent %= _buffer_division;
                replace_segment(_buffer_division, mCompleteEvent);
            }
            _buffer_index_prev = _buffer_index;

            return mSample;
        }

        void replace_segment(const uint32_t number_of_segments, const uint32_t segment_index) const {
            if (segment_index >= number_of_segments) {
                std::cerr << "Segment index out of range" << std::endl;
                return;
            }

            const uint32_t lengthOfSegment = _buffer_length / number_of_segments;
            float*         startOfSegment  = _buffer + (segment_index * lengthOfSegment);

            _stream_data_provider->fill_buffer(startOfSegment, lengthOfSegment);
        }

        float* get_buffer() const {
            return _buffer;
        }

        int8_t get_sector() const {
            return _complete_event;
        }

        uint8_t num_sectors() const {
            return _buffer_division;
        }

        uint32_t get_buffer_length() const {
            return _buffer_length;
        }

        float get_current_buffer_position() const {
            return _buffer_index;
        }

        void process(float* signal_buffer, const uint32_t buffer_length) {
            for (uint16_t i = 0; i < buffer_length; i++) {
                signal_buffer[i] = process();
            }
        }

        void interpolate_samples(bool const interpolate_samples) {
            _interpolate_samples = interpolate_samples;
        }

        bool interpolate_samples() const {
            return _interpolate_samples;
        }

        float get_speed() const {
            return _step_size;
        }

        void set_speed(const float speed) {
            _step_size = speed;
        }

    private:
        static constexpr int8_t NO_EVENT = -1;

        StreamDataProvider* _stream_data_provider;
        uint32_t            _buffer_length;
        float*              _buffer;
        const uint8_t       _buffer_division;
        const uint8_t       _buffer_segment_offset;
        float               _sample_rate;
        float               _amplitude;
        float               _step_size;
        bool                _interpolate_samples;
        float               _buffer_index;
        float               _buffer_index_prev;
        int8_t              _complete_event;

        int32_t wrapIndex(int32_t i) const {
            if (i < 0) {
                i += _buffer_length;
            } else if (i >= _buffer_length) {
                i -= _buffer_length;
            }
            return i;
        }

        static float convert_sample(const float pRawSample) {
            return pRawSample;
        }

        int8_t checkCompleteEvent(const uint8_t num_events) const {
            for (int i = 0; i < num_events; ++i) {
                const float mBorder = _buffer_length * i / static_cast<float>(_buffer_division);
                if (crossedBorder(_buffer_index_prev, _buffer_index, mBorder)) {
                    return i;
                }
            }
            return NO_EVENT;
        }

        static bool crossedBorder(const float prev, const float current, const float border) {
            return (border == 0 && prev > current) ||
                   (prev < border && current >= border) ||
                   (prev > current && current >= border);
        }
    };
} // namespace klangwellen
