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
#include <unordered_map>

#include <QSortFilterProxyModel>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QMenu>

#include "WalletTransactionsModel.h"
#include "CharacterRepository.h"
#include "ItemCostProvider.h"
#include "PriceUtils.h"
#include "TextUtils.h"

#include "WalletTransactionView.h"

namespace Evernus
{
    WalletTransactionView::WalletTransactionView(ItemCostProvider &itemCostProvider,
                                                 const CharacterRepository &characterRepository,
                                                 QWidget *parent)
        : StyledTreeView{parent}
        , mItemCostProvider{itemCostProvider}
        , mCharacterRepository{characterRepository}
    {
        initialize();
    }

    WalletTransactionView::WalletTransactionView(const QString &objectName,
                                                 ItemCostProvider &itemCostProvider,
                                                 const CharacterRepository &characterRepository,
                                                 QWidget *parent)
        : StyledTreeView{objectName, parent}
        , mItemCostProvider{itemCostProvider}
        , mCharacterRepository{characterRepository}
    {
        initialize();
    }

    void WalletTransactionView::setModel(QAbstractItemModel *model)
    {
        StyledTreeView::setModel(model);

        mModel = dynamic_cast<WalletTransactionsModel *>(model);
        mProxy = nullptr;

        auto selection = selectionModel();
        if (selection != nullptr && mModel != nullptr)
        {
            connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                    this, &WalletTransactionView::selectTransaction);
        }
    }

    void WalletTransactionView::setModels(QAbstractProxyModel *proxy, WalletTransactionsModel *model)
    {
        StyledTreeView::setModel(proxy);

        mModel = model;
        mProxy = proxy;

        auto selection = selectionModel();
        if (selection != nullptr && mModel != nullptr)
        {
            connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                    this, &WalletTransactionView::selectTransaction);
        }
    }

    void WalletTransactionView::setCharacter(Character::IdType id)
    {
        mCharacterId = id;
    }

    void WalletTransactionView::updateCharacters()
    {
        mCharsMenu->clear();

        auto characters = mCharacterRepository.getEnabledQuery();
        while (characters.next())
        {
            auto action = mCharsMenu->addAction(characters.value("name").toString(), this, SLOT(addItemCostForCharacter()));
            action->setData(characters.value("id"));
        }
    }

    void WalletTransactionView::selectTransaction(const QItemSelection &selected)
    {
        if (selected.isEmpty())
        {
            mCurrentTransaction = QModelIndex{};

            mCopySuggestedPriceAct->setText(getDefaultCopySuggestedPriceText());
            mCopySuggestedPriceAct->setEnabled(false);
            mAddItemCostAct->setEnabled(false);
            mAddItemCostForAct->setEnabled(false);
            mShowInEveAct->setEnabled(false);
        }
        else
        {
            mCurrentTransaction = (mProxy != nullptr) ?
                                  (mProxy->mapToSource(selected.indexes().first())) :
                                  (selected.indexes().first());

            const auto price = getSuggestedPrice(mModel->getPrice(mCurrentTransaction.row()));
            mCopySuggestedPriceAct->setText(tr("Copy suggested price: %1").arg(TextUtils::currencyToString(price, locale())));

            mCopySuggestedPriceAct->setEnabled(true);
            mAddItemCostAct->setEnabled(true);
            mAddItemCostForAct->setEnabled(true);
            mShowInEveAct->setEnabled(true);
        }
    }

    void WalletTransactionView::addItemCostForCharacter()
    {
        const auto action = qobject_cast<const QAction *>(sender());
        Q_ASSERT(action != nullptr);

        addItemCost(action->data().value<Character::IdType>());
    }

    void WalletTransactionView::copySuggestedPrice() const
    {
        const auto price = getSuggestedPrice(mModel->getPrice(mCurrentTransaction.row()));
        QApplication::clipboard()->setText(QString::number(price, 'f', 2));
    }

    void WalletTransactionView::getTypeAndShowInEve()
    {
        const auto row = mCurrentTransaction.row();
        emit showInEve(mModel->getTypeId(row), mModel->getOwnerId(row));
    }

    double WalletTransactionView::getSuggestedPrice(double price) const
    {
        const auto priceDelta = PriceUtils::getPriceDelta();

        if (mModel->getType(mCurrentTransaction.row()) == WalletTransaction::Type::Buy)
            price += priceDelta;
        else
            price -= priceDelta;

        return price;
    }

    void WalletTransactionView::initialize()
    {
        auto separator = new QAction{this};
        separator->setSeparator(true);

        addAction(separator);

        mAddItemCostAct = new QAction{tr("Add to item costs"), this};
        mAddItemCostAct->setEnabled(false);
        connect(mAddItemCostAct, &QAction::triggered, this, [=] {
            addItemCost(mCharacterId);
        });

        addAction(mAddItemCostAct);

        mAddItemCostForAct = new QAction{tr("Add to item costs for:"), this};
        mAddItemCostForAct->setEnabled(false);

        mCharsMenu = new QMenu{this};
        updateCharacters();

        mAddItemCostForAct->setMenu(mCharsMenu);
        addAction(mAddItemCostForAct);

        mCopySuggestedPriceAct = new QAction{getDefaultCopySuggestedPriceText(), this};
        mCopySuggestedPriceAct->setEnabled(false);
        connect(mCopySuggestedPriceAct, &QAction::triggered, this, &WalletTransactionView::copySuggestedPrice);

        addAction(mCopySuggestedPriceAct);

        mShowInEveAct = new QAction{tr("Show in EVE"), this};
        mShowInEveAct->setEnabled(false);
        connect(mShowInEveAct, &QAction::triggered, this, &WalletTransactionView::getTypeAndShowInEve);

        addAction(mShowInEveAct);
    }

    void WalletTransactionView::addItemCost(Character::IdType characterId)
    {
        const auto selection = selectionModel()->selectedIndexes();

        struct ItemData
        {
            uint mQuantity;
            double mPrice;
        };

        std::unordered_map<EveType::IdType, ItemData> aggrData;
        for (const auto &index : selection)
        {
            if (index.column() != 0)
                continue;

            const auto mappedIndex = (mProxy != nullptr) ? (mProxy->mapToSource(index)) : (index);
            const auto row = mappedIndex.row();

            auto &data = aggrData[mModel->getTypeId(row)];

            const auto quantity = mModel->getQuantity(row);

            data.mQuantity += quantity;
            data.mPrice += quantity * mModel->getPrice(row);
        }

        for (const auto &data : aggrData)
            mItemCostProvider.setForCharacterAndType(characterId, data.first, data.second.mPrice / data.second.mQuantity);
    }

    QString WalletTransactionView::getDefaultCopySuggestedPriceText()
    {
        return tr("Copy suggested price");
    }
}
