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
#include <QTimer>

#include "MarketOrderModel.h"
#include "ItemCostProvider.h"
#include "CommonScriptAPI.h"
#include "EveDataProvider.h"
#include "PriceSettings.h"
#include "OrderSettings.h"
#include "ExternalOrder.h"
#include "MarketOrder.h"
#include "ScriptUtils.h"
#include "UISettings.h"

#include "MarketOrderFilterProxyModel.h"

namespace Evernus
{
    const MarketOrderFilterProxyModel::StatusFilters MarketOrderFilterProxyModel::defaultStatusFilter = Changed | Active;
    const MarketOrderFilterProxyModel::PriceStatusFilters MarketOrderFilterProxyModel::defaultPriceStatusFilter = EveryPriceStatus;

    MarketOrderFilterProxyModel::MarketOrderFilterProxyModel(const EveDataProvider &dataProvider,
                                                             const ItemCostProvider &itemCostProvider,
                                                             QObject *parent)
        : LeafFilterProxyModel{parent}
        , mDataProvider{dataProvider}
        , mItemCostProvider{itemCostProvider}
    {
        setSortCaseSensitivity(Qt::CaseInsensitive);

        QSettings settings;
        mStatusFilter = static_cast<StatusFilters>(
            settings.value(UISettings::marketOrderStateFilterKey, static_cast<int>(defaultStatusFilter)).toInt());
        mPriceStatusFilter = static_cast<PriceStatusFilters>(
            settings.value(UISettings::marketOrderPriceStatusFilterKey, static_cast<int>(defaultPriceStatusFilter)).toInt());

        CommonScriptAPI::insertAPI(mEngine, mDataProvider);
    }

    void MarketOrderFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
    {
        Q_ASSERT(sourceModel == nullptr || dynamic_cast<MarketOrderModel *>(sourceModel) != nullptr);
        LeafFilterProxyModel::setSourceModel(sourceModel);
    }

    MarketOrderFilterProxyModel::StatusFilters MarketOrderFilterProxyModel::getStatusFilter() const
    {
        return mStatusFilter;
    }

    MarketOrderFilterProxyModel::PriceStatusFilters MarketOrderFilterProxyModel::getPriceStatusFilter() const
    {
        return mPriceStatusFilter;
    }

    void MarketOrderFilterProxyModel::setStatusFilter(const StatusFilters &filter)
    {
        mStatusFilter = filter;
        invalidateFilter();
    }

    void MarketOrderFilterProxyModel::setPriceStatusFilter(const PriceStatusFilters &filter)
    {
        mPriceStatusFilter = filter;
        invalidateFilter();
    }

    void MarketOrderFilterProxyModel::setTextFilter(const QString &text, bool script)
    {
        if (script)
        {
            mFilterFunction = mEngine.evaluate("(function process(order) {\nreturn " + text + ";\n})");

            if (mFilterFunction.isError())
                emit scriptError(mFilterFunction.toString());

            setFilterWildcard(QString{});
        }
        else
        {
            mFilterFunction = QJSValue{};
            setFilterWildcard(text);
        }
    }

    void MarketOrderFilterProxyModel::unscheduleScriptError()
    {
        const auto error = mExceptions.join('\n');

        mScriptErrorScheduled = false;
        mExceptions.clear();

        emit scriptError(error);
    }

    bool MarketOrderFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        const auto parentResult = LeafFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
        if (!parentResult)
            return false;

        const auto index = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto order = static_cast<MarketOrderModel *>(sourceModel())->getOrder(index);

        if (order == nullptr)
            return hasAcceptedChildren(sourceRow, sourceParent);

