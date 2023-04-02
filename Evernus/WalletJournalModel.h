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
#include <QString>
#include <QDate>

#include "WalletJournalEntryRepository.h"
#include "Character.h"

namespace Evernus
{
    class CharacterRepository;
    class EveDataProvider;

    class WalletJournalModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        typedef WalletJournalEntryRepository::EntryType EntryType;

        WalletJournalModel(const WalletJournalEntryRepository &journalRepo,
                           const CharacterRepository &characterRepository,
                           const EveDataProvider &dataProvider,
                           bool corp,
                           QObject *parent = nullptr);
        virtual ~WalletJournalModel() = default;

        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void setFilter(Character::IdType id, const QDate &from, const QDate &till, EntryType type, bool combineCharacters);
        void setCombineCharacters(bool flag);

        void reset();

    private slots:
        void updateNames();

    private:
        // note: take a look at updateNames() when changing those (indexes)
        enum
        {
            ignoredColumn,
            timestampColumn,
            typeColumn,
            firstPartyColumn,
            secondPartyColumn,
            contextColumn,
            amountColumn,
            balanceColumn,
            reasonColumn,
            idColumn,
        };

        const WalletJournalEntryRepository &mJournalRepository;
        const CharacterRepository &mCharacterRepository;
        const EveDataProvider &mDataProvider;

        Character::IdType mCharacterId = Character::invalidId;
        QDate mFrom, mTill;
        EntryType mType = EntryType::All;
        bool mCombineCharacters = false;

        std::vector<QVariantList> mData;

        QStringList mColumns;

        bool mCorp = false;

        void processData(const WalletJournalEntryRepository::EntityList &entries);

        static QString translateRefType(QString type);
    };
}
