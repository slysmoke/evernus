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
#include <QStandardItemModel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>

#include "EveDataProvider.h"

#include "ItemTypeSelectDialog.h"

namespace Evernus
{
    ItemTypeSelectDialog::ItemTypeSelectDialog(const EveDataProvider &dataProvider, QWidget *parent)
        : QDialog(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

        mTypeCombo = new QComboBox{this};
        mainLayout->addWidget(mTypeCombo);

        auto model = new QStandardItemModel{this};

        const auto &types = dataProvider.getAllTradeableTypeNames();
        for (const auto &type : types)
        {
            auto item = new QStandardItem{type.second};
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(Qt::Unchecked);
            item->setData(type.first);

            model->appendRow(item);
        }

        model->sort(0);

        if (Q_LIKELY(!types.empty()))
        {
            auto item = model->item(0);
            item->setCheckState(Qt::Checked); // auto check first, because the combo is not visible

            mSelectedTypes.emplace(item->data().template value<TypeSet::value_type>());
        }

        connect(model, &QStandardItemModel::itemChanged, this, [=](const auto item) {
            const auto id = item->data().template value<TypeSet::value_type>();
            if (item->checkState() == Qt::Checked)
                mSelectedTypes.emplace(id);
            else
                mSelectedTypes.erase(id);
        });

        mTypeCombo->setEditable(true);
        mTypeCombo->setInsertPolicy(QComboBox::NoInsert);
        mTypeCombo->setModel(model);

        auto buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
        mainLayout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        setWindowTitle(tr("Select Item Type"));
    }

    ItemTypeSelectDialog::TypeSet ItemTypeSelectDialog::getSelectedTypes() const
    {
        return mSelectedTypes;
    }
}
