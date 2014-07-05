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
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPushButton>
#include <QDateEdit>
#include <QLocale>
#include <QLabel>

#include "qcustomplot.h"

#include "DateFilteredPlotWidget.h"

namespace Evernus
{
    DateFilteredPlotWidget::DateFilteredPlotWidget(QWidget *parent)
        : QWidget{parent}
    {
        auto mainLayout = new QVBoxLayout{};
        setLayout(mainLayout);

        auto filterLayout = new QHBoxLayout{};
        mainLayout->addLayout(filterLayout);

        filterLayout->addWidget(new QLabel{tr("From:"), this});

        mFromEdit = new QDateEdit{this};
        filterLayout->addWidget(mFromEdit);
        mFromEdit->setCalendarPopup(true);
        connect(mFromEdit, &QDateEdit::dateChanged, this, &DateFilteredPlotWidget::fromChanged);

        filterLayout->addWidget(new QLabel{tr("To:"), this});

        mToEdit = new QDateEdit{this};
        filterLayout->addWidget(mToEdit);
        mToEdit->setCalendarPopup(true);
        connect(mToEdit, &QDateEdit::dateChanged, this, &DateFilteredPlotWidget::toChanged);

        auto saveBtn = new QPushButton{QIcon{":/images/image.png"}, tr("Save..."), this};
        filterLayout->addWidget(saveBtn);
        saveBtn->setFlat(true);
        connect(saveBtn, &QPushButton::clicked, this, &DateFilteredPlotWidget::saveBalancePlot);

        filterLayout->addStretch();

        mPlot = new QCustomPlot{this};
        mainLayout->addWidget(mPlot);
        mPlot->setMinimumHeight(200);
        mPlot->xAxis->setAutoTicks(false);
        mPlot->xAxis->setAutoTickLabels(true);
        mPlot->xAxis->setTickLabelRotation(60);
        mPlot->xAxis->setSubTickCount(0);
        mPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
        mPlot->xAxis->setDateTimeFormat(locale().dateFormat());
    }

    QDate DateFilteredPlotWidget::getFrom() const
    {
        return mFromEdit->date();
    }

    void DateFilteredPlotWidget::setFrom(const QDate &date)
    {
        mFromEdit->setDate(date);
    }

    QDate DateFilteredPlotWidget::getTo() const
    {
        return mToEdit->date();
    }

    void DateFilteredPlotWidget::setTo(const QDate &date)
    {
        mToEdit->setDate(date);
    }

    QCustomPlot &DateFilteredPlotWidget::getPlot() const
    {
        return *mPlot;
    }

    void DateFilteredPlotWidget::saveBalancePlot()
    {
        const auto file = QFileDialog::getSaveFileName(this, tr("Save plot"), QString{}, "Images (*.png *.jpg *.jpeg *.bmp *.ppm *.xbm *.xpm)");
        if (file.isEmpty())
            return;

        mPlot->saveRastered(file, width(), height(), 1., nullptr);
    }

    void DateFilteredPlotWidget::fromChanged(const QDate &date)
    {
        if (date > mToEdit->date())
            mToEdit->setDate(date.addDays(1));
        else
            emit filterChanged();
    }

    void DateFilteredPlotWidget::toChanged(const QDate &date)
    {
        if (date < mFromEdit->date())
            mFromEdit->setDate(date.addDays(-1));
        else
            emit filterChanged();
    }
}