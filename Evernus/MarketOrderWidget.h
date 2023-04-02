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

#include "MarketOrderArchiveModel.h"
#include "ExternalOrderImporter.h"
#include "MarketOrderSellModel.h"
#include "CharacterBoundWidget.h"
#include "MarketOrderBuyModel.h"

class QPushButton;
class QComboBox;
class QCheckBox;
class QColor;

namespace Evernus
{
    class MarketOrderViewWithTransactions;
    class WalletTransactionRepository;
    class ExternalOrderRepository;
    class FilterTextRepository;
    class MarketOrderProvider;
    class CharacterRepository;
    class CacheTimerProvider;
    class ItemCostProvider;
    class EveDataProvider;
    class DateRangeWidget;
    class MarketOrderView;

    class MarketOrderWidget
        : public CharacterBoundWidget
    {
        Q_OBJECT

    public:
        MarketOrderWidget(MarketOrderProvider &orderProvider,
                          MarketOrderProvider &corpOrderProvider,
                          const CacheTimerProvider &cacheTimerProvider,
                          const EveDataProvider &dataProvider,
                          ItemCostProvider &itemCostProvider,
                          const WalletTransactionRepository &transactionsRepo,
                          const CharacterRepository &characterRepository,
                          const FilterTextRepository &filterRepo,
                          const ExternalOrderRepository &externalOrderRepo,
                          bool corp,
                          QWidget *parent = nullptr);
        virtual ~MarketOrderWidget() = default;

    signals:
        void characterChanged(Character::IdType id);

        void importFromLogs(Character::IdType id);

        void importPricesFromWeb(Character::IdType id, const TypeLocationPairs &target);
        void importPricesFromFile(Character::IdType id, const TypeLocationPairs &target);

        void openMarginTool();

        void showExternalOrders(EveType::IdType id);
        void showInEve(EveType::IdType id, Character::IdType ownerId);

        void fpcExecutorChanged(QObject *executor);

    public slots:
        void updateData();
        void updateCharacters();

    private slots:
        void changeGrouping();
        void saveChosenTab(int index);

        void prepareItemImportFromWeb();
        void prepareItemImportFromFile();

        void setArchiveRange(const QDate &from, const QDate &to);

        void showScriptError(const QString &message);

        void changeFPCExecutor();

        void setOrderNotes(MarketOrder::IdType id, const QString &notes);
        void setOrderStation(MarketOrder::IdType orderId, uint stationId);
        void setColorTag(MarketOrder::IdType orderId, const QColor &color);

    private:
        static const QString settingsLastTabKey;
        static const QString settingsShowForAllKey;

        MarketOrderProvider &mOrderProvider;
        const CharacterRepository &mCharacterRepository;

        MarketOrderViewWithTransactions *mSellView = nullptr;
        MarketOrderViewWithTransactions *mBuyView = nullptr;
        MarketOrderViewWithTransactions *mArchiveView = nullptr;
        MarketOrderView *mCombinedSellView = nullptr;
        MarketOrderView *mCombinedBuyView = nullptr;

        MarketOrderSellModel mSellModel;
        MarketOrderBuyModel mBuyModel;
        MarketOrderArchiveModel mArchiveModel;

        QPushButton *mLogImportBtn = nullptr;
        QComboBox *mGroupingCombo = nullptr;
        QCheckBox *mShowForAllCharacters = nullptr;
        DateRangeWidget *mArchiveRangeEdit = nullptr;

        bool mCorp = false;

        virtual void handleNewCharacter(Character::IdType id) override;

        TypeLocationPairs getImportTarget() const;

        void expandAll();
    };
}
