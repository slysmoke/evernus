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
#include <QSaveFile>

namespace Evernus
{
    class FileDownload
        : public QObject
    {
        Q_OBJECT

    public:
        FileDownload(const QUrl &addr, const QString &dest, QObject *parent = nullptr);
        virtual ~FileDownload() = default;

    signals:
        void finished(bool success);
        void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    private slots:
        void process();
        void finish();

    private:
        QNetworkAccessManager mNetworkManager;
        QSaveFile mOutput;
    };
}
