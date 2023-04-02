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
#include <vector>

#include "LMeveAPIInterface.h"
#include "LMeveTask.h"

namespace Evernus
{
    class LMeveAPIManager
        : public QObject
    {
        Q_OBJECT

    public:
        template<class T>
        using Callback = std::function<void (T &&data, const QString &error)>;

        typedef std::vector<LMeveTask> TaskList;

        using QObject::QObject;
        virtual ~LMeveAPIManager() = default;

        void fetchTasks(Character::IdType characterId, const Callback<TaskList> &callback) const;

    private:
        LMeveAPIInterface mInterface;

        mutable std::unordered_set<Character::IdType> mPendingTaskRequests;

        static void handlePotentialError(const QByteArray &response, const QString &error);
    };
}
