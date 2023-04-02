/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Http Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Http Public License for more details.
 *
 *  You should have received a copy of the GNU Http Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QSettings>
#include <QUrlQuery>
#include <QLocale>
#include <QColor>

#include "CharacterRepository.h"
#include "HttpSettings.h"

#include "qxthttpsessionmanager.h"
#include "qxtwebevent.h"

#include "HttpService.h"

namespace Evernus
{
    const QString HttpService::characterIdName = "characterId";

    HttpService::HttpService(MarketOrderProvider &orderProvider,
                             MarketOrderProvider &corpOrderProvider,
                             const EveDataProvider &dataProvider,
                             const CharacterRepository &characterRepo,
                             const CacheTimerProvider &cacheTimerProvider,
                             const ItemCostProvider &itemCostProvider,
                             QxtHttpSessionManager *sm,
                             QObject *parent)
        : QxtWebSlotService(sm, parent)
        , mCharacterRepo(characterRepo)
        , mCrypt(HttpSettings::cryptKey)
        , mSellModel(orderProvider, dataProvider, itemCostProvider, cacheTimerProvider, characterRepo, false)
        , mCorpSellModel(corpOrderProvider, dataProvider, itemCostProvider, cacheTimerProvider, characterRepo, true)
        , mBuyModel(orderProvider, dataProvider, cacheTimerProvider, characterRepo, false)
        , mCorpBuyModel(corpOrderProvider, dataProvider, cacheTimerProvider, characterRepo, true)
        , mSellModelProxy(dataProvider, itemCostProvider)
        , mBuyModelProxy(dataProvider, itemCostProvider)
        , mCorpSellModelProxy(dataProvider, itemCostProvider)
        , mCorpBuyModelProxy(dataProvider, itemCostProvider)
    {
        mSellModelProxy.setSourceModel(&mSellModel);
        mBuyModelProxy.setSourceModel(&mBuyModel);
        mCorpSellModelProxy.setSourceModel(&mCorpSellModel);
        mCorpBuyModelProxy.setSourceModel(&mCorpBuyModel);

        mMainTemplate.open(":/html/http_template.html");
        mIndexTemplate.open(":/html/http_index_template.html");
        mOrdersTemplate.open(":/html/http_orders_template.html");

        mMainTemplate["index-link-text"] = tr("Select Character");
        mMainTemplate["character-orders-link-text"] = tr("Character Orders");
        mMainTemplate["corporation-orders-link-text"] = tr("Corporation Orders");

        mIndexTemplate["select-character-text"] = tr("Select character:");
        mIndexTemplate["ok-text"] = tr("OK");
        mIndexTemplate["character-id-name"] = characterIdName;

        mOrdersTemplate["filters-text"] = tr("Filters");
        mOrdersTemplate["status-filter-text"] = tr("Status filter:");
        mOrdersTemplate["changed-filter-value"] = QString::number(MarketOrderFilterProxyModel::Changed);
        mOrdersTemplate["changed-filter-text"] = tr("Changed");
        mOrdersTemplate["active-filter-value"] = QString::number(MarketOrderFilterProxyModel::Active);
        mOrdersTemplate["active-filter-text"] = tr("Active");
        mOrdersTemplate["fulfilled-filter-value"] = QString::number(MarketOrderFilterProxyModel::Fulfilled);
        mOrdersTemplate["fulfilled-filter-text"] = tr("Fulfilled");
        mOrdersTemplate["cancelled-filter-value"] = QString::number(MarketOrderFilterProxyModel::Cancelled);
        mOrdersTemplate["cancelled-filter-text"] = tr("Cancelled");
        mOrdersTemplate["pending-filter-value"] = QString::number(MarketOrderFilterProxyModel::Pending);
        mOrdersTemplate["pending-filter-text"] = tr("Pending");
        mOrdersTemplate["deleted-filter-value"] = QString::number(MarketOrderFilterProxyModel::CharacterDeleted);
        mOrdersTemplate["deleted-filter-text"] = tr("Deleted");
        mOrdersTemplate["expired-filter-value"] = QString::number(MarketOrderFilterProxyModel::Expired);
        mOrdersTemplate["expired-filter-text"] = tr("Expired");
        mOrdersTemplate["price-status-filter-text"] = tr("Price status filter:");
        mOrdersTemplate["ok-filter-value"] = QString::number(MarketOrderFilterProxyModel::Ok);
        mOrdersTemplate["ok-filter-text"] = tr("Ok");
        mOrdersTemplate["no-data-filter-value"] = QString::number(MarketOrderFilterProxyModel::NoData);
        mOrdersTemplate["no-data-filter-text"] = tr("No data");
        mOrdersTemplate["data-too-old-filter-value"] = QString::number(MarketOrderFilterProxyModel::DataTooOld);
        mOrdersTemplate["data-too-old-filter-text"] = tr("Data too old");
        mOrdersTemplate["apply-text"] = tr("Apply");
        mOrdersTemplate["sell-orders-text"] = tr("Sell Orders");
        mOrdersTemplate["buy-orders-text"] = tr("Buy Orders");

        QLocale locale;
        switch (locale.language()) {
        case QLocale::Polish:
            mOrdersTemplate["dt-language-url"] = "//cdn.datatables.net/plug-ins/725b2a2115b/i18n/Polish.json";
            break;
        default:
            mOrdersTemplate["dt-language-url"] = "//cdn.datatables.net/plug-ins/725b2a2115b/i18n/English.json";
        }

        mCorpOrdersTemplate = mOrdersTemplate;

        fillTableTemplate(mOrdersTemplate, mSellModel, mBuyModel);
        fillTableTemplate(mCorpOrdersTemplate, mCorpSellModel, mCorpBuyModel);
    }

