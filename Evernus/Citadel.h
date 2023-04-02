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

#include <QDateTime>
#include <QString>

#include "EveType.h"
#include "Entity.h"

namespace Evernus
{
    class Citadel final
        : public Entity<quint64>
    {
    public:
        using Entity<quint64>::Entity;

        Citadel() = default;
        Citadel(const Citadel &) = default;
        Citadel(Citadel &&) = default;
        virtual ~Citadel() = default;

        EveType::IdType getTypeId() const noexcept;
        void setTypeId(EveType::IdType id) noexcept;

        QString getName() const &;
        QString &&getName() && noexcept;
        void setName(const QString &name);
        void setName(QString &&name);

        uint getRegionId() const noexcept;
        void setRegionId(uint id) noexcept;

        uint getSolarSystemId() const noexcept;
        void setSolarSystemId(uint id) noexcept;

        double getX() const noexcept;
        void setX(double value) noexcept;

        double getY() const noexcept;
        void setY(double value) noexcept;

        double getZ() const noexcept;
        void setZ(double value) noexcept;

        QDateTime getFirstSeen() const;
        void setFirstSeen(const QDateTime &dt);

        QDateTime getLastSeen() const;
        void setLastSeen(const QDateTime &dt);

        bool isPublic() const noexcept;
        void setPublic(bool flag) noexcept;

        bool isIgnored() const noexcept;
        void setIgnored(bool flag) noexcept;

        bool canHaveMarket() const noexcept;
        bool canImportMarket() const noexcept;

        Citadel &operator =(const Citadel &) = default;
        Citadel &operator =(Citadel &&) = default;

    private:
        EveType::IdType mTypeId = EveType::invalidId;
        QString mName;
        uint mRegionId = 0;
        uint mSystemId = 0;
        double mX = 0.;
        double mY = 0.;
        double mZ = 0.;
        QDateTime mLastSeen;
        QDateTime mFirstSeen;
        bool mPublic = false;
        bool mIgnored = false;
    };
}
