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
#include <queue>

#include <QtDebug>

#include <QStandardPaths>
#include <QSqlDatabase>
#include <QDataStream>
#include <QSqlQuery>
#include <QSettings>
#include <QFile>
#include <QDir>

#include "DatabaseConnectionProvider.h"
#include "EveDataManagerProvider.h"
#include "MarketOrderRepository.h"
#include "UISettings.h"

#include "CachingEveDataProvider.h"

namespace Evernus
{
    const QString CachingEveDataProvider::nameCacheFileName = "generic_names";
    const QString CachingEveDataProvider::raceCacheFileName = "race_names";
    const QString CachingEveDataProvider::bloodlineCacheFileName = "bloodline_names";
    const QString CachingEveDataProvider::ancestryCacheFileName = "ancestry_names";

    const QString CachingEveDataProvider::systemDistanceCacheFileName = "system_distances";

    const QStringList CachingEveDataProvider::oreGroupNames = {
        QStringLiteral("Veldspar"),
        QStringLiteral("Scordite"),
        QStringLiteral("Pyroxeres"),
        QStringLiteral("Plagioclase"),
        QStringLiteral("Omber"),
        QStringLiteral("Kernite"),
        QStringLiteral("Jaspet"),
        QStringLiteral("Hemorphite"),
        QStringLiteral("Hedbergite"),
        QStringLiteral("Gneiss"),
        QStringLiteral("Dark Ochre"),
        QStringLiteral("Spodumain"),
        QStringLiteral("Crokite"),
        QStringLiteral("Bistot"),
        QStringLiteral("Arkonor"),
        QStringLiteral("Mercoxit"),
        QStringLiteral("Ice")
    };

    CachingEveDataProvider::CachingEveDataProvider(const EveTypeRepository &eveTypeRepository,
                                                   const MetaGroupRepository &metaGroupRepository,
                                                   const ExternalOrderRepository &externalOrderRepository,
                                                   const MarketOrderRepository &marketOrderRepository,
                                                   const MarketOrderRepository &corpMarketOrderRepository,
                                                   const MarketGroupRepository &marketGroupRepository,
                                                   const CitadelRepository &citadelRepository,
                                                   const EveDataManagerProvider &dataManagerProvider,
                                                   const DatabaseConnectionProvider &connectionProvider,
                                                   QObject *parent)
        : EveDataProvider{parent}
        , mEveTypeRepository{eveTypeRepository}
        , mMetaGroupRepository{metaGroupRepository}
        , mExternalOrderRepository{externalOrderRepository}
        , mMarketOrderRepository{marketOrderRepository}
        , mCorpMarketOrderRepository{corpMarketOrderRepository}
        , mMarketGroupRepository{marketGroupRepository}
        , mCitadelRepository{citadelRepository}
        , mDataManagerProvider{dataManagerProvider}
        , mConnectionProvider{connectionProvider}
    {
        readCache(nameCacheFileName, mGenericNameCache);
        findManufaturingActivity();
        handleNewPreferences();

        connect(this, &CachingEveDataProvider::genericNameRequested, this, &CachingEveDataProvider::fetchGenericName);
    }

    CachingEveDataProvider::~CachingEveDataProvider()
    {
        try
        {
            const auto dataCacheDir = getCacheDir();
            if (dataCacheDir.mkpath(QStringLiteral(".")))
            {
                cacheWrite(nameCacheFileName, mGenericNameCache);
                cacheWrite(systemDistanceCacheFileName, mSystemDistances);
                cacheWrite(raceCacheFileName, mRaceNameCache);
                cacheWrite(bloodlineCacheFileName, mBloodlineNameCache);
                cacheWrite(ancestryCacheFileName, mAncestryNameCache);
            }
        }
        catch (...)
        {
        }
    }

    QString CachingEveDataProvider::getTypeName(EveType::IdType id) const
    {
        return getEveType(id)->getName();
    }

    QString CachingEveDataProvider::getTypeMarketGroupParentName(EveType::IdType id) const
    {
        const auto type = getEveType(id);
        const auto marketGroupId = type->getMarketGroupId();
        return (marketGroupId) ? (getMarketGroupParent(*marketGroupId)->getName()) : (QString{});
    }

    QString CachingEveDataProvider::getTypeMarketGroupName(EveType::IdType id) const
    {
        const auto type = getEveType(id);
        const auto marketGroupId = type->getMarketGroupId();
        return (marketGroupId) ? (getMarketGroup(*marketGroupId)->getName()) : (QString{});
    }

    MarketGroup::IdType CachingEveDataProvider::getTypeMarketGroupParentId(EveType::IdType id) const
    {
        const auto type = getEveType(id);
        const auto marketGroupId = type->getMarketGroupId();
        return (marketGroupId) ? (getMarketGroupParent(*marketGroupId)->getId()) : (MarketGroup::invalidId);
    }

    const std::unordered_map<EveType::IdType, QString> &CachingEveDataProvider::getAllTradeableTypeNames() const
    {
        if (!mTradeableTypeNameCache.empty())
            return mTradeableTypeNameCache;

        mTradeableTypeNameCache = mEveTypeRepository.fetchAllTradeableNames();
        return mTradeableTypeNameCache;
    }

    const CachingEveDataProvider::TypeList &CachingEveDataProvider::getAllTradeableTypeIds() const
    {
        if (!mTradeableTypeCache.empty())
            return mTradeableTypeCache;

        mTradeableTypeCache = mEveTypeRepository.fetchAllTradeableIds();
        return mTradeableTypeCache;
    }

