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
#include "LeafFilterProxyModel.h"

namespace Evernus
{
    bool LeafFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (filterAcceptsRowItself(sourceRow, sourceParent))
            return true;

        auto parent = sourceParent;
        while (parent.isValid())
        {
            if (filterAcceptsRowItself(parent.row(), parent.parent()))
                return true;

            parent = parent.parent();
        }

        return hasAcceptedChildren(sourceRow, sourceParent);
    }

    bool LeafFilterProxyModel::hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const
    {
        const auto item = sourceModel()->index(sourceRow, 0, sourceParent);
        if (!item.isValid() || !item.model()->hasChildren(item))
            return false;

        const auto childCount = item.model()->rowCount(item);
        for (auto i = 0; i < childCount; ++i)
        {
            if (filterAcceptsRowItself(i, item))
                return true;

            if (hasAcceptedChildren(i, item))
                return true;
        }

        return false;
    }

    bool LeafFilterProxyModel::filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const
    {
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }
}
