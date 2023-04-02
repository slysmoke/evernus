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
#include <QVBoxLayout>
#include <QTabWidget>

#include "IndustryManufacturingWidget.h"
#include "IndustryMiningLedgerWidget.h"

#include "IndustryWidget.h"

namespace Evernus
{
    IndustryWidget::IndustryWidget(const EveDataProvider &dataProvider,
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
                                   QWidget *parent)
        : QWidget{parent}
    {
        const auto mainLayout = new QVBoxLayout{this};

        const auto tabs = new QTabWidget{this};
        mainLayout->addWidget(tabs);

        mManufacturingWidget = new IndustryManufacturingWidget{dataProvider,
                                                               regionStationPresetRepository,
                                                               typeRepo,
                                                               groupRepo,
                                                               characterRepo,
                                                               setupRepo,
                                                               interfaceManager,
                                                               taskManager,
                                                               assetProvider,
                                                               costProvider,
                                                               this};
        tabs->addTab(mManufacturingWidget, tr("Manufacturing planner"));
        connect(mManufacturingWidget, &IndustryManufacturingWidget::updateExternalOrders,
                this, &IndustryWidget::updateExternalOrders);
        connect(mManufacturingWidget, &IndustryManufacturingWidget::showInEve,
                this, &IndustryWidget::showInEve);
        connect(mManufacturingWidget, &IndustryManufacturingWidget::showExternalOrders,
                this, &IndustryWidget::showExternalOrders);

        mMiningLedgerWidget = new IndustryMiningLedgerWidget{cacheTimerProvider,
                                                             dataProvider,
                                                             ledgerRepo,
                                                             taskManager,
                                                             interfaceManager,
                                                             this};
        tabs->addTab(mMiningLedgerWidget, tr("Mining ledger"));
        connect(mMiningLedgerWidget, &IndustryMiningLedgerWidget::importFromAPI,
                this, &IndustryWidget::importMiningLedger);
        connect(mMiningLedgerWidget, &IndustryMiningLedgerWidget::updateExternalOrders,
                this, &IndustryWidget::updateExternalOrders);
    }

    void IndustryWidget::handleNewPreferences()
    {
        Q_ASSERT(mManufacturingWidget != nullptr);
        mManufacturingWidget->handleNewPreferences();
    }

    void IndustryWidget::setCharacter(Character::IdType id)
    {
        Q_ASSERT(mManufacturingWidget != nullptr);
        Q_ASSERT(mMiningLedgerWidget != nullptr);

        mManufacturingWidget->setCharacter(id);
        mMiningLedgerWidget->setCharacter(id);
    }

    void IndustryWidget::refreshAssets()
    {
        Q_ASSERT(mMiningLedgerWidget != nullptr);
        mManufacturingWidget->refreshAssets();
    }

    void IndustryWidget::refreshMiningLedger()
    {
        Q_ASSERT(mMiningLedgerWidget != nullptr);
        mMiningLedgerWidget->refresh();
    }
}
