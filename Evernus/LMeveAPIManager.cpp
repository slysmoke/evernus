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
#include <stdexcept>

#include <boost/throw_exception.hpp>

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtDebug>

#include "Defines.h"

#include "LMeveAPIManager.h"

namespace Evernus
{
    void LMeveAPIManager::fetchTasks(Character::IdType characterId, const Callback<TaskList> &callback) const
    {
        if (mPendingTaskRequests.find(characterId) != std::end(mPendingTaskRequests))
            return;

        mPendingTaskRequests.emplace(characterId);

#if EVERNUS_CLANG_LAMBDA_CAPTURE_BUG
        mInterface.fetchTasks(characterId, [=, callback = callback](const QByteArray &response, const QString &error) {
#else
        mInterface.fetchTasks(characterId, [=](const QByteArray &response, const QString &error) {
#endif
            try
            {
                mPendingTaskRequests.erase(characterId);

                qDebug() << "Got JSON reply:" << response;

                handlePotentialError(response, error);

                QJsonParseError error;
                const auto doc = QJsonDocument::fromJson(response, &error);
                if (doc.isNull())
                    BOOST_THROW_EXCEPTION(std::runtime_error{error.errorString().toStdString()});

                if (doc.isObject())
                {
                    const auto obj = doc.object();
                    const auto error = obj.value("errorMsg");
                    if (!error.isUndefined())
                        BOOST_THROW_EXCEPTION(std::runtime_error{error.toString().toStdString()});

                    BOOST_THROW_EXCEPTION(std::runtime_error{tr("Unexpected object received!").toStdString()});
                }

                if (!doc.isArray())
                    BOOST_THROW_EXCEPTION(std::runtime_error{tr("Expected task array!").toStdString()});

                const auto array = doc.array();

                TaskList result;
                result.reserve(array.size());

                for (const auto &elem : array)
                    result.emplace_back(LMeveTask{elem});

                callback(std::move(result), QString{});
            }
            catch (const std::exception &e)
            {
                callback(TaskList{}, e.what());
            }
        });
    }

    void LMeveAPIManager::handlePotentialError(const QByteArray &response, const QString &error)
    {
        if (!error.isEmpty())
            BOOST_THROW_EXCEPTION(std::runtime_error{error.toStdString()});

        if (response.isEmpty())
            BOOST_THROW_EXCEPTION(std::runtime_error{tr("Empty response from server!").toStdString()});
    }
}
