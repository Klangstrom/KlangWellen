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
 * - [x] void process(AudioSignal&)
 * - [x] void process(float*, uint32_t)
 * - [x] void process(float*, float*, uint32_t)
 */

#pragma once

#include <stdint.h>

#include "AudioSignal.h"

namespace klangwellen {
    class Gain {
    public:
        Gain() : _gain(1.0f) {}

        float get_gain() const {
            return _gain;
        }

        void set_gain(float pGain) {
            _gain = pGain;
        }

        float process(float signal) const {
            return signal * _gain;
        }

        void process(AudioSignal& signal) const {
            signal.left *= _gain;
            signal.right *= _gain;
        }

        void process(float*         signal_buffer_left,
                     float*         signal_buffer_right,
                     const uint32_t buffer_length) const {
            for (int i = 0; i < buffer_length; i++) {
                signal_buffer_left[i] *= _gain;
                signal_buffer_right[i] *= _gain;
            }
        }

        void process(float*         signal_buffer,
                     const uint32_t buffer_length) const {
            for (int i = 0; i < buffer_length; i++) {
                signal_buffer[i] *= _gain;
            }
        }

    private:
        float _gain;
    };
} // namespace klangwellen
