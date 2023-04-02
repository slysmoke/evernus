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
#include <QApplication>
#include <QMessageBox>
#include <QSslError>
#include <QSettings>
#include <QtDebug>

#include "NetworkSettings.h"

#include "SecurityHelper.h"

namespace Evernus
{
    namespace SecurityHelper
    {
        void handleSslErrors(const QList<QSslError> &errors, QNetworkReply &reply)
        {
            qWarning() << "Encountered SSL errors:" << errors;

            QSettings settings;
            if (settings.value(NetworkSettings::ignoreSslErrorsKey, NetworkSettings::ignoreSslErrorsDefault).toBool())
            {
                qWarning() << "Ignoring all.";

                reply.ignoreSslErrors();
                return;
            }

            QStringList errorTexts;
            for (const auto &error : errors)
                errorTexts << error.errorString();

            const auto ret = QMessageBox::question(QApplication::activeWindow(),
                                                   QCoreApplication::translate("SecurityHelper", "Security error"),
                                                   QCoreApplication::translate("SecurityHelper",
                "Encountered SSL errors:\n%1\nAre you sure you wish to proceed (doing so can compromise your account security)?").arg(errorTexts.join("\n")));
            if (ret == QMessageBox::Yes)
                reply.ignoreSslErrors(errors);
        }
    }
}
