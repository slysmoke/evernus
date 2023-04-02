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
#include <future>

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QApplication>
#include <QProgressBar>
#include <QJsonObject>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QSettings>
#include <QSaveFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QUrlQuery>
#include <QLabel>
#include <QMovie>
#include <QtDebug>
#include <QFile>
#include <QUuid>
#include <QFont>
#include <QUrl>

#include "DatabaseUtils.h"
#include "SyncSettings.h"
#include "ReplyTimeout.h"

#include "qxtabstracthttpconnector.h"
#include "qxtabstractwebservice.h"
#include "qxtwebevent.h"

#include "qdropbox2file.h"

#include "SyncDialog.h"

#define STR_VALUE(s) #s
#define EVERNUS_TEXT(s) STR_VALUE(s)
#define EVERNUS_DROPBOX_APP_KEY_TEXT EVERNUS_TEXT(EVERNUS_DROPBOX_APP_KEY)
#define EVERNUS_DROPBOX_APP_SECRET_TEXT EVERNUS_TEXT(EVERNUS_DROPBOX_APP_SECRET)

namespace Evernus
{
    class SyncDialog::DropboxWebService
        : public QxtAbstractWebService
    {
        Q_OBJECT

    public:
        DropboxWebService(QxtAbstractWebSessionManager *manager, QString authState, QObject *parent)
            : QxtAbstractWebService{manager, parent}
            , mAuthState{std::move(authState)}
        {
        }

        virtual ~DropboxWebService() = default;

        virtual void pageRequestedEvent(QxtWebRequestEvent *event) override
        {
            Q_ASSERT(event != nullptr);

            // NOTE: don't use raw literal or automoc will not see the class
            postEvent(new QxtWebPageEvent{event->sessionID, event->requestID,
                "<!doctype html>"
                "<html>"
                "    <head>"
                "        <meta http-equiv=\"refresh\" content=\"0; url=http://evernus.com/\" />"
                "    </head>"
                "    <body>"
                "    </body>"
                "</html>"
            });

            QUrlQuery query{event->url};

            const auto code = query.queryItemValue("code");
            if (code.isEmpty())
                return;

            if (query.queryItemValue("state") != mAuthState)
                return;

            emit codeAcquired(code);
        }

    signals:
        void codeAcquired(const QString &code);

    private:
        QString mAuthState;
    };

    const QString SyncDialog::mainDbPath = "/sandbox/main.db";
    const QString SyncDialog::redirectLink = "https://slysmoke.github.io/evernus-auth/index.html#/";
    const QString SyncDialog::localRedirectLink = QStringLiteral("http://localhost:%1").arg(localPort);

    QDateTime SyncDialog::mLastSyncTime;

