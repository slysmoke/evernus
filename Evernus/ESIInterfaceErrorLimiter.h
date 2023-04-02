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
#include <vector>
#include <chrono>
#include <mutex>

#include <QDateTime>
#include <QTimer>

namespace Evernus
{
    class ESIInterfaceErrorLimiter final
        : public QObject
    {
        Q_OBJECT

    public:
        using Callback = std::function<void ()>;

        explicit ESIInterfaceErrorLimiter(QObject *parent = nullptr);
        ESIInterfaceErrorLimiter(const ESIInterfaceErrorLimiter &) = default;
        ESIInterfaceErrorLimiter(ESIInterfaceErrorLimiter &&) = default;
        virtual ~ESIInterfaceErrorLimiter() = default;

        void addCallback(Callback callback, const std::chrono::seconds &timeout);

        ESIInterfaceErrorLimiter &operator =(const ESIInterfaceErrorLimiter &) = default;
        ESIInterfaceErrorLimiter &operator =(ESIInterfaceErrorLimiter &&) = default;

    private slots:
        void processPendingRequestsAfterErrors();

    private:
        std::vector<Callback> mPendingErrorRequests;
        QTimer mErrorRetryTimer;
        QDateTime mNextErrorLimitRetry;
        std::mutex mErrorRequestMutex;
    };
}
