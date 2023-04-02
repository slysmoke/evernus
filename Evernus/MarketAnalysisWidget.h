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

#include "MarketAnalysisDataFetcher.h"
#include "ExternalOrderImporter.h"
#include "MarketDataProvider.h"
#include "ExternalOrder.h"
#include "TaskConstants.h"
#include "Character.h"

class QComboBox;
class QCheckBox;

namespace Evernus
{
    class ScrapmetalReprocessingArbitrageWidget;
    class OreReprocessingArbitrageWidget;
    class DontSaveImportedOrdersCheckBox;
    class RegionStationPresetRepository;
    class RegionTypePresetRepository;
    class InterRegionAnalysisWidget;
    class ImportingAnalysisWidget;
    class MarketOrderRepository;
    class MarketGroupRepository;
    class RegionAnalysisWidget;
    class CharacterRepository;
    class PriceTypeComboBox;
    class EveTypeRepository;
    class EveDataProvider;
    class TaskManager;

    class MarketAnalysisWidget
        : public QWidget
        , public MarketDataProvider
    {
        Q_OBJECT

    public:
        MarketAnalysisWidget(const EveDataProvider &dataProvider,
                             ESIInterfaceManager &interfaceManager,
                             TaskManager &taskManager,
                             const MarketOrderRepository &orderRepo,
                             const MarketOrderRepository &corpOrderRepo,
                             const EveTypeRepository &typeRepo,
                             const MarketGroupRepository &groupRepo,
                             const CharacterRepository &characterRepo,
                             const RegionTypePresetRepository &regionTypePresetRepo,
                             const RegionStationPresetRepository &regionStationPresetRepository,
                             QWidget *parent = nullptr);
        virtual ~MarketAnalysisWidget() = default;

        virtual const HistoryMap *getHistory(uint regionId) const override;
        virtual const HistoryRegionMap *getHistory() const override;
        virtual const OrderResultType *getOrders() const override;

    signals:
        void updateExternalOrders(const std::vector<ExternalOrder> &orders);
        void preferencesChanged();

        void showInEve(EveType::IdType id, Character::IdType ownerId);

    public slots:
        void setCharacter(Character::IdType id);
        void showForCurrentRegion();

    private slots:
        void prepareOrderImport();

        void importData(const TypeLocationPairs &pairs);
        void storeOrders();

        void updateOrderTask(const QString &text);
        void updateHistoryTask(const QString &text);

        void endOrderTask(const MarketAnalysisDataFetcher::OrderResultType &orders, const QString &error);
        void endHistoryTask(const MarketAnalysisDataFetcher::HistoryResultType &history, const QString &error);

    private:
        const EveDataProvider &mDataProvider;
        TaskManager &mTaskManager;
        const MarketOrderRepository &mOrderRepo;
        const MarketOrderRepository &mCorpOrderRepo;
        const EveTypeRepository &mTypeRepo;
        const MarketGroupRepository &mGroupRepo;
        const CharacterRepository &mCharacterRepo;
        const RegionTypePresetRepository &mRegionTypePresetRepo;

        RegionAnalysisWidget *mRegionAnalysisWidget = nullptr;
        InterRegionAnalysisWidget *mInterRegionAnalysisWidget = nullptr;
        ImportingAnalysisWidget *mImportingAnalysisWidget = nullptr;
        OreReprocessingArbitrageWidget *mOreReprocessingArbitrageWidget = nullptr;
        ScrapmetalReprocessingArbitrageWidget *mScrapmetalReprocessingArbitrageWidget = nullptr;

        DontSaveImportedOrdersCheckBox *mDontSaveBtn = nullptr;
        QCheckBox *mIgnoreExistingOrdersBtn = nullptr;
        PriceTypeComboBox *mSrcPriceTypeCombo = nullptr;
        PriceTypeComboBox *mDstPriceTypeCombo = nullptr;

        uint mOrderSubtask = TaskConstants::invalidTask;
        uint mHistorySubtask = TaskConstants::invalidTask;

        MarketAnalysisDataFetcher::OrderResultType mOrders;
        MarketAnalysisDataFetcher::HistoryResultType mHistory;

        MarketAnalysisDataFetcher mDataFetcher;

        Character::IdType mCharacterId = Character::invalidId;

        void checkCompletion();
        void recalculateAllData();
    };
}