    const CachingEveDataProvider::TypeList &CachingEveDataProvider::getCitadelTypeIds() const
    {
        if (!mCitadelTypeCache.empty())
            return mCitadelTypeCache;

        mCitadelTypeCache = mEveTypeRepository.fetchCitadelIds();
        return mCitadelTypeCache;
    }

    QString CachingEveDataProvider::getTypeMetaGroupName(EveType::IdType id) const
    {
        const auto it = mTypeMetaGroupCache.find(id);
        if (it != std::end(mTypeMetaGroupCache))
            return it->second->getName();

        MetaGroupRepository::EntityPtr result;

        try
        {
            result = mMetaGroupRepository.fetchForType(id);
        }
        catch (const MetaGroupRepository::NotFoundException &)
        {
            result = std::make_shared<MetaGroup>();
        }

        mTypeMetaGroupCache.emplace(id, result);
        return result->getName();
    }

    QString CachingEveDataProvider::getGenericName(quint64 id) const
    {
        if (Q_UNLIKELY(id == 0))
            return tr("(unknown)");

        std::lock_guard<std::recursive_mutex> lock{mGenericNameCacheMutex};

        if (mGenericNameCache.contains(id))
            return mGenericNameCache[id];

        if (mPendingNameRequests.find(id) == std::end(mPendingNameRequests))
        {
            emit genericNameRequested(id);
        }

        return tr("(unknown)");
    }

    bool CachingEveDataProvider::hasGenericName(quint64 id) const
    {
        if (Q_UNLIKELY(id == 0))
            return true;

        std::lock_guard<std::recursive_mutex> lock{mGenericNameCacheMutex};
        return mGenericNameCache.contains(id);
    }

    double CachingEveDataProvider::getTypeVolume(EveType::IdType id) const
    {
        std::lock_guard<std::mutex> lock{mTypeCacheMutex};

        const auto it = mTypeCache.find(id);
        if (it != std::end(mTypeCache))
            return (mUsePackagedVolume) ? (getPackagedVolume(*it->second)) : (it->second->getVolume());

        EveTypeRepository::EntityPtr result;

        try
        {
            result = mEveTypeRepository.find(id);
        }
        catch (const EveTypeRepository::NotFoundException &)
        {
            result = std::make_shared<EveType>();
        }

        mTypeCache.emplace(id, result);
        return (mUsePackagedVolume) ? (getPackagedVolume(*result)) : (result->getVolume());
    }

    std::shared_ptr<ExternalOrder> CachingEveDataProvider::getTypeStationSellPrice(EveType::IdType id, quint64 stationId) const
    {
        return getTypeSellPrice(id, stationId, true);
    }

    std::shared_ptr<ExternalOrder> CachingEveDataProvider::getTypeRegionSellPrice(EveType::IdType id, uint regionId) const
    {
        const auto key = std::make_pair(id, regionId);
        const auto it = mRegionSellPrices.find(key);
        if (it != std::end(mRegionSellPrices))
            return it->second;

        std::shared_ptr<ExternalOrder> result;

        try
        {
            result = mExternalOrderRepository.findSellByTypeAndRegion(id, regionId, mMarketOrderRepository, mCorpMarketOrderRepository);
        }
        catch (const ExternalOrderRepository::NotFoundException &)
        {
            result = std::make_shared<ExternalOrder>();
        }

        mRegionSellPrices.emplace(key, result);
        return result;
    }

    std::shared_ptr<ExternalOrder> CachingEveDataProvider::getTypeBuyPrice(EveType::IdType id, quint64 stationId, int range) const
    {
        std::lock_guard<std::recursive_mutex> lock{mExternalOrderCacheMutex};

        const auto key = std::make_pair(id, stationId);
        const auto it = mBuyPrices.find(key);
        if (it != std::end(mBuyPrices))
            return it->second;

        auto result = std::make_shared<ExternalOrder>();
        mBuyPrices.emplace(key, result);

        const auto solarSystemId = getStationSolarSystemId(stationId);
        if (solarSystemId == 0)
            return result;

        const auto regionId = getSolarSystemRegionId(solarSystemId);
        if (regionId == 0)
            return result;

        const uint realRange = (range < 0) ? (0) : (range);
        const auto &orders = getExternalOrders(id, regionId);
        for (const auto &order : orders)
        {
            if (order->getPrice() <= result->getPrice())
                continue;

            if (order->getRange() == -1)
            {
                if (range == -1)
                {
                    if (order->getStationId() != stationId)
                        continue;
                }
                else
                {
                    if (getDistance(solarSystemId, order->getSolarSystemId()) > realRange)
                        continue;
                }
            }
            else if (getDistance(solarSystemId, order->getSolarSystemId()) > order->getRange() + realRange)
            {
                continue;
            }

            result = order;
        }

        mBuyPrices[key] = result;
        return result;
    }

    void CachingEveDataProvider::updateExternalOrders(const std::vector<ExternalOrder> &orders)
    {
        TypeLocationPairs affectedOrders;

        std::vector<std::reference_wrapper<const ExternalOrder>> toStore;
        for (const auto &order : orders)
        {
            toStore.emplace_back(std::cref(order));
            affectedOrders.emplace(std::make_pair(order.getTypeId(), order.getRegionId()));
        }

        clearExternalOrderCaches();

        mExternalOrderRepository.removeObsolete(affectedOrders);
        mExternalOrderRepository.batchStore(toStore, true);
    }

    void CachingEveDataProvider::clearExternalOrders()
    {
        clearExternalOrderCaches();
        mExternalOrderRepository.removeAll();
    }

