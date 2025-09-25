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
 * - [ ] float process()
 * - [x] float process(float)
 * - [ ] void process(AudioSignal&)
 * - [x] void process(float*, uint32_t)
 * - [ ] void process(float*, float*, uint32_t)
 */

// @TODO Delay is broken, fix it

#pragma once

#include <stdint.h>
#include <stdio.h>

#include "KlangWellen.h"

namespace klangwellen {

    /**
     * a delay line.
     */
    class Delay {
    public:
        /**
         * @param sample_rate the sample rate in Hz.
         * @param echo_length in seconds
         * @param decay_rate  the decay of the echo, a value between 0 and 1. 1 meaning no decay, 0 means immediate decay
         * @param wet         the wet mix, a value between 0 and 1. 0 means dry only, 1 means wet only.
         */
        explicit Delay(const uint32_t sample_rate, const float echo_length = 0.5, const float decay_rate = 0.75, const float wet = 0.8) : _sample_rate(sample_rate) {
            set_decay_rate(decay_rate);
            set_echo_length(echo_length);
            set_wet(wet);
            adaptEchoLength();
        }

        /**
         * @param echo_length new echo buffer length in seconds.
         */
        void set_echo_length(const float echo_length) {
            _new_echo_length = echo_length;
        }

        /**
         * A decay, should be a value between zero and one.
         *
         * @param decay_rate the new decay (preferably between zero and one).
         */
        void set_decay_rate(const float decay_rate) {
            _decay_rate = decay_rate;
        }

        float get_decay_rate() const {
            return _decay_rate;
        }

        void set_wet(const float wet) {
            _wet = KlangWellen::clamp(wet, 0, 1);
        }

        float get_wet() const {
            return _wet;
        }

        float process(const float input) {
            if (KlangWellen::abs(input) < 0.00001f) {
                return 0; // avoid denormals
            }
            if (_buffer == nullptr) {
                return input;
            }
            if (_buffer_length == 0) {
                return input; // no delay allocated yet
            }

            adaptEchoLength();

            if (_buffer_position >= _buffer_length) {
                _buffer_position = 0;
            }

            const float dry  = 1.0f - _wet;
            const float echo = _buffer[_buffer_position];

            // Output: dry + wet * echo
            const float output = input * dry + echo * _wet;

            // Write back only the feedback content (echoed input), not the mixed output
            _buffer[_buffer_position] = input + echo * _decay_rate;

            _buffer_position++;

            return output;

            // const float signal = input + _buffer[_buffer_position] * _decay_rate;
            // _buffer[_buffer_position] = signal;
            // _buffer_position++;
            // return signal * _wet;
        }

        void process(float*         signal_buffer,
                     const uint32_t length) {
            for (uint32_t i = 0; i < length; i++) {
                signal_buffer[i] = process(signal_buffer[i]);
            }
        }

    private:
        int32_t        _buffer_position  = 0;
        float          _decay_rate       = 0;
        float          _wet              = 0;
        float*         _buffer           = nullptr;
        bool           _allocated_buffer = false;
        int32_t        _buffer_length    = 0;
        float          _new_echo_length  = 0;
        const uint32_t _sample_rate;

        void adaptEchoLength() {
            if (_new_echo_length > 0) {
                const uint32_t _new_buffer_length = static_cast<uint32_t>(_sample_rate * _new_echo_length);
                if (_new_buffer_length == 0) {
                    // reset buffer to avoid out-of-bounds
                    if (_allocated_buffer && _buffer != nullptr) {
                        delete[] _buffer;
                    }
                    _buffer           = nullptr;
                    _buffer_length    = 0;
                    _buffer_position  = 0;
                    _allocated_buffer = false;
                    _new_echo_length  = -1;
                    return;
                }

                const auto _new_buffer = new float[_new_buffer_length]{0};

                // Copy as much as fits; keep read head alignment to avoid large jumps
                if (_buffer != nullptr && _buffer_length > 0) {
                    const uint32_t copyCount = (_new_buffer_length < static_cast<uint32_t>(_buffer_length))
                                                   ? _new_buffer_length
                                                   : static_cast<uint32_t>(_buffer_length);
                    // Copy most recent samples into end of new buffer to minimize click
                    // Compute start in old buffer
                    int32_t start = _buffer_position - static_cast<int32_t>(copyCount);
                    while (start < 0) start += _buffer_length;
                    for (uint32_t i = 0; i < copyCount; ++i) {
                        int32_t idx = start + static_cast<int32_t>(i);
                        if (idx >= _buffer_length) idx -= _buffer_length;
                        _new_buffer[_new_buffer_length - copyCount + i] = _buffer[idx];
                    }
                    // Position read at start of newest segment
                    _buffer_position = (_new_buffer_length - copyCount);
                } else {
                    _buffer_position = 0;
                }

                if (_allocated_buffer && _buffer != nullptr) {
                    delete[] _buffer;
                }
                _allocated_buffer = true;
                _buffer           = _new_buffer;
                _buffer_length    = _new_buffer_length;
            }
            _new_echo_length = -1;
        }
    };
} // namespace klangwellen