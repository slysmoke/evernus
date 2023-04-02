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
#include "TradeableTypesTreeView.h"

namespace Evernus
{
    TradeableTypesTreeView::TradeableTypesTreeView(const EveTypeRepository &typeRepo,
                                                   const MarketGroupRepository &groupRepo,
                                                   QWidget *parent)
        : QTreeView{parent}
        , mTypeModel{typeRepo, groupRepo}
    {
        mTypeProxy.setSourceModel(&mTypeModel);
        mTypeProxy.sort(0);

        setHeaderHidden(true);
        setModel(&mTypeProxy);

        connect(&mTypeModel, &TradeableTypesTreeModel::dataChanged, this, [=](const auto &, const auto &, const auto &roles) {
            if (!mTypesChangeScheduled && roles.contains(Qt::CheckStateRole))
            {
                mTypesChangeScheduled = true;
                QMetaObject::invokeMethod(this, "emitTypesChanged", Qt::QueuedConnection);
            }
        });
    }

    TradeableTypesTreeView::TypeSet TradeableTypesTreeView::getSelectedTypes() const
    {
        return mTypeModel.getSelectedTypes();
    }

    void TradeableTypesTreeView::selectTypes(const TypeSet &types)
    {
        mTypeModel.selectTypes(types);
    }

    void TradeableTypesTreeView::deselectAll()
    {
        mTypeModel.deselectAll();
    }

    void TradeableTypesTreeView::emitTypesChanged()
    {
        mTypesChangeScheduled = false;
        emit typesChanged();
    }
}
