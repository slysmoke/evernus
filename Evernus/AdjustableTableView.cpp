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
#include <QHeaderView>
#include <QSettings>

#include "UISettings.h"

#include "AdjustableTableView.h"

namespace Evernus
{
    AdjustableTableView::AdjustableTableView(const QString &name, QWidget *parent)
        : QTableView{parent}
    {
        setObjectName(name);
        horizontalHeader()->setSectionsMovable(true);

        connect(horizontalHeader(), &QHeaderView::sectionMoved, this, &AdjustableTableView::saveHeaderState);
        connect(horizontalHeader(), &QHeaderView::sectionResized, this, &AdjustableTableView::saveHeaderState);
    }

    void AdjustableTableView::saveHeaderState()
    {
        const auto name = objectName();
        if (!name.isEmpty())
        {
            QSettings settings;
            settings.setValue(UISettings::headerStateKey.arg(name), horizontalHeader()->saveState());
        }
    }

    void AdjustableTableView::restoreHeaderState()
    {
        const auto name = objectName();
        if (!name.isEmpty())
        {
            QSettings settings;

            const auto state = settings.value(UISettings::headerStateKey.arg(name)).toByteArray();
            if (!state.isEmpty())
                horizontalHeader()->restoreState(state);
        }
    }
}
