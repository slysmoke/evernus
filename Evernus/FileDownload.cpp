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

#include <QNetworkReply>
#include <QFileInfo>
#include <QDir>

#include <QtDebug>

#include "FileDownload.h"

namespace Evernus
{
    FileDownload::FileDownload(const QUrl &addr, const QString &dest, QObject *parent)
        : QObject{parent}
        , mOutput{dest}
    {
        QFileInfo info{dest};
        QDir{}.mkpath(info.dir().path());

        qDebug() << "Downloading" << addr << "to" << dest;

        if (!mOutput.open(QIODevice::WriteOnly | QIODevice::Truncate))
            BOOST_THROW_EXCEPTION(std::runtime_error(tr("Error creating file: %1").arg(dest).toStdString()));

        auto reply = mNetworkManager.get(QNetworkRequest{addr});
        connect(reply, &QNetworkReply::readyRead, this, &FileDownload::process);
        connect(reply, &QNetworkReply::finished, this, &FileDownload::finish);
        connect(reply, &QNetworkReply::downloadProgress, this, &FileDownload::downloadProgress);
        connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    }

    void FileDownload::process()
    {
        const auto reply = static_cast<QNetworkReply *>(sender());
        mOutput.write(reply->readAll());
    }

    void FileDownload::finish()
    {
        const auto reply = static_cast<QNetworkReply *>(sender());
        auto success = reply->error() == QNetworkReply::NoError;

        qDebug() << "Download success:" << success;

        if (success)
            success = mOutput.commit();

        emit finished(success);
    }
}
