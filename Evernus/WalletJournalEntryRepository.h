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

#include "WalletJournalEntry.h"
#include "Repository.h"

namespace Evernus
{
    class Character;

    class WalletJournalEntryRepository
        : public Repository<WalletJournalEntry>
    {
    public:
        enum class EntryType
        {
            All,
            Incomig,
            Outgoing
        };

        WalletJournalEntryRepository(bool corp, const DatabaseConnectionProvider &connectionProvider);
        virtual ~WalletJournalEntryRepository() = default;

        virtual QString getTableName() const override;
        virtual QString getIdColumn() const override;

        virtual EntityPtr populate(const QSqlRecord &record) const override;

        void create(const Repository<Character> &characterRepo) const;

        WalletJournalEntry::IdType getLatestEntryId(Character::IdType characterId) const;

        void setIgnored(WalletJournalEntry::IdType id, bool ignored) const;
        void deleteOldEntries(const QDateTime &from) const;
        void deleteAll() const;

        EntityList fetchInRange(const QDateTime &from,
                                const QDateTime &till,
                                EntryType type) const;

        EntityList fetchForCharacterInRange(Character::IdType characterId,
                                            const QDateTime &from,
                                            const QDateTime &till,
                                            EntryType type) const;
        EntityList fetchForCorporationInRange(quint64 corporationId,
                                              const QDateTime &from,
                                              const QDateTime &till,
                                              EntryType type) const;

    private:
        bool mCorp = false;

        virtual QStringList getColumns() const override;
        virtual void bindValues(const WalletJournalEntry &entity, QSqlQuery &query) const override;
        virtual void bindPositionalValues(const WalletJournalEntry &entity, QSqlQuery &query) const override;

        template<class T>
        EntityList fetchForColumnInRange(T id,
                                         const QDateTime &from,
                                         const QDateTime &till,
                                         EntryType type,
                                         const QString &column) const;
    };
}
