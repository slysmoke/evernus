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
#include <QRegularExpression>
#include <QTextDocument>
#include <QLocale>
#include <QColor>
#include <QHash>
#include <QFont>

#include "CharacterRepository.h"
#include "EveDataProvider.h"
#include "TextUtils.h"

#include "WalletJournalModel.h"

namespace Evernus
{
    WalletJournalModel::WalletJournalModel(const WalletJournalEntryRepository &journalRepo,
                                           const CharacterRepository &characterRepository,
                                           const EveDataProvider &dataProvider,
                                           bool corp,
                                           QObject *parent)
        : QAbstractTableModel{parent}
        , mJournalRepository{journalRepo}
        , mCharacterRepository{characterRepository}
        , mDataProvider{dataProvider}
        , mCorp{corp}
    {
        mColumns
            << tr("Ignored")
            << tr("Date")
            << tr("Type")
            << tr("First party")
            << tr("Second party")
            << tr("Additional data")
            << tr("Amount")
            << tr("Balance after")
            << tr("Reason");

        connect(&mDataProvider, &EveDataProvider::namesChanged, this, &WalletJournalModel::updateNames);
    }

    Qt::ItemFlags WalletJournalModel::flags(const QModelIndex &index) const
    {
        auto flags = QAbstractTableModel::flags(index);
        if (index.isValid() && index.column() == ignoredColumn)
            flags |= Qt::ItemIsUserCheckable;

        return flags;
    }

    QVariant WalletJournalModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return mColumns[section];

        return QVariant{};
    }

    int WalletJournalModel::columnCount(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return mColumns.count();

        return 0;
    }

    QVariant WalletJournalModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
             return QVariant{};

        const auto row = index.row();
        const auto column = index.column();

        switch (role) {
        case Qt::UserRole:
            switch (column) {
            case firstPartyColumn:
            case secondPartyColumn:
            case contextColumn:
                if (!mData[row][column].isNull())
                    return mDataProvider.getGenericName(mData[row][column].toULongLong());
            }
            return mData[row][column];
        case Qt::DisplayRole:
            switch (column) {
            case ignoredColumn:
                return QVariant{};
            case timestampColumn:
                return TextUtils::dateTimeToString(mData[row][timestampColumn].toDateTime().toLocalTime(), QLocale{});
            case firstPartyColumn:
            case secondPartyColumn:
                if (!mData[row][column].isNull())
                    return mDataProvider.getGenericName(mData[row][column].toULongLong());
                break;
            case amountColumn:
            case balanceColumn:
                return TextUtils::currencyToString(mData[row][column].toDouble(), QLocale{});
            }

            return mData[row][column];
        case Qt::ForegroundRole:
            if (column == amountColumn)
                return (mData[row][amountColumn].toDouble() < 0.) ? (QColor{Qt::red}) : (QColor{Qt::green});
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
            if (column == amountColumn || column == balanceColumn)
                return Qt::AlignRight;
        }

        return {};
    }

    bool WalletJournalModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (role == Qt::CheckStateRole && index.isValid() && index.column() == ignoredColumn)
        {
            auto &data = mData[index.row()];

            const auto ignored = value.toInt() == Qt::Checked;
            data[ignoredColumn] = ignored;

            mJournalRepository.setIgnored(data[idColumn].value<WalletJournalEntry::IdType>(), ignored);

            emit dataChanged(index, index, QVector<int>{} << Qt::CheckStateRole << Qt::FontRole);
            return true;
        }

        return false;
    }

    int WalletJournalModel::rowCount(const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return static_cast<int>(mData.size());

        return 0;
    }

    void WalletJournalModel::setFilter(Character::IdType id, const QDate &from, const QDate &till, EntryType type, bool combineCharacters)
    {
        mCharacterId = id;
        mFrom = from;
        mTill = till;
        mType = type;
        mCombineCharacters = combineCharacters;

        reset();
    }

    void WalletJournalModel::setCombineCharacters(bool flag)
    {
        mCombineCharacters = flag;
        reset();
    }

    void WalletJournalModel::reset()
    {
        beginResetModel();

        mData.clear();
        if (Q_LIKELY(mCharacterId != Character::invalidId || mCombineCharacters))
        {
            try
            {
                WalletJournalEntryRepository::EntityList entries;
                if (mCombineCharacters)
                {
                    const auto idName = mCharacterRepository.getIdColumn();
                    auto query = mCharacterRepository.getEnabledQuery();

                    while (query.next())
                    {
                        const auto id = query.value(idName).value<Character::IdType>();

                        WalletJournalEntryRepository::EntityList newEntries;
                        if (mCorp)
                        {
                            newEntries = mJournalRepository.fetchForCorporationInRange(mCharacterRepository.getCorporationId(id),
                                                                                       QDateTime{mFrom}.toUTC(),
                                                                                       QDateTime{mTill}.addDays(1).toUTC(),
                                                                                       mType);
                        }
                        else
                        {
                            newEntries = mJournalRepository.fetchForCharacterInRange(id,
                                                                                     QDateTime{mFrom}.toUTC(),
                                                                                     QDateTime{mTill}.addDays(1).toUTC(),
                                                                                     mType);
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
                        entries = mJournalRepository.fetchForCorporationInRange(mCharacterRepository.getCorporationId(mCharacterId),
                                                                                QDateTime{mFrom}.toUTC(),
                                                                                QDateTime{mTill}.addDays(1).toUTC(),
                                                                                mType);
                    }
                    else
                    {
                        entries = mJournalRepository.fetchForCharacterInRange(mCharacterId,
                                                                              QDateTime{mFrom}.toUTC(),
                                                                              QDateTime{mTill}.addDays(1).toUTC(),
                                                                              mType);
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

    void WalletJournalModel::updateNames()
    {
        emit dataChanged(index(0, firstPartyColumn), index(rowCount() - 1, contextColumn), { Qt::UserRole, Qt::DisplayRole });
    }

    void WalletJournalModel::processData(const WalletJournalEntryRepository::EntityList &entries)
    {
        mData.reserve(entries.size());

        QRegularExpression re{QStringLiteral("^DESC: ")};

        for (const auto &entry : entries)
        {
            const auto contextId = entry->getContextId();
            const auto firstPartyId = entry->getFirstPartyId();
            const auto secondPartyId = entry->getSecondPartyId();
            const auto amount = entry->getAmount();
            const auto balance = entry->getBalance();

            mData.emplace_back();
            auto &data = mData.back();

            QTextDocument reasonDoc;
            reasonDoc.setHtml(entry->getReason().remove(re));

            data
                << entry->isIgnored()
                << entry->getTimestamp()
                << translateRefType(entry->getRefType())
                << ((firstPartyId) ? (*firstPartyId) : (QVariant{}))
                << ((secondPartyId) ? (*secondPartyId) : (QVariant{}))
                << ((contextId) ? (QStringLiteral("%1: %2").arg(entry->getContextIdType()).arg(*contextId)) : (QVariant{}))
                << ((amount) ? (QVariant{*amount}) : (QVariant{}))
                << ((balance) ? (QVariant{*balance}) : (QVariant{}))
                << reasonDoc.toPlainText()
                << entry->getId();
        }
    }

    QString WalletJournalModel::translateRefType(QString type)
    {
        return type.replace('_', ' ');
    }
}
