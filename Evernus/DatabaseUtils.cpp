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

#include <boost/throw_exception.hpp>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtDebug>
#include <QFile>

#include "DatabaseUtils.h"

namespace Evernus::DatabaseUtils
{
    QString getDbPath()
    {
        return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/db/";
    }

    QString getDbFilePath(const QString &dbName)
    {
        return getDbPath() + dbName;
    }

    void execQuery(QSqlQuery &query)
    {
        qDebug() << "SQL:" << query.lastQuery();
        if (!query.exec())
        {
            auto error = query.lastError();
            qCritical() << error;

            BOOST_THROW_EXCEPTION(std::runtime_error{error.text().toStdString()});
        }
    }

    QString backupDatabase(const QSqlDatabase &db)
    {
        return backupDatabase(db.databaseName());
    }

    QString backupDatabase(const QString &dbPath)
    {
        const auto dbBak = dbPath + QStringLiteral(".bak");

        QFile::remove(dbBak);
        QFile::copy(dbPath, dbBak);

        return dbBak;
    }
}
