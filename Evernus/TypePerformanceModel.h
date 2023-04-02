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

#include "Character.h"
#include "EveType.h"

class QDate;

namespace Evernus
{
    class WalletTransactionRepository;
    class CharacterRepository;
    class EveDataProvider;

    class TypePerformanceModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        enum
        {
            nameColumn,
            sellVolumeColumn,
            buyVolumeColumn,
            profitColumn,
            profitPerItemColumn,
            profitPerM3,
            marginColumn,

            numColumns
        };

        TypePerformanceModel(const WalletTransactionRepository &transactionRepository,
                             const WalletTransactionRepository &corpTransactionRepository,
                             const CharacterRepository &characterRepository,
                             const EveDataProvider &dataProvider,
                             QObject *parent = nullptr);
        TypePerformanceModel(const TypePerformanceModel &) = default;
        TypePerformanceModel(TypePerformanceModel &&) = default;
        virtual ~TypePerformanceModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void reset(const QDate &from,
                   const QDate &to,
                   bool combineCharacters,
                   bool combineCorp,
                   Character::IdType characterId);

        TypePerformanceModel &operator =(const TypePerformanceModel &) = default;
        TypePerformanceModel &operator =(TypePerformanceModel &&) = default;

    private:
        struct ItemData
        {
            EveType::IdType mId = EveType::invalidId;
            quint64 mSellVolume = 0;
            quint64 mBuyVolume = 0;
            double mProfit = 0.;
            double mMargin = 0.;
        };

        const WalletTransactionRepository &mTransactionRepository, &mCorpTransactionRepository;
        const CharacterRepository &mCharacterRepository;
        const EveDataProvider &mDataProvider;

        std::vector<ItemData> mData;
    };
}
