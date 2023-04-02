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
#include "Citadel.h"

namespace Evernus
{
    EveType::IdType Citadel::getTypeId() const noexcept
    {
        return mTypeId;
    }

    void Citadel::setTypeId(EveType::IdType id) noexcept
    {
        mTypeId = id;
    }

    QString Citadel::getName() const &
    {
       return mName;
    }

    QString &&Citadel::getName() && noexcept
    {
       return std::move(mName);
    }

    void Citadel::setName(const QString &name)
    {
       mName = name;
    }

    void Citadel::setName(QString &&name)
    {
       mName = std::move(name);
    }

    uint Citadel::getRegionId() const noexcept
    {
       return mRegionId;
    }

    void Citadel::setRegionId(uint id) noexcept
    {
       mRegionId = id;
    }

    uint Citadel::getSolarSystemId() const noexcept
    {
       return mSystemId;
    }

    void Citadel::setSolarSystemId(uint id) noexcept
    {
       mSystemId = id;
    }

    double Citadel::getX() const noexcept
    {
       return mX;
    }

    void Citadel::setX(double value) noexcept
    {
       mX = value;
    }

    double Citadel::getY() const noexcept
    {
       return mY;
    }

    void Citadel::setY(double value) noexcept
    {
       mY = value;
    }

    double Citadel::getZ() const noexcept
    {
       return mZ;
    }

    void Citadel::setZ(double value) noexcept
    {
       mZ = value;
    }

    QDateTime Citadel::getFirstSeen() const
    {
       return mFirstSeen;
    }

    void Citadel::setFirstSeen(const QDateTime &dt)
    {
       mFirstSeen = dt;
    }

    QDateTime Citadel::getLastSeen() const
    {
       return mLastSeen;
    }

    void Citadel::setLastSeen(const QDateTime &dt)
    {
       mLastSeen = dt;
    }

    bool Citadel::isPublic() const noexcept
    {
       return mPublic;
    }

    void Citadel::setPublic(bool flag) noexcept
    {
       mPublic = flag;
    }

    bool Citadel::isIgnored() const noexcept
    {
       return mIgnored;
    }

    void Citadel::setIgnored(bool flag) noexcept
    {
       mIgnored = flag;
    }

    bool Citadel::canHaveMarket() const noexcept
    {
        return mTypeId != 35832; // Astrahus
    }

    bool Citadel::canImportMarket() const noexcept
    {
        return canHaveMarket() && !isIgnored();
    }
}
