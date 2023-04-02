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

#include "TypeAggregatedDetailsFilterWidget.h"
#include "TypeAggregatedGraphWidget.h"

#include "TypeAggregatedDetailsWidget.h"

namespace Evernus
{
    TypeAggregatedDetailsWidget::TypeAggregatedDetailsWidget(History history, QWidget *parent, Qt::WindowFlags flags)
        : SizeRememberingWidget(QStringLiteral("typeAggregatedDetailsWidget/size"), parent, flags)
    {
        auto mainLayout = new QVBoxLayout{this};

        mFilterWidget = new TypeAggregatedDetailsFilterWidget{this};
        mainLayout->addWidget(mFilterWidget);

        mGraphWidget = new TypeAggregatedGraphWidget{std::move(history), this};
        mainLayout->addWidget(mGraphWidget);
        connect(mFilterWidget, &TypeAggregatedDetailsFilterWidget::addTrendLine, mGraphWidget, &TypeAggregatedGraphWidget::addTrendLine);
        connect(mFilterWidget, &TypeAggregatedDetailsFilterWidget::applyFilter, mGraphWidget, &TypeAggregatedGraphWidget::applyFilter);
        connect(mFilterWidget, &TypeAggregatedDetailsFilterWidget::showLegend, mGraphWidget, &TypeAggregatedGraphWidget::showLegend);
        mGraphWidget->applyFilter(mFilterWidget->getFrom(),
                                  mFilterWidget->getTo(),
                                  mFilterWidget->getSMADays(),
                                  mFilterWidget->getMACDFastDays(),
                                  mFilterWidget->getMACDSlowDays(),
                                  mFilterWidget->getMACDEMADays(),
                                  mFilterWidget->getVolumeType());
    }

    void TypeAggregatedDetailsWidget::handleNewPreferences()
    {
        mGraphWidget->handleNewPreferences();
    }
}
