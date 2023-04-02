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
#include <QSettings>

#include "ReplyTimeout.h"

namespace Evernus
{
    QTimer ReplyTimeout::mTimer;

    ReplyTimeout::ReplyTimeout(QNetworkReply &reply)
        : QObject{&reply}
    {
        QSettings settings;
        mMaxTime = settings.value(NetworkSettings::maxReplyTimeKey, NetworkSettings::maxReplyTimeDefault)
            .value<std::chrono::seconds::rep>();

        if (!mTimer.isActive())
            mTimer.start(5000);

        connect(&mTimer, &QTimer::timeout, this, &ReplyTimeout::checkTimeout);
    }

    void ReplyTimeout::checkTimeout()
    {
        auto delta = std::chrono::steady_clock::now() - mStartTime;
        if (std::chrono::duration_cast<std::chrono::seconds>(delta).count() >= mMaxTime)
        {
            auto reply = static_cast<QNetworkReply *>(parent());
            if (reply->isRunning())
                reply->abort();
        }
    }
}
