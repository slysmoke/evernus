// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include <QMessageBox>

#include "MarketAnalysisUtils.h"

namespace Evernus
{
    namespace MarketAnalysisUtils
    {
        void showMissingHistoryMessage(QWidget *parent)
        {
            QMessageBox::information(
                parent,
                QT_TRANSLATE_NOOP("MarketAnalysisUtils", "Missing history"),
                QT_TRANSLATE_NOOP("MarketAnalysisUtils", "Some item types are missing their history. Probably Eve servers have not returned one.")
            );
        }
    }
}
