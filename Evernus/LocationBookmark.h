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

#include <QtGlobal>

#include "Entity.h"

namespace Evernus
{
    class EveDataProvider;

    class LocationBookmark
        : public Entity<uint>
    {
    public:
        using Entity::Entity;
        virtual ~LocationBookmark() = default;

        uint getRegionId() const noexcept;
        void setRegionId(uint id) noexcept;

        uint getSolarSystemId() const noexcept;
        void setSolarSystemId(uint id) noexcept;

        quint64 getStationId() const noexcept;
        void setStationId(quint64 id) noexcept;

        QString toString(const EveDataProvider &dataProvider) const;

    private:
        uint mRegionId = 0;
        uint mSolarSystemId = 0;
        quint64 mStationId = 0;
    };
}
