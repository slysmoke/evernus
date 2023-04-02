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
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLabel>

#include "DeviationSourceWidget.h"

namespace Evernus
{
    const char * const DeviationSourceWidget::typePropertyName = "type";

    DeviationSourceWidget::DeviationSourceWidget(QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

        mainLayout->addWidget(new QLabel{tr("Calculate deviation from:"), this});

        auto radio = createTypeButton(tr("Median price"), ExternalOrderModel::DeviationSourceType::Median);
        mainLayout->addWidget(radio);
        radio->blockSignals(true);
        radio->setChecked(true);
        radio->blockSignals(false);
        radio->setToolTip(tr("Uses median price of displayed orders as reference."));

        radio = createTypeButton(tr("Best buy/sell price"), ExternalOrderModel::DeviationSourceType::Best);
        mainLayout->addWidget(radio);
        radio->setToolTip(tr("Uses lowest sell order for buy orders or highest buy order for sell orders in the order station as reference."));

        radio = createTypeButton(tr("Custom cost"), ExternalOrderModel::DeviationSourceType::Cost);
        mainLayout->addWidget(radio);
        radio->setToolTip(tr("Uses custom item cost as reference."));

        mFixedValueBtn = createTypeButton(tr("Fixed price"), ExternalOrderModel::DeviationSourceType::Fixed);
        mainLayout->addWidget(mFixedValueBtn);
        mFixedValueBtn->setToolTip(tr("Uses given fixed value as reference."));

        mPriceEdit = new QDoubleSpinBox{this};
        mainLayout->addWidget(mPriceEdit);
        mPriceEdit->setSuffix("ISK");
        mPriceEdit->setMinimum(1.);
        mPriceEdit->setMaximum(1000000000000.);
        mPriceEdit->setEnabled(false);
        connect(mPriceEdit, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &DeviationSourceWidget::valueChanged);

        connect(mFixedValueBtn, &QRadioButton::toggled, mPriceEdit, &QDoubleSpinBox::setEnabled);
    }

    ExternalOrderModel::DeviationSourceType DeviationSourceWidget::getCurrentType() const noexcept
    {
        return mCurrentType;
    }

    double DeviationSourceWidget::getCurrentValue() const
    {
        return mPriceEdit->value();
    }

    void DeviationSourceWidget::setDeviationValue(double value)
    {
        mPriceEdit->blockSignals(true);
        mPriceEdit->setValue(value);
        mPriceEdit->blockSignals(false);

        if (mFixedValueBtn->isChecked())
            emit sourceChanged(ExternalOrderModel::DeviationSourceType::Fixed, value);
        else
            mFixedValueBtn->setChecked(true);
    }

    void DeviationSourceWidget::typeChanged(bool checked)
    {
        if (checked)
        {
            mCurrentType = static_cast<ExternalOrderModel::DeviationSourceType>(sender()->property(typePropertyName).toInt());
            emit sourceChanged(mCurrentType, mPriceEdit->value());
        }
    }

    void DeviationSourceWidget::valueChanged(double value)
    {
        emit sourceChanged(mCurrentType, value);
    }

    QRadioButton *DeviationSourceWidget::createTypeButton(const QString &text, ExternalOrderModel::DeviationSourceType type)
    {
        auto radioBtn = new QRadioButton{text, this};
        radioBtn->setProperty(typePropertyName, static_cast<int>(type));
        connect(radioBtn, &QRadioButton::toggled, this, &DeviationSourceWidget::typeChanged);

        return radioBtn;
    }
}
