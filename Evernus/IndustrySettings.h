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

#include <QString>

#include "IndustryUtils.h"

namespace Evernus
{
    namespace IndustrySettings
    {
        const auto manufacturingFacilityTypeDefault = static_cast<int>(IndustryUtils::FacilityType::Station);
        const auto manufacturingSecurityStatusDefault = static_cast<int>(IndustryUtils::SecurityStatus::HighSec);
        const auto manufacturingMaterialRigDefault = static_cast<int>(IndustryUtils::RigType::None);
        const auto manufacturingTimeRigDefault = static_cast<int>(IndustryUtils::RigType::None);
        const auto manufacturingFacilitySizeDefault = static_cast<int>(IndustryUtils::Size::Medium);
        const auto manufacturingFacilityTaxDefault = 10.;
        const auto dontSaveLargeOrdersDefault = true;
        const auto miningLedgerImportForMiningRegionsDefault = true;

        const auto srcManufacturingRegionKey = QStringLiteral("industry/manufacturing/srcRegion");
        const auto dstManufacturingRegionKey = QStringLiteral("industry/manufacturing/dstRegion");
        const auto srcManufacturingStationKey = QStringLiteral("industry/manufacturing/srcStation");
        const auto dstManufacturingStationKey = QStringLiteral("industry/manufacturing/dstStation");
        const auto facilityManufacturingStationKey = QStringLiteral("industry/manufacturing/facilityStation");
        const auto manufacturingTypesKey = QStringLiteral("industry/manufacturing/types");
        const auto manufacturingFacilityTypeKey = QStringLiteral("industry/manufacturing/facilityType");
        const auto manufacturingSecurityStatusKey = QStringLiteral("industry/manufacturing/securityStatus");
        const auto manufacturingMaterialRigKey = QStringLiteral("industry/manufacturing/materialRig");
        const auto manufacturingTimeRigKey = QStringLiteral("industry/manufacturing/timeRig");
        const auto manufacturingFacilitySizeKey = QStringLiteral("industry/manufacturing/facilitySize");
        const auto manufacturingFacilityTaxKey = QStringLiteral("industry/manufacturing/facilityTax");
        const auto dontSaveLargeOrdersKey = QStringLiteral("industry/dontSaveOrders");
        const auto miningLedgerImportRegionsKey = QStringLiteral("industry/miningLedger/importRegions");
        const auto miningLedgerSellStationKey = QStringLiteral("industry/miningLedger/sellStation");
        const auto miningLedgerImportForMiningRegionsKey = QStringLiteral("industry/miningLedger/importForMiningRegions");
    }
}