    void HttpService::index(QxtWebRequestEvent *event)
    {
        auto query = mCharacterRepo.getEnabledQuery();

        QStringList options;
        while (query.next())
        {
            options << QString("<option value='%1'>%2</option>")
                           .arg(query.value(0).value<Character::IdType>())
                           .arg(query.value(1).toString());
        }

        mIndexTemplate["characters"] = options.join("\n");
        renderContent(event, mIndexTemplate.render());
    }

    void HttpService::characterOrders(QxtWebRequestEvent *event)
    {
        const auto characterId = getCharacterId(event);

        mSellModel.setCharacter(characterId);
        mBuyModel.setCharacter(characterId);

        renderOrders(event, mBuyModelProxy, mSellModelProxy, mOrdersTemplate);
    }

    void HttpService::corporationOrders(QxtWebRequestEvent *event)
    {
        const auto characterId = getCharacterId(event);

        mCorpSellModel.setCharacter(characterId);
        mCorpBuyModel.setCharacter(characterId);

        renderOrders(event, mCorpBuyModelProxy, mCorpSellModelProxy, mCorpOrdersTemplate);
    }

    void HttpService::pageRequestedEvent(QxtWebRequestEvent *event)
    {
        auto authHeader = event->headers.value("Authorization");
        if (!authHeader.startsWith("Basic "))
        {
            postUnauthorized(event);
            return;
        }

        authHeader.remove(0, 6);

        const auto auth = QByteArray::fromBase64(authHeader.toLatin1());
        if (!auth.isEmpty())
        {
            const auto sep = auth.indexOf(':');
            const auto user = auth.left(sep);
            const auto password = auth.mid(sep + 1);

            QSettings settings;
            if (user == settings.value(HttpSettings::userKey).toByteArray() &&
                password == mCrypt.decryptToByteArray(settings.value(HttpSettings::passwordKey).toString()))
            {
                QUrlQuery query{event->url.query()};
                if (!query.hasQueryItem(characterIdName) && !isIndexAction(event))
                {
                    postEvent(new QxtWebRedirectEvent{event->sessionID, event->requestID, "/"});
                }
                else
                {
                    mMainTemplate["character-id"] = query.queryItemValue(characterIdName);
                    QxtWebSlotService::pageRequestedEvent(event);
                }
            }
            else
            {
                postUnauthorized(event);
            }
        }
        else
        {
            postUnauthorized(event);
        }
    }

