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
#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <set>

#include <QAbstractItemModel>

#include "IndustryManufacturingSetup.h"
#include "IndustryCostIndices.h"
#include "CharacterRepository.h"
#include "EveDataProvider.h"
#include "ExternalOrder.h"
#include "IndustryUtils.h"
#include "MarketPrices.h"
#include "Character.h"
#include "PriceType.h"
#include "EveType.h"

namespace Evernus
{
    class ItemCostProvider;
    class AssetProvider;

    class IndustryManufacturingSetupModel
        : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        using RegionList = std::unordered_set<uint>;

        enum
        {
            NameRole = Qt::UserRole,
            TypeIdRole,
            QuantityProducedRole,
            QuantityRequiredRole,
            SourceRole,
            TimeRole,
            RunsRole,
            RunsEditRole,
            MaterialEfficiencyRole,
            MaterialEfficiencyEditRole,
            TimeEfficiencyRole,
            TimeEfficiencyEditRole,
            TotalTimeRole,
            CostRole,
            ProfitRole,
        };

        static const QString totalCostKey;
        static const QString valueKey;

        IndustryManufacturingSetupModel(IndustryManufacturingSetup &setup,
                                        const EveDataProvider &dataProvider,
                                        const AssetProvider &assetProvider,
                                        const ItemCostProvider &costProvider,
                                        const CharacterRepository &characterRepo,
                                        QObject *parent = nullptr);
        IndustryManufacturingSetupModel(const IndustryManufacturingSetupModel &) = default;
        IndustryManufacturingSetupModel(IndustryManufacturingSetupModel &&) = default;
        virtual ~IndustryManufacturingSetupModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual QHash<int, QByteArray> roleNames() const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

        void refreshData();
        void refreshAssets();

        void setSource(EveType::IdType id, IndustryManufacturingSetup::InventorySource source);

        void setCharacter(Character::IdType id);
        void setFacilityType(IndustryUtils::FacilityType type);
        void setSecurityStatus(IndustryUtils::SecurityStatus status);
        void setMaterialRigType(IndustryUtils::RigType type);
        void setTimeRigType(IndustryUtils::RigType type);
        void setFacilitySize(IndustryUtils::Size size);

        void setPriceTypes(PriceType src, PriceType dst);
        void setFacilityTax(double value);
        void setOrders(const std::vector<ExternalOrder> &orders,
                       const RegionList &srcRegions,
                       const RegionList &dstRegions,
                       quint64 srcStation,
                       quint64 dstStation);
        void setMarketPrices(MarketPrices prices);
        void setCostIndices(IndustryCostIndices indices);
        void setManufacturingStation(quint64 stationId);

        void blockInteractions(bool flag) noexcept;

        double getSystemCostIndex() const noexcept;

        void signalMaterialEfficiencyExternallyChanged(EveType::IdType id);
        void signalTimeEfficiencyExternallyChanged(EveType::IdType id);

        IndustryManufacturingSetupModel &operator =(const IndustryManufacturingSetupModel &) = default;
        IndustryManufacturingSetupModel &operator =(IndustryManufacturingSetupModel &&) = default;

    private:
        class TreeItem;

        using TreeItemPtr = std::unique_ptr<TreeItem>;

        class TreeItem final
        {
        public:
            TreeItem(IndustryManufacturingSetupModel &model,
                     const IndustryManufacturingSetup &setup);
            TreeItem(EveType::IdType typeId,
                     IndustryManufacturingSetupModel &model,
                     const IndustryManufacturingSetup &setup);
            ~TreeItem() = default;

            EveType::IdType getTypeId() const noexcept;

            quint64 getEffectiveQuantityRequired() const;
            quint64 getQuantityRequiredForParent() const;

            uint getQuantityRequired() const noexcept;
            void setQuantityRequired(uint value) noexcept;

            uint getQuantityProduced() const noexcept;

            void setAssetQuantity(quint64 value) noexcept;

            uint getEffectiveRuns() const;
            uint getRuns() const noexcept;
            void setRuns(uint value) noexcept;

            std::chrono::seconds getEffectiveTime() const;
            std::chrono::seconds getEffectiveTotalTime() const;

            QVariantMap getCost() const;
            QVariantMap getProfit() const;

            TreeItem *getChild(int row) const;
            TreeItem *getParent() const noexcept;

            int getRow() const noexcept;

            int getChildCount() const noexcept;

            void appendChild(std::unique_ptr<TreeItem> child);
            void clearChildren() noexcept;

            inline auto begin() noexcept
            {
                return std::begin(mChildItems);
            }

            inline auto begin() const noexcept
            {
                return std::begin(mChildItems);
            }

            inline auto end() noexcept
            {
                return std::end(mChildItems);
            }

            inline auto end() const noexcept
            {
                return std::end(mChildItems);
            }

