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

#include <unordered_map>
#include <functional>
#include <vector>

#include <QNetworkAccessManager>

#include "SimpleCrypt.h"
#include "Character.h"

class QNetworkReply;

namespace Evernus
{
    class LMeveAPIInterface
        : public QObject
    {
        Q_OBJECT

    public:
        typedef std::function<void (const QByteArray &response, const QString &error)> Callback;

        explicit LMeveAPIInterface(QObject *parent = nullptr);
        virtual ~LMeveAPIInterface() = default;

        void fetchTasks(Character::IdType characterId, const Callback &callback) const;

    private slots:
        void processReply();

    private:
        typedef std::vector<std::pair<QString, QString>> QueryParams;

        mutable SimpleCrypt mCrypt;

        mutable QNetworkAccessManager mNetworkManager;

        mutable std::unordered_map<QNetworkReply *, Callback> mPendingCallbacks;

        void makeRequest(const QString &endpoint,
                         const Callback &callback,
                         const QueryParams &additionalParams = QueryParams{}) const;
    };
}
