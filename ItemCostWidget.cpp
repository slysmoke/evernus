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
#include <QDoubleValidator>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSettings>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>

#include "ItemCostEditDialog.h"
#include "ItemCostProvider.h"
#include "StyledTreeView.h"
#include "ImportSettings.h"
#include "PriceSettings.h"

#include "qxtcsvmodel.h"

#include "ItemCostWidget.h"

namespace Evernus
{
    ItemCostWidget::ItemCostWidget(const ItemCostProvider &costProvider,
                                   const EveDataProvider &eveDataProvider,
                                   QWidget *parent)
        : QWidget{parent}
        , mCostProvider{costProvider}
        , mEveDataProvider{eveDataProvider}
        , mModel{mCostProvider, mEveDataProvider}
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolBarLayout);

        mAddBtn = new QPushButton{QIcon{QStringLiteral(":/images/add.png")}, tr("Add..."), this};
        toolBarLayout->addWidget(mAddBtn);
        mAddBtn->setFlat(true);
        connect(mAddBtn, &QPushButton::clicked, this, &ItemCostWidget::addCost);

        mEditBtn = new QPushButton{QIcon{QStringLiteral(":/images/pencil.png")}, tr("Edit..."), this};
        toolBarLayout->addWidget(mEditBtn);
        mEditBtn->setFlat(true);
        mEditBtn->setDisabled(true);
        connect(mEditBtn, &QPushButton::clicked, this, &ItemCostWidget::editCost);

        mRemoveBtn = new QPushButton{QIcon{QStringLiteral(":/images/delete.png")}, tr("Remove"), this};
        toolBarLayout->addWidget(mRemoveBtn);
        mRemoveBtn->setFlat(true);
        mRemoveBtn->setDisabled(true);
        connect(mRemoveBtn, &QPushButton::clicked, this, &ItemCostWidget::deleteCost);

        auto deleteAllBtn = new QPushButton{QIcon{QStringLiteral(":/images/cross.png")}, tr("Remove all"), this};
        toolBarLayout->addWidget(deleteAllBtn);
        deleteAllBtn->setFlat(true);
        connect(deleteAllBtn, &QPushButton::clicked, this, &ItemCostWidget::deleteAllCost);

        const auto importMenu = new QMenu{this};
        importMenu->addAction(tr("Import CSV..."), this, &ItemCostWidget::importCsv);
        importMenu->addAction(tr("Export CSV..."), this, &ItemCostWidget::exportCsv);

        const auto importBtn = new QPushButton{QIcon{QStringLiteral(":/images/page_excel.png")}, tr("Import/export  "), this};
        toolBarLayout->addWidget(importBtn);
        importBtn->setFlat(true);
        importBtn->setMenu(importMenu);

        auto costValidator = new QDoubleValidator{this};
        QSettings settings;

        toolBarLayout->addWidget(new QLabel{tr("Constant added cost:"), this});

        auto constCostEdit = new QLineEdit{
            settings.value(PriceSettings::itemConstCostAddKey, PriceSettings::itemConstCostAddDefault).toString(), this};
        toolBarLayout->addWidget(constCostEdit);
        constCostEdit->setValidator(costValidator);
        connect(constCostEdit, &QLineEdit::textEdited, this, [](const auto &text) {
            QSettings settings;
            settings.setValue(PriceSettings::itemConstCostAddKey, text.toDouble());
        });

        toolBarLayout->addWidget(new QLabel{tr("Relative added cost:"), this});

        auto relCostEdit = new QDoubleSpinBox{this};
        relCostEdit->setMinimum(std::numeric_limits<double>::lowest());
        toolBarLayout->addWidget(relCostEdit);
        relCostEdit->setValue(settings.value(PriceSettings::itemRelativeCostAddKey, PriceSettings::itemRelativeCostAddDefault).toDouble());
        relCostEdit->setSuffix(locale().percent());
        connect(relCostEdit, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [](auto value) {
            QSettings settings;
            settings.setValue(PriceSettings::itemRelativeCostAddKey, value);
        });

        mFilterEdit = new QLineEdit{this};
        toolBarLayout->addWidget(mFilterEdit, 1);
		
        mFilterEdit->setPlaceholderText(tr("type in wildcard and press Enter"));
        mFilterEdit->setClearButtonEnabled(true);
        connect(mFilterEdit, &QLineEdit::returnPressed, this, &ItemCostWidget::applyWildcard);

        mProxy.setSortRole(Qt::UserRole);
        mProxy.setSourceModel(&mModel);
        mProxy.setFilterCaseSensitivity(Qt::CaseInsensitive);

        mView = new StyledTreeView{QStringLiteral("itemCostView"), this};
        mainLayout->addWidget(mView, 1);
        mView->setModel(&mProxy);
        connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &ItemCostWidget::selectCost);
    }

    void ItemCostWidget::setCharacter(Character::IdType id)
    {
        mCharacterId = id;
        mModel.setCharacter(mCharacterId);

        mView->header()->resizeSections(QHeaderView::ResizeToContents);

        mAddBtn->setDisabled(mCharacterId == Character::invalidId);
        mEditBtn->setDisabled(true);
        mRemoveBtn->setDisabled(true);
    }

    void ItemCostWidget::updateData()
    {
        if (mBlockUpdate)
            return;

        mModel.reset();
        mView->header()->resizeSections(QHeaderView::ResizeToContents);

        mEditBtn->setDisabled(true);
        mRemoveBtn->setDisabled(true);
    }

    void ItemCostWidget::addCost()
    {
        ItemCost cost;
        cost.setCharacterId(mCharacterId);

        showCostEditDialog(cost);
    }

    void ItemCostWidget::editCost()
    {
        Q_ASSERT(mSelectedCosts.count() > 0);

        const auto index = mProxy.mapToSource(mSelectedCosts.first());
        const auto id = mModel.getId(index.row());

        try
        {
            auto cost = mCostProvider.findItemCost(id);
            showCostEditDialog(*cost);
        }
        catch (const ItemCostProvider::NotFoundException &)
        {
        }
    }

    void ItemCostWidget::deleteCost()
    {
        Q_ASSERT(mSelectedCosts.count() > 0);

        mBlockUpdate = true;

        try
        {
            for (const auto &index : mSelectedCosts)
            {
                const auto realIndex = mProxy.mapToSource(index);
                if (realIndex.column() != 0)
                    continue;

                const auto id = mModel.getId(realIndex.row());
                mCostProvider.removeItemCost(id);
            }
        }
        catch (...)
        {
            mBlockUpdate = false;
            updateData();

            throw;
        }

        mBlockUpdate = false;
        updateData();
    }

    void ItemCostWidget::deleteAllCost()
    {
        mCostProvider.removeAllItemCosts(mCharacterId);
    }

    void ItemCostWidget::selectCost(const QItemSelection &selected, const QItemSelection &deselected)
    {
        Q_UNUSED(selected);
        Q_UNUSED(deselected);

        mEditBtn->setDisabled(selected.isEmpty());
        mRemoveBtn->setDisabled(selected.isEmpty());

        mSelectedCosts = mView->selectionModel()->selectedIndexes();
    }

    void ItemCostWidget::applyWildcard()
    {
        mProxy.setFilterWildcard(mFilterEdit->text());
    }

    void ItemCostWidget::importCsv()
    {
        QSettings settings;

        const auto fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open file"),
            settings.value(ImportSettings::itemPricesFileDirKey).toString(),
            tr("CSV (*.csv)")
        );

        if (fileName.isEmpty())
            return;

        QFileInfo info{fileName};
        settings.setValue(ImportSettings::itemPricesFileDirKey, info.dir().path());

        QxtCsvModel model{fileName, nullptr, false, getCsvSeparator()};
        if (model.columnCount() < 2)
        {
            QMessageBox::information(this, tr("CSV import"), tr("CSV file must have at least two columns: item id and value."));
            return;
        }

        const auto rows = model.rowCount();
        for (auto row = 0; row < rows; ++row)
        {
            const auto typeId = model.data(model.index(row, 0)).value<EveType::IdType>();
            if (typeId == 0)
                continue;

            ItemCost cost;
            cost.setCharacterId(mCharacterId);
            cost.setTypeId(typeId);
            cost.setCost(model.data(model.index(row, 1)).toDouble());

            mCostProvider.storeItemCost(cost);
        }
    }

    void ItemCostWidget::exportCsv()
    {
        QSettings settings;

        const auto fileName = QFileDialog::getSaveFileName(
            this,
            tr("Open file"),
            settings.value(ImportSettings::itemPricesFileDirKey).toString(),
            tr("CSV (*.csv)")
        );

        if (fileName.isEmpty())
            return;

        QFileInfo info{fileName};
        settings.setValue(ImportSettings::itemPricesFileDirKey, info.dir().path());

        const auto rows = mModel.rowCount();

        QxtCsvModel model;
        model.insertRows(0, rows);
        model.insertColumns(0, 2);

        for (auto row = 0; row < rows; ++row)
        {
            model.setData(model.index(row, 0), mModel.getTypeId(row));
            model.setData(model.index(row, 1), mModel.getCost(row));
        }

        model.toCSV(fileName, false, getCsvSeparator());
    }

    void ItemCostWidget::showCostEditDialog(ItemCost &cost)
    {
        ItemCostEditDialog dlg{cost, mEveDataProvider, this};
        if (dlg.exec() == QDialog::Accepted)
            mCostProvider.storeItemCost(cost);
    }

    QChar ItemCostWidget::getCsvSeparator()
    {
        QSettings settings;

        auto separator = settings.value(ImportSettings::csvSeparatorKey, ImportSettings::csvSeparatorDefault).toString();
        if (separator.length() != 1)
            separator = ImportSettings::csvSeparatorDefault;

        return separator[0];
    }
}
