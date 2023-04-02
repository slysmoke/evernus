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

#include <functional>

#include <QNetworkAccessManager>
#include <QList>
#include <QUrl>

class QJsonDocument;
class QSslError;
class QString;

namespace Evernus
{
    class StructureHuntInterface final
        : public QObject
    {
        Q_OBJECT

    public:
        using Callback = std::function<void (const QJsonDocument &result, const QString &error)>;

        StructureHuntInterface() = default;
        StructureHuntInterface(const StructureHuntInterface &) = default;
        StructureHuntInterface(StructureHuntInterface &&) = default;
        ~StructureHuntInterface() = default;

        void fetchCitadels(Callback callback) const;

        StructureHuntInterface &operator =(const StructureHuntInterface &) = default;
        StructureHuntInterface &operator =(StructureHuntInterface &&) = default;

    private slots:
        void processSslErrors(const QList<QSslError> &errors);

    private:
        static const QUrl url;

        mutable QNetworkAccessManager mNetworkAccessManager;
    };
}
