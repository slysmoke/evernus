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
#include "EveType.h"

namespace Evernus
{
    uint EveType::getGroupId() const noexcept
    {
        return mGroupId;
    }

    void EveType::setGroupId(uint id) noexcept
    {
        mGroupId = id;
    }

    QString EveType::getName() const &
    {
        return mName;
    }

    QString &&EveType::getName() && noexcept
    {
        return std::move(mName);
    }

    void EveType::setName(const QString &name)
    {
        mName = name;
    }

    void EveType::setName(QString &&name)
    {
        mName = std::move(name);
    }

    EveType::DescriptionType EveType::getDescription() const
    {
        return mDescription;
    }

    void EveType::setDescription(const DescriptionType &desc)
    {
        mDescription = desc;
    }

    double EveType::getMass() const noexcept
    {
        return mMass;
    }

    void EveType::setMass(double value) noexcept
    {
        mMass = value;
    }

    double EveType::getVolume() const noexcept
    {
        return mVolume;
    }

    void EveType::setVolume(double value) noexcept
    {
        mVolume = value;
    }

    double EveType::getCapacity() const noexcept
    {
        return mCapacity;
    }

    void EveType::setCapacity(double value) noexcept
    {
        mCapacity = value;
    }

    int EveType::getPortionSize() const noexcept
    {
        return mPortionSize;
    }

    void EveType::setPortionSize(int value) noexcept
    {
        mPortionSize = value;
    }

    EveType::RaceIdType EveType::getRaceId() const
    {
        return mRaceId;
    }

    void EveType::setRaceId(const RaceIdType &id)
    {
        mRaceId = id;
    }

    double EveType::getBasePrice() const noexcept
    {
        return mBasePrice;
    }

    void EveType::setBasePrice(double value) noexcept
    {
        mBasePrice = value;
    }

    bool EveType::isPublished() const noexcept
    {
        return mPublished;
    }

    void EveType::setPublished(bool flag) noexcept
    {
        mPublished = flag;
    }

    EveType::MarketGroupIdType EveType::getMarketGroupId() const
    {
        return mMarketGroup;
    }

    void EveType::setMarketGroupId(const MarketGroupIdType &id)
    {
        mMarketGroup = id;
    }

    uint EveType::getGraphicId() const noexcept
    {
        return mGraphicId;
    }

    void EveType::setGraphicId(uint id) noexcept
    {
        mGraphicId = id;
    }

    uint EveType::getIconId() const noexcept
    {
        return mIconId;
    }

    void EveType::setIconId(uint id) noexcept
    {
        mIconId = id;
    }

    double EveType::getRadius() const noexcept
    {
        return mRadius;
    }

    void EveType::setRadius(double value) noexcept
    {
        mRadius = value;
    }

    uint EveType::getSoundId() const noexcept
    {
        return mSoundId;
    }

    void EveType::setSoundId(uint id) noexcept
    {
        mSoundId = id;
    }
}
