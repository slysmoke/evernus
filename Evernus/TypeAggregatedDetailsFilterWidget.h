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
#include <QString>
#include <QDate>

#include "VolumeType.h"

class QDateEdit;
class QComboBox;
class QSpinBox;

namespace Evernus
{
    class TypeAggregatedDetailsFilterWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit TypeAggregatedDetailsFilterWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
        virtual ~TypeAggregatedDetailsFilterWidget() = default;

        QDate getFrom() const;
        QDate getTo() const;

        int getSMADays() const;

        int getMACDFastDays() const;
        int getMACDSlowDays() const;
        int getMACDEMADays() const;

        VolumeType getVolumeType() const;

    signals:
        void addTrendLine(const QDate &start, const QDate &end);
        void showLegend(bool flag);
        void applyFilter(const QDate &start,
                         const QDate &end,
                         int smaDays,
                         int macdFastDays,
                         int macdSlowDays,
                         int macdEmaDays,
                         VolumeType volumeType);

    private:
        QDateEdit *mFromEdit = nullptr;
        QDateEdit *mToEdit = nullptr;
        QSpinBox *mSMADaysEdit = nullptr;
        QSpinBox *mMACDFastDaysEdit = nullptr;
        QSpinBox *mMACDSlowDaysEdit = nullptr;
        QSpinBox *mMACDEMADaysEdit = nullptr;
        QComboBox *mVolumeTypeEdit = nullptr;
    };
}
