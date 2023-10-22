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
#include <QLocale>
#include <QColor>
#include <QIcon>
#include <QFont>


#include "MarketOrderProvider.h"
#include "CharacterRepository.h"
#include "CacheTimerProvider.h"
#include "ItemCostProvider.h"
#include "EveDataProvider.h"
#include "ExternalOrder.h"
#include "IconUtils.h"
#include "TextUtils.h"
#include "ItemCost.h"
#include "DarkModeColor.h"

#include "MarketOrderArchiveModel.h"

namespace Evernus
{
    MarketOrderArchiveModel::MarketOrderArchiveModel(MarketOrderProvider &orderProvider,
                                                     const EveDataProvider &dataProvider,
                                                     const ItemCostProvider &itemCostProvider,
                                                     const CharacterRepository &characterRepository,
                                                     bool corp,
                                                     QObject *parent)
        : MarketOrderTreeModel{dataProvider, parent}
        , mOrderProvider{orderProvider}
        , mItemCostProvider{itemCostProvider}
        , mCharacterRepository{characterRepository}
        , mCorp{corp}
    {
        if (mCorp)
            connect(&mDataProvider, &EveDataProvider::namesChanged, this, &MarketOrderArchiveModel::updateNames);
    }

    

    int MarketOrderArchiveModel::columnCount(const QModelIndex & /* parent */) const
    {
        return numColumns;
    }

    QVariant MarketOrderArchiveModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return QVariant{};

        const auto item = static_cast<const TreeItem *>(index.internalPointer());
        const auto column = index.column();

        if (mGrouping != Grouping::None)
        {
            if (item->parent() == &mRootItem)
            {
                if (role == Qt::UserRole || (role == Qt::DisplayRole && column == groupingColumn))
                    return item->getGroupName();

                return QVariant{};
            }
        }

        const auto data = item->getOrder();

