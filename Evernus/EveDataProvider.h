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

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <chrono>

#include <QDateTime>
#include <QVariant>
#include <QObject>

#include "CitadelRepository.h"
#include "MarketGroup.h"
#include "MetaGroup.h"
#include "EveType.h"

namespace Evernus
{
    class ExternalOrder;
    class Citadel;

    class EveDataProvider
        : public QObject
    {
        Q_OBJECT

    public:
        struct MaterialInfo
        {
            EveType::IdType mMaterialId;
            uint mQuantity;
        };

        struct ReprocessingInfo
        {
            uint mPortionSize;
            uint mGroupId;
            std::vector<MaterialInfo> mMaterials;
        };

        struct ManufacturingInfo
        {
            uint mQuantity;
            std::chrono::seconds mTime;
            std::vector<MaterialInfo> mMaterials;
            std::unordered_set<uint> mAdditionalsSkills;
        };

        struct MapTreeLocation
        {
            uint mParent;
            uint mId;
            QString mName;
        };

        using MapLocation = std::pair<uint, QString>;
        using Station = std::pair<quint64, QString>;
        using ReprocessingMap = std::unordered_map<EveType::IdType, ReprocessingInfo>;
        using TypeList = std::unordered_set<EveType::IdType>;

        static const uint industrySkillId = 3380;
        static const uint advancedIndustrySkillId = 3388;

        using QObject::QObject;
        virtual ~EveDataProvider() = default;

        virtual QString getTypeName(EveType::IdType id) const = 0;
        virtual QString getTypeMarketGroupParentName(EveType::IdType id) const = 0;
        virtual QString getTypeMarketGroupName(EveType::IdType id) const = 0;
        virtual MarketGroup::IdType getTypeMarketGroupParentId(EveType::IdType id) const = 0;
        virtual const std::unordered_map<EveType::IdType, QString> &getAllTradeableTypeNames() const = 0;
        virtual const TypeList &getAllTradeableTypeIds() const = 0;
        virtual const TypeList &getCitadelTypeIds() const = 0;
        virtual QString getTypeMetaGroupName(EveType::IdType id) const = 0;
        virtual QString getGenericName(quint64 id) const = 0;
        virtual bool hasGenericName(quint64 id) const = 0;

        virtual double getTypeVolume(EveType::IdType id) const = 0;
        virtual std::shared_ptr<ExternalOrder> getTypeStationSellPrice(EveType::IdType id, quint64 stationId) const = 0;
        virtual std::shared_ptr<ExternalOrder> getTypeRegionSellPrice(EveType::IdType id, uint regionId) const = 0;
        virtual std::shared_ptr<ExternalOrder> getTypeBuyPrice(EveType::IdType id, quint64 stationId, int range = -1) const = 0;

        virtual void updateExternalOrders(const std::vector<ExternalOrder> &orders) = 0;
        virtual void clearExternalOrders() = 0;
        virtual void clearExternalOrdersForType(EveType::IdType id) = 0;

        virtual QString getLocationName(quint64 id) const = 0;
        virtual QString getRegionName(uint id) const = 0;
        virtual QString getSolarSystemName(uint id) const = 0;

        virtual const std::vector<MapLocation> &getRegions() const = 0;
        virtual const std::vector<MapLocation> &getConstellations(uint regionId) const = 0;
        virtual const std::vector<MapTreeLocation> &getConstellations() const = 0;
        virtual const std::vector<MapLocation> &getSolarSystemsForConstellation(uint constellationId) const = 0;
        virtual const std::vector<MapLocation> &getSolarSystemsForRegion(uint regionId) const = 0;
        virtual const std::vector<MapTreeLocation> &getSolarSystems() const = 0;
        virtual const std::vector<Station> &getStations(uint solarSystemId) const = 0;

        virtual double getSolarSystemSecurityStatus(uint solarSystemId) const = 0;
        virtual uint getSolarSystemConstellationId(uint solarSystemId) const = 0;
        virtual uint getSolarSystemRegionId(uint systemId) const = 0;

        virtual uint getStationRegionId(quint64 stationId) const = 0;
        virtual uint getStationSolarSystemId(quint64 stationId) const = 0;

        virtual const CitadelRepository::EntityList getCitadelsForRegion(uint regionId) const = 0;
        virtual const CitadelRepository::EntityList &getCitadels() const = 0;

        virtual const ReprocessingMap &getOreReprocessingInfo() const = 0;
        virtual const ReprocessingMap &getTypeReprocessingInfo(const TypeList &requestedTypes) const = 0;

        virtual uint getGroupId(const QString &name) const = 0;

        virtual uint getDistance(uint startSystem, uint endSystem) const = 0;

        virtual QString getRaceName(uint raceId) const = 0;
        virtual QString getBloodlineName(uint bloodlineId) const = 0;
        virtual QString getAncestryName(uint ancestryId) const = 0;

        virtual const ManufacturingInfo &getTypeManufacturingInfo(EveType::IdType typeId) const = 0;
        virtual EveType::IdType getBlueprintOutputType(EveType::IdType blueprintId) const = 0;

        static quint64 getStationIdFromPath(const QVariantList &path);

    signals:
        void namesChanged() const;
    };
}
