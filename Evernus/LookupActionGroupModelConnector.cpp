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
#include <QSortFilterProxyModel>
#include <QAbstractItemView>

#include "LookupActionGroup.h"
#include "ModelWithTypes.h"

#include "LookupActionGroupModelConnector.h"

namespace Evernus
{
    LookupActionGroupModelConnector::LookupActionGroupModelConnector(ModelWithTypes &model,
                                                                     const QSortFilterProxyModel &proxy,
                                                                     QAbstractItemView &view,
                                                                     QObject *parent)
        : QObject{parent}
        , EveTypeProvider{}
        , mModel{model}
        , mProxy{proxy}
        , mView{view}
    {
        connect(view.selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &LookupActionGroupModelConnector::selectType);

        mLookupGroup = new LookupActionGroup{*this, this};
        mLookupGroup->setEnabled(false);
        view.addActions(mLookupGroup->actions());
    }

    EveType::IdType LookupActionGroupModelConnector::getTypeId() const
    {
        return mModel.getTypeId(mProxy.mapToSource(mView.currentIndex()));
    }

    void LookupActionGroupModelConnector::selectType(const QItemSelection &selected)
    {
        mLookupGroup->setEnabled(!selected.isEmpty());
    }
}
