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
#include <QCoreApplication>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QSettings>
#include <QtDebug>

#include "LMeveSettings.h"

#include "LMeveAPIInterface.h"

namespace Evernus
{
    LMeveAPIInterface::LMeveAPIInterface(QObject *parent)
        : QObject{parent}
        , mCrypt{LMeveSettings::lmeveCryptKey}
    {
    }

    void LMeveAPIInterface::fetchTasks(Character::IdType characterId, const Callback &callback) const
    {
        makeRequest("TASKS", callback, { std::make_pair("characterID", QString::number(characterId)) });
    }

    void LMeveAPIInterface::processReply()
    {
        auto reply = qobject_cast<QNetworkReply *>(sender());
        reply->deleteLater();

        const auto it = mPendingCallbacks.find(reply);
        Q_ASSERT(it != std::end(mPendingCallbacks));

        const auto error = reply->error();

        it->second(reply->readAll(), (error == QNetworkReply::NoError) ? (QString{}) : (reply->errorString()));
        mPendingCallbacks.erase(it);
    }

    void LMeveAPIInterface::makeRequest(const QString &endpoint,
                                        const Callback &callback,
                                        const QueryParams &additionalParams) const
    {
        QSettings settings;
        auto address = settings.value(LMeveSettings::urlKey).toString();
        if (address.isEmpty())
        {
            callback(QByteArray{}, tr("Missing LMeve url!"));
            return;
        }

        const auto key = settings.value(LMeveSettings::keyKey).toString();
        if (key.isEmpty())
        {
            callback(QByteArray{}, tr("Missing LMeve key!"));
            return;
        }

        const QString apiEndpoint = "/api.php";
        if (!address.endsWith(apiEndpoint))
            address += apiEndpoint;

        QUrl url{address};

        QUrlQuery queryData;
        queryData.addQueryItem("endpoint", endpoint);
        queryData.addQueryItem("key", mCrypt.decryptToString(key));

        for (const auto &param : additionalParams)
            queryData.addQueryItem(param.first, param. second);

        url.setQuery(queryData);

        qDebug() << "Sending LMeve request:" << url;

        QNetworkRequest request{url};
        request.setHeader(QNetworkRequest::UserAgentHeader,
                          QString{"%1 %2"}.arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()));

        auto reply = mNetworkManager.get(request);
        connect(reply, &QNetworkReply::finished, this, &LMeveAPIInterface::processReply, Qt::QueuedConnection);

        mPendingCallbacks[reply] = callback;
    }
}
