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
#include <vector>

#include <QAbstractItemModel>

namespace Evernus
{
    class EveDataProvider;

    class StationModel
        : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        explicit StationModel(const EveDataProvider &dataProvider, QObject *parent = nullptr);
        virtual ~StationModel() = default;

        virtual bool canFetchMore(const QModelIndex &parent) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual void fetchMore(const QModelIndex &parent) override;
        virtual bool hasChildren(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        QModelIndex index(quint64 id, const QModelIndex &parent = QModelIndex{});

        quint64 getStationId(const QModelIndex &index) const;
        quint64 getGenericId(const QModelIndex &index) const;

    private:
        struct LocationNode
        {
            enum class Type
            {
                Region,
                Constellation,
                SolarSystem,
                Station
            };

            quint64 mId = 0;
            LocationNode *mParent = nullptr;
            size_t mRow = 0;
            QString mName;
            Type mType = Type::Region;

            LocationNode(quint64 id, LocationNode *parent, size_t row, const QString &name, Type type);
        };

        const EveDataProvider &mDataProvider;

        mutable std::vector<LocationNode> mRegions;
        mutable std::unordered_map<uint, std::vector<LocationNode>> mConstellations, mSolarSystems, mStations;
    };
}
