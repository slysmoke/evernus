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
#include <boost/scope_exit.hpp>

#include <QFont>

#include "CitadelAccessCache.h"
#include "EveDataProvider.h"

#include "CitadelManagementModel.h"

namespace Evernus
{
    CitadelManagementModel::LocationNode::LocationNode(quint64 id, LocationNode *parent, size_t row, const QString &name, Type type)
        : mId{id}
        , mParent{parent}
        , mRow{row}
        , mName{name}
        , mType{type}
    {
    }

    CitadelManagementModel::CitadelManagementModel(const EveDataProvider &dataProvider,
                                               const CitadelAccessCache &citadelAccessCache,
                                               Character::IdType charId,
                                               QObject *parent)
        : QAbstractItemModel{parent}
        , mDataProvider{dataProvider}
        , mCitadelAccessCache{citadelAccessCache}
        , mCharacterId{charId}
    {
        fillStaticData();
        refresh();
    }

    int CitadelManagementModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant CitadelManagementModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        const auto node = static_cast<const LocationNode *>(index.internalPointer());
        Q_ASSERT(node != nullptr);

        switch (role) {
        case Qt::DisplayRole:
            return node->mName;
        case Qt::CheckStateRole:
            return getNodeCheckState(*node);
        case Qt::FontRole:
            if (node->mBlacklisted)
            {
                QFont font;
                font.setStrikeOut(true);

                return font;
            }
        }

