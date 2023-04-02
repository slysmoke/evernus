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
#include <QContextMenuEvent>
#include <QMenu>

#include "CheckableTreeView.h"

namespace Evernus
{
    void CheckableTreeView::contextMenuEvent(QContextMenuEvent *event)
    {
        Q_ASSERT(event != nullptr);

        const auto index = indexAt(event->pos());
        if (index.isValid())
        {
            const auto curModel = model();
            Q_ASSERT(curModel != nullptr);

            QMenu menu;
            menu.addAction(tr("Check"), this, [=] {
                curModel->setData(index, Qt::Checked, Qt::CheckStateRole);
            });
            menu.addAction(tr("Uncheck"), this, [=] {
                curModel->setData(index, Qt::Unchecked, Qt::CheckStateRole);
            });
            menu.addActions(actions());
            menu.exec(event->globalPos());

            event->accept();
        }
    }
}
