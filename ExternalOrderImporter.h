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
#include <vector>

#include <boost/functional/hash.hpp>

#include <QObject>

#include "EveType.h"

namespace Evernus
{
    class ExternalOrder;

    class ExternalOrderImporter
        : public QObject
    {
        Q_OBJECT

    public:
        typedef std::pair<EveType::IdType, quint64> TypeLocationPair;
        typedef std::unordered_set<TypeLocationPair, boost::hash<TypeLocationPair>> TypeLocationPairs;

        using QObject::QObject;
        virtual ~ExternalOrderImporter() = default;

        virtual void fetchExternalOrders(const TypeLocationPairs &target) const = 0;

    signals:
        void externalOrdersChanged(const std::vector<ExternalOrder> &orders) const;
        void error(const QString &info) const;
        void statusChanged(const QString &status) const;
    };
}
