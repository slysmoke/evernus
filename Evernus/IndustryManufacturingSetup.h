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

#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <vector>

#include <QtDebug>

#include "EveDataProvider.h"
#include "EveType.h"

class QDataStream;

namespace Evernus
{
    class EveDataProvider;

    class IndustryManufacturingSetup final
    {
    public:
        using TypeSet = std::unordered_set<EveType::IdType>;

        enum class InventorySource
        {
            BuyFromSource,
            Manufacture,
            AcquireForFree,
            BuyAtCustomCost,
            TakeAssetsThenBuyFromSource,
            TakeAssetsThenManufacture,
            TakeAssetsThenBuyAtCustomCost,
        };

        struct TypeSettings
        {
            InventorySource mSource = InventorySource::BuyFromSource;
            uint mMaterialEfficiency = 0;
            uint mTimeEfficiency = 0;
        };

        struct OutputSettings
        {
            uint mRuns = 1;
            uint mMaterialEfficiency = 0;
            uint mTimeEfficiency = 0;
        };

        struct Exception : std::runtime_error
        {
            using std::runtime_error::runtime_error;
            virtual ~Exception() = default;
        };

        struct NotSourceTypeException : Exception
        {
            using Exception::Exception;
            virtual ~NotSourceTypeException() = default;
        };

        struct NotOutputTypeException : Exception
        {
            using Exception::Exception;
            virtual ~NotOutputTypeException() = default;
        };

        struct MissingDataException : Exception
        {
            using Exception::Exception;
            virtual ~MissingDataException() = default;
        };

        using OutputTypeMap = std::unordered_map<EveType::IdType, OutputSettings>;

        explicit IndustryManufacturingSetup(const EveDataProvider &dataProvider);
        IndustryManufacturingSetup(const IndustryManufacturingSetup &) = default;
        IndustryManufacturingSetup(IndustryManufacturingSetup &&) = default;
        ~IndustryManufacturingSetup() = default;

        void setOutputTypes(const TypeSet &types);
        const OutputTypeMap &getOutputTypes() const;

        OutputTypeMap::size_type getOutputSize() const noexcept;

        const EveDataProvider::ManufacturingInfo &getManufacturingInfo(EveType::IdType typeId) const;
        const TypeSettings &getTypeSettings(EveType::IdType typeId) const;
        const OutputSettings &getOutputSettings(EveType::IdType typeId) const;

        TypeSet getAllTypes() const;

        void clear() noexcept;

        void setSource(EveType::IdType id, InventorySource source);
        void setRuns(EveType::IdType id, uint runs);
        void setMaterialEfficiency(EveType::IdType id, uint value);
        void setTimeEfficiency(EveType::IdType id, uint value);

        IndustryManufacturingSetup &operator =(const IndustryManufacturingSetup &) = default;
        IndustryManufacturingSetup &operator =(IndustryManufacturingSetup &&) = default;

    private:
        static const qint32 dataStreamVersion = 1;

        const EveDataProvider &mDataProvider;

        std::unordered_map<EveType::IdType, TypeSettings> mTypeSettings;
        std::unordered_map<EveType::IdType, EveDataProvider::ManufacturingInfo> mManufacturingInfo;

        OutputTypeMap mOutputTypes;

        void fillManufacturingInfo(EveType::IdType typeId, TypeSet &usedTypes);

        friend QDataStream &operator >>(QDataStream &stream, IndustryManufacturingSetup &setup);
        friend QDataStream &operator <<(QDataStream &stream, const IndustryManufacturingSetup &setup);

        friend QDebug operator <<(QDebug debug, const IndustryManufacturingSetup &setup);
    };
}
