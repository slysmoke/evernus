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
#include <cmath>

#include "IndustryUtils.h"

namespace Evernus
{
    namespace IndustryUtils
    {
        namespace
        {
            const float securityMods[] = { 1.f, 1.9f, 2.1f };
        }

        quint64 getRequiredQuantity(uint runs,
                                    uint baseQuantity,
                                    uint materialEfficiency,
                                    FacilityType facilityType,
                                    SecurityStatus securityStatus,
                                    RigType rigType)
        {
            Q_ASSERT(facilityType == FacilityType::AssemblyArray ||
                     facilityType == FacilityType::RapidAssemblyArray ||
                     facilityType == FacilityType::Station ||
                     facilityType == FacilityType::ThukkerComponentArray ||
                     facilityType == FacilityType::EngineeringComplex);

            auto facilityMod = 1.f;
            if (facilityType == FacilityType::AssemblyArray ||
                facilityType == FacilityType::RapidAssemblyArray ||
                facilityType == FacilityType::Station ||
                facilityType == FacilityType::ThukkerComponentArray)
            {
                const float mods[] = { 1.f, 0.98f, 0.85f, 1.05f };
                facilityMod = mods[static_cast<int>(facilityType)];
            }
            else
            {
                Q_ASSERT(securityStatus == SecurityStatus::HighSec ||
                         securityStatus == SecurityStatus::LowSec ||
                         securityStatus == SecurityStatus::NullSecWH);
                Q_ASSERT(rigType == RigType::None ||
                         rigType == RigType::T1 ||
                         rigType == RigType::T2);

                const float rigMods[] = { 0.f, 2.f, 2.4f };
                facilityMod = 0.99f * (100.f - (rigMods[static_cast<int>(rigType)] * securityMods[static_cast<int>(securityStatus)])) / 100.f;
            }

            const auto materialModifier = 1.f - materialEfficiency / 100.f;
            return std::max(static_cast<float>(runs), std::ceil(runs * baseQuantity * materialModifier * facilityMod));
        }

        std::chrono::seconds getProductionTime(std::chrono::seconds baseProductionTime,
                                               uint timeEfficiency,
                                               float implantBonus,
                                               float skillModifier,
                                               FacilityType facilityType,
                                               SecurityStatus securityStatus,
                                               Size facilitySize,
                                               RigType rigType)
        {
            Q_ASSERT(facilityType == FacilityType::AssemblyArray ||
                     facilityType == FacilityType::RapidAssemblyArray ||
                     facilityType == FacilityType::Station ||
                     facilityType == FacilityType::ThukkerComponentArray ||
                     facilityType == FacilityType::EngineeringComplex);

            auto facilityMod = 1.f;
            if (facilityType == FacilityType::AssemblyArray ||
                facilityType == FacilityType::RapidAssemblyArray ||
                facilityType == FacilityType::Station ||
                facilityType == FacilityType::ThukkerComponentArray)
            {
                const float mods[] = { 1.f, 0.75f, 0.75f, 0.65f };
                facilityMod = mods[static_cast<int>(facilityType)];
            }
            else
            {
                Q_ASSERT(securityStatus == SecurityStatus::HighSec ||
                         securityStatus == SecurityStatus::LowSec ||
                         securityStatus == SecurityStatus::NullSecWH);
                Q_ASSERT(facilitySize == Size::Medium ||
                         facilitySize == Size::Large ||
                         facilitySize == Size::XLarge);
                Q_ASSERT(rigType == RigType::None ||
                         rigType == RigType::T1 ||
                         rigType == RigType::T2);

                const float sizeMods[] = { 0.85f, 0.8f, 0.75f };
                const float rigMods[] = { 0.f, 20.f, 24.f };

                facilityMod = sizeMods[static_cast<int>(facilitySize)] *
                              (100.f - (rigMods[static_cast<int>(rigType)] * securityMods[static_cast<int>(securityStatus)])) / 100.f;
            }

            return std::chrono::duration_cast<std::chrono::seconds>(
                baseProductionTime * facilityMod * (1.f - implantBonus / 100.f) * skillModifier * (1.f - timeEfficiency / 100.f)
            );
        }
    }
}
