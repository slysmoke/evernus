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

#include <QFrame>

#include "MarketOrderModel.h"

namespace Evernus
{
    class MarketOrderInfoWidget
        : public QFrame
    {
        Q_OBJECT

    public:
        explicit MarketOrderInfoWidget(const MarketOrderModel::OrderInfo &info, QWidget *parent = nullptr);
        virtual ~MarketOrderInfoWidget() = default;

    private slots:
        void setAutoCopy(int state);

        void copyPrice();

    protected:
        virtual bool event(QEvent *event) override;

    private:
        QString mTargetPrice;
#ifdef Q_OS_MAC
        bool mWasDeactivated;
#endif
    };
}
