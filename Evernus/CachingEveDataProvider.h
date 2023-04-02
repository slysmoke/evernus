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
#include <mutex>

#include <QStringList>
#include <QHash>

#include <boost/functional/hash.hpp>

#include "ExternalOrderRepository.h"
#include "MarketGroupRepository.h"
#include "MetaGroupRepository.h"
#include "EveTypeRepository.h"
#include "EveDataProvider.h"
#include "ESIManager.h"
#include "Citadel.h"

class QDir;

namespace Evernus
{
    class DatabaseConnectionProvider;
    class EveDataManagerProvider;
    class MarketOrderRepository;

    class CachingEveDataProvider
        : public EveDataProvider
    {
        Q_OBJECT

    public:
        static const QString systemDistanceCacheFileName;

        CachingEveDataProvider(const EveTypeRepository &eveTypeRepository,
                               const MetaGroupRepository &metaGroupRepository,
                               const ExternalOrderRepository &externalOrderRepository,
                               const MarketOrderRepository &marketOrderRepository,
                               const MarketOrderRepository &corpMarketOrderRepository,
                               const MarketGroupRepository &marketGroupRepository,
                               const CitadelRepository &citadelRepository,
                               const EveDataManagerProvider &dataManagerProvider,
                               const DatabaseConnectionProvider &connectionProvider,
                               QObject *parent = nullptr);
        virtual ~CachingEveDataProvider();

        virtual QString getTypeName(EveType::IdType id) const override;
        virtual QString getTypeMarketGroupParentName(EveType::IdType id) const override;
        virtual QString getTypeMarketGroupName(EveType::IdType id) const override;
        virtual MarketGroup::IdType getTypeMarketGroupParentId(EveType::IdType id) const override;
        virtual const std::unordered_map<EveType::IdType, QString> &getAllTradeableTypeNames() const override;
        virtual const TypeList &getAllTradeableTypeIds() const override;
        virtual const TypeList &getCitadelTypeIds() const override;
        virtual QString getTypeMetaGroupName(EveType::IdType id) const override;
        virtual QString getGenericName(quint64 id) const override;
        virtual bool hasGenericName(quint64 id) const override;

        virtual double getTypeVolume(EveType::IdType id) const override;
        virtual std::shared_ptr<ExternalOrder> getTypeStationSellPrice(EveType::IdType id, quint64 stationId) const override;
        virtual std::shared_ptr<ExternalOrder> getTypeRegionSellPrice(EveType::IdType id, uint regionId) const override;
        virtual std::shared_ptr<ExternalOrder> getTypeBuyPrice(EveType::IdType id, quint64 stationId, int range = -1) const override;

        virtual void updateExternalOrders(const std::vector<ExternalOrder> &orders) override;
        virtual void clearExternalOrders() override;
        virtual void clearExternalOrdersForType(EveType::IdType id) override;

        virtual QString getLocationName(quint64 id) const override;
        virtual QString getRegionName(uint id) const override;
        virtual QString getSolarSystemName(uint id) const override;

        virtual const std::vector<MapLocation> &getRegions() const override;
        virtual const std::vector<MapLocation> &getConstellations(uint regionId) const override;
        virtual const std::vector<MapTreeLocation> &getConstellations() const override;
        virtual const std::vector<MapLocation> &getSolarSystemsForConstellation(uint constellationId) const override;
        virtual const std::vector<MapLocation> &getSolarSystemsForRegion(uint regionId) const override;
        virtual const std::vector<MapTreeLocation> &getSolarSystems() const override;
        virtual const std::vector<Station> &getStations(uint solarSystemId) const override;

        virtual double getSolarSystemSecurityStatus(uint solarSystemId) const override;
        virtual uint getSolarSystemConstellationId(uint solarSystemId) const override;
        virtual uint getSolarSystemRegionId(uint systemId) const override;

        virtual uint getStationRegionId(quint64 stationId) const override;
        virtual uint getStationSolarSystemId(quint64 stationId) const override;

        virtual const CitadelRepository::EntityList getCitadelsForRegion(uint regionId) const override;
        virtual const CitadelRepository::EntityList &getCitadels() const override;

        virtual const ReprocessingMap &getOreReprocessingInfo() const override;
        virtual const ReprocessingMap &getTypeReprocessingInfo(const TypeList &requestedTypes) const override;

        virtual uint getGroupId(const QString &name) const override;

        virtual uint getDistance(uint startSystem, uint endSystem) const override;

        virtual QString getRaceName(uint raceId) const override;
        virtual QString getBloodlineName(uint bloodlineId) const override;
        virtual QString getAncestryName(uint ancestryId) const override;

        virtual const ManufacturingInfo &getTypeManufacturingInfo(EveType::IdType typeId) const override;
        virtual EveType::IdType getBlueprintOutputType(EveType::IdType blueprintId) const override;

        void precacheNames();
        void precacheJumpMap();
        void precacheRefTypes();

        void clearExternalOrderCaches();
        void clearStationCache();
        void clearCitadelCache();

        void handleNewPreferences();

        std::shared_ptr<ExternalOrder> getTypeSellPrice(EveType::IdType id, quint64 stationId, bool dontThrow) const;

        static QDir getCacheDir();

    signals:
        void genericNameRequested(quint64 id) const;

    private slots:
        void fetchGenericName(quint64 id);

    private:
        using TypeLocationPair = std::pair<EveType::IdType, quint64>;
        using TypeRegionPair = std::pair<EveType::IdType, uint>;

        using NameMap = QHash<quint64, QString>;

        static const QString nameCacheFileName;
        static const QString raceCacheFileName;
        static const QString bloodlineCacheFileName;
        static const QString ancestryCacheFileName;

        static const QStringList oreGroupNames;

        const EveTypeRepository &mEveTypeRepository;
        const MetaGroupRepository &mMetaGroupRepository;
        const ExternalOrderRepository &mExternalOrderRepository;
        const MarketOrderRepository &mMarketOrderRepository, &mCorpMarketOrderRepository;
        const MarketGroupRepository &mMarketGroupRepository;
        const CitadelRepository &mCitadelRepository;

        const EveDataManagerProvider &mDataManagerProvider;

        const DatabaseConnectionProvider &mConnectionProvider;

        mutable std::unordered_map<EveType::IdType, QString> mTradeableTypeNameCache;
        mutable TypeList mTradeableTypeCache;
        mutable TypeList mCitadelTypeCache;
        mutable std::unordered_map<EveType::IdType, MetaGroupRepository::EntityPtr> mTypeMetaGroupCache;
        mutable std::unordered_map<EveType::IdType, EveTypeRepository::EntityPtr> mTypeCache;

        mutable std::unordered_map<TypeLocationPair, ExternalOrderRepository::EntityPtr, boost::hash<TypeLocationPair>>
        mStationSellPrices;
        mutable std::unordered_map<TypeLocationPair, ExternalOrderRepository::EntityPtr, boost::hash<TypeLocationPair>>
        mRegionSellPrices;
        mutable std::unordered_map<TypeLocationPair, ExternalOrderRepository::EntityPtr, boost::hash<TypeLocationPair>>
        mBuyPrices;

        mutable std::unordered_map<TypeRegionPair, ExternalOrderRepository::EntityList, boost::hash<TypeRegionPair>>
        mTypeRegionOrderCache;

        mutable std::unordered_map<quint64, QString> mLocationNameCache;

        mutable std::unordered_map<EveType::IdType, MarketGroupRepository::EntityPtr> mTypeMarketGroupParentCache;
        mutable std::unordered_map<EveType::IdType, MarketGroupRepository::EntityPtr> mTypeMarketGroupCache;

        mutable NameMap mGenericNameCache;
        mutable std::unordered_set<quint64> mPendingNameRequests;

        std::unordered_map<uint, std::unordered_multimap<uint, uint>> mSystemJumpMap;

        mutable std::unordered_map<uint, uint> mSolarSystemRegionCache;
        mutable std::unordered_map<uint, uint> mSolarSystemConstellationCache;
        mutable std::unordered_map<quint64, uint> mLocationSolarSystemCache;

        mutable std::mutex mLocationSolarSystemCacheMutex;
        mutable std::mutex mTypeCacheMutex;
        mutable std::recursive_mutex mExternalOrderCacheMutex;
        mutable std::mutex mSolarSystemRegionCacheMutex;
        mutable std::recursive_mutex mGenericNameCacheMutex;

        mutable std::unordered_map<uint, double> mSecurityStatuses;

        mutable std::vector<MapLocation> mRegionCache;
        mutable std::unordered_map<uint, std::vector<MapLocation>> mConstellationCache, mConstellationSolarSystemCache, mRegionSolarSystemCache;
        mutable std::unordered_map<uint, std::vector<Station>> mStationCache;
        mutable std::unordered_map<Citadel::IdType, CitadelRepository::EntityPtr> mCitadelCache;
        mutable std::unordered_map<uint, CitadelRepository::EntityList> mRegionCitadelCache;

        mutable std::vector<MapTreeLocation> mAllConstellationsCache;
        mutable std::vector<MapTreeLocation> mAllSolarSystemsCache;
        mutable CitadelRepository::EntityList mAllCitadelsCache;

        mutable std::unordered_map<uint, QString> mRegionNameCache;
        mutable std::unordered_map<uint, QString> mSolarSystemNameCache;

        mutable std::unordered_map<quint64, uint> mStationRegionCache;

        mutable QHash<QString, uint> mGroupIdCache;

        bool mUsePackagedVolume = false;

        mutable QHash<QPair<uint, uint>, uint> mSystemDistances;

        mutable ReprocessingMap mOreReprocessingInfo;
        mutable ReprocessingMap mTypeReprocessingInfo;

        mutable std::unordered_map<EveType::IdType, ManufacturingInfo> mTypeManufacturingInfoCache;
        mutable std::unordered_map<EveType::IdType, EveType::IdType> mBlueprintOutputCache;

        NameMap mRaceNameCache;
        NameMap mBloodlineNameCache;
        NameMap mAncestryNameCache;

        uint mManufacturingActivityId = 1; // 1 - fallback id at the time of writing

        EveTypeRepository::EntityPtr getEveType(EveType::IdType id) const;

        MarketGroupRepository::EntityPtr getMarketGroupParent(MarketGroup::IdType id) const;
        MarketGroupRepository::EntityPtr getMarketGroup(MarketGroup::IdType id) const;

        const ExternalOrderRepository::EntityList &getExternalOrders(EveType::IdType typeId, uint regionId) const;

        QString getCitadelName(Citadel::IdType id) const;
        uint getCitadelRegionId(Citadel::IdType id) const;
        uint getCitadelSolarSystemId(Citadel::IdType id) const;
        const Citadel &getCitadel(Citadel::IdType id) const;

        void findManufaturingActivity();
        void readCache(const QString &cacheFileName, NameMap &cache);

        template<class Cache>
        void cacheWrite(const QString &cacheFileName, const Cache &cache) const;

        static ESIManager::Callback<ESIManager::NameMap> getFillNameMapCallback(NameMap &target);

        static double getPackagedVolume(const EveType &type);
    };
}
