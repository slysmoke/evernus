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

#include "EveType.h"

namespace Evernus
{
    namespace TypeLookupUtils
    {
        const QString eveMarketdataUrl = "https://evetycoon.com/market/%1";
        const QString eveMarketerUrl = "https://evemarketbrowser.com/region/0/type/%1";
        const QString adam4eveUrl = "https://dev.adam4eve.eu/price_history.php?typeID=%1";

        void lookupType(EveType::IdType id, const QString &baseUrl);
    }
}
