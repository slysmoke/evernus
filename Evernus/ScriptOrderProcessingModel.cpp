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
#include <QJSEngine>

#include "ItemCostProvider.h"
#include "CommonScriptAPI.h"
#include "EveDataProvider.h"
#include "ScriptUtils.h"

#include "ScriptOrderProcessingModel.h"

namespace Evernus
{
    ScriptOrderProcessingModel::ScriptOrderProcessingModel(const EveDataProvider &dataProvider,
                                                           const ItemCostProvider &itemCostProvider,
                                                           QObject *parent)
        : QAbstractTableModel{parent}
        , mDataProvider{dataProvider}
        , mItemCostProvider{itemCostProvider}
    {
    }

    int ScriptOrderProcessingModel::columnCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (mMaxColumns);
    }

    QVariant ScriptOrderProcessingModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant{};

        if (role == Qt::DisplayRole)
            return mData[index.row()].at(index.column());

        return QVariant{};
    }

    int ScriptOrderProcessingModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    void ScriptOrderProcessingModel::clear()
    {
        beginResetModel();

        mMaxColumns = 0;
        mData.clear();

        endResetModel();
    }

    void ScriptOrderProcessingModel::reset(const MarketOrderRepository::EntityList &orders, const QString &script, Mode mode)
    {
        beginResetModel();

        mMaxColumns = 0;
        mData.clear();

        QJSEngine engine;
        CommonScriptAPI::insertAPI(engine, mDataProvider);

        if (mode == Mode::ForEach)
        {
            auto processFunction = engine.evaluate("(function process(order) {\n" + script + "\n})");
            if (processFunction.isError())
            {
                endResetModel();
                emit error(processFunction.toString());
                return;
            }

            for (const auto &order : orders)
            {
                const auto value
                    = processFunction.call(QJSValueList{} << ScriptUtils::wrapMarketOrder(engine, *order, mItemCostProvider.fetchForCharacterAndType(order->getCharacterId(), order->getTypeId())));
                if (value.isError())
                {
                    endResetModel();
                    emit error(value.toString());
                    return;
                }

                auto array = value.toVariant().toList();
                const auto size = array.size();
                if (size > mMaxColumns)
                    mMaxColumns = size;

                mData.emplace_back(std::move(array));
            }
        }
        else
        {
            auto processFunction = engine.evaluate("(function process(orders) {\n" + script + "\n})");
            if (processFunction.isError())
            {
                endResetModel();
                emit error(processFunction.toString());
                return;
            }

            auto arguments = engine.newArray(static_cast<uint>(orders.size()));
            for (auto i = 0u; i < orders.size(); ++i)
                arguments.setProperty(i, ScriptUtils::wrapMarketOrder(engine, *orders[i], mItemCostProvider.fetchForCharacterAndType(orders[i]->getCharacterId(), orders[i]->getTypeId())));

            const auto value = processFunction.call(QJSValueList{} << arguments);
            if (value.isError())
            {
                endResetModel();
                emit error(value.toString());
                return;
            }

            auto array = value.toVariant().toList();

            mMaxColumns = array.size();
            mData.emplace_back(std::move(array));
        }

        endResetModel();
    }
}
