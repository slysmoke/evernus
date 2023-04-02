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
#include <QtDebug>

#include "ESIInterfaceErrorLimiter.h"

namespace Evernus
{
    ESIInterfaceErrorLimiter::ESIInterfaceErrorLimiter(QObject *parent)
        : QObject{parent}
    {
        mErrorRetryTimer.setSingleShot(true);

        // queued because timeout crashes for some reason (someone died in the meantime?)
        connect(&mErrorRetryTimer, &QTimer::timeout,
                this, &ESIInterfaceErrorLimiter::processPendingRequestsAfterErrors, Qt::QueuedConnection);
    }

    void ESIInterfaceErrorLimiter::addCallback(Callback callback, const std::chrono::seconds &timeout)
    {
        const auto errorTime = QDateTime::currentDateTime().addSecs(timeout.count());

        std::lock_guard<std::mutex> lock{mErrorRequestMutex};
        mPendingErrorRequests.emplace_back(std::move(callback));

        if (mNextErrorLimitRetry < errorTime)
        {
            qDebug() << "Rescheduling request:" << errorTime;

            mNextErrorLimitRetry = errorTime;
            mErrorRetryTimer.start(timeout.count() * 1000);
        }
    }

    void ESIInterfaceErrorLimiter::processPendingRequestsAfterErrors()
    {
        decltype(mPendingErrorRequests) requests;

        {
            std::lock_guard<std::mutex> lock{mErrorRequestMutex};
            requests = std::move(mPendingErrorRequests);
        }

        qDebug() << "Processing pending error requests:" << requests.size();

        for (const auto &request : requests)
            request();
    }
}
