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
#include <QGridLayout>
#include <QLabel>

#include "TypeAggregatedDetailsFilterWidget.h"
#include "TypeAggregatedGraphWidget.h"

#include "DoubleTypeAggregatedDetailsWidget.h"

namespace Evernus
{
    DoubleTypeAggregatedDetailsWidget::DoubleTypeAggregatedDetailsWidget(MarketHistory firstHistory,
                                                                         MarketHistory secondHistory,
                                                                         const QString &firstInfo,
                                                                         const QString &secondInfo,
                                                                         QWidget *parent,
                                                                         Qt::WindowFlags flags)
        : SizeRememberingWidget(QStringLiteral("doubleTypeAggregatedDetailsWidget/size"), parent, flags)
    {
        const auto mainLayout = new QVBoxLayout{this};

        mFilterWidget = new TypeAggregatedDetailsFilterWidget{this};
        mainLayout->addWidget(mFilterWidget);

        const auto graphLayout = new QGridLayout{};
        mainLayout->addLayout(graphLayout);

        const auto createLabel = [=](const auto &text) {
            return new QLabel{QStringLiteral("<b>%1</b>").arg(text), this};
        };

        graphLayout->addWidget(createLabel(firstInfo), 0, 0, Qt::AlignCenter);
        graphLayout->addWidget(createLabel(secondInfo), 0, 1, Qt::AlignCenter);

        const auto createGraphWidget = [=](auto &&history, auto column) {
            const auto widget = new TypeAggregatedGraphWidget{std::move(history), this};
            graphLayout->addWidget(widget, 1, column);
            connect(mFilterWidget, &TypeAggregatedDetailsFilterWidget::addTrendLine, widget, &TypeAggregatedGraphWidget::addTrendLine);
            connect(mFilterWidget, &TypeAggregatedDetailsFilterWidget::applyFilter, widget, &TypeAggregatedGraphWidget::applyFilter);
            connect(mFilterWidget, &TypeAggregatedDetailsFilterWidget::showLegend, widget, &TypeAggregatedGraphWidget::showLegend);
            widget->applyFilter(mFilterWidget->getFrom(),
                                mFilterWidget->getTo(),
                                mFilterWidget->getSMADays(),
                                mFilterWidget->getMACDFastDays(),
                                mFilterWidget->getMACDSlowDays(),
                                mFilterWidget->getMACDEMADays(),
                                mFilterWidget->getVolumeType());

            return widget;
        };

        mFirstGraphWidget = createGraphWidget(std::move(firstHistory), 0);
        mSecondGraphWidget = createGraphWidget(std::move(secondHistory), 1);
    }

    void DoubleTypeAggregatedDetailsWidget::handleNewPreferences()
    {
        mFirstGraphWidget->handleNewPreferences();
        mSecondGraphWidget->handleNewPreferences();
    }
}
