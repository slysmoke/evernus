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

#include "MarketOrderFilterProxyModel.h"

class QCheckBox;

namespace Evernus
{
    class MarketOrderPriceStatusesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit MarketOrderPriceStatusesWidget(QWidget *parent = nullptr);
        virtual ~MarketOrderPriceStatusesWidget() = default;

        MarketOrderFilterProxyModel::PriceStatusFilters getStatusFilter() const noexcept;

    signals:
        void filterChanged(const MarketOrderFilterProxyModel::PriceStatusFilters &filter);

    private slots:
        void changeFilter(int state);

        void checkAll();

    private:
        static const char * const filterPropertyName;

        MarketOrderFilterProxyModel::PriceStatusFilters mCurrentFilter = MarketOrderFilterProxyModel::defaultPriceStatusFilter;

        QCheckBox *createCheckBox(MarketOrderFilterProxyModel::PriceStatusFilter filter, const QString &label);
        void setNewFilter(const MarketOrderFilterProxyModel::PriceStatusFilters &filter);
    };
}
