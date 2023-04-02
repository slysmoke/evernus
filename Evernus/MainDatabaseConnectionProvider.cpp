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
#include <stdexcept>
#include <sstream>
#include <thread>

#include <boost/throw_exception.hpp>

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSettings>
#include <QSqlQuery>

#include "DatabaseUtils.h"
#include "DbSettings.h"

#include "MainDatabaseConnectionProvider.h"

namespace Evernus
{
    QSqlDatabase MainDatabaseConnectionProvider::getConnection() const
    {
        std::ostringstream tid;
        tid << std::this_thread::get_id();

        const auto connName = QStringLiteral("main-%1").arg(QString::fromStdString(tid.str()));

        auto db = QSqlDatabase::database(connName, false);
        if (!db.isValid())
        {
            db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
            db.setConnectOptions(QStringLiteral("QSQLITE_BUSY_TIMEOUT=10000000"));
            db.setDatabaseName(DatabaseUtils::getDbFilePath(QStringLiteral("main.db")));

            if (!db.open())
                BOOST_THROW_EXCEPTION(std::runtime_error{QCoreApplication::translate("MainDatabaseConnectionProvider", "Error opening DB!").toStdString()});

            db.exec(QStringLiteral("PRAGMA foreign_keys = ON"));

            QSettings settings;

            // disable syncing changes to the disk between
            // each transaction. This means the database can become
            // corrupted in the event of a power failure or OS crash
            // but NOT in the event of an application error
            db.exec(QStringLiteral("PRAGMA synchronous = %1").arg(
                settings.value(DbSettings::synchronousKey, DbSettings::synchronousDefault).toInt()
            ));
        }

        return db;
    }
}
