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
 * - [x] void process(float*, uint32_t)
 * - [ ] void process(float*, float*, uint32_t)
 */

#pragma once

#include <stdint.h>

namespace klangwellen {
    class Ramp {
    public:
        explicit Ramp(const uint32_t sample_rate) : _sample_rate(sample_rate), _delta_fraction(0) {
            _start_value   = 0;
            _end_value     = 0;
            _current_value = 0;
            _duration      = 0;
            _is_done       = true;
        }

        void set_start(const float start_value) {
            _start_value = start_value;
        }

        void set_end(const float end_value) {
            _end_value = end_value;
        }

        /**
         * duration in sec
         */
        void set_duration(const float duration) {
            _duration = duration;
        }

        void set(const float start_value, const float end_value, const float duration) {
            _start_value = start_value;
            _end_value   = end_value;
            _duration    = duration;
        }

        float process() {
            if (!_is_done) {
                _current_value += _delta_fraction;
                if (_current_value > _end_value) {
                    _current_value = _end_value;
                    _is_done       = true;
                }
                return _current_value;
            }
            return _end_value;
        }

        void process(float*         signal_buffer,
                     const uint32_t length) {
            for (uint32_t i = 0; i < length; i++) {
                signal_buffer[i] = process();
            }
        }

        void start() {
            const float mDelta = _end_value - _start_value;
            _delta_fraction    = compute_delta_fraction(mDelta, _duration);
            _current_value     = _start_value;
            _is_done           = false;
        }

    private:
        uint32_t _sample_rate;
        float    _start_value;
        float    _end_value;
        float    _current_value;
        float    _duration;
        float    _delta_fraction;
        bool     _is_done;

        float compute_delta_fraction(const float delta, const float duration) const {
            return duration > 0 ? (delta / _sample_rate) / duration : delta;
        }
    };
} // namespace klangwellen