        return acceptsStatus(*order) && acceptsPriceStatus(*order) && acceptsByScript(*order);
    }

    bool MarketOrderFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        const auto role = sortRole();
        const auto l = (left.model() != nullptr) ? (left.model()->data(left, role)) : (QVariant{});
        const auto r = (right.model() != nullptr) ? (right.model()->data(right, role)) : (QVariant{});

        // volume?
        if (static_cast<QMetaType::Type>(l.type()) == QMetaType::QVariantList &&
            static_cast<QMetaType::Type>(r.type()) == QMetaType::QVariantList)
        {
            const auto ll = l.toList();
            const auto rl = r.toList();

            if (ll.size() == 2 && rl.size() == 2)
                return (ll[0].toDouble() / ll[1].toDouble()) < (rl[0].toDouble() / rl[1].toDouble());
        }

        return LeafFilterProxyModel::lessThan(left, right);
    }

    bool MarketOrderFilterProxyModel::acceptsStatus(const MarketOrder &order) const
    {
        const auto type = static_cast<MarketOrderModel *>(sourceModel())->getType();
        if (type == MarketOrderModel::Type::Neither)
            return true;

        if ((mStatusFilter & Changed) && (order.getDelta() != 0))
            return true;
        if ((mStatusFilter & Active) && (order.getState() == MarketOrder::State::Active))
            return true;
        if ((mStatusFilter & Fulfilled) && (order.getState() == MarketOrder::State::Fulfilled) && (order.getVolumeRemaining() == 0))
            return true;
        if ((mStatusFilter & Cancelled) && (order.getState() == MarketOrder::State::Cancelled))
            return true;
        if ((mStatusFilter & Pending) && (order.getState() == MarketOrder::State::Pending))
            return true;
        if ((mStatusFilter & CharacterDeleted) && (order.getState() == MarketOrder::State::CharacterDeleted))
            return true;
        if ((mStatusFilter & Expired) && (order.getState() == MarketOrder::State::Fulfilled) && (order.getVolumeRemaining() != 0))
            return true;

        return false;
    }

    bool MarketOrderFilterProxyModel::acceptsPriceStatus(const MarketOrder &order) const
    {
        if (mPriceStatusFilter == EveryPriceStatus)
            return true;

        const auto type = static_cast<MarketOrderModel *>(sourceModel())->getType();
        if (type == MarketOrderModel::Type::Neither)
            return true;

        QSettings settings;
        std::shared_ptr<ExternalOrder> price;

        if (type == MarketOrderModel::Type::Buy)
        {
           price = mDataProvider.getTypeBuyPrice(order.getTypeId(), order.getStationId(), order.getRange());
        }
        else
        {
           if (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool())
               price = mDataProvider.getTypeStationSellPrice(order.getTypeId(), order.getStationId());
           else
               price = mDataProvider.getTypeRegionSellPrice(order.getTypeId(), mDataProvider.getStationRegionId(order.getStationId()));
        }

        if (price->isNew())
            return mPriceStatusFilter & NoData;

        const auto maxAge = settings.value(PriceSettings::priceMaxAgeKey, PriceSettings::priceMaxAgeDefault).toInt();
        const auto tooOld = price->getUpdateTime() < QDateTime::currentDateTimeUtc().addSecs(-3600 * maxAge);

        if ((mPriceStatusFilter & DataTooOld) && (tooOld))
            return true;
        if ((mPriceStatusFilter & Ok) && (!tooOld) && (!price->isNew()))
            return true;

        return false;
    }

    bool MarketOrderFilterProxyModel::acceptsByScript(const MarketOrder &order) const
    {
        if (!mFilterFunction.isCallable())
            return true;

        auto scriptOrder = ScriptUtils::wrapMarketOrder(mEngine, order, mItemCostProvider.fetchForCharacterAndType(order.getCharacterId(), order.getTypeId()));

        std::shared_ptr<ExternalOrder> overbidOrder;

        const auto type = static_cast<MarketOrderModel *>(sourceModel())->getType();
        switch (type) {
        case MarketOrderModel::Type::Buy:
            overbidOrder = mDataProvider.getTypeBuyPrice(order.getTypeId(), order.getStationId(), order.getRange());
            break;
        case MarketOrderModel::Type::Sell:
            {
                QSettings settings;
                overbidOrder = (settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool()) ?
                               (mDataProvider.getTypeStationSellPrice(order.getTypeId(), order.getStationId())) :
                               (mDataProvider.getTypeRegionSellPrice(order.getTypeId(), mDataProvider.getStationRegionId(order.getStationId())));
            }
            break;
        default:
            break;
        }

        if (overbidOrder)
        {
            const auto diff = order.getPrice() - overbidOrder->getPrice();
            auto overbidObj = mEngine.newObject();

            overbidObj.setPrototype(ScriptUtils::wrapExternalOrder(mEngine, *overbidOrder));
            overbidObj.setProperty("diff", diff);
            overbidObj.setProperty("diffRatio", diff / order.getPrice());
            overbidObj.setProperty("normalizedDiff", (type == MarketOrderModel::Type::Buy) ? (-diff) : (diff));

            scriptOrder.setProperty("overbid", overbidObj);
        }

        const auto result = mFilterFunction.call(QJSValueList{} << scriptOrder);

        if (result.isError() && !mScriptErrorScheduled)
        {
            mExceptions << result.toString();
            mScriptErrorScheduled = true;
            QTimer::singleShot(0, this, SLOT(unscheduleScriptError()));
        }

        return result.toBool();
    }
}
