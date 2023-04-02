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
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>

#include "ExcelDoubleSpinBox.h"
#include "EveDataProvider.h"
#include "ItemCost.h"

#include "ItemCostEditDialog.h"

namespace Evernus
{
    ItemCostEditDialog::ItemCostEditDialog(ItemCost &cost, const EveDataProvider &dataProvider, QWidget *parent)
        : QDialog{parent}
        , mCost{cost}
    {
        auto mainLayout = new QVBoxLayout{this};

        mTypeCombo = new QComboBox{this};
        mainLayout->addWidget(mTypeCombo);

        const auto &types = dataProvider.getAllTradeableTypeNames();
        for (const auto &type : types)
        {
            const auto index = mTypeCombo->count();
            mTypeCombo->addItem(type.second, type.first);

            if (type.first == cost.getTypeId())
                mTypeCombo->setCurrentIndex(index);
        }

        mTypeCombo->setEditable(true);
        mTypeCombo->setInsertPolicy(QComboBox::NoInsert);
        mTypeCombo->model()->sort(0);
        mTypeCombo->setFocus();

        auto costLayout = new QHBoxLayout{};
        mainLayout->addLayout(costLayout);

        costLayout->addWidget(new QLabel{tr("Cost:"), this});

        mCostEdit = new ExcelDoubleSpinBox{this};
        costLayout->addWidget(mCostEdit);
        mCostEdit->setMaximum(1000000000000.);
        mCostEdit->setSuffix(" ISK");
        mCostEdit->setValue(mCost.getCost());

        costLayout->addStretch();

        auto buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
        mainLayout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        setWindowTitle(tr("Item Cost Edit"));
    }

    void ItemCostEditDialog::accept()
    {
        const auto data = mTypeCombo->currentData();
        if (data.isNull())
            return;

        mCost.setTypeId(data.value<EveType::IdType>());
        mCost.setCost(mCostEdit->value());

        QDialog::accept();
    }
}
