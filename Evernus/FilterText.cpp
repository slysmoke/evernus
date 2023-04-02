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
#include "FilterText.h"

namespace Evernus
{
    QString FilterText::getText() const &
    {
        return mText;
    }

    QString &&FilterText::getText() && noexcept
    {
        return std::move(mText);
    }

    void FilterText::setText(const QString &text)
    {
        mText = text;
    }

    void FilterText::setText(QString &&text)
    {
        mText = std::move(text);
    }
}