        switch (role) {
        case Qt::UserRole:
            switch (column) {
            case lastSeenColumn:
                return data->getLastSeen();
            case typeColumn:
                return static_cast<int>(data->getType());
            case nameColumn:
                return mDataProvider.getTypeName(data->getTypeId());
            case groupColumn:
                return getGroupName(data->getTypeId());
            case statusColumn:
                return static_cast<int>(data->getState());
            case customCostColumn:
                return mItemCostProvider.fetchForCharacterAndType(data->getCharacterId(), data->getTypeId())->getAdjustedCost();
            case priceColumn:
                return data->getPrice();
            case volumeColumn:
                return QVariantList{} << data->getVolumeRemaining() << data->getVolumeEntered();
            case profitColumn:
                if (data->getType() == MarketOrder::Type::Sell)
                {
                    const auto cost = mItemCostProvider.fetchForCharacterAndType(data->getCharacterId(), data->getTypeId());
                    return (data->getVolumeEntered() - data->getVolumeRemaining()) * (data->getPrice() - cost->getAdjustedCost());
                }
                break;
            case stationColumn:
                return mDataProvider.getLocationName(data->getStationId());
            case notesColumn:
                return data->getNotes();
            case ownerColumn:
                return mDataProvider.getGenericName(data->getCharacterId());
            }
            break;
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (column) {
                case lastSeenColumn:
                    return TextUtils::dateTimeToString(data->getLastSeen().toLocalTime(), locale);
                case typeColumn:
                    return (data->getType() == MarketOrder::Type::Buy) ? (tr("Buy")) : (tr("Sell"));
                case nameColumn:
                    return mDataProvider.getTypeName(data->getTypeId());
                case groupColumn:
                    return getGroupName(data->getTypeId());
                case statusColumn:
                    {
                        const char * const stateNames[] = {
                            QT_TR_NOOP("Active"),
                            QT_TR_NOOP("Closed"),
                            QT_TR_NOOP("Fulfilled"),
                            QT_TR_NOOP("Cancelled"),
                            QT_TR_NOOP("Pending"),
                            QT_TR_NOOP("Character Deleted")
                        };

                        const auto prefix = (data->getDelta() != 0) ? ("*") : ("");

                        if (data->getState() == MarketOrder::State::Fulfilled && data->getVolumeRemaining() > 0)
                            return tr("Expired");

                        if ((data->getState() >= MarketOrder::State::Active && data->getState() <= MarketOrder::State::CharacterDeleted))
                            return prefix + tr(stateNames[static_cast<size_t>(data->getState())]);
                    }
                    break;
                case customCostColumn:
                    {
                        const auto cost = mItemCostProvider.fetchForCharacterAndType(data->getCharacterId(), data->getTypeId());
                        if (!cost->isNew())
                            return TextUtils::currencyToString(cost->getAdjustedCost(), locale);
                    }
                    break;
                case priceColumn:
                    return TextUtils::currencyToString(data->getPrice(), locale);
                case volumeColumn:
                    return QString{"%1/%2"}.arg(locale.toString(data->getVolumeRemaining())).arg(locale.toString(data->getVolumeEntered()));
                case profitColumn:
                    if (data->getType() == MarketOrder::Type::Sell)
                    {
                        const auto cost = mItemCostProvider.fetchForCharacterAndType(data->getCharacterId(), data->getTypeId());
                        return TextUtils::currencyToString((data->getVolumeEntered() - data->getVolumeRemaining()) * (data->getPrice() - cost->getAdjustedCost()), locale);
                    }
                    break;
                case stationColumn:
                    return mDataProvider.getLocationName(data->getStationId());
                case notesColumn:
                    return data->getNotes();
                case ownerColumn:
                    return mDataProvider.getGenericName(data->getCharacterId());
                }
            }
            break;
        case Qt::FontRole:
            if (column == statusColumn && data->getDelta() != 0)
            {
                QFont font;
                font.setBold(true);

                return font;
            }
            break;
        case Qt::DecorationRole:
            if (column == nameColumn)
            {
                const auto metaIcon = IconUtils::getIconForMetaGroup(mDataProvider.getTypeMetaGroupName(data->getTypeId()));
                if (!metaIcon.isNull())
                    return metaIcon;
            }
            break;
        case Qt::ForegroundRole:
            switch (column) {
            case statusColumn:
                switch (data->getState()) {
                case MarketOrder::State::Active:
                    return Evernus::DarkModeColor::green();
                case MarketOrder::State::Closed:
                    return QColor{Qt::gray};
                case MarketOrder::State::Pending:
                    return QColor{Qt::cyan};
                case MarketOrder::State::Cancelled:
                case MarketOrder::State::CharacterDeleted:
                    return QColor{Qt::red};
                case MarketOrder::State::Fulfilled:
                    return (data->getState() == MarketOrder::State::Fulfilled && data->getVolumeRemaining() > 0) ?
                           (QColor{Qt::red}) :
                           (QColor{0, 64, 0});
                default:
                    break;
                }
                break;
            case profitColumn:
                return Evernus::DarkModeColor::green();
            case volumeColumn:
                return QColor{ Qt::black };
            }
        }

        return QVariant{};
    }

    QVariant MarketOrderArchiveModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
            case lastSeenColumn:
                return tr("Completed");
            case typeColumn:
                return tr("Type");
            case nameColumn:
                return tr("Name");
            case groupColumn:
                return tr("Group");
            case statusColumn:
                return tr("Status");
            case customCostColumn:
                return tr("Custom cost");
            case priceColumn:
                return tr("Price");
            case volumeColumn:
                return tr("Volume");
            case profitColumn:
                return tr("Profit");
            case stationColumn:
                return tr("Station");
            case notesColumn:
                return tr("Notes");
            case ownerColumn:
                return tr("Owner");
            }
        }

        return QVariant{};
    }

    MarketOrderModel::OrderInfo MarketOrderArchiveModel::getOrderInfo(const QModelIndex &index) const
    {
        return OrderInfo{};
    }

    WalletTransactionsModel::EntryType MarketOrderArchiveModel::getOrderTypeFilter(const QModelIndex &index) const
    {
        const auto item = static_cast<const TreeItem *>(index.internalPointer());
        const auto order = item->getOrder();
        if (order == nullptr)
            return WalletTransactionsModel::EntryType::All;

        return (order->getType() == MarketOrder::Type::Buy) ?
               (WalletTransactionsModel::EntryType::Buy) :
               (WalletTransactionsModel::EntryType::Sell);
    }

    bool MarketOrderArchiveModel::shouldShowPriceInfo(const QModelIndex &index) const
    {
        return false;
    }

    int MarketOrderArchiveModel::getVolumeColumn() const
    {
        return volumeColumn;
    }

    MarketOrderModel::Type MarketOrderArchiveModel::getType() const
    {
        return Type::Neither;
    }

    void MarketOrderArchiveModel::setCharacterAndRange(Character::IdType id, const QDateTime &from, const QDateTime &to)
    {
        mFrom = from;
        mTo = to;

        setCharacter(id);
    }

    void MarketOrderArchiveModel::updateNames()
    {
        if (mGrouping == Grouping::None)
        {
            emit dataChanged(index(0, ownerColumn),
                             index(rowCount() - 1, ownerColumn),
                             QVector<int>{}
                                 << Qt::UserRole
                                 << Qt::DisplayRole);
        }
        else
        {
            const auto rows = mRootItem.childCount();
            for (auto row = 0; row < rows; ++row)
            {
                const auto child = mRootItem.child(row);
                const auto children = child->childCount();
                const auto parent = index(row, 0);

                emit dataChanged(index(0, ownerColumn, parent),
                                 index(children - 1, ownerColumn, parent),
                                 QVector<int>{}
                                     << Qt::UserRole
                                     << Qt::DisplayRole);
            }
        }
    }

    MarketOrderTreeModel::OrderList MarketOrderArchiveModel::getOrders(Character::IdType characterId) const
    {
        try
        {
            return (mCorp) ?
                   (mOrderProvider.getArchivedOrdersForCorporation(mCharacterRepository.getCorporationId(characterId), mFrom, mTo)) :
                   (mOrderProvider.getArchivedOrders(characterId, mFrom, mTo));
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            return OrderList{};
        }
    }

    MarketOrderTreeModel::OrderList MarketOrderArchiveModel::getOrdersForAllCharacters() const
    {
        auto characters = mCharacterRepository.getEnabledQuery();
        const auto idName =  mCharacterRepository.getIdColumn();

        OrderList list;

        while (characters.next())
        {
            auto orders = getOrders(characters.value(idName).value<Character::IdType>());
            list.insert(std::end(list), std::make_move_iterator(std::begin(orders)), std::make_move_iterator(std::end(orders)));
        }

        return list;
    }

    void MarketOrderArchiveModel::handleOrderRemoval(const MarketOrder &order)
    {
        mOrderProvider.removeOrder(order.getId());
    }
}
