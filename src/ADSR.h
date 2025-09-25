/*
 * Klangwellen
 *
 * This file is part of the *Klangwellen* library (https://github.com/dennisppaul/klangwellen).
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
 * - [x] float process(float)
 * - [x] void process(AudioSignal&)
 * - [x] void process(float*, uint32_t)
 * - [x] void process(float*, float*, uint32_t)
 */

#pragma once

#include "Klangwellen.h"
#include "AudioSignal.h"

namespace klangwellen {
    class ADSR {
        /*
         *       @description(
         *
         *          |----->|-->|   |-->|
         *          |---A--|-D-|-S-|-R-|
         *          |      /\
         *          |     /  \
         *          |    /    \_____
         *          |   /        ^  \
         *          |  /         |   \
         *          | /          |    \
         *          |/___________|_____\
         *          |
         *          |Press          |Release
         *       )
         *
         */
    public:
        explicit ADSR(const uint32_t sample_rate = Klangwellen::DEFAULT_SAMPLE_RATE) : fSampleRate(sample_rate),
                                                                                       FADE_TO_ZERO_RATE_SEC(0.01f),
                                                                                       USE_FADE_TO_ZERO_STATE(false) {
            _amplitude = 0.0f;
            _attack    = Klangwellen::DEFAULT_ATTACK;
            _decay     = Klangwellen::DEFAULT_DECAY;
            _delta     = 0.0f;
            _release   = Klangwellen::DEFAULT_RELEASE;
            _sustain   = Klangwellen::DEFAULT_SUSTAIN;
            _state     = ENVELOPE_STATE::IDLE;
            setState(ENVELOPE_STATE::IDLE);
        }

        float process() {
            step();
            return _amplitude;
        }

        float process(float signal) {
            step();
            return signal * _amplitude;
        }

        void process(AudioSignal& signal) {
            step();
            signal.left *= _amplitude;
            signal.right *= _amplitude;
        }

        void process(float*         signal_buffer_left,
                     float*         signal_buffer_right,
                     const uint32_t buffer_length = Klangwellen::DEFAULT_AUDIOBLOCK_SIZE) {
            for (uint32_t i = 0; i < buffer_length; i++) {
                step();
                signal_buffer_left[i] *= _amplitude;
                signal_buffer_right[i] *= _amplitude;
            }
        }

        void process(float* signal_buffer, const uint32_t buffer_length = Klangwellen::DEFAULT_AUDIOBLOCK_SIZE) {
            for (uint32_t i = 0; i < buffer_length; i++) {
                step();
                signal_buffer[i] *= _amplitude;
            }
        }

        void start() {
            check_scheduled_attack_state();
        }

        void stop() {
            check_scheduled_release_state();
        }

        float get_attack() const {
            return _attack;
        }

        void set_attack(float pAttack) {
            _attack = pAttack;
        }

        void set_adsr(float pAttack, float pDecay, float pSustain, float pRelease) {
            set_attack(pAttack);
            set_decay(pDecay);
            set_sustain(pSustain);
            set_release(pRelease);
        }

        float get_decay() const {
            return _decay;
        }

        void set_decay(float pDecay) {
            _decay = pDecay;
        }

        float get_sustain() const {
            return _sustain;
        }

        void set_sustain(float pSustain) {
            _sustain = pSustain;
        }

        float get_release() const;

        void set_release(float pRelease) {
            _release = pRelease;
        }

    private:
        enum class ENVELOPE_STATE {
            IDLE,
            ATTACK,
            DECAY,
            SUSTAIN,
            RELEASE,
            PRE_ATTACK_FADE_TO_ZERO
        };
        const uint32_t fSampleRate;
        const float    FADE_TO_ZERO_RATE_SEC;
        const bool     USE_FADE_TO_ZERO_STATE;
        float          _amplitude;
        float          _attack;
        float          _decay;
        float          _delta;
        float          _release;
        ENVELOPE_STATE _state;
        float          _sustain;

        void check_scheduled_attack_state() {
            if (_amplitude > 0.0f) {
                if (USE_FADE_TO_ZERO_STATE) {
                    if (_state != ENVELOPE_STATE::PRE_ATTACK_FADE_TO_ZERO) {
                        _delta = compute_delta_fraction(-_amplitude, FADE_TO_ZERO_RATE_SEC);
                        setState(ENVELOPE_STATE::PRE_ATTACK_FADE_TO_ZERO);
                    }
                } else {
                    _delta = compute_delta_fraction(1.0f, _attack);
                    setState(ENVELOPE_STATE::ATTACK);
                }
            } else {
                _delta = compute_delta_fraction(1.0f, _attack);
                setState(ENVELOPE_STATE::ATTACK);
            }
        }

        void check_scheduled_release_state() {
            if (_state != ENVELOPE_STATE::RELEASE) {
                _delta = compute_delta_fraction(-_amplitude, _release);
                setState(ENVELOPE_STATE::RELEASE);
            }
        }

        float compute_delta_fraction(float pDelta, float pDuration) const {
            return pDuration > 0 ? (pDelta / static_cast<float>(fSampleRate)) / pDuration : pDelta;
        }

        void setState(ENVELOPE_STATE pState) {
            _state = pState;
        }

        void step() {
            switch (_state) {
                case ENVELOPE_STATE::IDLE:
                case ENVELOPE_STATE::SUSTAIN:
                    break;
                case ENVELOPE_STATE::ATTACK:
                    // increase amp to sustain_level in ATTACK sec
                    _amplitude += _delta;
                    if (_amplitude >= 1.0f) {
                        _amplitude = 1.0f;
                        _delta     = compute_delta_fraction(-(1.0f - _sustain), _decay);
                        setState(ENVELOPE_STATE::DECAY);
                    }
                    break;
                case ENVELOPE_STATE::DECAY:
                    // decrease amp to sustain_level in DECAY sec
                    _amplitude += _delta;
                    if (_amplitude <= _sustain) {
                        _amplitude = _sustain;
                        setState(ENVELOPE_STATE::SUSTAIN);
                    }
                    break;
                case ENVELOPE_STATE::RELEASE:
                    // decrease amp to 0.0 in RELEASE sec
                    _amplitude += _delta;
                    if (_amplitude <= 0.0f) {
                        _amplitude = 0.0f;
                        setState(ENVELOPE_STATE::IDLE);
                    }
                    break;
                case ENVELOPE_STATE::PRE_ATTACK_FADE_TO_ZERO:
                    _amplitude += _delta;
                    if (_amplitude <= 0.0f) {
                        _amplitude = 0.0f;
                        _delta     = compute_delta_fraction(1.0f, _attack);
                        setState(ENVELOPE_STATE::ATTACK);
                    }
                    break;
            }
        }
    };
    float ADSR::get_release() const {
        return _release;
    }
} // namespace klangwellen
