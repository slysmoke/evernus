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
#include <QLocale>
#include <QColor>
#include <QFont>
#include <QSettings>


#include "CharacterRepository.h"
#include "ItemCostProvider.h"
#include "EveDataProvider.h"
#include "TextUtils.h"
#include "UISettings.h"

#include "WalletTransactionsModel.h"

namespace Evernus
{
    WalletTransactionsModel::WalletTransactionsModel(const WalletTransactionRepository &transactionsRepo,
                                                     const CharacterRepository &characterRepository,
                                                     const EveDataProvider &dataProvider,
                                                     const ItemCostProvider &itemCostProvider,
                                                     bool corp,
                                                     QObject *parent)
        : QAbstractTableModel{parent}
        , mTransactionsRepository{transactionsRepo}
        , mCharacterRepository{characterRepository}
        , mDataProvider{dataProvider}
        , mItemCostProvider{itemCostProvider}
        , mCorp{corp}
    {
        mColumns
            << tr("Ignored")
            << tr("Date")
            << tr("Type")
            << tr("Quantity")
            << tr("Item")
            << tr("Price")
            << tr("Character")
            << tr("Client")
            << tr("Location");

        connect(&mDataProvider, &EveDataProvider::namesChanged, this, &WalletTransactionsModel::updateNames);
    }

    QSettings settings;

    Qt::ItemFlags WalletTransactionsModel::flags(const QModelIndex &index) const
    {
        auto flags = QAbstractTableModel::flags(index);
        if (index.isValid() && index.column() == ignoredColumn)
            flags |= Qt::ItemIsUserCheckable;

        return flags;
    }

    QVariant WalletTransactionsModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return mColumns[section];

