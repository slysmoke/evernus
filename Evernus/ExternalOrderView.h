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

#include <QWidget>

#include "ExternalOrderFilterProxyModel.h"
#include "MathUtils.h"
#include "Character.h"
#include "EveType.h"

class QLabel;

namespace Evernus
{
    class ExternalOrderModel;
    class ItemCostProvider;
    class StyledTreeView;

    class ExternalOrderView
        : public QWidget
    {
        Q_OBJECT

    public:
        ExternalOrderView(const ItemCostProvider &costProvider,
                          const EveDataProvider &dataProvider,
                          const QString &objectName,
                          QWidget *parent = nullptr);
        virtual ~ExternalOrderView() = default;

        void setModel(ExternalOrderModel *model);
        void setCharacterId(Character::IdType id);
        void setTypeId(EveType::IdType id);

        void setFilter(double minPrice, double maxPrice, uint minVolume, uint maxVolume, ExternalOrderFilterProxyModel::SecurityStatuses security);

        void sortByPrice();

        void addTreeViewAction(QAction *action);

        QModelIndex currentIndex() const;

    private slots:
        void handleModelReset();
        void copySuggestedPrice() const;

        void selectOrder();

    private:
        const ItemCostProvider &mCostProvider;
        const EveDataProvider &mDataProvider;

        Character::IdType mCharacterId = Character::invalidId;
        EveType::IdType mTypeId = EveType::invalidId;

        StyledTreeView *mView = nullptr;
        QLabel *mTotalPriceLabel = nullptr;
        QLabel *mTotalVolumeLabel = nullptr;
        QLabel *mTotalSizeLabel = nullptr;
        QLabel *mMinPriceLabel = nullptr;
        QLabel *mMedianPriceLabel = nullptr;
        QLabel *mMaxPriceLabel = nullptr;
        QLabel *mItemCostLabel = nullptr;

        ExternalOrderModel *mSource = nullptr;
        ExternalOrderFilterProxyModel mProxy;

        QModelIndex mCurrentOrder;

        QAction *mCopySuggestedPriceAct = nullptr;

        void setCustomCost();
        void setDefaultTotals();

        double getSuggestedPrice() const;

        void setAggregatedTotals(const MathUtils::AggregateData &data);

        static QString getDefaultCopySuggestedPriceText();
    };
}
