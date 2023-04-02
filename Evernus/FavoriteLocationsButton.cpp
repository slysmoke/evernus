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
#include <QIcon>

#include "RegionStationPresetDialog.h"

#include "FavoriteLocationsButton.h"

namespace Evernus
{
    FavoriteLocationsButton::FavoriteLocationsButton(const RegionStationPresetRepository &regionStationPresetRepository,
                                                     const EveDataProvider &dataProvider,
                                                     QWidget *parent)
        : QPushButton{parent}
        , mRegionStationPresetRepository{regionStationPresetRepository}
        , mDataProvider{dataProvider}
    {
        setIcon(QIcon{QStringLiteral(":/images/star.png")});
        setToolTip(tr("Select favorite locations."));
        connect(this, &FavoriteLocationsButton::clicked, this, &FavoriteLocationsButton::selectFavoriteLocations);
    }

    void FavoriteLocationsButton::selectFavoriteLocations()
    {
        RegionStationPresetDialog dlg{mRegionStationPresetRepository, mDataProvider, this};
        if (dlg.exec() == QDialog::Accepted)
            emit locationsChosen(dlg.getSrcRegions(), dlg.getSrcStationId(), dlg.getDstRegions(), dlg.getDstStationId());
    }
}
