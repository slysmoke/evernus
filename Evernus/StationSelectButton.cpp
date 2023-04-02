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
#include "StationSelectDialog.h"
#include "EveDataProvider.h"

#include "StationSelectButton.h"

namespace Evernus
{
    StationSelectButton::StationSelectButton(const EveDataProvider &dataProvider,
                                             QVariantList initialStationPath,
                                             QWidget *parent)
        : QPushButton{(EveDataProvider::getStationIdFromPath(initialStationPath) != 0) ?
                      (dataProvider.getLocationName(EveDataProvider::getStationIdFromPath(initialStationPath))) :
                      (tr("- any station -")), parent}
        , mDataProvider{dataProvider}
        , mStationPath(std::move(initialStationPath))
    {
        connect(this, &StationSelectButton::clicked, this, &StationSelectButton::selectStation);
    }

    StationSelectButton::StationSelectButton(const EveDataProvider &dataProvider, QWidget *parent)
        : QPushButton{tr("- any station -"), parent}
        , mDataProvider{dataProvider}
    {
        connect(this, &StationSelectButton::clicked, this, &StationSelectButton::selectStation);
    }

    quint64 StationSelectButton::getSelectedStationId() const
    {
        return EveDataProvider::getStationIdFromPath(mStationPath);
    }

    void StationSelectButton::setSelectedStationId(quint64 id)
    {
        if (id == 0)
        {
            setText(tr("- any station -"));

            mStationPath.clear();
            emit stationChanged(mStationPath);

            return;
        }

        const auto solarSystemId = mDataProvider.getStationSolarSystemId(id);
        const auto constellationId = mDataProvider.getSolarSystemConstellationId(solarSystemId);
        const auto regionId = mDataProvider.getSolarSystemRegionId(solarSystemId);

        mStationPath = {
            regionId,
            constellationId,
            solarSystemId,
            id
        };

        setStationNameText(id);

        emit stationChanged(mStationPath);
    }

    QVariantList StationSelectButton::getSelectedStationPath() const
    {
        return mStationPath;
    }

    void StationSelectButton::selectStation()
    {
        StationSelectDialog dlg{mDataProvider, true, this};

        dlg.selectPath(mStationPath);
        if (dlg.exec() == QDialog::Accepted)
        {
            const auto id = dlg.getStationId();
            if (id == 0)
            {
                mStationPath.clear();
                emit stationChanged({});
            }
            else
            {
                mStationPath = dlg.getSelectedPath();
                emit stationChanged(mStationPath);
            }

            setStationNameText(id);
        }
    }

    void StationSelectButton::setStationNameText(quint64 id)
    {
        if (id == 0)
            setText(tr("- any station -"));
        else
            setText(mDataProvider.getLocationName(id));
    }
}
