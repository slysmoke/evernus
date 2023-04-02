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

#include <unordered_set>
#include <mutex>

#include <boost/functional/hash.hpp>

#include <QDateTime>

#include "Character.h"
#include "Citadel.h"

class QDataStream;

namespace Evernus
{
    class CitadelAccessCache final
    {
    public:
        CitadelAccessCache() = default;
        CitadelAccessCache(const CitadelAccessCache &) = default;
        CitadelAccessCache(CitadelAccessCache &&) = default;
        ~CitadelAccessCache() = default;

        bool isAvailable(Character::IdType charId, Citadel::IdType citadelId) const;
        void blacklist(Character::IdType charId, Citadel::IdType citadelId);

        void clearIfObsolete(const QDateTime &minTimestamp);
        void clear();

        CitadelAccessCache &operator =(const CitadelAccessCache &) = default;
        CitadelAccessCache &operator =(CitadelAccessCache &&) = default;

    private:
        using CharacterCitadelPair = std::pair<Character::IdType, Citadel::IdType>;

        QDateTime mTimeStamp = QDateTime::currentDateTime();

        std::unordered_set<CharacterCitadelPair, boost::hash<CharacterCitadelPair>> mBlacklisted;
        mutable std::mutex mCacheMutex;

        friend QDataStream &operator <<(QDataStream &stream, const CitadelAccessCache &cache);
        friend QDataStream &operator >>(QDataStream &stream, CitadelAccessCache &cache);
    };
}
