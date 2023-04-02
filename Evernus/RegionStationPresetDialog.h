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

#include <QDialog>

class QComboBox;

namespace Evernus
{
    class RegionStationPresetRepository;
    class StationSelectButton;
    class EveDataProvider;
    class RegionComboBox;

    class RegionStationPresetDialog final
        : public QDialog
    {
        Q_OBJECT

    public:
        using RegionList = std::unordered_set<uint>;

        RegionStationPresetDialog(const RegionStationPresetRepository &regionStationPresetRepository,
                                  const EveDataProvider &dataProvider,
                                  QWidget *parent = nullptr);
        RegionStationPresetDialog(const RegionStationPresetDialog &) = default;
        RegionStationPresetDialog(RegionStationPresetDialog &&) = default;
        virtual ~RegionStationPresetDialog() = default;

        RegionList getSrcRegions() const;
        RegionList getDstRegions() const;

        quint64 getSrcStationId() const;
        quint64 getDstStationId() const;

        RegionStationPresetDialog &operator =(const RegionStationPresetDialog &) = default;
        RegionStationPresetDialog &operator =(RegionStationPresetDialog &&) = default;

    private slots:
        void load();
        void save();
        void remove();

    private:
        const RegionStationPresetRepository &mRegionStationPresetRepository;

        QComboBox *mPresetNameEdit = nullptr;

        RegionComboBox *mSrcRegions = nullptr;
        RegionComboBox *mDstRegions = nullptr;

        StationSelectButton *mSrcStationBtn = nullptr;
        StationSelectButton *mDstStationBtn = nullptr;
    };
}
