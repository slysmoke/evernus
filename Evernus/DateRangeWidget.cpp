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
#include <QHBoxLayout>
#include <QPushButton>
#include <QDateEdit>
#include <QLabel>
#include <QMenu>

#include "DateRangeWidget.h"

namespace Evernus
{
    DateRangeWidget::DateRangeWidget(QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QHBoxLayout{this};
        mainLayout->setContentsMargins(QMargins{});

        mainLayout->addWidget(new QLabel{tr("From:"), this});

        mFromEdit = new QDateEdit{this};
        mainLayout->addWidget(mFromEdit);
        mFromEdit->setCalendarPopup(true);
        connect(mFromEdit, &QDateEdit::dateChanged, this, &DateRangeWidget::fromChanged);

        mainLayout->addWidget(new QLabel{tr("To:"), this});

        mToEdit = new QDateEdit{this};
        mainLayout->addWidget(mToEdit);
        mToEdit->setCalendarPopup(true);
        connect(mToEdit, &QDateEdit::dateChanged, this, &DateRangeWidget::toChanged);

        auto presetBtn = new QPushButton{QIcon{":/images/calendar_view_month.png"}, tr("Quick date  "), this};
        mainLayout->addWidget(presetBtn);
        presetBtn->setFlat(true);

        auto presetMenu = new QMenu{this};
        connect(presetMenu->addAction(tr("Today")), &QAction::triggered, this, [this] {
            const auto date = QDate::currentDate();
            setRange(date, date);

            emit rangeChanged(date, date);
        });
        connect(presetMenu->addAction(tr("Past day")), &QAction::triggered, this, [this] {
            const auto date = QDate::currentDate().addDays(-1);
            setRange(date, date);

            emit rangeChanged(date, date);
        });
        connect(presetMenu->addAction(tr("This week")), &QAction::triggered, this, [this] {
            const auto to = QDate::currentDate();
            const auto from = to.addDays(-to.dayOfWeek() + 1);
            setRange(from, to);

            emit rangeChanged(from, to);
        });
        connect(presetMenu->addAction(tr("Past week")), &QAction::triggered, this, [this] {
            auto to = QDate::currentDate();
            to = to.addDays(-to.dayOfWeek());
            const auto from = to.addDays(-to.dayOfWeek() + 1);
            setRange(from, to);

            emit rangeChanged(from, to);
        });
        connect(presetMenu->addAction(tr("This month")), &QAction::triggered, this, [this] {
            const auto to = QDate::currentDate();
            const auto from = to.addDays(-to.day() + 1);
            setRange(from, to);

            emit rangeChanged(from, to);
        });
        connect(presetMenu->addAction(tr("Past month")), &QAction::triggered, this, [this] {
            auto to = QDate::currentDate();
            to = to.addDays(-to.day());
            const auto from = to.addDays(-to.day() + 1);
            setRange(from, to);

            emit rangeChanged(from, to);
        });

        presetBtn->setMenu(presetMenu);
    }

    QDate DateRangeWidget::getFrom() const
    {
        return mFromEdit->date();
    }

    QDate DateRangeWidget::getTo() const
    {
        return mToEdit->date();
    }

    void DateRangeWidget::setRange(const QDate &from, const QDate &to)
    {
        mFromEdit->blockSignals(true);
        mToEdit->blockSignals(true);

        mFromEdit->setDate(from);
        mToEdit->setDate(to);

        mFromEdit->blockSignals(false);
        mToEdit->blockSignals(false);
    }

    void DateRangeWidget::fromChanged(const QDate &date)
    {
        if (date > mToEdit->date())
            mToEdit->setDate(date.addDays(1));
        else
            emit rangeChanged(date, mToEdit->date());
    }

    void DateRangeWidget::toChanged(const QDate &date)
    {
        if (date < mFromEdit->date())
            mFromEdit->setDate(date.addDays(-1));
        else
            emit rangeChanged(mFromEdit->date(), date);
    }
}
