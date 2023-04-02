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
#include <QPainter>
#include <QPalette>

#include "StyledTreeViewItemDelegate.h"

namespace Evernus
{
    void StyledTreeViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyledItemDelegate::paint(painter, option, index);

        painter->save();

        if (option.state & QStyle::State_HasFocus)
        {
            painter->setPen(QPen{QPalette{}.color(QPalette::Text), 0., Qt::DashLine});
            painter->drawRect(option.rect.adjusted(0, 0, -1, -1));
        }
        else
        {
            painter->setPen(QPen{Qt::lightGray, 0., Qt::DotLine});
            painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
        }

        painter->restore();
    }
}
