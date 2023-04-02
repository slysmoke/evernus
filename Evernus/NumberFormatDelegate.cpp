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
#include "NumberFormatDelegate.h"

namespace Evernus
{
    QString NumberFormatDelegate::displayText(const QVariant &value, const QLocale &locale) const
    {
        const auto type = static_cast<QMetaType::Type>(value.type());
        if (type == QMetaType::Double || type == QMetaType::Float)
            return locale.toString(value.toReal(), 'f', 2);

        return QStyledItemDelegate::displayText(value, locale);
    }
}
