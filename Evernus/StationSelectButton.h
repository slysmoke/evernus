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

#include <QPushButton>
#include <QVariant>

namespace Evernus
{
    class EveDataProvider;

    class StationSelectButton
        : public QPushButton
    {
        Q_OBJECT

    public:
        StationSelectButton(const EveDataProvider &dataProvider,
                            QVariantList initialStationPath,
                            QWidget *parent = nullptr);
        explicit StationSelectButton(const EveDataProvider &dataProvider,
                                     QWidget *parent = nullptr);
        StationSelectButton(const StationSelectButton &) = default;
        StationSelectButton(StationSelectButton &&) = default;
        virtual ~StationSelectButton() = default;

        quint64 getSelectedStationId() const;
        void setSelectedStationId(quint64 id);

        QVariantList getSelectedStationPath() const;

        StationSelectButton &operator =(const StationSelectButton &) = default;
        StationSelectButton &operator =(StationSelectButton &&) = default;

    signals:
        void stationChanged(const QVariantList &path);

    private slots:
        void selectStation();

    private:
        const EveDataProvider &mDataProvider;

        QVariantList mStationPath;

        void setStationNameText(quint64 id);
    };
}
