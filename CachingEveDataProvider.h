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

#include <unordered_set>

#include <QHash>

#include <boost/functional/hash.hpp>

#include "ExternalOrderRepository.h"
#include "MarketGroupRepository.h"
#include "MetaGroupRepository.h"
#include "EveTypeRepository.h"
#include "RefType.h"

#include "EveDataProvider.h"

namespace Evernus
{
    class ConquerableStationRepository;
    class MarketOrderRepository;
    class RefTypeRepository;
    class APIManager;

    class CachingEveDataProvider
        : public EveDataProvider
    {
        Q_OBJECT

    public:
        CachingEveDataProvider(const EveTypeRepository &eveTypeRepository,
                               const MetaGroupRepository &metaGroupRepository,
                               const ExternalOrderRepository &externalOrderRepository,
                               const MarketOrderRepository &marketOrderRepository,
                               const MarketOrderRepository &corpMarketOrderRepository,
                               const ConquerableStationRepository &conquerableStationRepository,
                               const MarketGroupRepository &marketGroupRepository,
                               const RefTypeRepository &refTypeRepository,
                               const APIManager &apiManager,
                               const QSqlDatabase &eveDb,
                               QObject *parent = nullptr);
        virtual ~CachingEveDataProvider();

        virtual QString getTypeName(EveType::IdType id) const override;
        virtual QString getTypeMarketGroupParentName(EveType::IdType id) const override;
        virtual QString getTypeMarketGroupName(EveType::IdType id) const override;
        virtual MarketGroup::IdType getTypeMarketGroupParentId(EveType::IdType id) const override;
        virtual const std::unordered_map<EveType::IdType, QString> &getAllTypeNames() const override;
        virtual QString getTypeMetaGroupName(EveType::IdType id) const override;
        virtual QString getGenericName(quint64 id) const override;

        virtual double getTypeVolume(EveType::IdType id) const override;
        virtual std::shared_ptr<ExternalOrder> getTypeSellPrice(EveType::IdType id, quint64 stationId) const override;
        virtual std::shared_ptr<ExternalOrder> getTypeBuyPrice(EveType::IdType id, quint64 stationId) const override;

        virtual void updateExternalOrders(const std::vector<ExternalOrder> &orders) override;

        virtual QString getLocationName(quint64 id) const override;
        virtual QString getRegionName(uint id) const override;
        virtual QString getSolarSystemName(uint id) const override;

        virtual QString getRefTypeName(uint id) const override;

        virtual const std::vector<MapLocation> &getRegions() const override;
        virtual const std::vector<MapLocation> &getConstellations(uint regionId) const override;
        virtual const std::vector<MapLocation> &getSolarSystems(uint constellationId) const override;
        virtual const std::vector<Station> &getStations(uint solarSystemId) const override;

        virtual double getSolarSystemSecurityStatus(uint solarSystemId) const override;

        void precacheJumpMap();
        void precacheRefTypes();

        void clearExternalOrderCaches();
        void clearStationCache();

        std::shared_ptr<ExternalOrder> getTypeSellPrice(EveType::IdType id, quint64 stationId, bool dontThrow) const;

    private:
        typedef std::pair<EveType::IdType, quint64> TypeLocationPair;
        typedef std::pair<EveType::IdType, uint> TypeRegionPair;

        static const QString nameCacheFileName;

        const EveTypeRepository &mEveTypeRepository;
        const MetaGroupRepository &mMetaGroupRepository;
        const ExternalOrderRepository &mExternalOrderRepository;
        const MarketOrderRepository &mMarketOrderRepository, &mCorpMarketOrderRepository;
        const ConquerableStationRepository &mConquerableStationRepository;
        const MarketGroupRepository &mMarketGroupRepository;
        const RefTypeRepository &mRefTypeRepository;

        const APIManager &mAPIManager;

        QSqlDatabase mEveDb;

        mutable std::unordered_map<EveType::IdType, QString> mTypeNameCache;
        mutable std::unordered_map<EveType::IdType, MetaGroupRepository::EntityPtr> mTypeMetaGroupCache;
        mutable std::unordered_map<EveType::IdType, EveTypeRepository::EntityPtr> mTypeCache;

        mutable std::unordered_map<TypeLocationPair, ExternalOrderRepository::EntityPtr, boost::hash<TypeLocationPair>>
        mSellPrices;
        mutable std::unordered_map<TypeLocationPair, ExternalOrderRepository::EntityPtr, boost::hash<TypeLocationPair>>
        mBuyPrices;

        mutable std::unordered_map<TypeRegionPair, ExternalOrderRepository::EntityList, boost::hash<TypeRegionPair>>
        mTypeRegionOrderCache;

        mutable std::unordered_map<quint64, QString> mLocationNameCache;

        mutable std::unordered_map<EveType::IdType, MarketGroupRepository::EntityPtr> mTypeMarketGroupParentCache;
        mutable std::unordered_map<EveType::IdType, MarketGroupRepository::EntityPtr> mTypeMarketGroupCache;

        std::unordered_map<RefType::IdType, QString> mRefTypeNames;

        mutable QHash<quint64, QString> mGenericNameCache;
        mutable std::unordered_set<quint64> mPendingNameRequests;

        std::unordered_map<uint, std::unordered_multimap<uint, uint>> mSystemJumpMap;

        mutable std::unordered_map<uint, uint> mSolarSystemRegionCache;
        mutable std::unordered_map<quint64, uint> mLocationSolarSystemCache;

        mutable std::unordered_map<uint, double> mSecurityStatuses;

        mutable std::vector<MapLocation> mRegionCache;
        mutable std::unordered_map<uint, std::vector<MapLocation>> mConstellationCache, mSolarSystemCache;
        mutable std::unordered_map<uint, std::vector<Station>> mStationCache;

        mutable std::unordered_map<uint, QString> mRegionNameCache;
        mutable std::unordered_map<uint, QString> mSolarSystemNameCache;

        EveTypeRepository::EntityPtr getEveType(EveType::IdType id) const;

        MarketGroupRepository::EntityPtr getMarketGroupParent(MarketGroup::IdType id) const;
        MarketGroupRepository::EntityPtr getMarketGroup(MarketGroup::IdType id) const;

        uint getSolarSystemRegionId(uint stationId) const;
        uint getStationSolarSystemId(quint64 stationId) const;

        const ExternalOrderRepository::EntityList &getExternalOrders(EveType::IdType typeId, uint regionId) const;
    };
}