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

#include <QSortFilterProxyModel>
#include <QVariant>

#include "MiningLedgerSolarSystemsModel.h"
#include "MarketOrderDataFetcher.h"
#include "MiningLedgerTypesModel.h"
#include "CharacterBoundWidget.h"
#include "MiningLedgerModel.h"
#include "TaskConstants.h"

class QAbstractItemView;
class QRadioButton;

namespace Evernus
{
    class MiningLedgerRepository;
    class MiningLedgerBarGraph;
    class StationSelectButton;
    class ESIInterfaceManager;
    class CacheTimerProvider;
    class PriceTypeComboBox;
    class DateRangeWidget;
    class EveDataProvider;
    class RegionComboBox;
    class ExternalOrder;
    class TaskManager;

    class IndustryMiningLedgerWidget
        : public CharacterBoundWidget
    {
        Q_OBJECT

    public:
        IndustryMiningLedgerWidget(const CacheTimerProvider &cacheTimerProvider,
                                   const EveDataProvider &dataProvider,
                                   const MiningLedgerRepository &ledgerRepo,
                                   TaskManager &taskManager,
                                   ESIInterfaceManager &interfaceManager,
                                   QWidget *parent = nullptr);
        IndustryMiningLedgerWidget(const IndustryMiningLedgerWidget &) = default;
        IndustryMiningLedgerWidget(IndustryMiningLedgerWidget &&) = default;
        virtual ~IndustryMiningLedgerWidget() = default;

        IndustryMiningLedgerWidget &operator =(const IndustryMiningLedgerWidget &) = default;
        IndustryMiningLedgerWidget &operator =(IndustryMiningLedgerWidget &&) = default;

    signals:
        void updateExternalOrders(const std::vector<ExternalOrder> &orders);

    public slots:
        void refresh();

    private slots:
        void importData();
        void updateSellStation(const QVariantList &path);
        void updatePriceType();

        void updateOrderTask(const QString &text);
        void endOrderTask(const MarketOrderDataFetcher::OrderResultType &orders, const QString &error);

    private:
        const EveDataProvider &mDataProvider;
        TaskManager &mTaskManager;

        DateRangeWidget *mRangeFilter = nullptr;
        QRadioButton *mImportForSourceBtn = nullptr;
        RegionComboBox *mImportRegionsCombo = nullptr;
        StationSelectButton *mSellStationBtn = nullptr;
        PriceTypeComboBox *mSellPriceTypeCombo = nullptr;
        MiningLedgerBarGraph *mGraphWidget = nullptr;

        MiningLedgerModel mDetailsModel;
        QSortFilterProxyModel mDetailsProxy;

        MiningLedgerTypesModel mTypesModel;
        QSortFilterProxyModel mTypesProxy;

        MiningLedgerSolarSystemsModel mSolarSystemsModel;
        QSortFilterProxyModel mSolarSystemsProxy;

        MarketOrderDataFetcher mDataFetcher;

        uint mOrderTask = TaskConstants::invalidTask;

        virtual void handleNewCharacter(Character::IdType id) override;

        void createLookupActions(QAbstractItemView &view,
                                 ModelWithTypes &model,
                                 const QSortFilterProxyModel &proxy);
        QAbstractItemView *createDataView(QSortFilterProxyModel &proxy,
                                          const QString &name);
        QWidget *createAndLinkDataView(ModelWithTypes &model,
                                       QSortFilterProxyModel &proxy,
                                       const QString &name);
    };
}
