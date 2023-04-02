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

#include "MarketOrderTreeModel.h"

namespace Evernus
{
    class MarketOrderProvider;
    class CharacterRepository;
    class CacheTimerProvider;
    class ItemCostProvider;

    class MarketOrderArchiveModel
        : public MarketOrderTreeModel
    {
        Q_OBJECT

    public:
        MarketOrderArchiveModel(MarketOrderProvider &orderProvider,
                                const EveDataProvider &dataProvider,
                                const ItemCostProvider &itemCostProvider,
                                const CharacterRepository &characterRepository,
                                bool corp,
                                QObject *parent = nullptr);
        virtual ~MarketOrderArchiveModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        virtual OrderInfo getOrderInfo(const QModelIndex &index) const override;
        virtual WalletTransactionsModel::EntryType getOrderTypeFilter(const QModelIndex &index) const override;

        virtual bool shouldShowPriceInfo(const QModelIndex &index) const override;

        virtual int getVolumeColumn() const override;

        virtual Type getType() const override;

        void setCharacterAndRange(Character::IdType id, const QDateTime &from, const QDateTime &to);

    private slots:
        void updateNames();

    private:
        enum
        {
            lastSeenColumn,
            typeColumn,
            nameColumn,
            groupColumn,
            statusColumn,
            customCostColumn,
            priceColumn,
            volumeColumn,
            profitColumn,
            stationColumn,
            notesColumn,
            ownerColumn,

            numColumns
        };

        MarketOrderProvider &mOrderProvider;
        const ItemCostProvider &mItemCostProvider;
        const CharacterRepository &mCharacterRepository;

        bool mCorp = false;

        QDateTime mFrom, mTo;

        virtual OrderList getOrders(Character::IdType characterId) const override;
        virtual OrderList getOrdersForAllCharacters() const override;

        virtual void handleOrderRemoval(const MarketOrder &order) override;
    };
}
