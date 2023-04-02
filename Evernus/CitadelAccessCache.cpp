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
#include <QtDebug>

#include "QDataStreamUtils.h"

#include "CitadelAccessCache.h"

namespace Evernus
{
    bool CitadelAccessCache::isAvailable(Character::IdType charId, Citadel::IdType citadelId) const
    {
        std::lock_guard<std::mutex> lock{mCacheMutex};
        return mBlacklisted.find(std::make_pair(charId, citadelId)) == std::end(mBlacklisted);
    }

    void CitadelAccessCache::blacklist(Character::IdType charId, Citadel::IdType citadelId)
    {
        std::lock_guard<std::mutex> lock{mCacheMutex};
        mBlacklisted.emplace(std::make_pair(charId, citadelId));
    }

    void CitadelAccessCache::clearIfObsolete(const QDateTime &minTimestamp)
    {
        if (mTimeStamp < minTimestamp)
            clear();
    }

    void CitadelAccessCache::clear()
    {
        qDebug() << "Clearing citadel access cache.";

        std::lock_guard<std::mutex> lock{mCacheMutex};
        mBlacklisted.clear();
    }

    QDataStream &operator <<(QDataStream &stream, const CitadelAccessCache &cache)
    {
        stream
            << cache.mTimeStamp
            << cache.mBlacklisted;

        return stream;
    }

    QDataStream &operator >>(QDataStream &stream, CitadelAccessCache &cache)
    {
        stream
            >> cache.mTimeStamp
            >> cache.mBlacklisted;

        return stream;
    }
}
