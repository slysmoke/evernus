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

#include <QVariant>
#include <QWidget>

#include "RegionComboBox.h"

namespace Evernus
{
    class RegionStationPresetRepository;
    class StationSelectButton;
    class EveDataProvider;

    class SourceDestinationSelectWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        using RegionList = RegionComboBox::RegionList;

        SourceDestinationSelectWidget(const QVariantList &srcStationPath,
                                      const QVariantList &dstStationPath,
                                      const QString &srcRegionsSettingsKey,
                                      const QString &dstRegionsSettingsKey,
                                      const EveDataProvider &dataProvider,
                                      const RegionStationPresetRepository &regionStationPresetRepository,
                                      QWidget *parent = nullptr);
        SourceDestinationSelectWidget(const SourceDestinationSelectWidget &) = default;
        SourceDestinationSelectWidget(SourceDestinationSelectWidget &&) = default;
        virtual ~SourceDestinationSelectWidget() = default;

        RegionList getSrcSelectedRegionList() const;
        RegionList getDstSelectedRegionList() const;

        SourceDestinationSelectWidget &operator =(const SourceDestinationSelectWidget &) = default;
        SourceDestinationSelectWidget &operator =(SourceDestinationSelectWidget &&) = default;

    signals:
        void stationsChanged(const QVariantList &srcPath, const QVariantList &dstPath);

    private slots:
        void changeStations();
        void changeLocations(const RegionList &srcRegions, quint64 srcStationId,
                             const RegionList &dstRegions, quint64 dstStationId);

    private:
        RegionComboBox *mSrcRegionCombo = nullptr;
        RegionComboBox *mDstRegionCombo = nullptr;

        StationSelectButton *mSrcStationBtn = nullptr;
        StationSelectButton *mDstStationBtn = nullptr;
    };
}
