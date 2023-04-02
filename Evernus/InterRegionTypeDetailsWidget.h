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

#include <QWidget>

#include "MarketHistory.h"

namespace Evernus
{
    class DoubleTypeAggregatedDetailsWidget;
    class DoubleTypeCompareWidget;

    class InterRegionTypeDetailsWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        InterRegionTypeDetailsWidget(MarketHistory firstHistory,
                                     MarketHistory secondHistory,
                                     const QString &firstInfo,
                                     const QString &secondInfo,
                                     QWidget *parent = nullptr,
                                     Qt::WindowFlags flags = 0);
        InterRegionTypeDetailsWidget(const InterRegionTypeDetailsWidget &) = default;
        InterRegionTypeDetailsWidget(InterRegionTypeDetailsWidget &&) = default;
        virtual ~InterRegionTypeDetailsWidget() = default;

        InterRegionTypeDetailsWidget &operator =(const InterRegionTypeDetailsWidget &) = default;
        InterRegionTypeDetailsWidget &operator =(InterRegionTypeDetailsWidget &&) = default;

    public slots:
        void handleNewPreferences();

    private:
        DoubleTypeAggregatedDetailsWidget *mAggregatedDetailsWidget = nullptr;
        DoubleTypeCompareWidget *mCompareWidget = nullptr;
    };
}
