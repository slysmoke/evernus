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
#pragma once

#include <QPushButton>

namespace Evernus
{
    class ColorButton
        : public QPushButton
    {
        Q_OBJECT

    public:
        explicit ColorButton(const QColor &color, QWidget *parent = nullptr);
        virtual ~ColorButton() = default;

        QColor getColor() const;
        void setColor(QColor color);

    private slots:
        void selectColor();

    private:
        QColor mColor;

        QColor getIdealTextColor() const;
    };
}
