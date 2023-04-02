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

#include <chrono>

#include <QTimer>

#include "NetworkSettings.h"

class QNetworkReply;

namespace Evernus
{
    class ReplyTimeout
        : public QObject
    {
        Q_OBJECT

    public:
        explicit ReplyTimeout(QNetworkReply &reply);
        virtual ~ReplyTimeout() = default;

    private slots:
        void checkTimeout();

    private:
        // Single timer for all instances was introduced, because creating too many single shot timers reached resource
        // limit on Windows.
        static QTimer mTimer;

        std::chrono::steady_clock::time_point mStartTime = std::chrono::steady_clock::now();
        std::chrono::seconds::rep mMaxTime = NetworkSettings::maxReplyTimeDefault;
    };
}
