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

#include <optional>

#include <QString>

#include "MarketGroup.h"
#include "Entity.h"

namespace Evernus
{
    class EveType
        : public Entity<uint>
    {
    public:
        typedef std::optional<QString> DescriptionType;
        typedef std::optional<uint> RaceIdType;
        typedef std::optional<MarketGroup::IdType> MarketGroupIdType;

        using Entity::Entity;

        EveType() = default;
        EveType(const EveType &) = default;
        EveType(EveType &&) = default;
        virtual ~EveType() = default;

        uint getGroupId() const noexcept;
        void setGroupId(uint id) noexcept;

        QString getName() const &;
        QString &&getName() && noexcept;
        void setName(const QString &name);
        void setName(QString &&name);

        DescriptionType getDescription() const;
        void setDescription(const DescriptionType &desc);

        double getMass() const noexcept;
        void setMass(double value) noexcept;

        double getVolume() const noexcept;
        void setVolume(double value) noexcept;

        double getCapacity() const noexcept;
        void setCapacity(double value) noexcept;

        int getPortionSize() const noexcept;
        void setPortionSize(int value) noexcept;

        RaceIdType getRaceId() const;
        void setRaceId(const RaceIdType &id);

        double getBasePrice() const noexcept;
        void setBasePrice(double value) noexcept;

        bool isPublished() const noexcept;
        void setPublished(bool flag) noexcept;

        MarketGroupIdType getMarketGroupId() const;
        void setMarketGroupId(const MarketGroupIdType &id);

        uint getGraphicId() const noexcept;
        void setGraphicId(uint id) noexcept;

        uint getIconId() const noexcept;
        void setIconId(uint id) noexcept;

        double getRadius() const noexcept;
        void setRadius(double value) noexcept;

        uint getSoundId() const noexcept;
        void setSoundId(uint id) noexcept;

        EveType &operator =(const EveType &) = default;
        EveType &operator =(EveType &&) = default;

    private:
        uint mGroupId = 0;
        QString mName;
        DescriptionType mDescription;
        double mMass = 0.;
        double mVolume = 0.;
        double mCapacity = 0.;
        int mPortionSize = 0;
        RaceIdType mRaceId;
        double mBasePrice = 0.;
        bool mPublished = true;
        MarketGroupIdType mMarketGroup;
        uint mGraphicId = 0;
        uint mIconId = 0;
        double mRadius = 0.;
        uint mSoundId = 0;
    };
}
