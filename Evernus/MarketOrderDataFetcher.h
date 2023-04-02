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
#include <memory>
#include <map>

#include <QStringList>
#include <QObject>
#include <QString>
#include <QDate>

#include "AggregatedEventProcessor.h"
#include "MarketOrderRepository.h"
#include "ProgressiveCounter.h"
#include "TypeLocationPairs.h"
#include "ExternalOrder.h"
#include "ESIManager.h"
#include "Character.h"
#include "EveType.h"

namespace Evernus
{
    class MarketOrderDataFetcher
        : public QObject
    {
        Q_OBJECT

    public:
        using OrderResultType = std::shared_ptr<std::vector<ExternalOrder>>;

        MarketOrderDataFetcher(const EveDataProvider &dataProvider,
                               ESIInterfaceManager &interfaceManager,
                               QObject *parent = nullptr);
        virtual ~MarketOrderDataFetcher() = default;

        bool hasPendingOrderRequests() const noexcept;

    signals:
        void orderStatusUpdated(const QString &text);
        void orderImportEnded(const OrderResultType &result, const QString &error);
        void genericError(const QString &text);

    public slots:
        void importData(const TypeLocationPairs &pairs,
                        Character::IdType charId);

    private:
        const EveDataProvider &mDataProvider;

        ESIManager mESIManager;

        ProgressiveCounter mOrderCounter;
        bool mPreparingRequests = false;

        QStringList mAggregatedOrderErrors;

        OrderResultType mOrders;

        AggregatedEventProcessor mEventProcessor;

        void processOrders(std::vector<ExternalOrder> &&orders, const QString &errorText);

        void importWholeMarketData(const TypeLocationPairs &pairs);
        void importIndividualData(const TypeLocationPairs &pairs);
        void importCitadelData(const TypeLocationPairs &pairs,
                               Character::IdType charId);

        void finishOrderImport();

        void processEvents();

        static void filterOrders(std::vector<ExternalOrder> &orders, const TypeLocationPairs &pairs);
    };
}
