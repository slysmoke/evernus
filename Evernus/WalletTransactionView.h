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

#include "StyledTreeView.h"
#include "Character.h"
#include "EveType.h"

class QAbstractProxyModel;

namespace Evernus
{
    class WalletTransactionsModel;
    class CharacterRepository;
    class ItemCostProvider;

    class WalletTransactionView
        : public StyledTreeView
    {
        Q_OBJECT

    public:
        WalletTransactionView(ItemCostProvider &itemCostProvider,
                              const CharacterRepository &characterRepository,
                              QWidget *parent = nullptr);
        WalletTransactionView(const QString &objectName, ItemCostProvider &itemCostProvider,
                              const CharacterRepository &characterRepository,
                              QWidget *parent = nullptr);
        virtual ~WalletTransactionView() = default;

        virtual void setModel(QAbstractItemModel *model) override;

        void setModels(QAbstractProxyModel *proxy, WalletTransactionsModel *model);

        void setCharacter(Character::IdType id);

    signals:
        void showInEve(EveType::IdType id, Character::IdType ownerId);

    public slots:
        void updateCharacters();

    private slots:
        void selectTransaction(const QItemSelection &selected);

        void addItemCostForCharacter();
        void copySuggestedPrice() const;
        void getTypeAndShowInEve();

    private:
        ItemCostProvider &mItemCostProvider;
        const CharacterRepository &mCharacterRepository;

        QAction *mCopySuggestedPriceAct = nullptr;
        QAction *mAddItemCostAct = nullptr;
        QAction *mAddItemCostForAct = nullptr;
        QAction *mShowInEveAct = nullptr;
        QMenu *mCharsMenu = nullptr;

        QAbstractProxyModel *mProxy = nullptr;
        WalletTransactionsModel *mModel = nullptr;

        QModelIndex mCurrentTransaction;

        Character::IdType mCharacterId = Character::invalidId;

        double getSuggestedPrice(double price) const;

        void initialize();
        void addItemCost(Character::IdType characterId);

        static QString getDefaultCopySuggestedPriceText();
    };
}
