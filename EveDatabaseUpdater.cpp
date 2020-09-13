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
#include <QNetworkRequest>
#include <QDesktopWidget>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QApplication>
#include <QJsonObject>
#include <QMessageBox>
#include <QSettings>
#include <QFile>

#include <QtDebug>

#include "EveDatabaseConnectionProvider.h"
#include "UpdaterSettings.h"
#include "ReplyTimeout.h"
#include "FileDownload.h"

#include "EveDatabaseUpdater.h"

namespace Evernus
{
    EveDatabaseUpdater::EveDatabaseUpdater(bool force)
        : QObject{}
        , mForce{force}
    {
        qInfo() << "Checking for SDE updates...";

        mProgress.setRange(0, 100);
        mProgress.setWindowTitle(tr("Downloading..."));

        const auto reply = mNetworkAccessManager.get(QNetworkRequest{QUrl{"http://evernus.anver.ee/latest_version.json"}});
        connect(reply, &QNetworkReply::finished, this, [=] {
            checkUpdate(*reply);
        });

        new ReplyTimeout{*reply};
    }

    EveDatabaseUpdater::Status EveDatabaseUpdater::performUpdate(int argc, char **argv, bool force)
    {
        QApplication app{argc, argv};
        EveDatabaseUpdater updater{force};

        return (app.exec() == 0) ? (Status::Success) : (Status::Error);
    }

    void EveDatabaseUpdater::doUpdate(const QString &latestVersion)
    {
        if (!mForce)
        {
            const auto ret = QMessageBox::question(
                nullptr,
                tr("SDE update"),
                tr("Do you wish to update the Eve Static Data Export database? Evernus needs to have an up-to-date database to function properly.")
            );

            if (ret == QMessageBox::No)
            {
                QCoreApplication::exit();
                return;
            }
        }

        mProgress.move(qApp->desktop()->availableGeometry(&mProgress).center() - mProgress.rect().center());
        mProgress.show();

        const auto download = new FileDownload{QStringLiteral("http://evernus.anver.ee/eve.db"), EveDatabaseConnectionProvider::getDatabasePath(), this};
        connect(download, &FileDownload::downloadProgress, this, [=](auto received, auto total) {
            mProgress.setValue(received * 100 / total);
        });
        connect(download, &FileDownload::finished, this, [=](auto success) {
            download->deleteLater();

            if (!success)
            {
                QMessageBox::critical(&mProgress, tr("SDE update"), tr("Error downloading Eve database!"));
                QCoreApplication::exit(1);
            }
            else
            {
                QSettings settings;
                settings.setValue(UpdaterSettings::sdeVersionKey, latestVersion);

                QCoreApplication::exit();
            }
        });
    }

    void EveDatabaseUpdater::checkUpdate(QNetworkReply &reply)
    {
        const auto error = reply.error();

        qDebug() << "SDE update:" << error;

        if (error != QNetworkReply::NoError)
        {
            const auto ret = QMessageBox::critical(
                nullptr,
                tr("SDE update"),
                tr("Error retrieving latest SDE information: %1\n\nDo you want to try to continue? The application may not launch.").arg(reply.errorString()),
                QMessageBox::Yes | QMessageBox::No
            );

            QCoreApplication::exit((ret == QMessageBox::No) ? (1) : (0));
            return;
        }

        QSettings settings;

        const auto currentVersion = settings.value(UpdaterSettings::sdeVersionKey).toString();

        const auto doc = QJsonDocument::fromJson(reply.readAll());
        const auto latestVersion = doc.object().value(QStringLiteral("sdeVersion")).toString();

        qDebug() << "SDE versions:" << currentVersion << latestVersion;

        if (latestVersion > currentVersion || !QFile::exists(EveDatabaseConnectionProvider::getDatabasePath()) || mForce)
            doUpdate(latestVersion);
        else
            QCoreApplication::exit();
    }
}
