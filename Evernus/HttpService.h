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
#pragma once

#include "MarketOrderFilterProxyModel.h"
#include "MarketOrderSellModel.h"
#include "MarketOrderBuyModel.h"
#include "SimpleCrypt.h"

#include "qxtwebslotservice.h"
#include "qxthtmltemplate.h"

class QxtHttpSessionManager;

namespace Evernus
{
    class CharacterRepository;
    class MarketOrderProvider;
    class CacheTimerProvider;
    class ItemCostProvider;
    class EveDataProvider;

    class HttpService
        : public QxtWebSlotService
    {
        Q_OBJECT

    public:
        HttpService(MarketOrderProvider &orderProvider,
                    MarketOrderProvider &corpOrderProvider,
                    const EveDataProvider &dataProvider,
                    const CharacterRepository &characterRepo,
                    const CacheTimerProvider &cacheTimerProvider,
                    const ItemCostProvider &itemCostProvider,
                    QxtHttpSessionManager *sm,
                    QObject *parent = nullptr);
        virtual ~HttpService() = default;

    public slots:
        void index(QxtWebRequestEvent *event);
        void characterOrders(QxtWebRequestEvent *event);
        void corporationOrders(QxtWebRequestEvent *event);

    protected:
        virtual void pageRequestedEvent(QxtWebRequestEvent *event) override;

    private:
        typedef std::pair<MarketOrderFilterProxyModel::StatusFilters, MarketOrderFilterProxyModel::PriceStatusFilters> FilterPair;

        static const QString characterIdName;

        const CharacterRepository &mCharacterRepo;

        SimpleCrypt mCrypt;

        QxtHtmlTemplate mMainTemplate, mIndexTemplate, mOrdersTemplate, mCorpOrdersTemplate;

        MarketOrderSellModel mSellModel, mCorpSellModel;
        MarketOrderBuyModel mBuyModel, mCorpBuyModel;

        MarketOrderFilterProxyModel mSellModelProxy, mBuyModelProxy, mCorpSellModelProxy, mCorpBuyModelProxy;

        void renderOrders(QxtWebRequestEvent *event,
                          MarketOrderFilterProxyModel &buyModel,
                          MarketOrderFilterProxyModel &sellModel,
                          QxtHtmlTemplate &htmlTemplate);

        void renderContent(QxtWebRequestEvent *event, const QString &content);
        void postUnauthorized(QxtWebRequestEvent *event);

        static bool isIndexAction(QxtWebRequestEvent *event);
        static Character::IdType getCharacterId(QxtWebRequestEvent *event);
        static FilterPair getFilters(QxtWebRequestEvent *event);

        static void fillTableTemplate(QxtHtmlTemplate &htmlTemplate,
                                      const MarketOrderSellModel &sellModel,
                                      const MarketOrderBuyModel &buyModel);
    };
}
