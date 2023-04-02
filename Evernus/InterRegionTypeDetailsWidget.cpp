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
#include <QVBoxLayout>
#include <QTabWidget>

#include "DoubleTypeAggregatedDetailsWidget.h"
#include "DoubleTypeCompareWidget.h"

#include "InterRegionTypeDetailsWidget.h"

namespace Evernus
{
    InterRegionTypeDetailsWidget::InterRegionTypeDetailsWidget(MarketHistory firstHistory,
                                                               MarketHistory secondHistory,
                                                               const QString &firstInfo,
                                                               const QString &secondInfo,
                                                               QWidget *parent,
                                                               Qt::WindowFlags flags)
        : QWidget(parent, flags)
    {
        const auto mainLayout = new QVBoxLayout{this};

        const auto tabs = new QTabWidget{this};
        mainLayout->addWidget(tabs);

        mAggregatedDetailsWidget = new DoubleTypeAggregatedDetailsWidget{firstHistory,
                                                                         secondHistory,
                                                                         firstInfo,
                                                                         secondInfo,
                                                                         tabs};
        tabs->addTab(mAggregatedDetailsWidget, tr("Individual"));

        mCompareWidget = new DoubleTypeCompareWidget{std::move(firstHistory),
                                                     std::move(secondHistory),
                                                     firstInfo,
                                                     secondInfo,
                                                     tabs};
        tabs->addTab(mCompareWidget, tr("Combined"));
    }

    void InterRegionTypeDetailsWidget::handleNewPreferences()
    {
        mAggregatedDetailsWidget->handleNewPreferences();
        mCompareWidget->handleNewPreferences();
    }
}
