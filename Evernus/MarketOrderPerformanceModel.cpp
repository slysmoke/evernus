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
#include <algorithm>
#include <atomic>

#include <boost/scope_exit.hpp>

#include <QtConcurrent>

#include <QLocale>
#include <QDate>

#include "MarketOrderRepository.h"
#include "CharacterRepository.h"
#include "EveDataProvider.h"
#include "MarketOrder.h"
#include "TextUtils.h"

#include "MarketOrderPerformanceModel.h"

namespace Evernus
{
    MarketOrderPerformanceModel::MarketOrderPerformanceModel(const MarketOrderRepository &marketOrderRepository,
                                                             const MarketOrderRepository &corpMarketOrderRepository,
                                                             const CharacterRepository &characterRepository,
                                                             const EveDataProvider &dataProvider,
                                                             QObject *parent)
        : QAbstractTableModel{parent}
        , mMarketOrderRepository{marketOrderRepository}
        , mCorpMarketOrderRepository{corpMarketOrderRepository}
        , mCharacterRepository{characterRepository}
        , mDataProvider{dataProvider}
    {
    }

    int MarketOrderPerformanceModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant MarketOrderPerformanceModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        const auto column = index.column();
        const auto &data = mData[index.row()];

        switch (role) {
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (column) {
                case nameColumn:
                    return mDataProvider.getTypeName(data.mId);
                case volumeColumn:
                    return locale.toString(data.mVolume);
                case timeColumn:
                    return TextUtils::secondsToString(data.mTurnover);
                }
            }
            break;
        case Qt::UserRole:
            switch (column) {
            case nameColumn:
                return mDataProvider.getTypeName(data.mId);
            case volumeColumn:
                return data.mVolume;
            case timeColumn:
                return static_cast<qlonglong>(data.mTurnover.count());
            }
        }

        return {};
    }

    QVariant MarketOrderPerformanceModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
            case nameColumn:
                return tr("Name");
            case volumeColumn:
                return tr("Volume");
            case timeColumn:
                return tr("Median turnover time");
            }
        }

        return {};
    }

    int MarketOrderPerformanceModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    void MarketOrderPerformanceModel::reset(const QDate &from,
                                            const QDate &to,
                                            bool combineCharacters,
                                            bool combineCorp,
                                            Character::IdType characterId)
    {
        beginResetModel();

        BOOST_SCOPE_EXIT(this_) {
            this_->endResetModel();
        } BOOST_SCOPE_EXIT_END

        mData.clear();

        struct OrderData
        {
            quint64 mVolume = 0;
            std::vector<qint64> mTuronover;
        };

        std::unordered_map<EveType::IdType, OrderData> orderMap;

        const auto insertOrders = [&](auto &orders) {
            for (auto &order : orders)
            {
                Q_ASSERT(order);

                const auto firstSeen = order->getFirstSeen();
                const auto lastSeen = order->getLastSeen();

                // probably imported as fulfilled, ignore since we cannot predict anything
                if (firstSeen == lastSeen)
                    continue;

                auto &data = orderMap[order->getTypeId()];
                data.mVolume += order->getVolumeEntered();
                data.mTuronover.emplace_back(firstSeen.secsTo(lastSeen));
            }
        };

        auto orders = (combineCharacters) ?
                      (mMarketOrderRepository.fetchFulfilled(from, to)) :
                      (mMarketOrderRepository.fetchFulfilledForCharacter(from, to, characterId));

        insertOrders(orders);

        if (combineCorp)
        {
            orders = (combineCharacters) ?
                     (mCorpMarketOrderRepository.fetchFulfilled(from, to)) :
                     (mCorpMarketOrderRepository.fetchFulfilledForCorporation(from, to, mCharacterRepository.getCorporationId(characterId)));

            insertOrders(orders);
        }

        mData.resize(orderMap.size());
        std::atomic_size_t i{0};

        QtConcurrent::blockingMap(orderMap, [&](auto &order) {
            auto &data = mData[i++];
            data.mId = order.first;
            data.mVolume = order.second.mVolume;

            std::nth_element(std::begin(order.second.mTuronover),
                             std::next(std::begin(order.second.mTuronover), order.second.mTuronover.size() / 2),
                             std::end(order.second.mTuronover));
            data.mTurnover = std::chrono::minutes{order.second.mTuronover[order.second.mTuronover.size() / 2] / 60};
        });
    }
}