        private:
            IndustryManufacturingSetupModel &mModel;
            const IndustryManufacturingSetup &mSetup;
            TreeItem *mParent = nullptr;
            EveType::IdType mTypeId = EveType::invalidId;
            uint mQuantityRequired = 0;
            quint64 mAssetQuantity = 0;
            uint mRuns = 1;
            Evernus::EveDataProvider::ManufacturingInfo mManufacturingInfo;
            std::vector<TreeItemPtr> mChildItems;

            uint getMaterialEfficiency() const;
            uint getTimeEfficiency() const;

            bool isOutput() const;

            std::chrono::seconds getTimeToManufacture() const;

            double getJobCost() const;
        };

        struct AssetQuantity
        {
            quint64 mInitialQuantity = 0;
            quint64 mCurrentQuantity = 0;
        };

        struct MarketInfo
        {
            double mPrice;
            bool mAllVolumeMoved;
        };

        template<class T>
        using TypeMap = std::unordered_map<EveType::IdType, T>;

        IndustryManufacturingSetup &mSetup;
        const EveDataProvider &mDataProvider;
        const AssetProvider &mAssetProvider;
        const ItemCostProvider &mCostProvider;
        const CharacterRepository &mCharacterRepo;

        TreeItem mRoot{*this, mSetup};

        CharacterRepository::EntityPtr mCharacter;
        std::unordered_map<uint, int> mCharacterManufacturingSkills;

        std::unordered_multimap<EveType::IdType, std::reference_wrapper<TreeItem>> mTypeItemMap;

        TypeMap<AssetQuantity> mAssetQuantities;

        IndustryUtils::FacilityType mFacilityType = IndustryUtils::FacilityType::Station;
        IndustryUtils::SecurityStatus mSecurityStatus = IndustryUtils::SecurityStatus::HighSec;
        IndustryUtils::RigType mMaterialRigType = IndustryUtils::RigType::None;
        IndustryUtils::RigType mTimeRigType = IndustryUtils::RigType::None;
        IndustryUtils::Size mFacilitySize = IndustryUtils::Size::Medium;

        PriceType mSrcPrice = PriceType::Buy;
        PriceType mDstPrice = PriceType::Sell;

        double mFacilityTax = 10.;

        TypeMap<std::multiset<ExternalOrder, ExternalOrder::LowToHigh>> mSrcSellOrders;
        TypeMap<double> mSrcBuyPrices;
        TypeMap<double> mDstSellPrices;
        TypeMap<std::multiset<ExternalOrder, ExternalOrder::HighToLow>> mDstBuyOrders;

        uint mSrcSystemId = 0;

        MarketPrices mMarketPrices;
        IndustryCostIndices mCostIndices;

        bool mBlockInteractions = false;

        void setRuns(EveType::IdType id, uint runs);
        void setMaterialEfficiency(EveType::IdType id, uint value);
        void setTimeEfficiency(EveType::IdType id, uint value);

        void fillChildren(TreeItem &item);

        TreeItemPtr createOutputItem(EveType::IdType typeId,
                                     const IndustryManufacturingSetup::OutputSettings &settings);
        TreeItemPtr createSourceItem(const EveDataProvider::MaterialInfo &materialInfo);

        template<class Iterator>
        void fillAssetList(Iterator begin, Iterator end);

        quint64 takeAssets(EveType::IdType typeId, quint64 max);
        void fillItemAssets();

        void signalQuantityChange(EveType::IdType typeId);
        void signalTimeChange(EveType::IdType typeId);
        void signalRoleChange(const QVector<int> &roles);
        void signalRoleChange(EveType::IdType typeId, const QVector<int> &roles);
        void signalTimeChange();
        void signalParentDependentRolesChange(const TreeItem &item);
        void signalManufacturingRolesChange();
        void signalManufacturingRolesChange(EveType::IdType typeId);
        void signalRoleChange(TreeItem &item, const QVector<int> &roles);
        void roleAndQuantityChange(EveType::IdType typeId, const QVector<int> &roles);

        MarketInfo getSrcPrice(EveType::IdType typeId, quint64 quantity) const;
        double getSrcBuyPrice(EveType::IdType typeId, quint64 quantity) const;
        MarketInfo getSrcSellPrice(EveType::IdType typeId, quint64 quantity) const;
        MarketInfo getDstPrice(EveType::IdType typeId, quint64 quantity) const;
        MarketInfo getDstBuyPrice(EveType::IdType typeId, quint64 quantity) const;
        double getDstSellPrice(EveType::IdType typeId, quint64 quantity) const;

        template<class T>
        MarketInfo getSrcPriceFromOrderList(const T &orders, quint64 quantity) const;
        template<class T>
        MarketInfo getDstPriceFromOrderList(const T &orders, quint64 quantity) const;

        double getJobTax(double jobFee) const noexcept;
    };
}
