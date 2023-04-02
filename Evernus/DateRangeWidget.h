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

class QDateEdit;
class QDate;

namespace Evernus
{
    class DateRangeWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit DateRangeWidget(QWidget *parent = nullptr);
        virtual ~DateRangeWidget() = default;

        QDate getFrom() const;
        QDate getTo() const;

        void setRange(const QDate &from, const QDate &to);

    signals:
        void rangeChanged(const QDate &from, const QDate &to);

    private slots:
        void fromChanged(const QDate &date);
        void toChanged(const QDate &date);

    private:
        QDateEdit *mFromEdit = nullptr;
        QDateEdit *mToEdit = nullptr;
    };
}
