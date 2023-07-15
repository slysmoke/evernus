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
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QNetworkReply>

#include "SecurityHelper.h"
#include "ReplyTimeout.h"

#include "StructureHuntInterface.h"

namespace Evernus
{
    const QUrl StructureHuntInterface::url{"https://slysmoke.github.io/evernus-db/citadel.json"};
	//const QUrl StructureHuntInterface::url{"https://stop.hammerti.me.uk/api/citadel/all"};
    void StructureHuntInterface::fetchCitadels(Callback callback) const
    {
        QNetworkRequest request{url};
        request.setHeader(QNetworkRequest::UserAgentHeader,
                          QString{"%1 %2"}.arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()));

        auto reply = mNetworkAccessManager.get(request);
        Q_ASSERT(reply != nullptr);

        new ReplyTimeout{*reply};

        connect(reply, &QNetworkReply::sslErrors, this, &StructureHuntInterface::processSslErrors);
        connect(reply, &QNetworkReply::finished, this, [=, callback = std::move(callback)] {
            reply->deleteLater();

            const auto error = reply->error();
            if (error != QNetworkReply::NoError)
                callback(QJsonDocument{}, reply->errorString());
            else
                callback(QJsonDocument::fromJson(reply->readAll()), QString{});
        });
    }

    void StructureHuntInterface::processSslErrors(const QList<QSslError> &errors)
    {
        SecurityHelper::handleSslErrors(errors, *qobject_cast<QNetworkReply *>(sender()));
    }
}
