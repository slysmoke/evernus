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
#include <QSettings>
#include <QLocale>
#include <QColor>
#include <QIcon>
#include <QFont>

#include "MarketOrderProvider.h"
#include "CharacterRepository.h"
#include "CacheTimerProvider.h"
#include "ItemCostProvider.h"
#include "EveDataProvider.h"
#include "PriceSettings.h"
#include "OrderSettings.h"
#include "ExternalOrder.h"
#include "PriceUtils.h"
#include "IconUtils.h"
#include "TextUtils.h"

#include "MarketOrderSellModel.h"

namespace Evernus
{
    MarketOrderSellModel::MarketOrderSellModel(MarketOrderProvider &orderProvider,
                                               const EveDataProvider &dataProvider,
                                               const ItemCostProvider &itemCostProvider,
                                               const CacheTimerProvider &cacheTimerProvider,
                                               const CharacterRepository &characterRepository,
                                               bool corp,
                                               QObject *parent)
        : MarketOrderTreeModel{dataProvider, parent}
        , mOrderProvider{orderProvider}
        , mItemCostProvider{itemCostProvider}
        , mCacheTimerProvider{cacheTimerProvider}
        , mCharacterRepository{characterRepository}
        , mCorp{corp}
    {
        if (mCorp)
            connect(&mDataProvider, &EveDataProvider::namesChanged, this, &MarketOrderSellModel::updateNames);
    }

    int MarketOrderSellModel::columnCount(const QModelIndex &parent) const
    {
        return numColumns;
    }

    QVariant MarketOrderSellModel::data(const QModelIndex &index, int role) const
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
        const auto character = mCharacters.find(data->getCharacterId());

