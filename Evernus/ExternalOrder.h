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

#include <memory>

#include <QStringList>
#include <QDateTime>

#include "PriceType.h"
#include "ItemData.h"
#include "Entity.h"

namespace Evernus
{
    class ExternalOrder
        : public Entity<quint64>
    {
    public:
        using TypeIdType = ItemData::TypeIdType;
        using Type = PriceType;

        struct LowToHigh
        {
            inline bool operator ()(const ExternalOrder &a, const ExternalOrder &b) const noexcept
            {
                return a.getPrice() < b.getPrice();
            }
        };

        struct HighToLow
        {
            inline bool operator ()(const ExternalOrder &a, const ExternalOrder &b) const noexcept
            {
                return a.getPrice() > b.getPrice();
            }
        };

        static const short rangeStation = -1;
        static const short rangeSystem = 0;
        static const short rangeRegion = 32767;

        using Entity::Entity;

        ExternalOrder() = default;
        ExternalOrder(const ExternalOrder &) = default;
        ExternalOrder(ExternalOrder &&) = default;
        virtual ~ExternalOrder() = default;

        Type getType() const noexcept;
        void setType(Type type) noexcept;

        TypeIdType getTypeId() const noexcept;
        void setTypeId(TypeIdType id) noexcept;

        quint64 getStationId() const noexcept;
        void setStationId(quint64 id) noexcept;

        uint getSolarSystemId() const noexcept;
        void setSolarSystemId(uint id) noexcept;

        uint getRegionId() const noexcept;
        void setRegionId(uint id) noexcept;

        short getRange() const noexcept;
        void setRange(short value) noexcept;

        QDateTime getUpdateTime() const;
        void setUpdateTime(const QDateTime &dt);

        double getPrice() const noexcept;
        void setPrice(double value) noexcept;

        uint getVolumeEntered() const noexcept;
        void setVolumeEntered(uint value) noexcept;

        uint getVolumeRemaining() const noexcept;
        void setVolumeRemaining(uint value) noexcept;

        uint getMinVolume() const noexcept;
        void setMinVolume(uint value) noexcept;

        QDateTime getIssued() const;
        void setIssued(const QDateTime &dt);

        short getDuration() const noexcept;
        void setDuration(short value) noexcept;

        ExternalOrder &operator =(const ExternalOrder &) = default;
        ExternalOrder &operator =(ExternalOrder &&) = default;

        static ExternalOrder parseLogLine(const QStringList &values);

        static std::shared_ptr<ExternalOrder> nullOrder();

    private:
        Type mType = Type::Buy;
        TypeIdType mTypeId = TypeIdType{};
        quint64 mLocationId = 0;
        uint mSolarSystemId = 0;
        uint mRegionId = 0;
        short mRange = 32767;
        QDateTime mUpdateTime;
        double mPrice = 0.;
        uint mVolumeEntered = 0;
        uint mVolumeRemaining = 0;
        uint mMinVolume = 0;
        QDateTime mIssued;
        short mDuration = 0;
    };
}
