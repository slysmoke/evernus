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

#include <QAbstractTableModel>

#include "MarketOrderRepository.h"

namespace Evernus
{
    class ItemCostProvider;
    class EveDataProvider;

    class ScriptOrderProcessingModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        enum class Mode
        {
            ForEach,
            Aggregate
        };

        ScriptOrderProcessingModel(const EveDataProvider &dataProvider,
                                   const ItemCostProvider &itemCostProvider,
                                   QObject *parent = nullptr);
        virtual ~ScriptOrderProcessingModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void clear();
        void reset(const MarketOrderRepository::EntityList &orders, const QString &script, Mode mode);

    signals:
        void error(const QString &message);

    private:
        const EveDataProvider &mDataProvider;
        const ItemCostProvider &mItemCostProvider;

        std::vector<QVariantList> mData;
        int mMaxColumns = 0;
    };
}
