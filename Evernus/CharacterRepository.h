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

#include <unordered_set>

#include "Repository.h"
#include "Character.h"

namespace Evernus
{
    class CharacterRepository
        : public Repository<Character>
    {
    public:
        using Repository::Repository;
        virtual ~CharacterRepository() = default;

        virtual QString getTableName() const override;
        virtual QString getIdColumn() const override;

        virtual EntityPtr populate(const QSqlRecord &record) const override;

        void create() const;

        void updateSkill(Character::IdType id, const QString &skill, int level) const;
        void updateStanding(Character::IdType id, const QString &type, double value) const;
        void updateBrokersFee(Character::IdType id, const std::optional<double> &buy, const std::optional<double> &sell) const;
        void updateReprocessingImplantBonus(Character::IdType id, double value) const;
        void updateManufacturingTimeImplantBonus(Character::IdType id, double value) const;
        void updateAlphaClone(Character::IdType id, bool flag) const;

        bool hasCharacters() const;

        std::unordered_set<Character::IdType> fetchAllIds() const;

        QString getNameColumn() const;

        quint64 getCorporationId(Character::IdType id) const;
        QString getName(Character::IdType id) const;

        QSqlQuery getEnabledQuery() const;
        QString getCreateQuery() const;

    private:
        virtual QStringList getColumns() const override;
        virtual void bindValues(const Character &entity, QSqlQuery &query) const override;
        virtual void bindPositionalValues(const Character &entity, QSqlQuery &query) const override;
    };
}
