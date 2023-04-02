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
#include "EveDataProvider.h"

#include "LocationBookmark.h"

namespace Evernus
{
    uint LocationBookmark::getRegionId() const noexcept
    {
        return mRegionId;
    }

    void LocationBookmark::setRegionId(uint id) noexcept
    {
        mRegionId = id;
    }

    uint LocationBookmark::getSolarSystemId() const noexcept
    {
        return mSolarSystemId;
    }

    void LocationBookmark::setSolarSystemId(uint id) noexcept
    {
        mSolarSystemId = id;
    }

    quint64 LocationBookmark::getStationId() const noexcept
    {
        return mStationId;
    }

    void LocationBookmark::setStationId(quint64 id) noexcept
    {
        mStationId = id;
    }

    QString LocationBookmark::toString(const EveDataProvider &dataProvider) const
    {
        const auto name = QString{"%1 / %2 / %3"}
            .arg((mRegionId == 0) ? (QT_TRANSLATE_NOOP("LocationBookmark", "[all]")) : (dataProvider.getRegionName(mRegionId)))
            .arg((mSolarSystemId == 0) ? (QT_TRANSLATE_NOOP("LocationBookmark", "[all]")) : (dataProvider.getSolarSystemName(mSolarSystemId)))
            .arg((mStationId == 0) ? (QT_TRANSLATE_NOOP("LocationBookmark", "[all]")) : (dataProvider.getLocationName(mStationId)));

        return name;
    }
}
