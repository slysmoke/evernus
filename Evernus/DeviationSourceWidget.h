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

#include "ExternalOrderModel.h"

class QDoubleSpinBox;
class QRadioButton;

namespace Evernus
{
    class DeviationSourceWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit DeviationSourceWidget(QWidget *parent = nullptr);
        virtual ~DeviationSourceWidget() = default;

        ExternalOrderModel::DeviationSourceType getCurrentType() const noexcept;
        double getCurrentValue() const;

        void setDeviationValue(double value);

    signals:
        void sourceChanged(ExternalOrderModel::DeviationSourceType type, double value);

    private slots:
        void typeChanged(bool checked);
        void valueChanged(double value);

    private:
        static const char * const typePropertyName;

        QRadioButton *mFixedValueBtn = nullptr;
        QDoubleSpinBox *mPriceEdit = nullptr;
        ExternalOrderModel::DeviationSourceType mCurrentType = ExternalOrderModel::DeviationSourceType::Median;

        QRadioButton *createTypeButton(const QString &text, ExternalOrderModel::DeviationSourceType type);
    };
}