        return QVariant{};
    }

    int WalletTransactionsModel::columnCount(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return mColumns.count();

        return 0;
    }

    QVariant WalletTransactionsModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
             return {};

        const auto row = index.row();
        const auto column = index.column();

        switch (role) {
        case Qt::UserRole:
            if (column == typeIdColumn)
                return mDataProvider.getTypeName(mData[row][typeIdColumn].value<EveType::IdType>());
            if (column == characterColumn || column == clientColumn)
                return mDataProvider.getGenericName(mData[row][column].value<Character::IdType>());

            return mData[row][column];
        case Qt::DisplayRole:
            switch (column) {
            case ignoredColumn:
                return QVariant{};
            case timestampColumn:
                return TextUtils::dateTimeToString(mData[row][timestampColumn].toDateTime().toLocalTime(), QLocale{});
            case typeColumn:
                return (getType(row) == WalletTransaction::Type::Buy) ?
                       (tr("Buy")) :
                       (tr("Sell"));
            case quantityColumn:
                return QLocale{}.toString(mData[row][quantityColumn].toUInt());
            case typeIdColumn:
                return mDataProvider.getTypeName(mData[row][typeIdColumn].value<EveType::IdType>());
            case priceColumn:
                {
                    auto price = mData[row][priceColumn].toDouble();
                    if (static_cast<WalletTransaction::Type>(mData[row][typeColumn].toInt()) == WalletTransaction::Type::Buy)
                        price = -price;

                    return TextUtils::currencyToString(price, QLocale{});
                }
            case characterColumn:
            case clientColumn:
                return mDataProvider.getGenericName(mData[row][column].value<Character::IdType>());
            }

            return mData[row][column];
        case Qt::ForegroundRole:
            if (column == priceColumn)
            {
                return (getType(row) == WalletTransaction::Type::Buy) ?
                    (QColor{ Qt::red }) :
                    (QColor{ Qt::darkGreen });
                
            }
            break;
        case Qt::CheckStateRole:
            if (column == ignoredColumn)
                return (mData[row][ignoredColumn].toBool()) ? (Qt::Checked) : (Qt::Unchecked);
            break;
        case Qt::FontRole:
            if (mData[row][ignoredColumn].toBool())
            {
                QFont font;
                font.setStrikeOut(true);

                return font;
            }
            break;
        case Qt::TextAlignmentRole:
            if (column == priceColumn)
                return Qt::AlignRight;
        }

        return {};
    }

    bool WalletTransactionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (role == Qt::CheckStateRole && index.isValid() && index.column() == ignoredColumn)
        {
            auto &data = mData[index.row()];

            const auto ignored = value.toInt() == Qt::Checked;
            data[ignoredColumn] = ignored;

            mTransactionsRepository.setIgnored(data[idColumn].value<WalletTransaction::IdType>(), ignored);

            const auto quantity = data[quantityColumn].toUInt();
            const auto typeId = data[typeIdColumn].value<EveType::IdType>();
            const auto size = mDataProvider.getTypeVolume(typeId) * quantity;
            const auto type = static_cast<WalletTransaction::Type>(data[typeColumn].toInt());
            const auto price = data[priceColumn].toDouble();

            const auto adjustTotals = [=](auto mul) {
                mTotalSize += size * mul;

                if (type == WalletTransaction::Type::Buy)
                {
                    mTotalCost += price * quantity * mul;
                }
                else
                {
                    mTotalIncome += price * quantity * mul;
                    mTotalProfit +=
                        (price - mItemCostProvider.fetchForCharacterAndType(mCharacterId, typeId)->getAdjustedCost()) * quantity * mul;
                }
            };

            if (ignored)
            {
                adjustTotals(-1);
                mTotalQuantity -= quantity;
            }
            else
            {
                adjustTotals(1);
                mTotalQuantity += quantity;
            }

            emit dataChanged(index, index, { Qt::CheckStateRole, Qt::FontRole });
            return true;
        }

        return false;
    }

    int WalletTransactionsModel::rowCount(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return static_cast<int>(mData.size());

        return 0;
    }

    EveType::IdType WalletTransactionsModel::getTypeId(int row) const
    {
        return mData[row][typeIdColumn].value<EveType::IdType>();
    }

    uint WalletTransactionsModel::getQuantity(int row) const
    {
        return mData[row][quantityColumn].toUInt();
    }

    double WalletTransactionsModel::getPrice(int row) const
    {
        return mData[row][priceColumn].toDouble();
    }

    WalletTransaction::Type WalletTransactionsModel::getType(int row) const
    {
        return static_cast<WalletTransaction::Type>(mData[row][typeColumn].toInt());
    }

    Character::IdType WalletTransactionsModel::getOwnerId(int row) const
    {
        return mData[row][characterColumn].toUInt();
    }

    quint64 WalletTransactionsModel::getTotalQuantity() const noexcept
    {
        return mTotalQuantity;
    }

    double WalletTransactionsModel::getTotalSize() const noexcept
    {
        return mTotalSize;
    }

    double WalletTransactionsModel::getTotalIncome() const noexcept
    {
        return mTotalIncome;
    }

    double WalletTransactionsModel::getTotalCost() const noexcept
    {
        return mTotalCost;
    }

    double WalletTransactionsModel::getTotalProfit() const noexcept
    {
        return mTotalProfit;
    }

    void WalletTransactionsModel
    ::setFilter(Character::IdType id, const QDate &from, const QDate &till, EntryType type, bool combineCharacters, EveType::IdType typeId)
    {
        mCharacterId = id;
        mFrom = from;
        mTill = till;
        mType = type;
        mTypeId = typeId;
        mCombineCharacters = combineCharacters;

        reset();
    }

    void WalletTransactionsModel::setCombineCharacters(bool flag)
    {
        mCombineCharacters = flag;
        reset();
    }

    void WalletTransactionsModel::reset()
    {
        beginResetModel();

        mTotalQuantity = 0;
        mTotalSize = 0.;
        mTotalIncome = 0.;
        mTotalCost = 0.;
        mTotalProfit = 0.;

        mData.clear();
        if (Q_LIKELY(mCharacterId != Character::invalidId || mCombineCharacters))
        {
            try
            {
                WalletTransactionRepository::EntityList entries;
                if (mCombineCharacters)
                {
                    const auto idName = mCharacterRepository.getIdColumn();
                    auto query = mCharacterRepository.getEnabledQuery();

                    while (query.next())
                    {
                        const auto id = query.value(idName).value<Character::IdType>();

                        WalletTransactionRepository::EntityList newEntries;
                        if (mCorp)
                        {
                            newEntries = mTransactionsRepository.fetchForCorporationInRange(mCharacterRepository.getCorporationId(id),
                                                                                            QDateTime{mFrom}.toUTC(),
                                                                                            QDateTime{mTill}.addDays(1).toUTC(),
                                                                                            mType,
                                                                                            mTypeId);
                        }
                        else
                        {
                            newEntries = mTransactionsRepository.fetchForCharacterInRange(id,
                                                                                          QDateTime{mFrom}.toUTC(),
                                                                                          QDateTime{mTill}.addDays(1).toUTC(),
                                                                                          mType,
                                                                                          mTypeId);
                        }

                        entries.reserve(entries.size() + newEntries.size());
                                        entries.insert(std::end(entries),
                                                       std::make_move_iterator(std::begin(newEntries)),
                                                       std::make_move_iterator(std::end(newEntries)));
                    }
                }
                else
                {
                    if (mCorp)
                    {
                        entries = mTransactionsRepository.fetchForCorporationInRange(mCharacterRepository.getCorporationId(mCharacterId),
                                                                                     QDateTime{mFrom}.toUTC(),
                                                                                     QDateTime{mTill}.addDays(1).toUTC(),
                                                                                     mType,
                                                                                     mTypeId);
                    }
                    else
                    {
                        entries = mTransactionsRepository.fetchForCharacterInRange(mCharacterId,
                                                                                   QDateTime{mFrom}.toUTC(),
                                                                                   QDateTime{mTill}.addDays(1).toUTC(),
                                                                                   mType,
                                                                                   mTypeId);
                    }
                }

                processData(entries);
            }
            catch (const CharacterRepository::NotFoundException &)
            {
            }
        }

        endResetModel();
    }

    void WalletTransactionsModel::clear()
    {
        beginResetModel();
        mData.clear();
        endResetModel();
    }

    void WalletTransactionsModel::updateNames()
    {
        emit dataChanged(index(0, characterColumn), index(rowCount() - 1, characterColumn), { Qt::UserRole, Qt::DisplayRole });
        emit dataChanged(index(0, clientColumn), index(rowCount() - 1, clientColumn), { Qt::UserRole, Qt::DisplayRole });
    }

    void WalletTransactionsModel::processData(const WalletTransactionRepository::EntityList &entries)
    {
        mData.reserve(entries.size());

        for (const auto &entry : entries)
        {
            mData.emplace_back();
            auto &data = mData.back();

            const auto quantity = entry->getQuantity();

            data
                << entry->isIgnored()
                << entry->getTimestamp()
                << static_cast<int>(entry->getType())
                << quantity
                << entry->getTypeId()
                << entry->getPrice()
                << entry->getCharacterId()
                << entry->getClientId()
                << mDataProvider.getLocationName(entry->getLocationId())
                << entry->getId();

            if (!entry->isIgnored())
            {
                mTotalQuantity += quantity;
                mTotalSize += mDataProvider.getTypeVolume(entry->getTypeId()) * quantity;

                if (entry->getType() == WalletTransaction::Type::Buy)
                {
                    mTotalCost += entry->getPrice() * quantity;
                }
                else
                {
                    mTotalIncome += entry->getPrice() * quantity;
                    mTotalProfit +=
                        (entry->getPrice() - mItemCostProvider.fetchForCharacterAndType(mCharacterId, entry->getTypeId())->getAdjustedCost()) * quantity;
                }
            }
        }
    }
}
