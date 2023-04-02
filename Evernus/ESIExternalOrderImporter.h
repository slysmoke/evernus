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

#include "CallbackExternalOrderImporter.h"
#include "ESIManager.h"
#include "Character.h"

class QByteArray;

namespace Evernus
{
    class EveDataProvider;

    class ESIExternalOrderImporter
        : public CallbackExternalOrderImporter
    {
        Q_OBJECT

    public:
        ESIExternalOrderImporter(const EveDataProvider &dataProvider,
                                 ESIInterfaceManager &interfaceManager,
                                 QObject *parent = nullptr);
        ESIExternalOrderImporter(const ESIExternalOrderImporter &) = default;
        ESIExternalOrderImporter(ESIExternalOrderImporter &&) = default;
        virtual ~ESIExternalOrderImporter() = default;

        ESIExternalOrderImporter &operator =(const ESIExternalOrderImporter &) = default;
        ESIExternalOrderImporter &operator =(ESIExternalOrderImporter &&) = default;

    protected:
        ESIManager mManager;
    };
}