    SyncDialog::SyncDialog(Mode mode, QWidget *parent)
        : QDialog(parent)
        , mMode(mode)
        , mCrypt(Q_UINT64_C(0x4630e0cc6a00124b))
        , mAuthState(QUuid::createUuid().toString())
        , mAuthWebService(new DropboxWebService(&mAuthServer, mAuthState, this))
    {
        auto mainLayout = new QVBoxLayout{this};

        auto infoLayout = new QHBoxLayout{};
        mainLayout->addLayout(infoLayout);

        auto throbberMovie = new QMovie{":/images/loader.gif", QByteArray{}, this};

        QFont font;
        font.setPixelSize(16);

        auto throbber = new QLabel{this};
        infoLayout->addWidget(throbber);
        throbber->setMovie(throbberMovie);

        throbberMovie->start();

        auto throbberText = new QLabel{tr("Synchronizing..."), this};
        infoLayout->addWidget(throbberText, 1, Qt::AlignLeft);
        throbberText->setFont(font);

        mCancelBtn = new QPushButton{tr("Cancel"), this};
        infoLayout->addWidget(mCancelBtn);
        connect(mCancelBtn, &QPushButton::clicked, this, &SyncDialog::reject);

        mProgress = new QProgressBar{this};
        mainLayout->addWidget(mProgress);
        mProgress->setRange(0, 100);

        // since Dropbox allows for many redirect urls, let's try setting up local server and fall back to the webpage
        mAuthServer.setPort(localPort);
        mAuthServer.setListenInterface(QHostAddress::LocalHost);
        mAuthServer.setAutoCreateSession(false);
        mAuthServer.setStaticContentService(mAuthWebService);
        mAuthServer.setConnector(new QxtHttpServerConnector{this});

        if (mAuthServer.start())
        {
            mUsingAuthServer = true;

            mTokenGroup = new QGroupBox{this};
            mainLayout->addWidget(mTokenGroup);
            mTokenGroup->setVisible(false);

            auto tokenLayout = new QVBoxLayout{mTokenGroup};

            mTokenLabel = new QLabel{this};
            tokenLayout->addWidget(mTokenLabel);
            mTokenLabel->setOpenExternalLinks(true);
            mTokenLabel->setWordWrap(true);
        }
        else
        {
            mTokenGroup = new QGroupBox{this};
            mainLayout->addWidget(mTokenGroup);
            mTokenGroup->setVisible(false);

            auto tokenLayout = new QVBoxLayout{mTokenGroup};

            mTokenLabel = new QLabel{this};
            tokenLayout->addWidget(mTokenLabel);
            mTokenLabel->setOpenExternalLinks(true);
            mTokenLabel->setWordWrap(true);

            mAccessCodeEdit = new QLineEdit{this};
            tokenLayout->addWidget(mAccessCodeEdit);

            auto acceptTokenBtn = new QPushButton{tr("Proceed"), this};
            tokenLayout->addWidget(acceptTokenBtn);
            connect(acceptTokenBtn, &QPushButton::clicked, this, [=] {
                const auto code = mAccessCodeEdit->text();
                if (code.isEmpty())
                    return;

                requestToken(code);
            });
        }

        setWindowTitle(tr("Synchronization"));

        connect(&mDb, &QDropbox2::signal_errorOccurred, this, &SyncDialog::showError);
        connect(mAuthWebService, &DropboxWebService::codeAcquired, this, &SyncDialog::requestToken);

        QMetaObject::invokeMethod(this, "startSync", Qt::QueuedConnection);
    }

    bool SyncDialog::performedSync()
    {
        return mLastSyncTime.isValid();
    }

    void SyncDialog::startSync()
    {
        qDebug() << "Starting sync...";

        QSettings settings;

        const auto token = mCrypt.decryptToString(settings.value(SyncSettings::dbTokenKey).toString());
        if (token.isEmpty())
        {
            showTokenLink();
        }
        else
        {
            acceptToken(token);
        }
    }

    void SyncDialog::showTokenLink()
    {
        if (mUsingAuthServer)
        {
            const auto link = QDropbox2::authorizeLink(EVERNUS_DROPBOX_APP_KEY_TEXT, localRedirectLink, mAuthState);
            qDebug() << "Dropbox auth link:" << link;

            mTokenLabel->setText(tr("Dropbox requires authenticating Evernus first. Please click on "
                "<a href='%1'>this link</a>, authorize Evernus and wait for the application to proceed.").arg(link.toString()));
        }
        else
        {
            const auto link = QDropbox2::authorizeLink(EVERNUS_DROPBOX_APP_KEY_TEXT, redirectLink);
            qDebug() << "Dropbox auth link:" << link;

            mTokenLabel->setText(tr("Dropbox requires authenticating Evernus first. Please click on "
                "<a href='%1'>this link</a>, authorize Evernus and enter the resulting code below.").arg(link.toString()));
        }

        mTokenGroup->show();
        adjustSize();
    }

