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
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

#include <QtDebug>

#include "SSOAuthDialog.h"
#include "MainWindow.h"
#include "SSOUtils.h"
#include "ESIUrls.h"

#include "NewCharacterController.h"

namespace Evernus
{
    NewCharacterController::NewCharacterController(const QString &clientIdentifier,
                                                   const QString &clientSecret,
                                                   QObject *parent)
        : QAbstractOAuthReplyHandler{parent}
        , mFlow{clientIdentifier, clientSecret}
    {
        connect(&mFlow, &ESIOAuth2UnknownCharacterAuthorizationCodeFlow::characterConfirmed,
                this, &NewCharacterController::processCharacter);
        connect(&mFlow, &ESIOAuth2UnknownCharacterAuthorizationCodeFlow::error,
                this, &NewCharacterController::handleError);
        connect(&mFlow, &ESIOAuth2UnknownCharacterAuthorizationCodeFlow::authorizeWithBrowser,
                this, &NewCharacterController::requestSSOAuth);

        mFlow.setReplyHandler(this);
        mFlow.grant();
    }

    QString NewCharacterController::callback() const
    {
        return ESIUrls::callbackUrl;
    }

    void NewCharacterController::networkReplyFinished(QNetworkReply *reply)
    {
        Q_ASSERT(reply != nullptr);

       const auto data = reply->readAll();
       const auto replyError = reply->error();

       if (Q_UNLIKELY(replyError != QNetworkReply::NoError))
       {
           const auto errorObj = QJsonDocument::fromJson(data).object();
           const auto errorStr = (errorObj.contains(QStringLiteral("error"))) ? (errorObj.value(QStringLiteral("error")).toString()) : (reply->errorString());

           qWarning() << "New character error:" << replyError << errorStr << data;

           emit tokensReceived({{ QStringLiteral("error"), errorStr }});
           return;
       }

       emit replyDataReceived(data);
       emit tokensReceived(QJsonDocument::fromJson(data).object().toVariantMap());
    }

    void NewCharacterController::processCharacter(Character::IdType id)
    {
        deleteLater();
        emit authorizedCharacter(id, mFlow.token(), mFlow.refreshToken());
    }

    void NewCharacterController::handleError(const QString &message)
    {
        deleteLater();
        emit error(message);
    }

    void NewCharacterController::requestSSOAuth(const QUrl &url)
    {
        const auto authView = new SSOAuthDialog{url, static_cast<QWidget *>(parent())};
        authView->setWindowTitle(tr("SSO authentication"));
        authView->show();

        connect(authView, &SSOAuthDialog::aboutToClose, this, [=] {
            authView->deleteLater();
            handleError(tr("SSO authorization cancelled."));
        });
        connect(authView, &SSOAuthDialog::acquiredCode, this, [=](const auto &code) {
            authView->disconnect(this);
            authView->deleteLater();

            emit callbackReceived(SSOUtils::parseAuthorizationCode(code));
        });
    }
}
