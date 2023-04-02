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

#include <unordered_set>

class QSqlDatabase;
class QSqlRecord;
class QByteArray;
class QSqlQuery;
class QString;

namespace Evernus::DatabaseUtils
{
    QString getDbPath();
    QString getDbFilePath(const QString &dbName);
    void execQuery(QSqlQuery &query);
    QString backupDatabase(const QSqlDatabase &db);
    QString backupDatabase(const QString &dbPath);

    template<class T>
    std::unordered_set<T> decodeRawSet(const QSqlRecord &record, const QString &name);
    template<class T>
    QByteArray encodeRawSet(const std::unordered_set<T> &values);
}

#include "DatabaseUtils.inl"