    void HttpService::renderOrders(QxtWebRequestEvent *event,
                                   MarketOrderFilterProxyModel &buyModel,
                                   MarketOrderFilterProxyModel &sellModel,
                                   QxtHtmlTemplate &htmlTemplate)
    {
        const auto filters = getFilters(event);
        sellModel.setStatusFilter(filters.first);
        sellModel.setPriceStatusFilter(filters.second);
        buyModel.setStatusFilter(filters.first);
        buyModel.setPriceStatusFilter(filters.second);

        const auto renderer = [](const auto &model) {
            const auto columns = model.columnCount(QModelIndex{});
            const auto rows = model.rowCount(QModelIndex{});

            QStringList orders;
            for (auto row = 0; row < rows; ++row)
            {
                orders << "<tr>";

                for (auto column = 0; column < columns; ++column)
                {
                    const auto index = model.index(row, column);


                    orders
                        << "<td style='color: "
                        << model.data(index, Qt::ForegroundRole).template value<QColor>().name();

                    auto background = model.data(index, Qt::BackgroundRole).template value<QColor>();
                    if (background.isValid())
                        orders << "; background: " << background.name();

                    orders
                        << ";'>"
                        << model.data(index).toString()
                        << "</td>";
                }

                orders << "</tr>";
            }

            return orders.join(QString{});
        };

        htmlTemplate["sell-orders"] = renderer(sellModel);
        htmlTemplate["buy-orders"] = renderer(buyModel);

        const auto statusFilter = sellModel.getStatusFilter();
        const auto priceStatusFilter = sellModel.getPriceStatusFilter();

        htmlTemplate["changed-filter-checked"]
            = (statusFilter & MarketOrderFilterProxyModel::Changed) ? ("checked") : (QString{});
        htmlTemplate["active-filter-checked"]
            = (statusFilter & MarketOrderFilterProxyModel::Active) ? ("checked") : (QString{});
        htmlTemplate["fulfilled-filter-checked"]
            = (statusFilter & MarketOrderFilterProxyModel::Fulfilled) ? ("checked") : (QString{});
        htmlTemplate["cancelled-filter-checked"]
            = (statusFilter & MarketOrderFilterProxyModel::Cancelled) ? ("checked") : (QString{});
        htmlTemplate["pending-filter-checked"]
            = (statusFilter & MarketOrderFilterProxyModel::Pending) ? ("checked") : (QString{});
        htmlTemplate["deleted-filter-checked"]
            = (statusFilter & MarketOrderFilterProxyModel::CharacterDeleted) ? ("checked") : (QString{});
        htmlTemplate["expired-filter-checked"]
            = (statusFilter & MarketOrderFilterProxyModel::Expired) ? ("checked") : (QString{});

        htmlTemplate["ok-filter-checked"]
            = (priceStatusFilter & MarketOrderFilterProxyModel::Ok) ? ("checked") : (QString{});
        htmlTemplate["no-data-filter-checked"]
            = (priceStatusFilter & MarketOrderFilterProxyModel::NoData) ? ("checked") : (QString{});
        htmlTemplate["data-too-old-filter-checked"]
            = (priceStatusFilter & MarketOrderFilterProxyModel::DataTooOld) ? ("checked") : (QString{});

        renderContent(event, htmlTemplate.render());
    }

    void HttpService::renderContent(QxtWebRequestEvent *event, const QString &content)
    {
        mMainTemplate["content"] = content;
        postEvent(new QxtWebPageEvent{event->sessionID, event->requestID, mMainTemplate.render().toUtf8()});
    }

    void HttpService::postUnauthorized(QxtWebRequestEvent *event)
    {
        auto pageEvent = new QxtWebErrorEvent{event->sessionID, event->requestID, 401, "Not Authorized"};
        pageEvent->headers.insert("WWW-Authenticate", "Basic realm=\"Evernus\"");

        postEvent(pageEvent);
    }

    bool HttpService::isIndexAction(QxtWebRequestEvent *event)
    {
        auto args = event->url.path().split('/');
        args.removeFirst();
        if (args.at(args.count() - 1).isEmpty())
            args.removeLast();

        if (args.count() == 0)
            return true;

        const auto action = args.at(0).toUtf8();
        return action.trimmed().isEmpty() || action == "index";
    }

    Character::IdType HttpService::getCharacterId(QxtWebRequestEvent *event)
    {
        QUrlQuery query{event->url.query()};
        return query.queryItemValue(characterIdName).toULongLong();
    }

    HttpService::FilterPair HttpService::getFilters(QxtWebRequestEvent *event)
    {
        auto result = std::make_pair(MarketOrderFilterProxyModel::defaultStatusFilter, MarketOrderFilterProxyModel::defaultPriceStatusFilter);

        const auto statusFilterName = "statusFilter";
        const auto priceStatusFilterName = "priceStatusFilter";

        QUrlQuery query{event->url.query()};
        if (query.hasQueryItem(statusFilterName))
            result.first = static_cast<MarketOrderFilterProxyModel::StatusFilters>(query.queryItemValue(statusFilterName).toInt());
        if (query.hasQueryItem(priceStatusFilterName))
            result.second = static_cast<MarketOrderFilterProxyModel::PriceStatusFilters>(query.queryItemValue(priceStatusFilterName).toInt());

        return result;
    }

    void HttpService
    ::fillTableTemplate(QxtHtmlTemplate &htmlTemplate, const MarketOrderSellModel &sellModel, const MarketOrderBuyModel &buyModel)
    {
        const auto sellColumns = sellModel.columnCount(QModelIndex{});
        QStringList columns;

        for (auto i = 0; i < sellColumns; ++i)
            columns << QString{"<th>%1</th>"}.arg(sellModel.headerData(i, Qt::Horizontal).toString());

        htmlTemplate["sell-order-columns"] = columns.join(QString{});
        columns.clear();

        const auto buyColumns = buyModel.columnCount(QModelIndex{});

        for (auto i = 0; i < buyColumns; ++i)
            columns << QString{"<th>%1</th>"}.arg(buyModel.headerData(i, Qt::Horizontal).toString());

        htmlTemplate["buy-order-columns"] = columns.join(QString{});
    }
}
