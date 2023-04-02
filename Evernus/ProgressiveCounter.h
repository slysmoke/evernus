/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <cstddef>

namespace Evernus
{
    class ProgressiveCounter final
    {
    public:
        ProgressiveCounter() = default;
        ~ProgressiveCounter() = default;

        size_t getCount() const noexcept;
        void setCount(size_t count) noexcept;
        void incCount() noexcept;
        void addCount(size_t count) noexcept;

        bool isEmpty() const noexcept;
        bool advanceAndCheckBatch() noexcept;

        void reset() noexcept;
        void resetBatch() noexcept;
        void resetBatchIfEmpty() noexcept;

    private:
        size_t mCount = 0, mBatchCount = 0;
    };
}
