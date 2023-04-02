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
#include <unordered_map>

#include <boost/scope_exit.hpp>

#include <QLocale>
#include <QColor>
#include <QDate>

#include "WalletTransactionRepository.h"
#include "CharacterRepository.h"
#include "WalletTransaction.h"
#include "EveDataProvider.h"
#include "DatabaseUtils.h"
#include "TextUtils.h"

#include "TypePerformanceModel.h"

namespace Evernus
{
    TypePerformanceModel::TypePerformanceModel(const WalletTransactionRepository &transactionRepository,
                                               const WalletTransactionRepository &corpTransactionRepository,
                                               const CharacterRepository &characterRepository,
                                               const EveDataProvider &dataProvider,
                                               QObject *parent)
        : QAbstractTableModel{parent}
        , mTransactionRepository{transactionRepository}
        , mCorpTransactionRepository{corpTransactionRepository}
        , mCharacterRepository{characterRepository}
        , mDataProvider{dataProvider}
    {
    }

    int TypePerformanceModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant TypePerformanceModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        const auto column = index.column();
        const auto &data = mData[index.row()];

        switch (role) {
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (column) {
                case nameColumn:
                    return mDataProvider.getTypeName(data.mId);
                case sellVolumeColumn:
                    return locale.toString(data.mSellVolume);
                case buyVolumeColumn:
                    return locale.toString(data.mBuyVolume);
                case profitColumn:
                    return TextUtils::currencyToString(data.mProfit, locale);
                case profitPerItemColumn:
                    return (data.mSellVolume == 0) ? (QString{}) : (TextUtils::currencyToString(data.mProfit / data.mSellVolume, locale));
                case profitPerM3:
                    return TextUtils::currencyToString(data.mProfit / mDataProvider.getTypeVolume(data.mId), locale);
                case marginColumn:
                    return QStringLiteral("%1%2").arg(locale.toString(data.mMargin, 'f', 2)).arg(locale.percent());
                }
            }
            break;
        case Qt::UserRole:
            switch (column) {
            case nameColumn:
                return mDataProvider.getTypeName(data.mId);
            case sellVolumeColumn:
                return data.mSellVolume;
            case buyVolumeColumn:
                return data.mBuyVolume;
            case profitColumn:
                return data.mProfit;
            case profitPerItemColumn:
                return (data.mSellVolume == 0) ? (0.) : (data.mProfit / data.mSellVolume);
            case profitPerM3:
                return data.mProfit / mDataProvider.getTypeVolume(data.mId);
            case marginColumn:
                return data.mMargin;
            }
            break;
        case Qt::ForegroundRole:
            if (column == marginColumn)
                return TextUtils::getMarginColor(data.mMargin);
        }

        return {};
    }

    QVariant TypePerformanceModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
            case nameColumn:
                return tr("Name");
            case sellVolumeColumn:
                return tr("Sell volume");
            case buyVolumeColumn:
                return tr("Buy volume");
            case profitColumn:
                return tr("Profit");
            case profitPerItemColumn:
                return tr("Profit per item");
            case profitPerM3:
                return tr("Profit per m³");
            case marginColumn:
                return tr("Margin");
            }
        }

        return {};
    }

    int TypePerformanceModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    void TypePerformanceModel::reset(const QDate &from,
                                     const QDate &to,
                                     bool combineCharacters,
                                     bool combineCorp,
                                     Character::IdType characterId)
    {
        beginResetModel();

        BOOST_SCOPE_EXIT(this_) {
            this_->endResetModel();
        } BOOST_SCOPE_EXIT_END

        mData.clear();

        QSqlQuery query{mCharacterRepository.getDatabase()};

        if (combineCharacters && combineCorp)
        {
            query.prepare(QStringLiteral(R"(
SELECT type_id, type, price, quantity
    FROM (
        SELECT type_id, price, type, quantity FROM %1 WHERE ignored = 0 AND timestamp BETWEEN ? AND ?
        UNION
        SELECT type_id, price, type, quantity FROM %2 WHERE ignored = 0 AND timestamp BETWEEN ? AND ?
    )
    GROUP BY type_id, type
            )")
                .arg(mTransactionRepository.getTableName())
                .arg(mCorpTransactionRepository.getTableName())
            );

            query.addBindValue(from);
            query.addBindValue(to);
            query.addBindValue(from);
            query.addBindValue(to);
        }
        else if (combineCharacters)
        {
            query.prepare(QStringLiteral("SELECT type_id, type, price, quantity FROM %1 WHERE ignored = 0 AND timestamp BETWEEN ? AND ?")
                .arg(mTransactionRepository.getTableName())
            );

            query.addBindValue(from);
            query.addBindValue(to);
        }
        else if (combineCorp)
        {
            query.prepare(QStringLiteral(R"(
SELECT type_id, type, price, quantity
    FROM (
        SELECT type_id, price, type, quantity FROM %1 WHERE character_id = ? AND ignored = 0 AND timestamp BETWEEN ? AND ?
        UNION
        SELECT type_id, price, type, quantity FROM %2 WHERE corporation_id = ? AND ignored = 0 AND timestamp BETWEEN ? AND ?
    )
    GROUP BY type_id, type
            )")
                .arg(mTransactionRepository.getTableName())
                .arg(mCorpTransactionRepository.getTableName())
            );

            query.addBindValue(characterId);
            query.addBindValue(from);
            query.addBindValue(to);
            query.addBindValue(mCharacterRepository.getCorporationId(characterId));
            query.addBindValue(from);
            query.addBindValue(to);
        }
        else
        {
            query.prepare(QStringLiteral("SELECT type_id, type, price, quantity FROM %1 WHERE character_id = ? AND ignored = 0 AND timestamp BETWEEN ? AND ?")
                .arg(mTransactionRepository.getTableName())
            );

            query.addBindValue(characterId);
            query.addBindValue(from);
            query.addBindValue(to);
        }

        DatabaseUtils::execQuery(query);

        struct IntermediateData
        {
            quint64 mSellVolume = 0;
            quint64 mBuyVolume = 0;
            double mTotalIncome = 0.;
            double mTotalOutcome = 0.;
        };

        std::unordered_map<EveType::IdType, IntermediateData> itemData;

        while (query.next())
        {
            auto &data = itemData[query.value(0).value<EveType::IdType>()];

            const auto quantity = query.value(3).toULongLong();
            const auto price = query.value(2).toDouble();

            if (static_cast<WalletTransaction::Type>(query.value(1).toInt()) == WalletTransaction::Type::Buy)
            {
                data.mBuyVolume += quantity;
                data.mTotalOutcome += price * quantity;
            }
            else
            {
                data.mSellVolume += quantity;
                data.mTotalIncome += price * quantity;
            }
        }

        mData.reserve(itemData.size());

        for (const auto &item : itemData)
        {
            ItemData data;
            data.mId = item.first;
            data.mBuyVolume = item.second.mBuyVolume;
            data.mSellVolume = item.second.mSellVolume;
            data.mProfit = item.second.mTotalIncome - item.second.mTotalOutcome;
            data.mMargin = (qFuzzyIsNull(item.second.mTotalIncome)) ? (0.) : (100. * (item.second.mTotalIncome - item.second.mTotalOutcome) / item.second.mTotalIncome);

            mData.emplace_back(std::move(data));
        }
    }
}
