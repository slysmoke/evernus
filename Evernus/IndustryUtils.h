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

#include <chrono>

#include <QtGlobal>

namespace Evernus
{
    namespace IndustryUtils
    {
        // note: changing these requires changes to getRequiredQuantity()
        enum class FacilityType
        {
            Station,
            AssemblyArray,
            ThukkerComponentArray,
            RapidAssemblyArray,
            EngineeringComplex,
        };

        enum class SecurityStatus
        {
            HighSec,
            LowSec,
            NullSecWH,
        };

        enum class RigType
        {
            None,
            T1,
            T2,
        };

        enum class Size
        {
            Medium,
            Large,
            XLarge,
        };

        quint64 getRequiredQuantity(uint runs,
                                    uint baseQuantity,
                                    uint materialEfficiency,
                                    FacilityType facilityType,
                                    SecurityStatus securityStatus,
                                    RigType rigType);

        std::chrono::seconds getProductionTime(std::chrono::seconds baseProductionTime,
                                               uint timeEfficiency,
                                               float implantBonus,
                                               float skillModifier,
                                               FacilityType facilityType,
                                               SecurityStatus securityStatus,
                                               Size facilitySize,
                                               RigType rigType);
    }
}
