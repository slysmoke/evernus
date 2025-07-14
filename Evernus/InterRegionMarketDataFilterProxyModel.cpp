#include "InterRegionMarketDataFilterProxyModel.h"

namespace Evernus
{

InterRegionMarketDataFilterProxyModel::InterRegionMarketDataFilterProxyModel(
    int srcRegionColumn,
    int dstRegionColumn,
    int volumeColumn,
    int marginColumn,
    int srcBuyoutColumn,
    int dstBuyoutColumn,
    QObject *parent)
    : QSortFilterProxyModel{parent}
    , mSrcRegionColumn{srcRegionColumn}
    , mDstRegionColumn{dstRegionColumn}
    , mVolumeColumn{volumeColumn}
    , mMarginColumn{marginColumn}
    , mSrcBuyoutColumn{srcBuyoutColumn}
    , mDstBuyoutColumn{dstBuyoutColumn}
{
}

void InterRegionMarketDataFilterProxyModel::setFilter(
    RegionList srcRegions,
    RegionList dstRegions,
    VolumeValueType minVolume,
    VolumeValueType maxVolume,
    MarginValueType minMargin,
    MarginValueType maxMargin,
    PriceValueType srcBuyoutMin,
    PriceValueType srcBuyoutMax,
    PriceValueType dstBuyoutMin,
    PriceValueType dstBuyoutMax)
{
    mSrcRegions = std::move(srcRegions);
    mDstRegions = std::move(dstRegions);
    mMinVolume = std::move(minVolume);
    mMaxVolume = std::move(maxVolume);
    mMinMargin = std::move(minMargin);
    mMaxMargin = std::move(maxMargin);
    mSrcBuyoutMin = std::move(srcBuyoutMin);
    mSrcBuyoutMax = std::move(srcBuyoutMax);
    mDstBuyoutMin = std::move(dstBuyoutMin);
    mDstBuyoutMax = std::move(dstBuyoutMax);

    invalidateFilter();
}

bool InterRegionMarketDataFilterProxyModel::filterAcceptsRow(
    int sourceRow, const QModelIndex &sourceParent) const
{
    if (!QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
        return false;

    const auto source = sourceModel();
    if (Q_LIKELY(source != nullptr))
    {
        auto checkRegion = [=, &sourceParent](int column, const RegionList &regions) {
            const auto value
                = source->data(source->index(sourceRow, column, sourceParent), Qt::UserRole + 1).toUInt();
            return regions.find(value) != std::end(regions);
        };

        if (!checkRegion(mSrcRegionColumn, mSrcRegions) || !checkRegion(mDstRegionColumn, mDstRegions))
            return false;

        auto checkLimit = [source, sourceRow, &sourceParent](auto column, auto min, auto max) {
            if (min || max)
            {
                using Type = std::remove_reference_t<decltype(*min)>;
                const auto value
                    = source->data(source->index(sourceRow, column, sourceParent), Qt::UserRole).template value<Type>();
                if (min && value < *min)
                    return false;
                if (max && value > *max)
                    return false;
            }
            return true;
        };

        if (!checkLimit(mVolumeColumn, mMinVolume, mMaxVolume))
            return false;
        if (!checkLimit(mMarginColumn, mMinMargin, mMaxMargin))
            return false;
        if (!checkLimit(mSrcBuyoutColumn, mSrcBuyoutMin, mSrcBuyoutMax))
            return false;
        if (!checkLimit(mDstBuyoutColumn, mDstBuyoutMin, mDstBuyoutMax))
            return false;
    }

    return true;
}

} // namespace Evernus