        return {};
    }

    Qt::ItemFlags CitadelManagementModel::flags(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0;

        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    QModelIndex CitadelManagementModel::index(int row, int column, const QModelIndex &parent) const
    {
        if (column != 0)
            return {};

        if (!parent.isValid())
            return createIndex(row, column, mRegions[row].get());

        const auto node = static_cast<const LocationNode *>(parent.internalPointer());
        Q_ASSERT(node != nullptr);

        switch (node->mType) {
        case LocationNode::Type::Region:
            return createIndex(row, column, mConstellations[node->mId][row].get());
        case LocationNode::Type::Constellation:
            return createIndex(row, column, mSolarSystems[node->mId][row].get());
        case LocationNode::Type::SolarSystem:
            return createIndex(row, column, mCitadels[node->mId][row].get());
        default:
            return {};
        }
    }

    QModelIndex CitadelManagementModel::parent(const QModelIndex &index) const
    {
        if (!index.isValid())
            return {};

        const auto node = static_cast<const LocationNode *>(index.internalPointer());
        Q_ASSERT(node != nullptr);

        switch (node->mType) {
        case LocationNode::Type::Region:
            return {};
        case LocationNode::Type::Constellation:
        case LocationNode::Type::SolarSystem:
        case LocationNode::Type::Citadel:
            return createIndex(static_cast<int>(node->mParent->mRow), 0, node->mParent);
        }

        return {};
    }

    int CitadelManagementModel::rowCount(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return static_cast<int>(mRegions.size());

        const auto node = static_cast<const LocationNode *>(parent.internalPointer());
        Q_ASSERT(node != nullptr);

        switch (node->mType) {
        case LocationNode::Type::Region:
            return static_cast<int>(mConstellations[node->mId].size());
        case LocationNode::Type::Constellation:
            return static_cast<int>(mSolarSystems[node->mId].size());
        case LocationNode::Type::SolarSystem:
            return static_cast<int>(mCitadels[node->mId].size());
        default:
            return 0;
        }
    }

    bool CitadelManagementModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (index.isValid() && role == Qt::CheckStateRole)
        {
            setCheckState(index, value.toInt() == Qt::Checked);

            auto parentIndex = parent(index);
            while (parentIndex.isValid())
            {
                emit dataChanged(parentIndex, parentIndex, { Qt::CheckStateRole });
                parentIndex = parent(parentIndex);
            }

            return true;
        }

        return false;
    }

    void CitadelManagementModel::refresh()
    {
        beginResetModel();

        BOOST_SCOPE_EXIT(this_) {
            this_->endResetModel();
        } BOOST_SCOPE_EXIT_END

        mCitadels.clear();

        const auto &citadels = mDataProvider.getCitadels();
        for (const auto &citadel : citadels)
        {
            const auto solarSystem = mSolarSystemMap.find(citadel->getSolarSystemId());
            if (Q_LIKELY(solarSystem != std::end(mSolarSystemMap)))
            {
                auto &target = mCitadels[citadel->getSolarSystemId()];
                target.emplace_back(std::make_unique<LocationNode>(citadel->getId(), solarSystem->second, target.size(), citadel->getName(), LocationNode::Type::Citadel));

                const auto &citadelNode = target.back();
                citadelNode->mSelected = citadel->isIgnored();
                citadelNode->mBlacklisted = !mCitadelAccessCache.isAvailable(mCharacterId, citadel->getId());
            }
        }
    }

    CitadelManagementModel::CitadelList CitadelManagementModel::getSelectedCitadels() const
    {
        CitadelList result;
        for (const auto &systemCitadels : mCitadels)
        {
            for (const auto &citadel : systemCitadels.second)
            {
                Q_ASSERT(citadel);
                if (citadel->mSelected)
                    result.emplace(citadel->mId);
            }
        }

        return result;
    }

    Citadel::IdType CitadelManagementModel::getCitadel(const QModelIndex &index) const
    {
        if (!index.isValid())
            return Citadel::invalidId;

        const auto location = static_cast<const LocationNode *>(index.internalPointer());
        Q_ASSERT(location != nullptr);

        return (location->mType == LocationNode::Type::Citadel) ? (location->mId) : (Citadel::invalidId);
    }

    void CitadelManagementModel::fillStaticData()
    {
        beginResetModel();

        BOOST_SCOPE_EXIT(this_) {
            this_->endResetModel();
        } BOOST_SCOPE_EXIT_END

        const auto &regions = mDataProvider.getRegions();
        mRegions.reserve(regions.size());

        LocationCache regionMap, constellationMap;

        for (const auto &region : regions)
        {
            mRegions.emplace_back(std::make_unique<LocationNode>(region.first, nullptr, mRegions.size(), region.second, LocationNode::Type::Region));
            regionMap[mRegions.back()->mId] = mRegions.back().get();
        }

        const auto &constellations = mDataProvider.getConstellations();
        mConstellations.reserve(constellations.size());

        for (const auto &constellation : constellations)
        {
            const auto region = regionMap.find(constellation.mParent);
            if (Q_LIKELY(region != std::end(regionMap)))
            {
                auto &target = mConstellations[constellation.mParent];
                target.emplace_back(std::make_unique<LocationNode>(constellation.mId, region->second, target.size(), constellation.mName, LocationNode::Type::Constellation));
                constellationMap[target.back()->mId] = target.back().get();
            }
        }

        const auto &solarSystems = mDataProvider.getSolarSystems();
        mSolarSystems.reserve(solarSystems.size());

        for (const auto &solarSystem : solarSystems)
        {
            const auto constellation = constellationMap.find(solarSystem.mParent);
            if (Q_LIKELY(constellation != std::end(constellationMap)))
            {
                auto &target = mSolarSystems[solarSystem.mParent];
                target.emplace_back(std::make_unique<LocationNode>(solarSystem.mId, constellation->second, target.size(), solarSystem.mName, LocationNode::Type::SolarSystem));
                mSolarSystemMap[target.back()->mId] = target.back().get();
            }
        }
    }

    Qt::CheckState CitadelManagementModel::getNodeCheckState(const LocationNode &node) const noexcept
    {
        switch (node.mType) {
        case LocationNode::Type::Region:
            return getRegionNodeCheckState(node);
        case LocationNode::Type::Constellation:
            return getConstellationNodeCheckState(node);
        case LocationNode::Type::SolarSystem:
            return getSolarSystemNodeCheckState(node);
        case LocationNode::Type::Citadel:
            return getCitadelNodeCheckState(node);
        }

        return Qt::Unchecked;
    }

    Qt::CheckState CitadelManagementModel::getNodeCheckState(const LocationList &children) const noexcept
    {
        auto state = Qt::Unchecked;

        for (const auto &child : children)
        {
            Q_ASSERT(child);

            const auto childState = getNodeCheckState(*child);
            switch (childState) {
            case Qt::Checked:
                if (state == Qt::Unchecked)
                    state = Qt::Checked;
                break;
            case Qt::Unchecked:
                if (state == Qt::Checked)
                    return Qt::PartiallyChecked;
                break;
            case Qt::PartiallyChecked:
                return Qt::PartiallyChecked;
            }
        }

        return state;
    }

    Qt::CheckState CitadelManagementModel::getRegionNodeCheckState(const LocationNode &node) const noexcept
    {
        return getNodeCheckState(mConstellations[node.mId]);
    }

    Qt::CheckState CitadelManagementModel::getConstellationNodeCheckState(const LocationNode &node) const noexcept
    {
        return getNodeCheckState(mSolarSystems[node.mId]);
    }

    Qt::CheckState CitadelManagementModel::getSolarSystemNodeCheckState(const LocationNode &node) const noexcept
    {
        const auto &citadels = mCitadels[node.mId];
        return (citadels.empty()) ? ((node.mSelected) ? (Qt::Checked) : (Qt::Unchecked)) : (getNodeCheckState(citadels));
    }

    Qt::CheckState CitadelManagementModel::getCitadelNodeCheckState(const LocationNode &node) const noexcept
    {
        return (node.mSelected) ? (Qt::Checked) : (Qt::Unchecked);
    }

    void CitadelManagementModel::setCheckState(const QModelIndex &index, bool checked)
    {
        const auto node = static_cast<LocationNode *>(index.internalPointer());
        Q_ASSERT(node != nullptr);

        switch (node->mType) {
        case LocationNode::Type::Region:
            setCheckState(index, mConstellations[node->mId], checked);
            break;
        case LocationNode::Type::Constellation:
            setCheckState(index, mSolarSystems[node->mId], checked);
            break;
        case LocationNode::Type::SolarSystem:
            if (mCitadels[node->mId].empty())
                node->mSelected = checked;
            else
                setCheckState(index, mCitadels[node->mId], checked);
            break;
        case LocationNode::Type::Citadel:
            node->mSelected = checked;
        }

        emit dataChanged(index, index, { Qt::CheckStateRole });
    }

    void CitadelManagementModel::setCheckState(const QModelIndex &parent, const LocationList &children, bool checked)
    {
        for (auto row = 0u; row < children.size(); ++row)
            setCheckState(index(row, 0, parent), checked);
    }
}
