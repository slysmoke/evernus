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

#include <QSortFilterProxyModel>
#include <QTreeView>

#include "TradeableTypesTreeModel.h"

namespace Evernus
{
    class MarketGroupRepository;
    class EveTypeRepository;

    class TradeableTypesTreeView
        : public QTreeView
    {
        Q_OBJECT

    public:
        using TypeSet = TradeableTypesTreeModel::TypeSet;

        TradeableTypesTreeView(const EveTypeRepository &typeRepo,
                               const MarketGroupRepository &groupRepo,
                               QWidget *parent = nullptr);
        TradeableTypesTreeView(const TradeableTypesTreeView &) = default;
        TradeableTypesTreeView(TradeableTypesTreeView &&) = default;
        virtual ~TradeableTypesTreeView() = default;

        TypeSet getSelectedTypes() const;
        void selectTypes(const TypeSet &types);
        void deselectAll();

        TradeableTypesTreeView &operator =(const TradeableTypesTreeView &) = default;
        TradeableTypesTreeView &operator =(TradeableTypesTreeView &&) = default;

    signals:
        void typesChanged();

    private slots:
        void emitTypesChanged();

    private:
        TradeableTypesTreeModel mTypeModel;
        QSortFilterProxyModel mTypeProxy;

        bool mTypesChangeScheduled = false;
    };
}
