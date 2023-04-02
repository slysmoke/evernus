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

#include <QPushButton>

namespace Evernus
{
    class RegionStationPresetRepository;
    class EveDataProvider;

    class FavoriteLocationsButton final
        : public QPushButton
    {
        Q_OBJECT

    public:
        using RegionList = std::unordered_set<uint>;

        FavoriteLocationsButton(const RegionStationPresetRepository &regionStationPresetRepository,
                                const EveDataProvider &dataProvider,
                                QWidget *parent = nullptr);
        FavoriteLocationsButton(const FavoriteLocationsButton &) = default;
        FavoriteLocationsButton(FavoriteLocationsButton &&) = default;
        virtual ~FavoriteLocationsButton() = default;

        FavoriteLocationsButton &operator =(const FavoriteLocationsButton &) = default;
        FavoriteLocationsButton &operator =(FavoriteLocationsButton &&) = default;

    signals:
        void locationsChosen(const RegionList &srcRegions, quint64 srcStationId,
                             const RegionList &dstRegions, quint64 dstStationId);

    private slots:
        void selectFavoriteLocations();

    private:
        const RegionStationPresetRepository &mRegionStationPresetRepository;
        const EveDataProvider &mDataProvider;
    };
}
