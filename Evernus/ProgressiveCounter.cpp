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
#include "MathUtils.h"

#include "ProgressiveCounter.h"

namespace Evernus
{
    size_t ProgressiveCounter::getCount() const noexcept
    {
        return mCount;
    }

    void ProgressiveCounter::setCount(size_t count) noexcept
    {
        mCount = count;
    }

    void ProgressiveCounter::incCount() noexcept
    {
        ++mCount;
    }

    void ProgressiveCounter::addCount(size_t count) noexcept
    {
        mCount += count;
    }

    bool ProgressiveCounter::isEmpty() const noexcept
    {
        return mCount == 0;
    }

    bool ProgressiveCounter::advanceAndCheckBatch() noexcept
    {
        --mCount;
        ++mBatchCount;

        if (mBatchCount >= MathUtils::batchSize(mCount))
        {
            mBatchCount = 0;
            return true;
        }

        return false;
    }

    void ProgressiveCounter::reset() noexcept
    {
        mCount = mBatchCount = 0;
    }

    void ProgressiveCounter::resetBatch() noexcept
    {
        mBatchCount = 0;
    }

    void ProgressiveCounter::resetBatchIfEmpty() noexcept
    {
        if (isEmpty())
            mBatchCount = 0;
    }
}