        switch (role) {
        case Qt::ToolTipRole:
            if (column == priceColumn)
            {
                QSettings settings;

                const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                                   (mDataProvider.getTypeStationSellPrice(data->getTypeId(), data->getEffectiveStationId())) :
                                   (mDataProvider.getTypeRegionSellPrice(data->getTypeId(), mDataProvider.getStationRegionId(data->getEffectiveStationId())));
                if (price->isNew())
                    return tr("No price data -> Please import prices from Orders/Assets tab or by using Margin tool.");

                QLocale locale;

                if (price->getPrice() < data->getPrice())
                {
                    return tr("You have been undercut. Current price is %1 (%2 different from yours).\nClick the icon for details.")
                        .arg(TextUtils::currencyToString(price->getPrice(), locale))
                        .arg(TextUtils::currencyToString(price->getPrice() - data->getPrice(), locale));
                }

                const auto maxAge = settings.value(PriceSettings::priceMaxAgeKey, PriceSettings::priceMaxAgeDefault).toInt();
                if (price->getUpdateTime() < QDateTime::currentDateTimeUtc().addSecs(-3600 * maxAge))
                {
                    return tr("Price data is too old (valid on %1).\nPlease import prices from Orders/Assets tab or by using Margin tool.")
                        .arg(TextUtils::dateTimeToString(price->getUpdateTime().toLocalTime(), locale));
                }

                return tr("Your price was best on %1").arg(TextUtils::dateTimeToString(price->getUpdateTime().toLocalTime(), locale));
            }
            break;
        case Qt::DecorationRole:
            if (column == priceColumn)
            {
                QSettings settings;

                const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                                   (mDataProvider.getTypeStationSellPrice(data->getTypeId(), data->getEffectiveStationId())) :
                                   (mDataProvider.getTypeRegionSellPrice(data->getTypeId(), mDataProvider.getStationRegionId(data->getEffectiveStationId())));
                if (price->isNew())
                    return QIcon{":/images/error.png"};

                if (price->getPrice() < data->getPrice())
                    return QIcon{":/images/exclamation.png"};

                const auto maxAge = settings.value(PriceSettings::priceMaxAgeKey, PriceSettings::priceMaxAgeDefault).toInt();
                if (price->getUpdateTime() < QDateTime::currentDateTimeUtc().addSecs(-3600 * maxAge))
                    return QIcon{":/images/error.png"};

                return QIcon{":/images/accept.png"};
            }
            else if (column == nameColumn)
            {
                const auto metaIcon = IconUtils::getIconForMetaGroup(mDataProvider.getTypeMetaGroupName(data->getTypeId()));
                if (!metaIcon.isNull())
                    return metaIcon;
            }
            break;
        case Qt::UserRole:
            switch (column) {
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
            case priceStatusColumn:
                {
                    QSettings settings;

                    const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                                       (mDataProvider.getTypeStationSellPrice(data->getTypeId(), data->getEffectiveStationId())) :
                                       (mDataProvider.getTypeRegionSellPrice(data->getTypeId(), mDataProvider.getStationRegionId(data->getEffectiveStationId())));
                    if (price->isNew())
                        return 1;

                    const auto maxAge = settings.value(PriceSettings::priceMaxAgeKey, PriceSettings::priceMaxAgeDefault).toInt();
                    if (price->getUpdateTime() < QDateTime::currentDateTimeUtc().addSecs(-3600 * maxAge))
                        return 2;

                    return (price->getPrice() < data->getPrice()) ? (0) : (3);
                }
            case priceDifferenceColumn:
                {
                    QSettings settings;

                    const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                                       (mDataProvider.getTypeStationSellPrice(data->getTypeId(), data->getEffectiveStationId())) :
                                       (mDataProvider.getTypeRegionSellPrice(data->getTypeId(), mDataProvider.getStationRegionId(data->getEffectiveStationId())));
                    if (price->isNew())
                        break;

                    return price->getPrice() - data->getPrice();
                }
            case priceDifferencePercentColumn:
                {
                    const auto cost = mItemCostProvider.fetchForCharacterAndType(data->getCharacterId(), data->getTypeId());
                    if (cost->isNew() || qFuzzyIsNull(cost->getAdjustedCost()))
                        break;

                    QSettings settings;
                    const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                                       (mDataProvider.getTypeStationSellPrice(data->getTypeId(), data->getEffectiveStationId())) :
                                       (mDataProvider.getTypeRegionSellPrice(data->getTypeId(), mDataProvider.getStationRegionId(data->getEffectiveStationId())));

                    return (price->getPrice() - data->getPrice()) / cost->getAdjustedCost();
                }
            case volumeColumn:
                return QVariantList{} << data->getVolumeRemaining() << data->getVolumeEntered();
            case totalColumn:
                return data->getVolumeRemaining() * data->getPrice();
            case deltaColumn:
                return data->getDelta();
            case marginColumn:
                return getMargin(*data);
            case newMarginColumn:
                return getNewMargin(*data);
            case profitColumn:
                if (character != std::end(mCharacters))
                    return getProfitForVolume(data->getVolumeRemaining(), *character->second, *data);
                break;
            case totalProfitColumn:
                if (character != std::end(mCharacters))
                    return getProfitForVolume(data->getVolumeEntered(), *character->second, *data);
                break;
            case profitPerItemColumn:
                if (character != std::end(mCharacters))
                    return getProfitForVolume(1, *character->second, *data);
                break;
            case etaColumn:
                {
                    const double elapsed = data->getFirstSeen().daysTo(QDateTime::currentDateTimeUtc());
                    if (elapsed <= 0.)
                        return std::numeric_limits<double>::max();

                    const auto movement = (data->getVolumeEntered() - data->getVolumeRemaining()) / elapsed;
                    return (movement > 0.) ? (data->getVolumeRemaining() / movement) : (std::numeric_limits<double>::max());
                }
            case timeLeftColumn:
                {
                    const auto timeEnd = data->getIssued().addDays(data->getDuration()).toMSecsSinceEpoch() / 1000;
                    const auto timeCur = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;

                    if (timeEnd > timeCur)
                        return timeEnd - timeCur;
                }
                break;
            case orderAgeColumn:
                {
                    const auto timeStart = data->getIssued().toMSecsSinceEpoch() / 1000;
                    const auto timeCur = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;

                    if (timeCur > timeStart)
                        return timeCur - timeStart;
                }
                break;
            case firstSeenColumn:
                return data->getFirstSeen();
            case stationColumn:
                return mDataProvider.getLocationName(data->getStationId());
            case notesColumn:
                return data->getNotes();
            case ownerColumn:
                return getCharacterName(data->getCharacterId());
            case customStation:
                {
                    const auto id = data->getCustomStationId();
                    return (id) ? (mDataProvider.getLocationName(*id)) : (QVariant{});
                }
            }
            break;
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (column) {
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
                        const auto cost
                            = mItemCostProvider.fetchForCharacterAndType(data->getCharacterId(), data->getTypeId());
                        if (!cost->isNew())
                            return TextUtils::currencyToString(cost->getAdjustedCost(), locale);
                    }
                    break;
                case priceColumn:
                    return TextUtils::currencyToString(data->getPrice(), locale);
                case priceStatusColumn:
                    {
                        QSettings settings;

                        const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                                           (mDataProvider.getTypeStationSellPrice(data->getTypeId(), data->getEffectiveStationId())) :
                                           (mDataProvider.getTypeRegionSellPrice(data->getTypeId(), mDataProvider.getStationRegionId(data->getEffectiveStationId())));
                        if (price->isNew())
                            return tr("No price data");

                        const auto maxAge = settings.value(PriceSettings::priceMaxAgeKey, PriceSettings::priceMaxAgeDefault).toInt();
                        if (price->getUpdateTime() < QDateTime::currentDateTimeUtc().addSecs(-3600 * maxAge))
                            return tr("Data too old");
                    }
                    break;
                case priceDifferenceColumn:
                    {
                        QSettings settings;

                        const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                                           (mDataProvider.getTypeStationSellPrice(data->getTypeId(), data->getEffectiveStationId())) :
                                           (mDataProvider.getTypeRegionSellPrice(data->getTypeId(), mDataProvider.getStationRegionId(data->getEffectiveStationId())));
                        if (price->isNew())
                            break;

                        return TextUtils::currencyToString(price->getPrice() - data->getPrice(), locale);
                    }
                case priceDifferencePercentColumn:
                    {
                        const auto cost
                            = mItemCostProvider.fetchForCharacterAndType(data->getCharacterId(), data->getTypeId());
                        if (cost->isNew() || qFuzzyIsNull(cost->getAdjustedCost()))
                            break;

                        QSettings settings;

                        const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                                           (mDataProvider.getTypeStationSellPrice(data->getTypeId(), data->getEffectiveStationId())) :
                                           (mDataProvider.getTypeRegionSellPrice(data->getTypeId(), mDataProvider.getStationRegionId(data->getEffectiveStationId())));

                        return QString{"%1%2"}
                            .arg(locale.toString(100. * (price->getPrice() - data->getPrice()) / cost->getAdjustedCost(), 'f', 2))
                            .arg(locale.percent());
                    }
                case volumeColumn:
                    return QString{"%1/%2"}.arg(locale.toString(data->getVolumeRemaining())).arg(locale.toString(data->getVolumeEntered()));
                case totalColumn:
                    return TextUtils::currencyToString(data->getVolumeRemaining() * data->getPrice(), locale);
                case deltaColumn:
                    if (data->getDelta() != 0)
                        return locale.toString(data->getDelta());
                    break;
                case marginColumn:
                    return QString{"%1%2"}.arg(locale.toString(getMargin(*data), 'f', 2)).arg(locale.percent());
                case newMarginColumn:
                    return QString{"%1%2"}.arg(locale.toString(getNewMargin(*data), 'f', 2)).arg(locale.percent());
                case profitColumn:
                    if (character != std::end(mCharacters))
                        return TextUtils::currencyToString(getProfitForVolume(data->getVolumeRemaining(), *character->second, *data), locale);
                    break;
                case totalProfitColumn:
                    if (character != std::end(mCharacters))
                        return TextUtils::currencyToString(getProfitForVolume(data->getVolumeEntered(), *character->second, *data), locale);
                    break;
                case profitPerItemColumn:
                    if (character != std::end(mCharacters))
                        return TextUtils::currencyToString(getProfitForVolume(1, *character->second, *data), locale);
                    break;
                case etaColumn:
                    {
                        const double elapsed = data->getFirstSeen().daysTo(QDateTime::currentDateTimeUtc());
                        if (elapsed <= 0.)
                            return tr("unknown");

                        const auto movement = (data->getVolumeEntered() - data->getVolumeRemaining()) / elapsed;
                        if (movement <= 0.)
                            return tr("unknown");

                        const int eta = data->getVolumeRemaining() / movement;
                        return (eta > 0) ? (tr("%n day(s)", "", eta)) : (tr("today"));
                    }
                case timeLeftColumn:
                    {
                        const auto timeEnd = data->getIssued().addDays(data->getDuration()).toMSecsSinceEpoch() / 1000;
                        const auto timeCur = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;

                        if (timeEnd > timeCur)
                            return TextUtils::secondsToString(std::chrono::seconds{timeEnd - timeCur});
                    }
                    break;
                case orderAgeColumn:
                    {
                        const auto timeStart = data->getIssued().toMSecsSinceEpoch() / 1000;
                        const auto timeCur = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;

                        if (timeCur > timeStart)
                            return TextUtils::secondsToString(std::chrono::seconds{timeCur - timeStart});
                    }
                    break;
                case firstSeenColumn:
                    return TextUtils::dateTimeToString(data->getFirstSeen().toLocalTime(), locale);
                case stationColumn:
                    return mDataProvider.getLocationName(data->getStationId());
                case notesColumn:
                    return data->getNotes();
                case ownerColumn:
                    return getCharacterName(data->getCharacterId());
                case customStation:
                    {
                        const auto id = data->getCustomStationId();
                        return (id) ? (mDataProvider.getLocationName(*id)) : (QVariant{});
                    }
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
        case Qt::BackgroundRole:
            if (column == priceColumn && data->getState() == MarketOrder::State::Active)
            {
                QSettings settings;

                const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                                   (mDataProvider.getTypeStationSellPrice(data->getTypeId(), data->getEffectiveStationId())) :
                                   (mDataProvider.getTypeRegionSellPrice(data->getTypeId(), mDataProvider.getStationRegionId(data->getEffectiveStationId())));
                if (!price->isNew())
                {
                    if (price->getPrice() < data->getPrice())
                        return QColor{ 245, 170, 10 };

                    const auto maxAge = settings.value(PriceSettings::priceMaxAgeKey, PriceSettings::priceMaxAgeDefault).toInt();
                    if (price->getUpdateTime() < QDateTime::currentDateTimeUtc().addSecs(-3600 * maxAge))
                        return QColor{ 245, 170, 10 };
                }
            }
            else if (column == firstSeenColumn)
            {
                QSettings settings;
                const auto maxAge
                    = settings.value(OrderSettings::marketOrderMaxAgeKey, OrderSettings::marketOrderMaxAgeDefault).toInt();
                if (data->getFirstSeen() < QDateTime::currentDateTimeUtc().addDays(-maxAge))
                    return QColor{ 245, 170, 10 };
            }

            if (data->getColorTag().isValid())
                return data->getColorTag();
            break;
        case Qt::ForegroundRole:
            switch (column) {
            case statusColumn:
                switch (data->getState()) {
                case MarketOrder::State::Active:
                    return QColor{Qt::green};
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
            case customCostColumn:
            case priceStatusColumn:
                return QColor{Qt::red};
            case marginColumn:
                return TextUtils::getMarginColor(getMargin(*data));
            case newMarginColumn:
                return TextUtils::getMarginColor(getNewMargin(*data));
            case profitColumn:
            case totalProfitColumn:
            case profitPerItemColumn:
                return QColor{Qt::green};
            }
            break;
        case Qt::TextAlignmentRole:
            switch (column) {
            case priceStatusColumn:
                return Qt::AlignHCenter;
            case priceColumn:
            case priceDifferenceColumn:
            case totalColumn:
            case volumeColumn:
            case deltaColumn:
            case profitColumn:
            case totalProfitColumn:
            case profitPerItemColumn:
                return Qt::AlignRight;
            }
        }

        return QVariant{};
    }

