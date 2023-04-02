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

#include "ImportSettings.h"

#include "CharacterCorporationCombinedMarketOrderProvider.h"

namespace Evernus
{
    CharacterCorporationCombinedMarketOrderProvider
    ::CharacterCorporationCombinedMarketOrderProvider(MarketOrderProvider &charOrderProvider,
                                                      MarketOrderProvider &corpOrderProvider)
        : MarketOrderProvider{}
        , mCharOrderProvider{charOrderProvider}
        , mCorpOrderProvider{corpOrderProvider}
    {
    }

    MarketOrderProvider::OrderList CharacterCorporationCombinedMarketOrderProvider
    ::getSellOrders(Character::IdType characterId) const
    {
        if (!shouldCombine())
            return mCharOrderProvider.getSellOrders(characterId);

        const auto it = mSellOrders.find(characterId);
        if (it != std::end(mSellOrders))
            return it->second;

        auto charOrders = mCharOrderProvider.getSellOrders(characterId);
        auto corpOrders = mCorpOrderProvider.getSellOrders(characterId);

        charOrders.insert(std::end(charOrders),
                          std::make_move_iterator(std::begin(corpOrders)),
                          std::make_move_iterator(std::end(corpOrders)));

        return mSellOrders.emplace(characterId, std::move(charOrders)).first->second;
    }

    MarketOrderProvider::OrderList CharacterCorporationCombinedMarketOrderProvider
    ::getBuyOrders(Character::IdType characterId) const
    {
        if (!shouldCombine())
            return mCharOrderProvider.getBuyOrders(characterId);

        const auto it = mBuyOrders.find(characterId);
        if (it != std::end(mBuyOrders))
            return it->second;

        auto charOrders = mCharOrderProvider.getBuyOrders(characterId);
        auto corpOrders = mCorpOrderProvider.getBuyOrders(characterId);

        charOrders.insert(std::end(charOrders),
                          std::make_move_iterator(std::begin(corpOrders)),
                          std::make_move_iterator(std::end(corpOrders)));

        return mBuyOrders.emplace(characterId, std::move(charOrders)).first->second;
    }

    MarketOrderProvider::OrderList CharacterCorporationCombinedMarketOrderProvider
    ::getArchivedOrders(Character::IdType characterId, const QDateTime &from, const QDateTime &to) const
    {
        if (!shouldCombine())
            return mCharOrderProvider.getArchivedOrders(characterId, from, to);

        auto charOrders = mCharOrderProvider.getArchivedOrders(characterId, from, to);
        auto corpOrders = mCorpOrderProvider.getArchivedOrders(characterId, from, to);

        charOrders.insert(std::end(charOrders),
                          std::make_move_iterator(std::begin(corpOrders)),
                          std::make_move_iterator(std::end(corpOrders)));

        return charOrders;
    }

    MarketOrderProvider::OrderList CharacterCorporationCombinedMarketOrderProvider
    ::getSellOrdersForCorporation(quint64 corporationId) const
    {
        return mCorpOrderProvider.getSellOrdersForCorporation(corporationId);
    }

    MarketOrderProvider::OrderList CharacterCorporationCombinedMarketOrderProvider
    ::getBuyOrdersForCorporation(quint64 corporationId) const
    {
        return mCorpOrderProvider.getBuyOrdersForCorporation(corporationId);
    }

    MarketOrderProvider::OrderList CharacterCorporationCombinedMarketOrderProvider
    ::getArchivedOrdersForCorporation(quint64 corporationId, const QDateTime &from, const QDateTime &to) const
    {
        return mCorpOrderProvider.getArchivedOrdersForCorporation(corporationId, from, to);
    }

    void CharacterCorporationCombinedMarketOrderProvider::removeOrder(MarketOrder::IdType id)
    {
        mCharOrderProvider.removeOrder(id);
        mCorpOrderProvider.removeOrder(id);

        mSellOrders.clear();
        mBuyOrders.clear();
    }

    void CharacterCorporationCombinedMarketOrderProvider::setOrderNotes(MarketOrder::IdType id, const QString &notes)
    {
        mCharOrderProvider.setOrderNotes(id, notes);
        mCorpOrderProvider.setOrderNotes(id, notes);

        mSellOrders.clear();
        mBuyOrders.clear();
    }

    void CharacterCorporationCombinedMarketOrderProvider::setOrderStation(MarketOrder::IdType orderId, uint stationId)
    {
        mCharOrderProvider.setOrderStation(orderId, stationId);
        mCorpOrderProvider.setOrderStation(orderId, stationId);

        mSellOrders.clear();
        mBuyOrders.clear();
    }

    void CharacterCorporationCombinedMarketOrderProvider::setOrderColorTag(MarketOrder::IdType orderId, const QColor &color)
    {
        mCharOrderProvider.setOrderColorTag(orderId, color);
        mCorpOrderProvider.setOrderColorTag(orderId, color);

        mSellOrders.clear();
        mBuyOrders.clear();
    }

    void CharacterCorporationCombinedMarketOrderProvider::clearOrdersForCharacter(Character::IdType id) const
    {
        mSellOrders.erase(id);
        mBuyOrders.erase(id);
    }

    bool CharacterCorporationCombinedMarketOrderProvider::shouldCombine()
    {
        QSettings settings;
        return settings.value(ImportSettings::corpOrdersWithCharacterKey, ImportSettings::corpOrdersWithCharacterDefault).toBool();
    }
}
