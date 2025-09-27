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

#include <vector>
#include <initializer_list>

namespace klangwellen {
    struct ScaleData {
        ScaleData(const std::initializer_list<uint8_t> note_list) : notes(note_list) {}
        ScaleData(const ScaleData&)                = default;
        ScaleData& operator=(const ScaleData&)     = default;
        ScaleData(ScaleData&&) noexcept            = default;
        ScaleData& operator=(ScaleData&&) noexcept = default;

        size_t         length() const noexcept { return notes.size(); }
        const uint8_t* data() const noexcept { return notes.data(); }

        std::vector<uint8_t> notes;
    };

    namespace Scale {
        static inline const ScaleData CHROMATIC{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        static inline const ScaleData FIFTH{0, 7};
        static inline const ScaleData MINOR{0, 2, 3, 5, 7, 8, 10};
        static inline const ScaleData MAJOR{0, 2, 4, 5, 7, 9, 11};
        static inline const ScaleData MINOR_CHORD{0, 3, 7};
        static inline const ScaleData MAJOR_CHORD{0, 4, 7};
        static inline const ScaleData MINOR_CHORD_7{0, 3, 7, 11};
        static inline const ScaleData MAJOR_CHORD_7{0, 4, 7, 11};
        static inline const ScaleData MINOR_PENTATONIC{0, 3, 5, 7, 10};
        static inline const ScaleData MAJOR_PENTATONIC{0, 4, 5, 7, 11};
        static inline const ScaleData OCTAVE{0};
        static inline const ScaleData DIMINISHED{0, 3, 6, 9};

        static int floor_div(const int a, const int b) {
            // b > 0
            int q = a / b;
            int r = a % b;
            if ((r != 0) && ((r > 0) != (b > 0))) --q;
            return q;
        }

        static int floor_mod(const int a, const int b) {
            int r = a % b;
            if (r < 0) r += b;
            return r;
        }

        inline int note(const ScaleData& scale, const int baseNote, const int stepOffset) {
            const int mSize = static_cast<int>(scale.notes.size());
            if (mSize == 0) return baseNote;

            const int octave     = floor_div(stepOffset, mSize);
            const int index      = floor_mod(stepOffset, mSize);
            const int scaleEntry = scale.notes[static_cast<size_t>(index)];
            return baseNote + octave * 12 + scaleEntry;
        }
    } // namespace Scale
} // namespace klangwellen