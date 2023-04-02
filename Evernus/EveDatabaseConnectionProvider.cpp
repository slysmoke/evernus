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
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QFile>

#include <QtDebug>

#include "EveDatabaseConnectionProvider.h"

namespace Evernus
{
    QSqlDatabase EveDatabaseConnectionProvider::getConnection() const
    {
        std::ostringstream tid;
        tid << std::this_thread::get_id();

        const auto connName = QStringLiteral("eve-%1").arg(QString::fromStdString(tid.str()));

        auto db = QSqlDatabase::database(connName, false);
        if (!db.isValid())
        {
            db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);

            auto eveDbPath = getDatabasePath();
            qDebug() << "Eve DB path:" << eveDbPath;

            if (!QFile::exists(eveDbPath))
                BOOST_THROW_EXCEPTION(std::runtime_error{"Cannot find Eve DB!"});

            db.setDatabaseName(eveDbPath);
            db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));

            if (!db.open())
                BOOST_THROW_EXCEPTION(std::runtime_error{QCoreApplication::translate("MainDatabaseConnectionProvider", "Error opening DB!").toStdString()});
        }

        return db;
    }

    QString EveDatabaseConnectionProvider::getDatabasePath()
    {
        return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/resources/eve.db";
    }
}
