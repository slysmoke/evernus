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

#include <optional>

#include "Character.h"
#include "EveType.h"
#include "Entity.h"

class QJsonValue;

namespace Evernus
{
    class LMeveTask
        : public Entity<uint>
    {
    public:
        typedef std::optional<uint> Counter;

        using Entity::Entity;

        LMeveTask() = default;
        explicit LMeveTask(const QJsonValue &json);
        LMeveTask(const LMeveTask &) = default;
        LMeveTask(LMeveTask &&) = default;
        virtual ~LMeveTask() = default;

        Character::IdType getCharacterId() const noexcept;
        void setCharacterId(Character::IdType id) noexcept;

        EveType::IdType getTypeId() const noexcept;
        void setTypeId(EveType::IdType id) noexcept;

        QString getActivity() const &;
        QString &&getActivity() && noexcept;
        void setActivity(const QString &activity);
        void setActivity(QString &&activity);

        Counter getRuns() const;
        void setRuns(const Counter &value);

        Counter getRunsDone() const;
        void setRunsDone(const Counter &value);

        Counter getRunsCompleted() const;
        void setRunsCompleted(const Counter &value);

        Counter getJobsDone() const;
        void setJobsDone(const Counter &value);

        Counter getJobsSuccess() const;
        void setJobsSuccess(const Counter &value);

        Counter getJobsCompleted() const;
        void setJobsCompleted(const Counter &value);

        LMeveTask &operator =(const LMeveTask &) = default;
        LMeveTask &operator =(LMeveTask &&) = default;

    private:
        Character::IdType mCharacterId = Character::invalidId;
        EveType::IdType mTypeId = EveType::invalidId;
        QString mActivity;
        Counter mRuns, mRunsDone, mRunsCompleted;
        Counter mJobsDone, mJobsSuccess, mJobsCompleted;
    };
}