    void CachingEveDataProvider::clearExternalOrdersForType(EveType::IdType id)
    {
        clearExternalOrderCaches();
        mExternalOrderRepository.removeForType(id);
    }

    QString CachingEveDataProvider::getLocationName(quint64 id) const
    {
        const auto it = mLocationNameCache.find(id);
        if (it != std::end(mLocationNameCache))
            return it->second;

        QString result;
        if (id >= 66000000 && id <= 66014933)
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT stationName FROM staStations WHERE stationID = ?"));
            query.bindValue(0, id - 6000001);

            DatabaseUtils::execQuery(query);
            if (query.next())
                result = query.value(0).toString();
        }
        else if (id > 60000000 && id <= 61000000)
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT stationName FROM staStations WHERE stationID = ?"));
            query.bindValue(0, id);

            DatabaseUtils::execQuery(query);
            if (query.next())
                result = query.value(0).toString();
        }
        else
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT itemName FROM mapDenormalize WHERE itemID = ?"));
            query.bindValue(0, id);

            DatabaseUtils::execQuery(query);
            if (query.next())
                result = query.value(0).toString();
        }

        if (result.isEmpty())   // citadel?
        {
            result = getCitadelName(id);
            if (result.isEmpty())   // still nothing? give some feedback
                result = tr("- unknown location -");
        }

        mLocationNameCache.emplace(id, result);
        return result;
    }

    QString CachingEveDataProvider::getRegionName(uint id) const
    {
        const auto it = mRegionNameCache.find(id);
        if (it != std::end(mRegionNameCache))
            return it->second;

        QSqlQuery query{mConnectionProvider.getConnection()};
        query.prepare(QStringLiteral("SELECT regionName FROM mapRegions WHERE regionID = ?"));
        query.bindValue(0, id);

        DatabaseUtils::execQuery(query);
        query.next();

        return mRegionNameCache.emplace(id, query.value(0).toString()).first->second;
    }

    QString CachingEveDataProvider::getSolarSystemName(uint id) const
    {
        const auto it = mSolarSystemNameCache.find(id);
        if (it != std::end(mSolarSystemNameCache))
            return it->second;

        QSqlQuery query{mConnectionProvider.getConnection()};
        query.prepare(QStringLiteral("SELECT solarSystemName FROM mapSolarSystems WHERE solarSystemID = ?"));
        query.bindValue(0, id);

        DatabaseUtils::execQuery(query);
        if (!query.next())
            return mSolarSystemNameCache.emplace(id, QString{}).first->second;

        return mSolarSystemNameCache.emplace(id, query.value(0).toString()).first->second;
    }

    const std::vector<EveDataProvider::MapLocation> &CachingEveDataProvider::getRegions() const
    {
        if (mRegionCache.empty())
        {
            auto query = mConnectionProvider.getConnection().exec(QStringLiteral("SELECT regionID, regionName FROM mapRegions ORDER BY regionName"));

            const auto size = query.size();
            if (size > 0)
                mRegionCache.reserve(size);

            while (query.next())
                mRegionCache.emplace_back(std::make_pair(query.value(0).toUInt(), query.value(1).toString()));
        }

        return mRegionCache;
    }

    const std::vector<EveDataProvider::MapLocation> &CachingEveDataProvider::getConstellations(uint regionId) const
    {
        if (mConstellationCache.find(regionId) == std::end(mConstellationCache))
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT constellationID, constellationName FROM mapConstellations WHERE regionID = ? ORDER BY constellationName"));
            query.bindValue(0, regionId);

            DatabaseUtils::execQuery(query);

            auto &constellations = mConstellationCache[regionId];

            const auto size = query.size();
            if (size > 0)
                constellations.reserve(size);

            while (query.next())
                constellations.emplace_back(std::make_pair(query.value(0).toUInt(), query.value(1).toString()));
        }

        return mConstellationCache[regionId];
    }

    const std::vector<EveDataProvider::MapTreeLocation> &CachingEveDataProvider::getConstellations() const
    {
        if (BOOST_UNLIKELY(mAllConstellationsCache.empty()))
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT constellationID, constellationName, regionID FROM mapConstellations ORDER BY constellationName"));

            DatabaseUtils::execQuery(query);

            const auto size = query.size();
            if (size > 0)
                mAllConstellationsCache.reserve(size);

            mConstellationCache.clear();

            while (query.next())
            {
                MapTreeLocation location{query.value(2).toUInt(), query.value(0).toUInt(), query.value(1).toString()};

                mAllConstellationsCache.emplace_back(location);
                mConstellationCache[location.mParent].emplace_back(std::make_pair(location.mId, location.mName));
            }
        }

        return mAllConstellationsCache;
    }

    const std::vector<EveDataProvider::MapLocation> &CachingEveDataProvider::getSolarSystemsForConstellation(uint constellationId) const
    {
        if (mConstellationSolarSystemCache.find(constellationId) == std::end(mConstellationSolarSystemCache))
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT solarSystemID, solarSystemName FROM mapSolarSystems WHERE constellationID = ? ORDER BY solarSystemName"));
            query.bindValue(0, constellationId);

            DatabaseUtils::execQuery(query);

            auto &systems = mConstellationSolarSystemCache[constellationId];

            const auto size = query.size();
            if (size > 0)
                systems.reserve(size);

            while (query.next())
                systems.emplace_back(std::make_pair(query.value(0).toUInt(), query.value(1).toString()));
        }

        return mConstellationSolarSystemCache[constellationId];
    }

    const std::vector<EveDataProvider::MapLocation> &CachingEveDataProvider::getSolarSystemsForRegion(uint regionId) const
    {
        if (mRegionSolarSystemCache.find(regionId) == std::end(mRegionSolarSystemCache))
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT solarSystemID, solarSystemName FROM mapSolarSystems WHERE regionID = ? ORDER BY solarSystemName"));
            query.bindValue(0, regionId);

            DatabaseUtils::execQuery(query);

            auto &systems = mRegionSolarSystemCache[regionId];

            const auto size = query.size();
            if (size > 0)
                systems.reserve(size);

            while (query.next())
                systems.emplace_back(std::make_pair(query.value(0).toUInt(), query.value(1).toString()));
        }

        return mRegionSolarSystemCache[regionId];
    }

    const std::vector<EveDataProvider::MapTreeLocation> &CachingEveDataProvider::getSolarSystems() const
    {
        if (BOOST_UNLIKELY(mAllSolarSystemsCache.empty()))
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT solarSystemID, solarSystemName, constellationID FROM mapSolarSystems ORDER BY solarSystemName"));

            DatabaseUtils::execQuery(query);

            const auto size = query.size();
            if (size > 0)
                mAllSolarSystemsCache.reserve(size);

            mConstellationSolarSystemCache.clear();

            while (query.next())
            {
                MapTreeLocation location{query.value(2).toUInt(), query.value(0).toUInt(), query.value(1).toString()};

                mAllSolarSystemsCache.emplace_back(location);
                mConstellationSolarSystemCache[location.mParent].emplace_back(std::make_pair(location.mId, location.mName));
            }
        }

        return mAllSolarSystemsCache;
    }

    const std::vector<EveDataProvider::Station> &CachingEveDataProvider::getStations(uint solarSystemId) const
    {
        if (mStationCache.find(solarSystemId) == std::end(mStationCache))
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral(
                "SELECT stationID id, stationName name FROM staStations WHERE solarSystemID = ? "
                "UNION "
                "SELECT itemID id, itemName name FROM mapDenormalize WHERE solarSystemID = ?"));
            query.addBindValue(solarSystemId);
            query.addBindValue(solarSystemId);

            DatabaseUtils::execQuery(query);

            auto &stations = mStationCache[solarSystemId];

            const auto size = query.size();
            if (size > 0)
                stations.reserve(size);

            while (query.next())
                stations.emplace_back(std::make_pair(query.value(0).toUInt(), query.value(1).toString()));

            const auto citadels = mCitadelRepository.fetchForSolarSystem(solarSystemId);
            for (const auto &citadel : citadels)
                stations.emplace_back(std::make_pair(citadel->getId(), citadel->getName()));

            std::sort(std::begin(stations), std::end(stations), [](const auto &a, const auto &b) {
                return a.second < b.second;
            });
        }

        return mStationCache[solarSystemId];
    }

    double CachingEveDataProvider::getSolarSystemSecurityStatus(uint solarSystemId) const
    {
        const auto it = mSecurityStatuses.find(solarSystemId);
        if (it != std::end(mSecurityStatuses))
            return it->second;

        QSqlQuery query{mConnectionProvider.getConnection()};
        query.prepare(QStringLiteral("SELECT security FROM mapSolarSystems WHERE solarSystemID = ?"));
        query.bindValue(0, solarSystemId);

        DatabaseUtils::execQuery(query);

        if (!query.next())
            return mSecurityStatuses.emplace(solarSystemId, 0.).first->second;

        return mSecurityStatuses.emplace(solarSystemId, query.value(0).toDouble()).first->second;
    }

    uint CachingEveDataProvider::getSolarSystemConstellationId(uint solarSystemId) const
    {
        const auto it = mSolarSystemConstellationCache.find(solarSystemId);
        if (it != std::end(mSolarSystemConstellationCache))
            return it->second;

        QSqlQuery query{mConnectionProvider.getConnection()};
        query.prepare(QStringLiteral("SELECT constellationID FROM mapSolarSystems WHERE solarSystemID = ?"));
        query.bindValue(0, solarSystemId);

        DatabaseUtils::execQuery(query);
        query.next();

        const auto constellationId = query.value(0).toUInt();

        mSolarSystemConstellationCache.emplace(solarSystemId, constellationId);
        return constellationId;
    }

    uint CachingEveDataProvider::getStationRegionId(quint64 stationId) const
    {
        const auto it = mStationRegionCache.find(stationId);
        if (it != std::end(mStationRegionCache))
            return it->second;

        uint result = 0;
        if (stationId >= 66000000 && stationId <= 66014933)
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT regionID FROM staStations WHERE stationID = ?"));
            query.bindValue(0, stationId - 6000001);

            DatabaseUtils::execQuery(query);
            if (query.next())
                result = query.value(0).toUInt();
        }
        else if ((stationId >= 66014934 && stationId <= 67999999) ||
                 (stationId >= 60014861 && stationId <= 60014928) ||
                 (stationId > 61000000))
        {
            const auto systemId = getStationSolarSystemId(stationId);
            if (systemId != 0)
                result = getSolarSystemRegionId(systemId);
        }
        else if (stationId > 60000000 && stationId <= 61000000)
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT regionID FROM staStations WHERE stationID = ?"));
            query.bindValue(0, stationId);

            DatabaseUtils::execQuery(query);
            if (query.next())
                result = query.value(0).toUInt();
        }
        else
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT regionID FROM mapDenormalize WHERE itemID = ?"));
            query.bindValue(0, stationId);

            DatabaseUtils::execQuery(query);
            if (query.next())
                result = query.value(0).toUInt();
        }

        if (result == 0)   // citadel?
            result = getCitadelRegionId(stationId);

        mStationRegionCache.emplace(stationId, result);
        return result;
    }

    uint CachingEveDataProvider::getStationSolarSystemId(quint64 stationId) const
    {
        std::lock_guard<std::mutex> lock{mLocationSolarSystemCacheMutex};

        const auto it = mLocationSolarSystemCache.find(stationId);
        if (it != std::end(mLocationSolarSystemCache))
            return it->second;

        uint systemId = 0;
        if (stationId >= 66000000 && stationId <= 66014933)
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT solarSystemID FROM staStations WHERE stationID = ?"));
            query.bindValue(0, stationId - 6000001);

            DatabaseUtils::execQuery(query);
            if (query.next())
                systemId = query.value(0).toUInt();
        }
        else if (stationId > 60000000 && stationId <= 61000000)
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT solarSystemID FROM staStations WHERE stationID = ?"));
            query.bindValue(0, stationId);

            DatabaseUtils::execQuery(query);
            if (query.next())
                systemId = query.value(0).toUInt();
        }
        else
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT solarSystemID FROM mapDenormalize WHERE itemID = ?"));
            query.bindValue(0, stationId);

            DatabaseUtils::execQuery(query);
            if (query.next())
                systemId = query.value(0).toUInt();
        }

        if (systemId == 0)  // citadel?
            systemId = getCitadelSolarSystemId(stationId);

        mLocationSolarSystemCache.emplace(stationId, systemId);
        return systemId;
    }

    const CitadelRepository::EntityList CachingEveDataProvider::getCitadelsForRegion(uint regionId) const
    {
        const auto citadels = mRegionCitadelCache.find(regionId);
        if (citadels != std::end(mRegionCitadelCache))
            return citadels->second;

        return mRegionCitadelCache.emplace(regionId, mCitadelRepository.fetchForRegion(regionId)).first->second;
    }

    const CitadelRepository::EntityList &CachingEveDataProvider::getCitadels() const
    {
        if (BOOST_UNLIKELY(mAllCitadelsCache.empty()))
        {
            mAllCitadelsCache = mCitadelRepository.fetchAll();

            mRegionCitadelCache.clear();

            for (const auto &citadel : mAllCitadelsCache)
                mRegionCitadelCache[citadel->getRegionId()].emplace_back(citadel);
        }

        return mAllCitadelsCache;
    }

    const CachingEveDataProvider::ReprocessingMap &CachingEveDataProvider::getOreReprocessingInfo() const
    {
        if (mOreReprocessingInfo.empty())
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral(R"(
SELECT m.typeID, m.materialTypeID, m.quantity, t.portionSize, t.groupID FROM invTypeMaterials m INNER JOIN (
    SELECT typeID, portionSize, groupID FROM invTypes WHERE groupID IN (
        SELECT groupID FROM invGroups WHERE groupName IN ('%1')
    ) AND marketGroupID IS NOT NULL
) t ON t.typeID = m.typeID
            )").arg(oreGroupNames.join(QStringLiteral("', '"))));

            DatabaseUtils::execQuery(query);

            while (query.next())
            {
                auto &info = mOreReprocessingInfo[query.value(0).value<EveType::IdType>()];
                info.mPortionSize = query.value(3).toUInt();
                info.mGroupId = query.value(4).toUInt();
                info.mMaterials.emplace_back(MaterialInfo{
                    query.value(1).value<EveType::IdType>(),
                    query.value(2).toUInt()
                });
            }
        }

        return mOreReprocessingInfo;
    }

    const CachingEveDataProvider::ReprocessingMap &CachingEveDataProvider::getTypeReprocessingInfo(const TypeList &requestedTypes) const
    {
        auto reducedTypes = requestedTypes;
        for (const auto &info : mTypeReprocessingInfo)
            reducedTypes.erase(info.first);

        if (!reducedTypes.empty())
        {
            QStringList typeNames;
            reducedTypes.reserve(reducedTypes.size());

            for (const auto id : reducedTypes)
                typeNames << QString::number(id);

            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral(R"(
SELECT m.typeID, m.materialTypeID, m.quantity, t.portionSize, t.groupID FROM invTypeMaterials m INNER JOIN (
    SELECT typeID, portionSize, groupID FROM invTypes WHERE typeID IN (%1) AND marketGroupID IS NOT NULL
) t ON t.typeID = m.typeID
            )").arg(typeNames.join(", ")));

            DatabaseUtils::execQuery(query);

            while (query.next())
            {
                auto &info = mTypeReprocessingInfo[query.value(0).value<EveType::IdType>()];
                info.mPortionSize = query.value(3).toUInt();
                info.mGroupId = query.value(4).toUInt();
                info.mMaterials.emplace_back(MaterialInfo{
                    query.value(1).value<EveType::IdType>(),
                    query.value(2).toUInt()
                });
            }
        }

        return mTypeReprocessingInfo;
    }

    uint CachingEveDataProvider::getGroupId(const QString &name) const
    {
        if (mGroupIdCache.contains(name))
            return mGroupIdCache[name];

        QSqlQuery query{mConnectionProvider.getConnection()};
        query.prepare(QStringLiteral("SELECT groupID FROM invGroups WHERE groupName = ?"));
        query.addBindValue(name);

        DatabaseUtils::execQuery(query);

        if (query.next())
        {
            const auto id = query.value(0).toUInt();

            mGroupIdCache[name] = id;
            return id;
        }

        mGroupIdCache[name] = 0;
        return 0;
    }

    void CachingEveDataProvider::precacheNames()
    {
        readCache(raceCacheFileName, mRaceNameCache);
        readCache(bloodlineCacheFileName, mBloodlineNameCache);
        readCache(ancestryCacheFileName, mAncestryNameCache);

        if (mRaceNameCache.isEmpty())
            mDataManagerProvider.getESIManager().fetchRaces(getFillNameMapCallback(mRaceNameCache));
        if (mBloodlineNameCache.isEmpty())
            mDataManagerProvider.getESIManager().fetchBloodlines(getFillNameMapCallback(mBloodlineNameCache));
        if (mAncestryNameCache.isEmpty())
            mDataManagerProvider.getESIManager().fetchAncestries(getFillNameMapCallback(mAncestryNameCache));
    }

    void CachingEveDataProvider::precacheJumpMap()
    {
        auto query = mConnectionProvider.getConnection().exec(QStringLiteral("SELECT fromRegionID, fromSolarSystemID, toSolarSystemID FROM mapSolarSystemJumps WHERE fromRegionID = toRegionID"));
        while (query.next())
            mSystemJumpMap[query.value(0).toUInt()].emplace(query.value(1).toUInt(), query.value(2).toUInt());

        const auto dataCacheDir = getCacheDir();

        QFile distanceCache{dataCacheDir.filePath(systemDistanceCacheFileName)};
        if (distanceCache.open(QIODevice::ReadOnly))
        {
            QDataStream cacheStream{&distanceCache};
            cacheStream >> mSystemDistances;
        }
    }

    void CachingEveDataProvider::clearExternalOrderCaches()
    {
        std::lock_guard<std::recursive_mutex> lock{mExternalOrderCacheMutex};

        mStationSellPrices.clear();
        mBuyPrices.clear();
        mTypeRegionOrderCache.clear();
    }

    void CachingEveDataProvider::clearStationCache()
    {
        mStationCache.clear();
    }

    void CachingEveDataProvider::clearCitadelCache()
    {
        mCitadelCache.clear();
        mRegionCitadelCache.clear();
        mAllCitadelsCache.clear();
    }

    void CachingEveDataProvider::handleNewPreferences()
    {
        QSettings settings;

        std::lock_guard<std::mutex> lock{mTypeCacheMutex};
        mUsePackagedVolume = settings.value(UISettings::usePackagedVolumeKey, UISettings::usePackagedVolumeDefault).toBool();
    }

    std::shared_ptr<ExternalOrder> CachingEveDataProvider::getTypeSellPrice(EveType::IdType id, quint64 stationId, bool dontThrow) const
    {
        std::lock_guard<std::recursive_mutex> lock{mExternalOrderCacheMutex};

        const auto key = std::make_pair(id, stationId);
        const auto it = mStationSellPrices.find(key);
        if (it != std::end(mStationSellPrices))
            return it->second;

        std::shared_ptr<ExternalOrder> result;

        try
        {
            result = mExternalOrderRepository.findSellByTypeAndStation(id, stationId, mMarketOrderRepository, mCorpMarketOrderRepository);
        }
        catch (const ExternalOrderRepository::NotFoundException &)
        {
            if (!dontThrow)
                throw;

            result = std::make_shared<ExternalOrder>();
        }

        mStationSellPrices.emplace(key, result);
        return result;
    }

    void CachingEveDataProvider::fetchGenericName(quint64 id)
    {
        qDebug() << "Fetching generic name:" << id;

        // potential recursive lock if sync invokation
        std::lock_guard<std::recursive_mutex> lock{mGenericNameCacheMutex};

        mPendingNameRequests.emplace(id);

        mDataManagerProvider.getESIManager().fetchGenericName(id, [=](auto &&data, const auto &error) {
            qDebug() << "Got generic name:" << id << data << " (" << error << ")";

            std::lock_guard<std::recursive_mutex> lock{mGenericNameCacheMutex};

            mPendingNameRequests.erase(id);

            if (error.isEmpty())
                mGenericNameCache[id] = std::move(data);
            else
                mGenericNameCache[id] = tr("(unknown)");

            if (mPendingNameRequests.empty())
                emit namesChanged();
        });
    }

    EveTypeRepository::EntityPtr CachingEveDataProvider::getEveType(EveType::IdType id) const
    {
        const auto it = mTypeCache.find(id);
        if (it != std::end(mTypeCache))
            return it->second;

        EveTypeRepository::EntityPtr type;

        try
        {
            type = mEveTypeRepository.find(id);
        }
        catch (const EveTypeRepository::NotFoundException &)
        {
            type = std::make_shared<EveType>();
        }

        return mTypeCache.emplace(id, type).first->second;
    }

    MarketGroupRepository::EntityPtr CachingEveDataProvider::getMarketGroupParent(MarketGroup::IdType id) const
    {
        const auto it = mTypeMarketGroupParentCache.find(id);
        if (it != std::end(mTypeMarketGroupParentCache))
            return it->second;

        MarketGroupRepository::EntityPtr result;

        try
        {
            result = mMarketGroupRepository.findParent(id);
        }
        catch (const MarketGroupRepository::NotFoundException &)
        {
            result = std::make_shared<MarketGroup>();
        }

        return mTypeMarketGroupParentCache.emplace(id, result).first->second;
    }

    MarketGroupRepository::EntityPtr CachingEveDataProvider::getMarketGroup(MarketGroup::IdType id) const
    {
        const auto it = mTypeMarketGroupCache.find(id);
        if (it != std::end(mTypeMarketGroupCache))
            return it->second;

        MarketGroupRepository::EntityPtr result;

        try
        {
            result = mMarketGroupRepository.find(id);
        }
        catch (const MarketGroupRepository::NotFoundException &)
        {
            result = std::make_shared<MarketGroup>();
        }

        return mTypeMarketGroupCache.emplace(id, result).first->second;
    }

    uint CachingEveDataProvider::getSolarSystemRegionId(uint systemId) const
    {
        std::lock_guard<std::mutex> lock{mSolarSystemRegionCacheMutex};

        const auto it = mSolarSystemRegionCache.find(systemId);
        if (it != std::end(mSolarSystemRegionCache))
            return it->second;

        QSqlQuery query{mConnectionProvider.getConnection()};
        query.prepare(QStringLiteral("SELECT regionID FROM mapSolarSystems WHERE solarSystemID = ?"));
        query.bindValue(0, systemId);

        DatabaseUtils::execQuery(query);
        query.next();

        const auto regionId = query.value(0).toUInt();

        mSolarSystemRegionCache.emplace(systemId, regionId);
        return regionId;
    }

    const ExternalOrderRepository::EntityList &CachingEveDataProvider::getExternalOrders(EveType::IdType typeId, uint regionId) const
    {
        std::lock_guard<std::recursive_mutex> lock{mExternalOrderCacheMutex};

        const auto key = std::make_pair(typeId, regionId);
        const auto it = mTypeRegionOrderCache.find(key);
        if (it != std::end(mTypeRegionOrderCache))
            return it->second;

        return mTypeRegionOrderCache.emplace(
            key, mExternalOrderRepository.findBuyByTypeAndRegion(typeId, regionId, mMarketOrderRepository, mCorpMarketOrderRepository))
                .first->second;
    }

    uint CachingEveDataProvider::getDistance(uint startSystem, uint endSystem) const
    {
        const auto key = qMakePair(startSystem, endSystem);
        const auto it = mSystemDistances.find(key);
        if (it != std::end(mSystemDistances))
            return it.value();

        const auto makeUnreachable = [=] {
            const auto value = std::numeric_limits<uint>::max();
            mSystemDistances[key] = value;
            mSystemDistances[qMakePair(endSystem, startSystem)] = value;

            return value;
        };

        const auto regionId = getSolarSystemRegionId(startSystem);
        if (regionId == 0)
            return makeUnreachable();

        const auto jIt = mSystemJumpMap.find(regionId);
        if (jIt == std::end(mSystemJumpMap))
            return makeUnreachable();

        const auto &jumpMap = jIt->second;

        std::unordered_set<uint> visited;
        std::queue<std::pair<uint, uint>> candidates;

        visited.emplace(startSystem);
        candidates.emplace(std::make_pair(startSystem, 0u));

        while (!candidates.empty())
        {
            const auto current = candidates.front();
            candidates.pop();

            const auto depth = current.second;
            if (current.first == endSystem)
            {
                mSystemDistances[key] = depth;
                mSystemDistances[qMakePair(endSystem, startSystem)] = depth;

                return depth;
            }

            const auto children = jumpMap.equal_range(current.first);
            for (auto it = children.first; it != children.second; ++it)
            {
                if (visited.find(it->second) == std::end(visited))
                {
                    visited.emplace(it->second);
                    candidates.emplace(std::make_pair(it->second, depth + 1));
                }
            }
        }

        return makeUnreachable();
    }

    QString CachingEveDataProvider::getRaceName(uint raceId) const
    {
        return mRaceNameCache.value(raceId);
    }

    QString CachingEveDataProvider::getBloodlineName(uint bloodlineId) const
    {
        return mBloodlineNameCache.value(bloodlineId);
    }

    QString CachingEveDataProvider::getAncestryName(uint ancestryId) const
    {
        return mAncestryNameCache.value(ancestryId);
    }

    const CachingEveDataProvider::ManufacturingInfo &CachingEveDataProvider::getTypeManufacturingInfo(EveType::IdType typeId) const
    {
        auto it = mTypeManufacturingInfoCache.find(typeId);
        if (it == std::end(mTypeManufacturingInfoCache))
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral(R"(
SELECT m.materialTypeID, m.quantity, p.quantity, a.time, s.skillID FROM industryActivityMaterials m
    INNER JOIN industryActivityProducts p
        ON m.typeID = p.typeID AND m.activityID = p.activityID AND p.productTypeID = ? AND p.activityID = ?
    INNER JOIN industryActivity a
        ON a.typeID = p.typeID AND a.activityID = m.activityID
    INNER JOIN industryActivitySkills s
        ON s.typeID = p.typeID AND s.activityID = m.activityID
            )"));
            query.addBindValue(typeId);
            query.addBindValue(mManufacturingActivityId);

            ManufacturingInfo info{0};

            DatabaseUtils::execQuery(query);

            std::unordered_set<EveType::IdType> usedMaterials;

            while (query.next())
            {
                const auto skillId = query.value(4).toUInt();
                if (skillId != industrySkillId && skillId != advancedIndustrySkillId)
                    info.mAdditionalsSkills.insert(skillId);

                const auto materialId = query.value(0).value<EveType::IdType>();
                if (usedMaterials.find(materialId) != std::end(usedMaterials))
                    continue;

                if (info.mQuantity == 0)
                {
                    info.mQuantity = query.value(2).toUInt();
                    info.mTime = std::chrono::seconds{query.value(3).toUInt()};
                }

                usedMaterials.emplace(materialId);
                info.mMaterials.emplace_back(MaterialInfo{materialId, query.value(1).toUInt()});
            }

            it = mTypeManufacturingInfoCache.emplace(typeId, std::move(info)).first;
        }

        return it->second;
    }

    EveType::IdType CachingEveDataProvider::getBlueprintOutputType(EveType::IdType blueprintId) const
    {
        auto type = mBlueprintOutputCache.find(blueprintId);
        if (type == std::end(mBlueprintOutputCache))
        {
            QSqlQuery query{mConnectionProvider.getConnection()};
            query.prepare(QStringLiteral("SELECT productTypeID FROM industryActivityProducts WHERE typeID = ? AND activityID = ?"));
            query.addBindValue(blueprintId);
            query.addBindValue(mManufacturingActivityId);

            DatabaseUtils::execQuery(query);

            if (query.next())
                type = mBlueprintOutputCache.emplace(blueprintId, query.value(0).value<EveType::IdType>()).first;
            else
                type = mBlueprintOutputCache.emplace(blueprintId, EveType::invalidId).first;
        }

        return type->second;
    }

    QString CachingEveDataProvider::getCitadelName(Citadel::IdType id) const
    {
        return getCitadel(id).getName();
    }

    uint CachingEveDataProvider::getCitadelRegionId(Citadel::IdType id) const
    {
        return getCitadel(id).getRegionId();
    }

    uint CachingEveDataProvider::getCitadelSolarSystemId(Citadel::IdType id) const
    {
        return getCitadel(id).getSolarSystemId();
    }

    const Citadel &CachingEveDataProvider::getCitadel(Citadel::IdType id) const
    {
        auto citadel = mCitadelCache.find(id);
        if (citadel == std::end(mCitadelCache))
        {
            if (mCitadelCache.empty())
            {
                auto citadels = mCitadelRepository.fetchAll();

                mCitadelCache.reserve(citadels.size());
                for (auto &citadel : citadels)
                    mCitadelCache.emplace(citadel->getId(), std::move(citadel));

                citadel = mCitadelCache.find(id);
                if (citadel == std::end(mCitadelCache))
                    citadel = mCitadelCache.emplace(id, std::make_shared<Citadel>(id)).first;
            }
            else
            {
                citadel = mCitadelCache.emplace(id, std::make_shared<Citadel>(id)).first;
            }
        }

        Q_ASSERT(citadel->second);
        return *citadel->second;
    }

    double CachingEveDataProvider::getPackagedVolume(const EveType &type)
    {
        // https://bitbucket.org/krojew/evernus/issue/30/utilize-packaged-size-for-total-size
        // thank you CCP for this cool and unexpected feature!
        const auto groupId = type.getGroupId();
        switch (groupId) {
        case 29:
        case 1022:
        case 31:
            return 500.;
        case 448:
        case 12:
        case 649:
        case 952:
        case 340:
        case 1699:
        case 1700:
        case 1706:
            return 1000.;
        case 324:
        case 830:
        case 893:
        case 25:
        case 831:
        case 237:
        case 834:
            return 2500.;
        case 543:
        case 463:
            return 3750.;
        case 420:
        case 541:
        case 963:
            return 5000.;
        case 906:
        case 26:
        case 833:
        case 358:
        case 894:
        case 832:
            return 10000.;
        case 1201:
        case 419:
        case 540:
            return 15000.;
        case 380:
        case 1202:
        case 28:
            return 20000.;
        case 27:
        case 898:
        case 381:
        case 900:
            return 50000.;
        case 941:
            return 500000.;
        case 883:
        case 547:
        case 485:
        case 513:
        case 902:
        case 659:
            return 1000000.;
        case 30:
            return 10000000.f;
            // these here are a pain - the group id is not enough
        case 38:
        case 40:
        case 41:
        case 61:
        case 62:
        case 63:
        case 67:
        case 68:
        case 71:
        case 76:
        case 325:
        case 585:
        case 1199:
        case 1697:
        case 1698:
            if (type.getVolume() > 1000.)
                return 1000.;
            break;
        case 60:
            if (type.getVolume() > 2000.)
                return 2000.;
        }

        return type.getVolume();
    }

    void CachingEveDataProvider::findManufaturingActivity()
    {
        QSqlQuery query{mConnectionProvider.getConnection()};
        query.prepare(QStringLiteral("SELECT activityID FROM ramActivities WHERE activityName = ?"));
        query.bindValue(0, QStringLiteral("Manufacturing"));

        DatabaseUtils::execQuery(query);
        if (query.next())
            mManufacturingActivityId = query.value(0).toUInt();
        else
            qWarning() << "Manufacturing activity id not found - assuming:" << mManufacturingActivityId;
    }

    void CachingEveDataProvider::readCache(const QString &cacheFileName, NameMap &cache)
    {
        QFile cacheFile{getCacheDir().filePath(cacheFileName)};
        if (cacheFile.open(QIODevice::ReadOnly))
        {
            QDataStream cacheStream{&cacheFile};
            cacheStream >> cache;
        }
    }

    template<class Cache>
    void CachingEveDataProvider::cacheWrite(const QString &cacheFileName, const Cache &cache) const
    {
        QFile cacheFile{getCacheDir().filePath(cacheFileName)};
        if (cacheFile.open(QIODevice::WriteOnly))
        {
            QDataStream cacheStream{&cacheFile};
            cacheStream << cache;
        }
    }

    QDir CachingEveDataProvider::getCacheDir()
    {
        return QDir{QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/data")};
    }

    ESIManager::Callback<ESIManager::NameMap> CachingEveDataProvider::getFillNameMapCallback(NameMap &target)
    {
        return [&](auto &&data, const auto &error, const auto &expires) {
            Q_UNUSED(expires);

            if (!error.isEmpty())
            {
                qWarning() << "Error precaching races:" << error;
                return;
            }

            for (const auto &race : data)
                target[race.first] = race.second;
        };
    }
}
