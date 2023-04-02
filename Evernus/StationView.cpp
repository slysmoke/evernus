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
#include "StationView.h"

namespace Evernus
{
    StationView::StationView(const EveDataProvider &dataProvider, QWidget *parent)
        : QColumnView{parent}
        , mStationModel{dataProvider}
    {
        setModel(&mStationModel);

        auto dummyWidget = new QWidget{this};
        dummyWidget->setMaximumSize(0, 0);
        dummyWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setPreviewWidget(dummyWidget);
        connect(this, &StationView::updatePreviewWidget, [dummyWidget] {
            dummyWidget->parentWidget()->parentWidget()->setMinimumWidth(0);
            dummyWidget->parentWidget()->parentWidget()->setMaximumWidth(0);
        });

        connect(selectionModel(), &QItemSelectionModel::currentChanged,
                this, &StationView::selectStation);
    }

    QVariantList StationView::getSelectedPath() const
    {
        QVariantList path;

        auto current = selectionModel()->currentIndex();
        while (current.isValid())
        {
            path.prepend(mStationModel.getGenericId(current));
            current = mStationModel.parent(current);
        }

        return path;
    }

    void StationView::selectPath(const QVariantList &path)
    {
        if (path.size() == 4)
        {
            QModelIndex index;
            for (const auto &element : path)
                index = mStationModel.index(element.value<quint64>(), index);

            setCurrentIndex(index);
        }
    }

    quint64 StationView::getStationId() const noexcept
    {
        return mCurrentStationId;
    }

    void StationView::selectStation(const QModelIndex &index)
    {
        mCurrentStationId = mStationModel.getStationId(index);
        emit stationChanged(mCurrentStationId);
    }
}
