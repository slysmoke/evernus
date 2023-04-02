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
#include <QProgressBar>
#include <QObject>

class QNetworkReply;

namespace Evernus
{
    class EveDatabaseUpdater final
        : public QObject
    {
        Q_OBJECT

    public:
        enum class Status
        {
            Success,
            Error,
        };

        EveDatabaseUpdater(const EveDatabaseUpdater &) = delete;
        EveDatabaseUpdater(EveDatabaseUpdater &&) = delete;

        EveDatabaseUpdater &operator =(const EveDatabaseUpdater &) = delete;
        EveDatabaseUpdater &operator =(EveDatabaseUpdater &&) = delete;

        static Status performUpdate(int argc, char **argv, bool force);

    private:
        bool mForce = false;

        QNetworkAccessManager mNetworkAccessManager;

        QProgressBar mProgress;

        explicit EveDatabaseUpdater(bool force);
        virtual ~EveDatabaseUpdater() = default;

        void doUpdate(const QString &latestVersion);
        void checkUpdate(QNetworkReply &reply);
    };
}
