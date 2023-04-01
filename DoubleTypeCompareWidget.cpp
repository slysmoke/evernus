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
#include <QDateTime>
#include <QSettings>
#include <QVector>
#include <QPen>

#include "DateFilteredPlotWidget.h"
#include "UISettings.h"

#include "qcustomplot.h"

#include "DoubleTypeCompareWidget.h"

namespace Evernus
{
    DoubleTypeCompareWidget::DoubleTypeCompareWidget(MarketHistory firstHistory,
                                                     MarketHistory secondHistory,
                                                     const QString &firstInfo,
                                                     const QString &secondInfo,
                                                     QWidget *parent)
        : QWidget{parent}
        , mFirstHistory{std::move(firstHistory)}
        , mSecondHistory{std::move(secondHistory)}
    {
        const auto mainLayout = new QVBoxLayout{this};

        const auto current = QDate::currentDate().addDays(-1);

        QSharedPointer<QCPAxisTickerDateTime> xTicker{new QCPAxisTickerDateTime{}};
        xTicker->setDateTimeFormat(locale().dateFormat(QLocale::NarrowFormat));

        mHistoryPlot = new DateFilteredPlotWidget{this};
        mainLayout->addWidget(mHistoryPlot);
        mHistoryPlot->setTo(current);
        mHistoryPlot->setFrom(current.addDays(-90));
        connect(mHistoryPlot, &DateFilteredPlotWidget::filterChanged,
                this, &DoubleTypeCompareWidget::applyFilter);

        auto &plot = mHistoryPlot->getPlot();
        plot.yAxis->setNumberPrecision(2);
        plot.yAxis->setLabel(tr("Avg. price [ISK]"));

        mFirstPriceGraph = plot.addGraph();
        mFirstPriceGraph->setPen(QPen{Qt::red});
        mFirstPriceGraph->setName(firstInfo);

        mSecondPriceGraph = plot.addGraph();
        mSecondPriceGraph->setPen(QPen{Qt::green});
        mSecondPriceGraph->setName(secondInfo);

        const auto volumeAxisRect = new QCPAxisRect{&plot};
        plot.plotLayout()->addElement(1, 0, volumeAxisRect);
        volumeAxisRect->setRangeDrag(Qt::Horizontal);
        volumeAxisRect->setRangeZoom(Qt::Horizontal);
        volumeAxisRect->axis(QCPAxis::atLeft)->setRange(0., 100.);
        volumeAxisRect->axis(QCPAxis::atLeft)->setLabel(tr("Volume"));

        mVolumeDateAxis = volumeAxisRect->axis(QCPAxis::atBottom);
        mVolumeDateAxis->setLayer("axes");
        mVolumeDateAxis->setTicker(xTicker);
        mVolumeDateAxis->grid()->setLayer("grid");

        connect(plot.xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
                mVolumeDateAxis, QOverload<const QCPRange &>::of(&QCPAxis::setRange));
        connect(mVolumeDateAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
                plot.xAxis, QOverload<const QCPRange &>::of(&QCPAxis::setRange));

        const auto marginGroup = new QCPMarginGroup{&plot};
        plot.axisRect()->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);
        volumeAxisRect->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);

        mFirstVolumeGraph = plot.addGraph(mVolumeDateAxis, volumeAxisRect->axis(QCPAxis::atLeft));
        mFirstVolumeGraph->setPen(QPen{Qt::darkRed});
        mFirstVolumeGraph->removeFromLegend();

        mSecondVolumeGraph = plot.addGraph(mVolumeDateAxis, volumeAxisRect->axis(QCPAxis::atLeft));
        mSecondVolumeGraph->setPen(QPen{Qt::darkGreen});
        mSecondVolumeGraph->removeFromLegend();

        applyGraphFormats();
        applyFilter();
    }

    void DoubleTypeCompareWidget::handleNewPreferences()
    {
        applyGraphFormats();
        mHistoryPlot->getPlot().replot();
    }

    void DoubleTypeCompareWidget::applyFilter()
    {
        const auto start = mHistoryPlot->getFrom();
        const auto end = mHistoryPlot->getTo();
        const auto size = start.daysTo(end) + 1;

        auto &plot = mHistoryPlot->getPlot();

        QVector<double> dates, firstPrices, secondPrices, firstVolumes, secondVolumes;
        dates.reserve(size);
        firstPrices.reserve(size);
        secondPrices.reserve(size);
        firstVolumes.reserve(size);
        secondVolumes.reserve(size);

        for (auto date = start; date <= end; date = date.addDays(1))
        {
            dates << QDateTime{date}.toMSecsSinceEpoch() / 1000.;

            auto it = mFirstHistory.find(date);
            if (it == std::end(mFirstHistory))
            {
                firstPrices << 0.;
                firstVolumes << 0.;
            }
            else
            {
                firstPrices << it->second.mAvgPrice;
                firstVolumes << it->second.mVolume;
            }

            it = mSecondHistory.find(date);
            if (it == std::end(mSecondHistory))
            {
                secondPrices << 0.;
                secondVolumes << 0.;
            }
            else
            {
                secondPrices << it->second.mAvgPrice;
                secondVolumes << it->second.mVolume;
            }
        }

        mFirstPriceGraph->setData(dates, firstPrices);
        mSecondPriceGraph->setData(dates, secondPrices);
        mFirstVolumeGraph->setData(dates, firstVolumes);
        mSecondVolumeGraph->setData(dates, secondVolumes);

        plot.xAxis->rescale();
        plot.yAxis->rescale();
        mFirstVolumeGraph->keyAxis()->rescale();
        mFirstVolumeGraph->valueAxis()->rescale();

        plot.replot();
    }

    void DoubleTypeCompareWidget::applyGraphFormats()
    {
        auto &plot = mHistoryPlot->getPlot();

        QSettings settings;
        plot.yAxis->setNumberFormat(
           settings.value(UISettings::plotNumberFormatKey, UISettings::plotNumberFormatDefault).toString());

        if (settings.value(UISettings::applyDateFormatToGraphsKey, UISettings::applyDateFormatToGraphsDefault).toBool())
        {
            auto ticker = qSharedPointerCast<QCPAxisTickerDateTime>(plot.xAxis->ticker());
            ticker->setDateTimeFormat(settings.value(UISettings::dateTimeFormatKey, ticker->dateTimeFormat()).toString());
            ticker = qSharedPointerCast<QCPAxisTickerDateTime>(mVolumeDateAxis->ticker());
            ticker->setDateTimeFormat(settings.value(UISettings::dateTimeFormatKey, ticker->dateTimeFormat()).toString());
        }
    }
}
