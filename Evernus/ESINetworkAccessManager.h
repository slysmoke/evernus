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

#include <QNetworkAccessManager>

namespace Evernus
{
    class ESINetworkAccessManager
        : public QNetworkAccessManager
    {
    public:
        ESINetworkAccessManager() = default;
        ESINetworkAccessManager(const QString &clientId, const QString &clientSecret, QObject *parent = nullptr);
        ESINetworkAccessManager(const ESINetworkAccessManager &) = default;
        ESINetworkAccessManager(ESINetworkAccessManager &&) = default;
        virtual ~ESINetworkAccessManager() = default;

        ESINetworkAccessManager &operator =(const ESINetworkAccessManager &) = default;
        ESINetworkAccessManager &operator =(ESINetworkAccessManager &&) = default;

    protected:
        virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &originalReq, QIODevice *outgoingData = nullptr) override;

    private:
        QByteArray mAutorization;
    };
}
