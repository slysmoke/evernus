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

#include <vector>

#include <QObject>

#include "TypeLocationPairs.h"
#include "Character.h"

namespace Evernus
{
    class ExternalOrder;

    class ExternalOrderImporter
        : public QObject
    {
        Q_OBJECT

    public:
        using QObject::QObject;
        virtual ~ExternalOrderImporter() = default;

        virtual void fetchExternalOrders(Character::IdType id, const TypeLocationPairs &target) const = 0;

    signals:
        void externalOrdersChanged(const QString &info, const std::vector<ExternalOrder> &orders) const;
        void genericError(const QString &info) const;
        void statusChanged(const QString &status) const;
    };
}
