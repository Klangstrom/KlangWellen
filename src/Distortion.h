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

#pragma once

#include <cmath>

namespace klangwellen {
    struct DistortionTypes {
        static constexpr int DISTORTION_HARD_CLIPPING             = 0;
        static constexpr int DISTORTION_FOLDBACK                  = 1;
        static constexpr int DISTORTION_FOLDBACK_SINGLE           = 2;
        static constexpr int DISTORTION_FULL_WAVE_RECTIFICATION   = 3;
        static constexpr int DISTORTION_HALF_WAVE_RECTIFICATION   = 4;
        static constexpr int DISTORTION_INFINITE_CLIPPING         = 5;
        static constexpr int DISTORTION_SOFT_CLIPPING_CUBIC       = 6;
        static constexpr int DISTORTION_SOFT_CLIPPING_ARC_TANGENT = 7;
        static constexpr int DISTORTION_BIT_CRUSHING              = 8;
    };

    class Distortion {
    public:
        Distortion() {
            set_clip(1.0f);
            set_amplification(1.0f);
            set_type(DistortionTypes::DISTORTION_HARD_CLIPPING);
            set_bits(8);
        }

        float get_amplification() const noexcept { return mAmplification; }
        float get_clip() const noexcept { return mClip; }
        int   get_type() const noexcept { return mDistortionType; }
        int   get_bits() const noexcept { return mBits; }

        void set_amplification(const float amplification) noexcept { mAmplification = amplification; }
        void set_clip(const float clip) noexcept { mClip = clip; }
        void set_type(const int type) noexcept { mDistortionType = type; }
        void set_bits(const int bits) noexcept {
            mBits = bits;
            if (mBits < 1) mBits = 1;
            // steps = 2^(bits-1)
            mSteps = static_cast<int>(std::pow(2.0, static_cast<double>(mBits - 1)));
            if (mSteps <= 0) mSteps = 1;
        }

        // process one sample
        float process(const float signal) const noexcept {
            const float amplified = signal * mAmplification;
            switch (mDistortionType) {
                case DistortionTypes::DISTORTION_HARD_CLIPPING:
                    return limit_clip(amplified);
                case DistortionTypes::DISTORTION_FOLDBACK:
                    return limit_foldback(amplified);
                case DistortionTypes::DISTORTION_FOLDBACK_SINGLE:
                    return limit_foldback_single(amplified);
                case DistortionTypes::DISTORTION_FULL_WAVE_RECTIFICATION:
                    return std::fabs(amplified);
                case DistortionTypes::DISTORTION_HALF_WAVE_RECTIFICATION:
                    return amplified < 0.0f ? 0.0f : amplified;
                case DistortionTypes::DISTORTION_INFINITE_CLIPPING:
                    return amplified < 0.0f ? -mClip : (amplified > 0.0f ? mClip : 0.0f);
                case DistortionTypes::DISTORTION_SOFT_CLIPPING_CUBIC:
                    return static_cast<float>(static_cast<double>(amplified) - static_cast<double>(mClip) * std::pow(static_cast<double>(amplified), 3.0));
                case DistortionTypes::DISTORTION_SOFT_CLIPPING_ARC_TANGENT:
                    return static_cast<float>((2.0 / M_PI) * std::atan(static_cast<double>(mClip) * amplified));
                case DistortionTypes::DISTORTION_BIT_CRUSHING:
                    return static_cast<float>(std::floor(static_cast<double>(amplified) * mSteps) / mSteps);
                default:
                    return 0.0f;
            }
        }

    private:
        float limit_clip(const float v) const noexcept {
            if (v > mClip) return mClip;
            if (v < -mClip) return -mClip;
            return v;
        }

        float limit_foldback(float v) const noexcept {
            int i = 0;
            while (v > mClip || v < -mClip) {
                v = limit_foldback_single(v);
                if (LOCK_GUARD) {
                    ++i;
                    if (i > MAX_NUM_OF_ITERATIONS) {
                        return 0.0f;
                    }
                }
            }
            return v;
        }

        float limit_foldback_single(const float v) const noexcept {
            if (v > mClip) {
                const float w = 2.0f * mClip - v;
                return w;
            }
            if (v < -mClip) {
                const float w = -2.0f * mClip - v;
                return w;
            }
            return v;
        }

        static constexpr int  MAX_NUM_OF_ITERATIONS = 16;
        static constexpr bool LOCK_GUARD            = true;

        float mAmplification  = 1.0f;
        int   mBits           = 8;
        float mClip           = 1.0f;
        int   mDistortionType = DistortionTypes::DISTORTION_HARD_CLIPPING;
        int   mSteps          = 1;
    };

} // namespace klangwellen