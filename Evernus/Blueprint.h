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
    class Blueprint
    {
    public:
        enum class Location
        {
            AutoFit,
            Cargo,
            CorpseBay,
            DroneBay,
            FleetHangar,
            Deliveries,
            HiddenModifiers,
            Hangar,
            HangarAll,
            LoSlot0,
            LoSlot1,
            LoSlot2,
            LoSlot3,
            LoSlot4,
            LoSlot5,
            LoSlot6,
            LoSlot7,
            MedSlot0,
            MedSlot1,
            MedSlot2,
            MedSlot3,
            MedSlot4,
            MedSlot5,
            MedSlot6,
            MedSlot7,
            HiSlot0,
            HiSlot1,
            HiSlot2,
            HiSlot3,
            HiSlot4,
            HiSlot5,
            HiSlot6,
            HiSlot7,
            AssetSafety,
            Locked,
            Unlocked,
            Implant,
            QuafeBay,
            RigSlot0,
            RigSlot1,
            RigSlot2,
            RigSlot3,
            RigSlot4,
            RigSlot5,
            RigSlot6,
            RigSlot7,
            ShipHangar,
            SpecializedFuelBay,
            SpecializedOreHold,
            SpecializedGasHold,
            SpecializedMineralHold,
            SpecializedSalvageHold,
            SpecializedShipHold,
            SpecializedSmallShipHold,
            SpecializedMediumShipHold,
            SpecializedLargeShipHold,
            SpecializedIndustrialShipHold,
            SpecializedAmmoHold,
            SpecializedCommandCenterHold,
            SpecializedPlanetaryCommoditiesHold,
            SpecializedMaterialBay,
            SubSystemSlot0,
            SubSystemSlot1,
            SubSystemSlot2,
            SubSystemSlot3,
            SubSystemSlot4,
            SubSystemSlot5,
            SubSystemSlot6,
            SubSystemSlot7,
            FighterBay,
            FighterTube0,
            FighterTube1,
            FighterTube2,
            FighterTube3,
            FighterTube4,
            Module,
			FrigateEscapeBay
        };

        static const int magicBPCQuantity = -2;

        Blueprint() = default;
        Blueprint(const Blueprint &) = default;
        Blueprint(Blueprint &&) = default;
        virtual ~Blueprint() = default;

        quint64 getId() const noexcept;
        void setId(quint64 value) noexcept;

        Location getLocation() const noexcept;
        void setLocation(Location location) noexcept;

        quint64 getLocationId() const noexcept;
        void setLocationId(quint64 value) noexcept;

        uint getMaterialEfficiency() const noexcept;
        void setMaterialEfficiency(uint value) noexcept;

        uint getTimeEfficiency() const noexcept;
        void setTimeEfficiency(uint value) noexcept;

        int getQuantity() const noexcept;
        void setQuantity(int value) noexcept;

        int getRuns() const noexcept;
        void setRuns(int value) noexcept;

        EveType::IdType getTypeId() const noexcept;
        void setTypeId(EveType::IdType id) noexcept;

        bool isBPC() const noexcept;

        Blueprint &operator =(const Blueprint &) = default;
        Blueprint &operator =(Blueprint &&) = default;

    private:
        quint64 mId = 0;
        Location mLocation = Location::Hangar;
        quint64 mLocationId = 0;
        uint mMaterialEfficiency = 0;
        uint mTimeEfficiency = 0;
        int mQuantity = 0;
        int mRuns = 0;
        EveType::IdType mTypeId = EveType::invalidId;
    };
}
