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
#include <QMessageBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QLocale>
#include <QLabel>

#include "qcustomplot.h"

#include "DateFilteredPlotWidget.h"

namespace Evernus
{
    DateFilteredPlotWidget::DateFilteredPlotWidget(QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

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

        auto labelBtn = new QCheckBox{tr("Show time labels"), this};
        filterLayout->addWidget(labelBtn);
        labelBtn->setChecked(true);
        connect(labelBtn, &QCheckBox::stateChanged, this, &DateFilteredPlotWidget::showLabels);

        auto legendBtn = new QCheckBox{tr("Show legend"), this};
        filterLayout->addWidget(legendBtn);
        legendBtn->setChecked(true);

        auto saveBtn = new QPushButton{QIcon{":/images/image.png"}, tr("Save..."), this};
        filterLayout->addWidget(saveBtn);
        saveBtn->setFlat(true);
        connect(saveBtn, &QPushButton::clicked, this, &DateFilteredPlotWidget::saveBalancePlot);

        filterLayout->addStretch();

        QSharedPointer<QCPAxisTickerDateTime> xTicker{new QCPAxisTickerDateTime{}};
        xTicker->setDateTimeFormat(locale().dateFormat(QLocale::NarrowFormat));

        mPlot = new QCustomPlot{this};
        mainLayout->addWidget(mPlot);
        mPlot->setMinimumHeight(300);
        mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        mPlot->legend->setVisible(true);
        mPlot->xAxis->setTickLabelRotation(60);
        mPlot->xAxis->setTicker(std::move(xTicker));
        connect(mPlot, &QCustomPlot::mouseMove, this, &DateFilteredPlotWidget::mouseMove);
        connect(legendBtn, &QCheckBox::stateChanged, this, [this](bool checked) {
            mPlot->legend->setVisible(checked);
            mPlot->replot();
        });

        auto locale = mPlot->locale();
        locale.setNumberOptions(0);
        mPlot->setLocale(locale);
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

    QString DateFilteredPlotWidget::getDateTimeFormat() const
    {
        return qSharedPointerCast<QCPAxisTickerDateTime>(mPlot->xAxis->ticker())->dateTimeFormat();
    }

    void DateFilteredPlotWidget::setDateTimeFormat(const QString &format)
    {
        qSharedPointerCast<QCPAxisTickerDateTime>(mPlot->xAxis->ticker())->setDateTimeFormat(format);
    }

    void DateFilteredPlotWidget::saveBalancePlot()
    {
        const auto file = QFileDialog::getSaveFileName(this, tr("Save plot"), QString{}, tr("Images (*.png *.jpg *.jpeg *.bmp *.ppm *.xbm *.xpm)"));
        if (file.isEmpty())
            return;

        if (!mPlot->saveRastered(file, width(), height(), 1., nullptr))
            QMessageBox::warning(this, tr("Error"), tr("Error saving image."));
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

    void DateFilteredPlotWidget::showLabels(int state)
    {
        mPlot->xAxis->setTickLabels(state == Qt::Checked);
        mPlot->replot();
    }
}
