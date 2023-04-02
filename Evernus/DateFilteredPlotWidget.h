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

class QCustomPlot;
class QDateEdit;
class QDate;

namespace Evernus
{
    class DateFilteredPlotWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit DateFilteredPlotWidget(QWidget *parent = nullptr);
        virtual ~DateFilteredPlotWidget() = default;

        QDate getFrom() const;
        void setFrom(const QDate &date);

        QDate getTo() const;
        void setTo(const QDate &date);

        QCustomPlot &getPlot() const;

        QString getDateTimeFormat() const;
        void setDateTimeFormat(const QString &format);

    signals:
        void filterChanged();

        void mouseMove(QMouseEvent *event);

    public slots:
        void saveBalancePlot();

    private slots:
        void fromChanged(const QDate &date);
        void toChanged(const QDate &date);

        void showLabels(int state);

    private:
        QDateEdit *mFromEdit = nullptr;
        QDateEdit *mToEdit = nullptr;

        QCustomPlot *mPlot = nullptr;
    };
}
