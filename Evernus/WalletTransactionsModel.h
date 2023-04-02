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

#include <QAbstractTableModel>
#include <QDate>

#include "WalletTransactionRepository.h"
#include "Character.h"

namespace Evernus
{
    class CharacterRepository;
    class ItemCostProvider;
    class EveDataProvider;

    class WalletTransactionsModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        typedef WalletTransactionRepository::EntryType EntryType;

        WalletTransactionsModel(const WalletTransactionRepository &transactionsRepo,
                                const CharacterRepository &characterRepository,
                                const EveDataProvider &dataProvider,
                                const ItemCostProvider &itemCostProvider,
                                bool corp,
                                QObject *parent = nullptr);
        virtual ~WalletTransactionsModel() = default;

        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        EveType::IdType getTypeId(int row) const;
        uint getQuantity(int row) const;
        double getPrice(int row) const;
        WalletTransaction::Type getType(int row) const;
        Character::IdType getOwnerId(int row) const;

        quint64 getTotalQuantity() const noexcept;
        double getTotalSize() const noexcept;
        double getTotalIncome() const noexcept;
        double getTotalCost() const noexcept;
        double getTotalProfit() const noexcept;

        void setFilter(Character::IdType id, const QDate &from, const QDate &till, EntryType type, bool combineCharacters, EveType::IdType typeId = EveType::invalidId);
        void setCombineCharacters(bool flag);

        void reset();
        void clear();

    private slots:
        void updateNames();

    private:
        enum
        {
            ignoredColumn,
            timestampColumn,
            typeColumn,
            quantityColumn,
            typeIdColumn,
            priceColumn,
            characterColumn,
            clientColumn,
            locationColumn,
            idColumn
        };

        const WalletTransactionRepository &mTransactionsRepository;
        const CharacterRepository &mCharacterRepository;
        const EveDataProvider &mDataProvider;
        const ItemCostProvider &mItemCostProvider;

        Character::IdType mCharacterId = Character::invalidId;
        QDate mFrom, mTill;
        EntryType mType = EntryType::All;
        EveType::IdType mTypeId = EveType::invalidId;
        bool mCombineCharacters = false;

        std::vector<QVariantList> mData;

        QStringList mColumns;

        bool mCorp = false;

        quint64 mTotalQuantity = 0;
        double mTotalSize = 0.;
        double mTotalIncome = 0.;
        double mTotalCost = 0.;
        double mTotalProfit = 0.;

        void processData(const WalletTransactionRepository::EntityList &entries);
    };
}