    QVariant MarketOrderSellModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
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
            case priceStatusColumn:
                return tr("Price status");
            case priceDifferenceColumn:
                return tr("Price difference");
            case priceDifferencePercentColumn:
                return tr("Price difference, %1").arg(QLocale{}.percent());
            case volumeColumn:
                return tr("Volume");
            case totalColumn:
                return tr("Total");
            case deltaColumn:
                return tr("Delta");
            case marginColumn:
                return tr("Margin");
            case newMarginColumn:
                return tr("Best margin");
            case profitColumn:
                return tr("Profit");
            case totalProfitColumn:
                return tr("Total profit");
            case profitPerItemColumn:
                return tr("Profit per item");
            case etaColumn:
                return tr("ETA");
            case timeLeftColumn:
                return tr("Time left");
            case orderAgeColumn:
                return tr("Order age");
            case firstSeenColumn:
                return tr("First issued");
            case stationColumn:
                return tr("Station");
            case notesColumn:
                return tr("Notes");
            case ownerColumn:
                return tr("Owner");
            case customStation:
                return tr("Custom station");
            }
        }

        return QVariant{};
    }

    MarketOrderModel::OrderInfo MarketOrderSellModel::getOrderInfo(const QModelIndex &index) const
    {
        const auto item = static_cast<const TreeItem *>(index.internalPointer());
        const auto order = item->getOrder();
        if (Q_UNLIKELY(order == nullptr))
            return OrderInfo{};

        QSettings settings;

        const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                           (mDataProvider.getTypeStationSellPrice(order->getTypeId(), order->getEffectiveStationId())) :
                           (mDataProvider.getTypeRegionSellPrice(order->getTypeId(), mDataProvider.getStationRegionId(order->getEffectiveStationId())));

        OrderInfo info;
        info.mOrderPrice = order->getPrice();
        info.mMarketPrice = (price->isNew()) ? (info.mOrderPrice) : (price->getPrice());
        info.mOrderLocalTimestamp = mCacheTimerProvider.getLocalUpdateTimer(order->getCharacterId(), TimerType::MarketOrders);
        info.mMarketLocalTimestamp = price->getUpdateTime().toLocalTime();

		if (info.mMarketPrice < info.mOrderPrice || settings.value(PriceSettings::copyNonOverbidPriceKey, PriceSettings::copyNonOverbidPriceDefault).toBool())

			info.mTargetPrice = PriceUtils::round_to_digits(info.mMarketPrice,4) - PriceUtils::getPriceStep(info.mMarketPrice);
			// info.mTargetPrice = info.mMarketPrice - PriceUtils::getPriceDelta();
        else
            info.mTargetPrice = info.mOrderPrice;

        if (settings.value(PriceSettings::limitSellCopyToCostKey, PriceSettings::limitSellCopyToCostDefault).toBool())
        {
            const auto cost = mItemCostProvider.fetchForCharacterAndType(order->getCharacterId(), order->getTypeId());
            if (!cost->isNew())
            {
                if (settings.value(PriceSettings::limitSellCopyToTotalCostKey, PriceSettings::limitSellCopyToTotalCostDefault).toBool())
                {
                    const auto character = mCharacters.find(order->getCharacterId());
                    const auto taxes = PriceUtils::calculateTaxes(*character->second);
                    const auto realCost = PriceUtils::getBuyPrice(cost->getAdjustedCost(), taxes);

                    if (info.mTargetPrice < realCost)
                        info.mTargetPrice = realCost;
                }
                else if (info.mTargetPrice < cost->getAdjustedCost())
                {
                    info.mTargetPrice = cost->getAdjustedCost();
                }
            }
        }

        return info;
    }

    WalletTransactionsModel::EntryType MarketOrderSellModel::getOrderTypeFilter(const QModelIndex &index) const
    {
        Q_UNUSED(index);
        return WalletTransactionsModel::EntryType::Sell;
    }

    bool MarketOrderSellModel::shouldShowPriceInfo(const QModelIndex &index) const
    {
        const auto item = static_cast<const TreeItem *>(index.internalPointer());
        return index.column() == priceColumn && item->getOrder() != nullptr;
    }

    int MarketOrderSellModel::getVolumeColumn() const
    {
        return volumeColumn;
    }

    MarketOrderModel::Type MarketOrderSellModel::getType() const
    {
        return Type::Sell;
    }

    double MarketOrderSellModel::getTotalCost() const noexcept
    {
        return mTotalCost;
    }

    double MarketOrderSellModel::getTotalIncome() const noexcept
    {
        return mTotalIncome;
    }

    void MarketOrderSellModel::updateNames()
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

    MarketOrderTreeModel::OrderList MarketOrderSellModel::getOrders(Character::IdType characterId) const
    {
        mTotalCost = 0.;
        mTotalIncome = 0.;

        const auto character = mCharacters.find(characterId);
        if (Q_UNLIKELY(character == std::end(mCharacters)))
            return OrderList{};

        const auto orders = (mCorp) ?
                            (mOrderProvider.getSellOrdersForCorporation(character->second->getCorporationId())) :
                            (mOrderProvider.getSellOrders(characterId));

        const auto taxes = PriceUtils::calculateTaxes(*character->second);
        for (const auto &order : orders)
        {
            if (order->getState() != MarketOrder::State::Active)
                continue;

            const auto cost = mItemCostProvider.fetchForCharacterAndType(characterId, order->getTypeId());

            mTotalCost += order->getVolumeEntered() * PriceUtils::getBuyPrice(cost->getAdjustedCost(), taxes);
            mTotalIncome += order->getVolumeEntered() * PriceUtils::getSellPrice(order->getPrice(), taxes);
        }

        return orders;
    }

    MarketOrderTreeModel::OrderList MarketOrderSellModel::getOrdersForAllCharacters() const
    {
        auto totalCost = 0.;
        auto totalIncome = 0.;

        OrderList list;
        for (const auto &character : mCharacters)
        {
            auto orders = getOrders(character.first);
            list.insert(std::end(list), std::make_move_iterator(std::begin(orders)), std::make_move_iterator(std::end(orders)));

            totalCost += mTotalCost;
            totalIncome += mTotalIncome;
        }

        mTotalCost = totalCost;
        mTotalIncome = totalIncome;

        return list;
    }

    void MarketOrderSellModel::handleNewCharacter(Character::IdType characterId)
    {
        try
        {
            mCharacters[characterId] = mCharacterRepository.find(characterId);
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            mCharacters.erase(characterId);
        }
    }

    void MarketOrderSellModel::handleAllCharacters()
    {
        mCharacters.clear();

        const auto characters = mCharacterRepository.fetchAll();
        for (auto &character : characters)
        {
            if (character->isEnabled())
                mCharacters[character->getId()] = std::move(character);
        }
    }

    void MarketOrderSellModel::handleOrderRemoval(const MarketOrder &order)
    {
        mOrderProvider.removeOrder(order.getId());
    }

    QString MarketOrderSellModel::getCharacterName(Character::IdType id) const
    {
        return mDataProvider.getGenericName(id);
    }

    double MarketOrderSellModel::getMargin(const MarketOrder &order) const
    {
        const auto character = mCharacters.find(order.getCharacterId());
        if (Q_UNLIKELY(character == std::end(mCharacters)))
            return 0.;

        const auto taxes = PriceUtils::calculateTaxes(*character->second);
        const auto cost = mItemCostProvider.fetchForCharacterAndType(order.getCharacterId(), order.getTypeId());
        return PriceUtils::getMargin(cost->getAdjustedCost(), order.getPrice(), taxes);
    }

    double MarketOrderSellModel::getNewMargin(const MarketOrder &order) const
    {
        const auto character = mCharacters.find(order.getCharacterId());
        if (Q_UNLIKELY(character == std::end(mCharacters)))
            return 0.;

        QSettings settings;

        const auto price = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                           (mDataProvider.getTypeStationSellPrice(order.getTypeId(), order.getEffectiveStationId())) :
                           (mDataProvider.getTypeRegionSellPrice(order.getTypeId(), mDataProvider.getStationRegionId(order.getEffectiveStationId())));
        auto newPrice = price->getPrice();
        if (qFuzzyIsNull(newPrice))
            newPrice = order.getPrice();
        else
            newPrice -= PriceUtils::getPriceDelta();

        const auto cost = mItemCostProvider.fetchForCharacterAndType(order.getCharacterId(), order.getTypeId());

        const auto taxes = PriceUtils::calculateTaxes(*character->second);
        return PriceUtils::getMargin(cost->getAdjustedCost(), newPrice, taxes);
    }

    double MarketOrderSellModel
    ::getProfitForVolume(uint volume, const Character &character, const MarketOrder &order) const
    {
        const auto taxes = PriceUtils::calculateTaxes(character);
        const auto cost = mItemCostProvider.fetchForCharacterAndType(character.getId(), order.getTypeId());
        const auto realCost = PriceUtils::getBuyPrice(cost->getAdjustedCost(), taxes);
        const auto realPrice = PriceUtils::getSellPrice(order.getPrice(), taxes);
        return volume * (realPrice - realCost);
    }
}
