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

#include "WalletTransactionsModel.h"
#include "CharacterBoundWidget.h"

class QSortFilterProxyModel;
class QItemSelection;
class QCheckBox;
class QLabel;

namespace Evernus
{
    class WalletTransactionRepository;
    class WalletEntryFilterWidget;
    class WalletTransactionView;
    class FilterTextRepository;
    class CharacterRepository;
    class CacheTimerProvider;
    class ItemCostProvider;
    class EveDataProvider;

    class WalletTransactionsWidget
        : public CharacterBoundWidget
    {
        Q_OBJECT

    public:
        typedef WalletTransactionsModel::EntryType EntryType;

        WalletTransactionsWidget(const WalletTransactionRepository &walletRepo,
                                 const CharacterRepository &characterRepository,
                                 const FilterTextRepository &filterRepo,
                                 const CacheTimerProvider &cacheTimerProvider,
                                 const EveDataProvider &dataProvider,
                                 ItemCostProvider &itemCostProvider,
                                 bool corp,
                                 QWidget *parent = nullptr);
        virtual ~WalletTransactionsWidget() = default;

    signals:
        void showInEve(EveType::IdType id, Character::IdType ownerId);

    public slots:
        void updateData();
        void updateCharacters();

        void updateFilter(const QDate &from, const QDate &to, const QString &filter, int type);

    private:
        WalletTransactionsModel mModel;
        QSortFilterProxyModel *mFilterModel = nullptr;

        WalletEntryFilterWidget *mFilter = nullptr;
        QCheckBox *mCombineBtn = nullptr;

        WalletTransactionView *mView = nullptr;

        QLabel *mTotalTransactionsLabel = nullptr;
        QLabel *mTotalQuantityLabel = nullptr;
        QLabel *mTotalSizeLabel = nullptr;
        QLabel *mTotalIncomeLabel = nullptr;
        QLabel *mTotalCostLabel = nullptr;
        QLabel *mTotalBalanceLabel = nullptr;
        QLabel *mTotalProfitLabel = nullptr;

        virtual void handleNewCharacter(Character::IdType id) override;

        void updateInfo();
    };
}
