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

#include "ESIExternalOrderImporter.h"

namespace Evernus
{
    class ESIWholeExternalOrderImporter
        : public ESIExternalOrderImporter
    {
    public:
        ESIWholeExternalOrderImporter(const EveDataProvider &dataProvider,
                                      ESIInterfaceManager &interfaceManager,
                                      QObject *parent = nullptr);
        virtual ~ESIWholeExternalOrderImporter() = default;

        virtual void fetchExternalOrders(Character::IdType id, const TypeLocationPairs &target) const override;

    private:
        const EveDataProvider &mDataProvider;

        mutable TypeLocationPairs mCurrentTarget;

        virtual void filterOrders(std::vector<ExternalOrder> &orders) const override;
    };
}
