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
#include <QDateTime>
#include <QDialog>

#include "qxthttpsessionmanager.h"
#include "qdropbox2.h"

#include "SimpleCrypt.h"

class QProgressBar;
class QPushButton;
class QGroupBox;
class QLineEdit;
class QLabel;

namespace Evernus
{
    class SyncDialog
        : public QDialog
    {
        Q_OBJECT

    public:
        enum class Mode
        {
            Download,
            Upload
        };

        explicit SyncDialog(Mode mode, QWidget *parent = nullptr);
        virtual ~SyncDialog() = default;

        static bool performedSync();

    private slots:
        void startSync();
        void requestToken(const QString &code);
        void updateProgress(qint64 current, qint64 total);
        void showError(int errorCode, const QString& errorMessage);

        void acceptLocalConnection();

    private:
        class DropboxWebService;

        static const quint16 localPort = 62345;

        static const QString mainDbPath;
        static const QString redirectLink;
        static const QString localRedirectLink;

        static QDateTime mLastSyncTime;

        Mode mMode = Mode::Download;
        bool mStarted = false;

        SimpleCrypt mCrypt;
        QDropbox2 mDb;

        QPushButton *mCancelBtn = nullptr;
        QProgressBar *mProgress = nullptr;
        QGroupBox *mTokenGroup = nullptr;
        QLabel *mTokenLabel = nullptr;
        QLineEdit *mAccessCodeEdit = nullptr;

        QNetworkAccessManager mNetworkAccessManager;

        QxtHttpSessionManager mAuthServer;
        QString mAuthState;
        DropboxWebService *mAuthWebService = nullptr;
        bool mUsingAuthServer = false;

        void showTokenLink();
        void acceptToken(const QString &token);
        void setToken(const QString &token);
        void processMetadata(const QDropbox2EntityInfo &json);
        void handleNoFiles();

        void requestMetadata();
        void downloadFiles();
        void uploadFiles();

        static QString getMainDbPath();

        template<class T>
        static void asyncExec(T &&func);
    };
}
