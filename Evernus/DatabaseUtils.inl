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
#include <algorithm>

#include <QDataStream>
#include <QSqlRecord>
#include <QVariant>

namespace Evernus::DatabaseUtils
{
    template<class T>
    std::unordered_set<T> decodeRawSet(const QSqlRecord &record, const QString &name)
    {
        QDataStream stream{record.value(name).toByteArray()};

        QVariantList list;
        stream >> list;

        std::unordered_set<T> result;
        result.reserve(list.size());

        for (const auto &value : list)
            result.emplace(value.template value<T>());

        return result;
    }

    template<class T>
    QByteArray encodeRawSet(const std::unordered_set<T> &values)
    {
        QVariantList list;
        std::copy(std::begin(values), std::end(values), std::back_inserter(list));

        QByteArray data;

        QDataStream stream{&data, QIODevice::WriteOnly};
        stream << list;

        return data;
    }
}
