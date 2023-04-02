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
#include <QHBoxLayout>
#include <QLabel>

#include "FavoriteLocationsButton.h"
#include "StationSelectButton.h"

#include "SourceDestinationSelectWidget.h"

namespace Evernus
{
    SourceDestinationSelectWidget::SourceDestinationSelectWidget(const QVariantList &srcStationPath,
                                                                 const QVariantList &dstStationPath,
                                                                 const QString &srcRegionsSettingsKey,
                                                                 const QString &dstRegionsSettingsKey,
                                                                 const EveDataProvider &dataProvider,
                                                                 const RegionStationPresetRepository &regionStationPresetRepository,
                                                                 QWidget *parent)
        : QWidget{parent}
    {
        const auto mainLayout = new QHBoxLayout{this};
        mainLayout->setContentsMargins(0, 0, 0, 0);

        mainLayout->addWidget(new QLabel{tr("Source:"), this});
        mSrcRegionCombo = new RegionComboBox{dataProvider, srcRegionsSettingsKey, this};
        mainLayout->addWidget(mSrcRegionCombo);

        mSrcStationBtn = new StationSelectButton{dataProvider, srcStationPath, this};
        mainLayout->addWidget(mSrcStationBtn);
        connect(mSrcStationBtn, &StationSelectButton::stationChanged,
                this, &SourceDestinationSelectWidget::changeStations);

        mainLayout->addWidget(new QLabel{tr("Destination:"), this});
        mDstRegionCombo = new RegionComboBox{dataProvider, dstRegionsSettingsKey, this};
        mainLayout->addWidget(mDstRegionCombo);

        mDstStationBtn = new StationSelectButton{dataProvider, dstStationPath, this};
        mainLayout->addWidget(mDstStationBtn);
        connect(mDstStationBtn, &StationSelectButton::stationChanged,
                this, &SourceDestinationSelectWidget::changeStations);

        const auto locationFavBtn = new FavoriteLocationsButton{regionStationPresetRepository, dataProvider, this};
        mainLayout->addWidget(locationFavBtn);
        connect(locationFavBtn, &FavoriteLocationsButton::locationsChosen,
                this, &SourceDestinationSelectWidget::changeLocations);
    }

    SourceDestinationSelectWidget::RegionList SourceDestinationSelectWidget::getSrcSelectedRegionList() const
    {
        return mSrcRegionCombo->getSelectedRegionList();
    }

    SourceDestinationSelectWidget::RegionList SourceDestinationSelectWidget::getDstSelectedRegionList() const
    {
        return mDstRegionCombo->getSelectedRegionList();
    }

    void SourceDestinationSelectWidget::changeStations()
    {
        emit stationsChanged(mSrcStationBtn->getSelectedStationPath(), mDstStationBtn->getSelectedStationPath());
    }

    void SourceDestinationSelectWidget::changeLocations(const RegionList &srcRegions, quint64 srcStationId,
                                                        const RegionList &dstRegions, quint64 dstStationId)
    {
        mSrcRegionCombo->setSelectedRegionList(srcRegions);
        mDstRegionCombo->setSelectedRegionList(dstRegions);

        mSrcStationBtn->blockSignals(true); // prevent double signal
        mSrcStationBtn->setSelectedStationId(srcStationId);
        mSrcStationBtn->blockSignals(false);

        mDstStationBtn->setSelectedStationId(dstStationId);
    }
}
