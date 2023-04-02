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

#include <vector>

#include <QWidget>

#include "Character.h"

class QByteArray;

namespace Evernus
{
    class IndustryManufacturingSetupRepository;
    class RegionStationPresetRepository;
    class IndustryManufacturingWidget;
    class IndustryMiningLedgerWidget;
    class TradeableTypesTreeView;
    class MiningLedgerRepository;
    class MarketGroupRepository;
    class CharacterRepository;
    class ESIInterfaceManager;
    class CacheTimerProvider;
    class EveTypeRepository;
    class ItemCostProvider;
    class EveDataProvider;
    class RegionComboBox;
    class AssetProvider;
    class ExternalOrder;
    class TaskManager;

    class IndustryWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        IndustryWidget(const EveDataProvider &dataProvider,
                       const RegionStationPresetRepository &regionStationPresetRepository,
                       const EveTypeRepository &typeRepo,
                       const MarketGroupRepository &groupRepo,
                       const CharacterRepository &characterRepo,
                       const IndustryManufacturingSetupRepository &setupRepo,
                       const MiningLedgerRepository &ledgerRepo,
                       ESIInterfaceManager &interfaceManager,
                       TaskManager &taskManager,
                       const AssetProvider &assetProvider,
                       const ItemCostProvider &costProvider,
                       const CacheTimerProvider &cacheTimerProvider,
                       QWidget *parent = nullptr);
        IndustryWidget(const IndustryWidget &) = default;
        IndustryWidget(IndustryWidget &&) = default;
        virtual ~IndustryWidget() = default;

        IndustryWidget &operator =(const IndustryWidget &) = default;
        IndustryWidget &operator =(IndustryWidget &&) = default;

    signals:
        void updateExternalOrders(const std::vector<ExternalOrder> &orders);
        void importMiningLedger(Character::IdType id);
        void showInEve(EveType::IdType id);
        void showExternalOrders(EveType::IdType id);

    public slots:
        void handleNewPreferences();
        void setCharacter(Character::IdType id);

        void refreshAssets();
        void refreshMiningLedger();

    private:
        IndustryManufacturingWidget *mManufacturingWidget = nullptr;
        IndustryMiningLedgerWidget *mMiningLedgerWidget = nullptr;
    };
}
