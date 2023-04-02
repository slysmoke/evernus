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

#include "Repository.h"
#include "WalletSnapshot.h"

namespace Evernus
{
    class Character;

    class WalletSnapshotRepository
        : public Repository<WalletSnapshot>
    {
    public:
        using Repository::Repository;
        virtual ~WalletSnapshotRepository() = default;

        virtual QString getTableName() const override;
        virtual QString getIdColumn() const override;

        virtual EntityPtr populate(const QSqlRecord &record) const override;

        void create(const Repository<Character> &characterRepo) const;

        EntityList fetchRange(const QDateTime &from, const QDateTime &to) const;
        EntityList fetchRange(Character::IdType characterId, const QDateTime &from, const QDateTime &to) const;

    private:
        virtual QStringList getColumns() const override;
        virtual void bindValues(const WalletSnapshot &entity, QSqlQuery &query) const override;
        virtual void bindPositionalValues(const WalletSnapshot &entity, QSqlQuery &query) const override;
    };
}
