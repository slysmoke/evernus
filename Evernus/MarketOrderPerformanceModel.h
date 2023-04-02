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

#include <vector>
#include <chrono>

#include <QAbstractTableModel>

#include "Character.h"
#include "EveType.h"

class QDate;

namespace Evernus
{
    class MarketOrderRepository;
    class CharacterRepository;
    class EveDataProvider;

    class MarketOrderPerformanceModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        enum
        {
            nameColumn,
            volumeColumn,
            timeColumn,

            numColumns
        };

        MarketOrderPerformanceModel(const MarketOrderRepository &marketOrderRepository,
                                    const MarketOrderRepository &corpMarketOrderRepository,
                                    const CharacterRepository &characterRepository,
                                    const EveDataProvider &dataProvider,
                                    QObject *parent = nullptr);
        MarketOrderPerformanceModel(const MarketOrderPerformanceModel &) = default;
        MarketOrderPerformanceModel(MarketOrderPerformanceModel &&) = default;
        virtual ~MarketOrderPerformanceModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void reset(const QDate &from,
                   const QDate &to,
                   bool combineCharacters,
                   bool combineCorp,
                   Character::IdType characterId);

        MarketOrderPerformanceModel &operator =(const MarketOrderPerformanceModel &) = default;
        MarketOrderPerformanceModel &operator =(MarketOrderPerformanceModel &&) = default;

    private:
        struct ItemData
        {
            EveType::IdType mId = EveType::invalidId;
            quint64 mVolume = 0;
            std::chrono::minutes mTurnover;
        };

        const MarketOrderRepository &mMarketOrderRepository;
        const MarketOrderRepository &mCorpMarketOrderRepository;
        const CharacterRepository &mCharacterRepository;
        const EveDataProvider &mDataProvider;

        std::vector<ItemData> mData;
    };
}
