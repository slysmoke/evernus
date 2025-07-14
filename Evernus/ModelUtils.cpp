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
#include <QMap>
#include <QVector>
#include <QHeaderView>
#include <algorithm>

#include "UISettings.h"
#include "ModelUtils.h"

namespace Evernus {
    namespace ModelUtils {
        void copyRowsToClipboard(const QModelIndexList& indexes,
            const QAbstractItemModel& model,
            const QHeaderView* header)
        {
            if (indexes.isEmpty())
                return;

            const QString defaultDelim = "\t";

            QSettings settings;
            QString delim = settings.value(UISettings::columnDelimiterDefault, defaultDelim).toString();
            if (delim.isEmpty())
                delim = defaultDelim;

            // Собираем данные по строкам и столбцам.
            QMap<int, QMap<int, QString>> rowMap;
            for (const QModelIndex& index : indexes) {
                if (index.isValid()) {
                    rowMap[index.row()][index.column()] = model.data(index, Qt::DisplayRole).toString();
                }
            }

            QList<int> rows = rowMap.keys();
            QStringList result;
            for (int row : rows) {
                const QMap<int, QString>& columns = rowMap[row];
                QList<int> colNumbers = columns.keys();


                if (header) {
                    std::sort(colNumbers.begin(), colNumbers.end(),
                        [header](int col1, int col2) {
                            return header->visualIndex(col1) < header->visualIndex(col2);
                        });
                }
                else {
                    std::sort(colNumbers.begin(), colNumbers.end());
                }

                QStringList rowData;
                for (int col : colNumbers) {
                    rowData << columns[col];
                }
                result << rowData.join(delim);
            }

            QApplication::clipboard()->setText(result.join("\n"));
        }
    }
}