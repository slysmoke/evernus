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
#include <QApplication>
#include <QClipboard>
#include <QSettings>
#include <QString>

#include "UISettings.h"

#include "ModelUtils.h"

namespace Evernus
{
    namespace ModelUtils
    {
        void copyRowsToClipboard(const QModelIndexList &indexes, const QAbstractItemModel &model)
        {
            if (indexes.isEmpty())
                return;

            QSettings settings;
            const auto delim
                = settings.value(UISettings::columnDelimiterKey, UISettings::columnDelimiterDefault).value<char>();

            QString result;

            auto prevRow = indexes.first().row();
            for (const auto &index : indexes)
            {
                if (prevRow != index.row())
                {
                    prevRow = index.row();
                    result[result.size() - 1] = '\n';
                }

                result.append(model.data(index).toString());
                result.append(delim);
            }

            result.chop(1);
            QApplication::clipboard()->setText(result);
        }
    }
}