    void SyncDialog::requestToken(const QString &code)
    {
        qDebug() << "Requesting access token...";

        mTokenGroup->hide();
        adjustSize();

        activateWindow();

        QNetworkRequest request{QUrl{QStringLiteral("https://www.dropbox.com/oauth2/token")}};
        request.setHeader(QNetworkRequest::UserAgentHeader,
                          QStringLiteral("%1 %2").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

        QUrlQuery query;
        query.addQueryItem(QStringLiteral("code"), code);
        query.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("authorization_code"));
        query.addQueryItem(QStringLiteral("client_id"), EVERNUS_DROPBOX_APP_KEY_TEXT);
        query.addQueryItem(QStringLiteral("client_secret"), EVERNUS_DROPBOX_APP_SECRET_TEXT);
        query.addQueryItem(QStringLiteral("redirect_uri"), (mUsingAuthServer) ? (localRedirectLink) : (redirectLink));

        auto reply = mNetworkAccessManager.post(request, query.toString().toLatin1());
        Q_ASSERT(reply != nullptr);

        new ReplyTimeout{*reply};

        connect(reply, &QNetworkReply::sslErrors, this, [=] {
            qWarning() << "Dropbox SSL errors!";
            QMessageBox::warning(this, tr("Security warning"), tr("Encountered certificate errors while contacting Dropbox. Aborting synchronization."));
        });
        connect(reply, &QNetworkReply::finished, this, [=] {
            reply->deleteLater();

            const auto error = reply->error();
            if (error != QNetworkReply::NoError)
            {
                qWarning() << "Dropbox token request error:" << error;
                QMessageBox::warning(this, tr("Dropbox error"), reply->errorString());
            }
            else
            {
                const auto json = QJsonDocument::fromJson(reply->readAll());
                setToken(json.object().value(QStringLiteral("access_token")).toString());
            }
        });
    }

    void SyncDialog::acceptToken(const QString &token)
    {
        mDb.setAccessToken(token);
        requestMetadata();
    }

