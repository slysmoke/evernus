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

#include "DatabaseConnectionProvider.h"

namespace Evernus
{
    class EveDatabaseConnectionProvider final
        : public DatabaseConnectionProvider
    {
    public:
        EveDatabaseConnectionProvider() = default;
        EveDatabaseConnectionProvider(const EveDatabaseConnectionProvider &) = default;
        EveDatabaseConnectionProvider(EveDatabaseConnectionProvider &&) = default;
        virtual ~EveDatabaseConnectionProvider() = default;

        virtual QSqlDatabase getConnection() const override;

        EveDatabaseConnectionProvider &operator =(const EveDatabaseConnectionProvider &) = default;
        EveDatabaseConnectionProvider &operator =(EveDatabaseConnectionProvider &&) = default;

        static QString getDatabasePath();
    };
}
