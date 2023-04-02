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
#include <QDataStream>

namespace Evernus
{
    template<class First, class Second>
    QDataStream &operator <<(QDataStream &stream, const std::pair<First, Second> &pair)
    {
        return stream
            << pair.first
            << pair.second;
    }

    template<class First, class Second>
    QDataStream &operator >>(QDataStream &stream, std::pair<First, Second> &pair)
    {
        return stream
            >> pair.first
            >> pair.second;
    }

    template<class Key, class T, class Hash, class KeyEqual, class Allocator>
    QDataStream &operator <<(QDataStream &stream, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &map)
    {
        stream << static_cast<quint64>(map.size());
        for (const auto &pair : map)
            stream << pair;

        return stream;
    }

    template<class Key, class T, class Hash, class KeyEqual, class Allocator>
    QDataStream &operator >>(QDataStream &stream, std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &map)
    {
        map.clear();

        quint64 size = 0;
        stream >> size;

        for (auto i = 0u; i < size; ++i)
        {
            std::pair<Key, T> value;
            stream >> value;

            map.insert(value);
        }

        return stream;
    }

    template<class Key, class Hash, class KeyEqual, class Allocator>
    QDataStream &operator <<(QDataStream &stream, const std::unordered_set<Key, Hash, KeyEqual, Allocator> &set)
    {
        stream << static_cast<quint64>(set.size());
        for (const auto &key : set)
            stream << key;

        return stream;
    }

    template<class Key, class Hash, class KeyEqual, class Allocator>
    QDataStream &operator >>(QDataStream &stream, std::unordered_set<Key, Hash, KeyEqual, Allocator> &set)
    {
        set.clear();

        quint64 size = 0;
        stream >> size;

        for (auto i = 0u; i < size; ++i)
        {
            Key value;
            stream >> value;

            set.insert(value);
        }

        return stream;
    }
}