    void SyncDialog::showError(int errorCode, const QString& errorMessage)
    {
        qDebug() << "Error:" << errorCode << errorMessage;

        QMessageBox::warning(this, tr("Synchronization"), tr("Error: %1 (%2)").arg(errorCode).arg(errorMessage));
        QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);
    }

    void SyncDialog::acceptLocalConnection()
    {

    }

    void SyncDialog::setToken(const QString &token)
    {
        QSettings settings;
        settings.setValue(SyncSettings::dbTokenKey, mCrypt.encryptToString(token));

        acceptToken(token);
    }

    void SyncDialog::processMetadata(const QDropbox2EntityInfo &info)
    {
        if (mStarted)
            return;

        if (mMode == Mode::Download)
        {
            QFileInfo currentInfo{getMainDbPath()};
            if (currentInfo.lastModified() > info.clientModified())
            {
                QSettings settings;
                if (settings.value(SyncSettings::firstSyncKey, true).toBool())
                {
                    const auto ret = QMessageBox::question(this,
                                                           tr("Synchronization"),
                                                           tr("Your local database is newer than cloud one. Do you wish to replace your local copy?"));
                    if (ret == QMessageBox::Yes)
                    {
                        downloadFiles();
                        return;
                    }
                }

                mLastSyncTime = QDateTime::currentDateTimeUtc();
                QMetaObject::invokeMethod(this, "accept", Qt::QueuedConnection);
            }
            else
            {
                downloadFiles();
            }
        }
        else
        {
            if (mLastSyncTime.isValid() && info.clientModified() > mLastSyncTime)
            {
                const auto ret = QMessageBox::question(this,
                                                       tr("Synchronization"),
                                                       tr("Something modified cloud data since last synchronization. Do you wish to overwrite it?"));
                if (ret == QMessageBox::No)
                {
                    QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);
                    return;
                }
            }

            uploadFiles();
        }
    }

    void SyncDialog::handleNoFiles()
    {
        if (mMode == Mode::Download)
            reject();
        else if (!mStarted)
            uploadFiles();
    }

    void SyncDialog::updateProgress(qint64 current, qint64 total)
    {
        if (total > 0)
            mProgress->setValue(current * 100 / total);
    }

    void SyncDialog::requestMetadata()
    {
        qDebug() << "Requesting metadata...";

        QDropbox2File file{mainDbPath, &mDb};
        connect(&file, &QDropbox2File::signal_errorOccurred, this, &SyncDialog::showError);

        const auto metadata = file.metadata();
        if (file.error() == QDropbox2::Error::NoError || file.error() == QDropbox2::Error::FileNotFound)
            processMetadata(metadata);
    }

    void SyncDialog::downloadFiles()
    {
        qDebug() << "Downloading files...";

        mStarted = true;

        QDropbox2File mainDb{mainDbPath, &mDb};
        connect(&mainDb, &QDropbox2File::signal_downloadProgress, this, &SyncDialog::updateProgress);
        connect(&mainDb, &QDropbox2File::signal_errorOccurred, this, &SyncDialog::showError);
        connect(mCancelBtn, &QPushButton::clicked, &mainDb, &QDropbox2File::slot_abort, Qt::QueuedConnection);

        if (mainDb.open(QIODevice::ReadOnly))
        {
            qDebug() << "Saving main db...";

            mCancelBtn->setEnabled(false);
            DatabaseUtils::backupDatabase(getMainDbPath());

            QSaveFile file{getMainDbPath()};
            if (!file.open(QIODevice::WriteOnly))
            {
                QMessageBox::warning(this, tr("Synchronization"), tr("Couldn't open file for writing! Synchronization failed."));
                QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);
                return;
            }

            asyncExec([&] {
                file.write(qUncompress(mainDb.readAll()));
            });
            mCancelBtn->setEnabled(true);

            if (file.commit())
            {
                mLastSyncTime = mainDb.metadata().clientModified();
            }
            else
            {
                QMessageBox::warning(this, tr("Synchronization"), tr("Couldn't write destination file! Synchronization failed."));
                QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);
            }
        }

        QMetaObject::invokeMethod(this, "accept", Qt::QueuedConnection);
    }

    void SyncDialog::uploadFiles()
    {
        qDebug() << "Uploading files...";

        mStarted = true;
        mCancelBtn->setEnabled(false);

        QDropbox2File mainDb{mainDbPath, &mDb};
        mainDb.setOverwrite(true);
        connect(&mainDb, &QDropbox2File::signal_uploadProgress, this, &SyncDialog::updateProgress);
        connect(mCancelBtn, &QPushButton::clicked, &mainDb, &QDropbox2File::slot_abort);

        if (!mainDb.open(QIODevice::WriteOnly))
        {
            QMessageBox::warning(this, tr("Synchronization"), tr("Couldn't open remote file! Synchronization failed."));
            QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);
            return;
        }

        QFile localMainDb{getMainDbPath()};
        if (!localMainDb.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, tr("Synchronization"), tr("Couldn't open local file! Synchronization failed."));
            QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);
            return;
        }

        asyncExec([&] {
            const auto data = qCompress(localMainDb.readAll(), 9);
            mainDb.write(data);
        });
        mCancelBtn->setEnabled(true);

        mainDb.close();

        QSettings settings;
        settings.setValue(SyncSettings::firstSyncKey, false);

        mLastSyncTime = mainDb.metadata().clientModified();

        QMetaObject::invokeMethod(this, "accept", Qt::QueuedConnection);
    }

    QString SyncDialog::getMainDbPath()
    {
        return DatabaseUtils::getDbPath() + "main.db";
    }

    template<class T>
    void SyncDialog::asyncExec(T &&func)
    {
        auto future = std::async(std::launch::async, std::forward<T>(func));
        while (future.wait_for(std::chrono::seconds{0}) != std::future_status::ready)
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

#include "SyncDialog.moc"
