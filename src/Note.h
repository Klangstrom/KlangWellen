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

#include <stdint.h>

namespace klangwellen {
    struct Note {
        /**
         *                       Note   MIDI   Freq (Hz)
         */
        static constexpr uint8_t C_0  = 12; // 16.35 Hz
        static constexpr uint8_t Db_0 = 13; // 17.32 Hz (C#0)
        static constexpr uint8_t D_0  = 14; // 18.35 Hz
        static constexpr uint8_t Eb_0 = 15; // 19.45 Hz (D#0)
        static constexpr uint8_t E_0  = 16; // 20.60 Hz
        static constexpr uint8_t F_0  = 17; // 21.83 Hz
        static constexpr uint8_t Gb_0 = 18; // 23.12 Hz (F#0)
        static constexpr uint8_t G_0  = 19; // 24.50 Hz
        static constexpr uint8_t Ab_0 = 20; // 25.96 Hz (G#0)
        static constexpr uint8_t A_0  = 21; // 27.50 Hz
        static constexpr uint8_t Bb_0 = 22; // 29.14 Hz (A#0)
        static constexpr uint8_t B_0  = 23; // 30.87 Hz

        static constexpr uint8_t C_1  = 24; // 32.70 Hz
        static constexpr uint8_t Db_1 = 25; // 34.65 Hz
        static constexpr uint8_t D_1  = 26; // 36.71 Hz
        static constexpr uint8_t Eb_1 = 27; // 38.89 Hz
        static constexpr uint8_t E_1  = 28; // 41.20 Hz
        static constexpr uint8_t F_1  = 29; // 43.65 Hz
        static constexpr uint8_t Gb_1 = 30; // 46.25 Hz
        static constexpr uint8_t G_1  = 31; // 49.00 Hz
        static constexpr uint8_t Ab_1 = 32; // 51.91 Hz
        static constexpr uint8_t A_1  = 33; // 55.00 Hz
        static constexpr uint8_t Bb_1 = 34; // 58.27 Hz
        static constexpr uint8_t B_1  = 35; // 61.74 Hz

        static constexpr uint8_t C_2  = 36; // 65.41 Hz
        static constexpr uint8_t Db_2 = 37; // 69.30 Hz
        static constexpr uint8_t D_2  = 38; // 73.42 Hz
        static constexpr uint8_t Eb_2 = 39; // 77.78 Hz
        static constexpr uint8_t E_2  = 40; // 82.41 Hz
        static constexpr uint8_t F_2  = 41; // 87.31 Hz
        static constexpr uint8_t Gb_2 = 42; // 92.50 Hz
        static constexpr uint8_t G_2  = 43; // 98.00 Hz
        static constexpr uint8_t Ab_2 = 44; // 103.83 Hz
        static constexpr uint8_t A_2  = 45; // 110.00 Hz
        static constexpr uint8_t Bb_2 = 46; // 116.54 Hz
        static constexpr uint8_t B_2  = 47; // 123.47 Hz

        static constexpr uint8_t C_3  = 48; // 130.81 Hz
        static constexpr uint8_t Db_3 = 49; // 138.59 Hz
        static constexpr uint8_t D_3  = 50; // 146.83 Hz
        static constexpr uint8_t Eb_3 = 51; // 155.56 Hz
        static constexpr uint8_t E_3  = 52; // 164.81 Hz
        static constexpr uint8_t F_3  = 53; // 174.61 Hz
        static constexpr uint8_t Gb_3 = 54; // 185.00 Hz
        static constexpr uint8_t G_3  = 55; // 196.00 Hz
        static constexpr uint8_t Ab_3 = 56; // 207.65 Hz
        static constexpr uint8_t A_3  = 57; // 220.00 Hz
        static constexpr uint8_t Bb_3 = 58; // 233.08 Hz
        static constexpr uint8_t B_3  = 59; // 246.94 Hz

        static constexpr uint8_t C_4  = 60; // 261.63 Hz
        static constexpr uint8_t Db_4 = 61; // 277.18 Hz
        static constexpr uint8_t D_4  = 62; // 293.66 Hz
        static constexpr uint8_t Eb_4 = 63; // 311.13 Hz
        static constexpr uint8_t E_4  = 64; // 329.63 Hz
        static constexpr uint8_t F_4  = 65; // 349.23 Hz
        static constexpr uint8_t Gb_4 = 66; // 369.99 Hz
        static constexpr uint8_t G_4  = 67; // 392.00 Hz
        static constexpr uint8_t Ab_4 = 68; // 415.30 Hz
        static constexpr uint8_t A_4  = 69; // 440.00 Hz
        static constexpr uint8_t Bb_4 = 70; // 466.16 Hz
        static constexpr uint8_t B_4  = 71; // 493.88 Hz

        static constexpr uint8_t C_5  = 72; // 523.25 Hz
        static constexpr uint8_t Db_5 = 73; // 554.37 Hz
        static constexpr uint8_t D_5  = 74; // 587.33 Hz
        static constexpr uint8_t Eb_5 = 75; // 622.25 Hz
        static constexpr uint8_t E_5  = 76; // 659.25 Hz
        static constexpr uint8_t F_5  = 77; // 698.46 Hz
        static constexpr uint8_t Gb_5 = 78; // 739.99 Hz
        static constexpr uint8_t G_5  = 79; // 783.99 Hz
        static constexpr uint8_t Ab_5 = 80; // 830.61 Hz
        static constexpr uint8_t A_5  = 81; // 880.00 Hz
        static constexpr uint8_t Bb_5 = 82; // 932.33 Hz
        static constexpr uint8_t B_5  = 83; // 987.77 Hz

        static constexpr uint8_t C_6  = 84; // 1046.50 Hz
        static constexpr uint8_t Db_6 = 85; // 1108.73 Hz
        static constexpr uint8_t D_6  = 86; // 1174.66 Hz
        static constexpr uint8_t Eb_6 = 87; // 1244.51 Hz
        static constexpr uint8_t E_6  = 88; // 1318.51 Hz
        static constexpr uint8_t F_6  = 89; // 1396.91 Hz
        static constexpr uint8_t Gb_6 = 90; // 1479.98 Hz
        static constexpr uint8_t G_6  = 91; // 1567.98 Hz
        static constexpr uint8_t Ab_6 = 92; // 1661.22 Hz
        static constexpr uint8_t A_6  = 93; // 1760.00 Hz
        static constexpr uint8_t Bb_6 = 94; // 1864.66 Hz
        static constexpr uint8_t B_6  = 95; // 1975.53 Hz

        static constexpr uint8_t C_7  = 96;  // 2093.00 Hz
        static constexpr uint8_t Db_7 = 97;  // 2217.46 Hz
        static constexpr uint8_t D_7  = 98;  // 2349.32 Hz
        static constexpr uint8_t Eb_7 = 99;  // 2489.02 Hz
        static constexpr uint8_t E_7  = 100; // 2637.02 Hz
        static constexpr uint8_t F_7  = 101; // 2793.83 Hz
        static constexpr uint8_t Gb_7 = 102; // 2959.96 Hz
        static constexpr uint8_t G_7  = 103; // 3135.96 Hz
        static constexpr uint8_t Ab_7 = 104; // 3322.44 Hz
        static constexpr uint8_t A_7  = 105; // 3520.00 Hz
        static constexpr uint8_t Bb_7 = 106; // 3729.31 Hz
        static constexpr uint8_t B_7  = 107; // 3951.07 Hz

        static constexpr uint8_t C_8 = 108; // 4186.01 Hz
    };
} // namespace klangwellen