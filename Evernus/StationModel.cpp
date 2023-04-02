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
#include "EveDataProvider.h"

#include "StationModel.h"

namespace Evernus
{
    StationModel::LocationNode::LocationNode(quint64 id, LocationNode *parent, size_t row, const QString &name, Type type)
        : mId{id}
        , mParent{parent}
        , mRow{row}
        , mName{name}
        , mType{type}
    {
    }

    StationModel::StationModel(const EveDataProvider &dataProvider, QObject *parent)
        : QAbstractItemModel{parent}
        , mDataProvider{dataProvider}
    {
    }

    bool StationModel::canFetchMore(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return mRegions.empty();

        const auto node = static_cast<const LocationNode *>(parent.internalPointer());
        switch (node->mType) {
        case LocationNode::Type::Region:
            return mConstellations.find(node->mId) == std::end(mConstellations);
        case LocationNode::Type::Constellation:
            return mSolarSystems.find(node->mId) == std::end(mSolarSystems);
        case LocationNode::Type::SolarSystem:
            return mStations.find(node->mId) == std::end(mStations);
        default:
            return false;
        }
    }

    int StationModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant StationModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return {};

        if (role == Qt::DisplayRole)
            return static_cast<const LocationNode *>(index.internalPointer())->mName;

        return {};
    }

    void StationModel::fetchMore(const QModelIndex &parent)
    {
        if (!parent.isValid())
        {
            const auto &regions = mDataProvider.getRegions();
            mRegions.reserve(regions.size());

            for (const auto &region : regions)
                mRegions.emplace_back(LocationNode{region.first, nullptr, mRegions.size(), region.second, LocationNode::Type::Region});

            return;
        }

        const auto node = static_cast<LocationNode *>(parent.internalPointer());
        switch (node->mType) {
        case LocationNode::Type::Region:
            {
                const auto &constellations = mDataProvider.getConstellations(node->mId);
                auto &target = mConstellations[node->mId];

                target.reserve(constellations.size());

                for (const auto &constellation : constellations)
                    target.emplace_back(LocationNode{constellation.first, node, target.size(), constellation.second, LocationNode::Type::Constellation});

                break;
            }
        case LocationNode::Type::Constellation:
            {
                const auto &systems = mDataProvider.getSolarSystemsForConstellation(node->mId);
                auto &target = mSolarSystems[node->mId];

                target.reserve(systems.size());

                for (const auto &system : systems)
                    target.emplace_back(LocationNode{system.first, node, target.size(), system.second, LocationNode::Type::SolarSystem});

                break;
            }
        case LocationNode::Type::SolarSystem:
            {
                const auto &stations = mDataProvider.getStations(node->mId);
                auto &target = mStations[node->mId];

                target.reserve(stations.size());

                for (const auto &station : stations)
                    target.emplace_back(LocationNode{station.first, node, target.size(), station.second, LocationNode::Type::Station});

                break;
            }
        default:
            break;
        }
    }

    bool StationModel::hasChildren(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return true;

        const auto node = static_cast<LocationNode *>(parent.internalPointer());
        return node->mType == LocationNode::Type::Region ||
               node->mType == LocationNode::Type::Constellation ||
               node->mType == LocationNode::Type::SolarSystem;
    }

    QModelIndex StationModel::index(int row, int column, const QModelIndex &parent) const
    {
        if (column != 0 || row < 0)
            return {};

        if (!parent.isValid())
        {
            // https://bugreports.qt-project.org/browse/QTBUG-40624
            if (mRegions.empty() || row >= static_cast<int>(mRegions.size()))
                return {};

            return createIndex(row, column, &mRegions[row]);
        }

        const auto node = static_cast<const LocationNode *>(parent.internalPointer());
        const auto safeIndex = [=](auto &collection) {
            return (row < static_cast<int>(collection[node->mId].size())) ? (createIndex(row, column, &collection[node->mId][row])) : (QModelIndex{});
        };

        switch (node->mType) {
        case LocationNode::Type::Region:
            return safeIndex(mConstellations);
        case LocationNode::Type::Constellation:
            return safeIndex(mSolarSystems);
        case LocationNode::Type::SolarSystem:
            return safeIndex(mStations);
        default:
            return {};
        }
    }

    QModelIndex StationModel::parent(const QModelIndex &index) const
    {
        if (!index.isValid())
            return {};

        const auto node = static_cast<const LocationNode *>(index.internalPointer());
        switch (node->mType) {
        case LocationNode::Type::Region:
            return {};
        case LocationNode::Type::Constellation:
        case LocationNode::Type::SolarSystem:
        case LocationNode::Type::Station:
            return createIndex(static_cast<int>(node->mParent->mRow), 0, node->mParent);
        }

        return {};
    }

    int StationModel::rowCount(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return static_cast<int>(mRegions.size());

        const auto node = static_cast<const LocationNode *>(parent.internalPointer());
        switch (node->mType) {
        case LocationNode::Type::Region:
            return static_cast<int>(mConstellations[node->mId].size());
        case LocationNode::Type::Constellation:
            return static_cast<int>(mSolarSystems[node->mId].size());
        case LocationNode::Type::SolarSystem:
            return static_cast<int>(mStations[node->mId].size());
        default:
            return 0;
        }
    }

    QModelIndex StationModel::index(quint64 id, const QModelIndex &parent)
    {
        if (canFetchMore(parent))
            fetchMore(parent);

        const auto finder = [id, &parent, this](const auto &collection) {
            const auto it = std::find_if(std::begin(collection), std::end(collection), [id](const auto &node) {
                return node.mId == id;
            });

            return (it == std::end(collection)) ? (QModelIndex{}) : (index(std::distance(std::begin(collection), it), 0, parent));
        };

        if (!parent.isValid())
            return finder(mRegions);

        const auto node = static_cast<const LocationNode *>(parent.internalPointer());
        switch (node->mType) {
        case LocationNode::Type::Region:
            return finder(mConstellations[node->mId]);
        case LocationNode::Type::Constellation:
            return finder(mSolarSystems[node->mId]);
        case LocationNode::Type::SolarSystem:
            return finder(mStations[node->mId]);
        default:
            return {};
        }
    }

    quint64 StationModel::getStationId(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0;

        const auto node = static_cast<const LocationNode *>(index.internalPointer());
        return (node->mType == LocationNode::Type::Station) ? (node->mId) : (0);
    }

    quint64 StationModel::getGenericId(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0;

        const auto node = static_cast<const LocationNode *>(index.internalPointer());
        return node->mId;
    }
}
