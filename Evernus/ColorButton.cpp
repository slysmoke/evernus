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
#include <QColorDialog>

#include "ColorButton.h"

namespace Evernus
{
    ColorButton::ColorButton(const QColor &color, QWidget *parent)
        : QPushButton{parent}
    {
        setObjectName("colorButton");
        setColor(color);
        connect(this, &ColorButton::clicked, this, &ColorButton::selectColor);
    }

    QColor ColorButton::getColor() const
    {
        return mColor;
    }

    void ColorButton::setColor(QColor color)
    {
        mColor = std::move(color);
        setStyleSheet(QString{"#colorButton { background-color : %1; color : %2; }"}.arg(mColor.name()).arg(getIdealTextColor().name()));
        setText(mColor.name());
    }

    void ColorButton::selectColor()
    {
        auto color = QColorDialog::getColor(mColor, this);
        if (color.isValid())
            setColor(std::move(color));
    }

    QColor ColorButton::getIdealTextColor() const
    {
        const auto threshold = 105;
        const auto bckgDelta = mColor.red() * 0.299f + mColor.green() * 0.587f + mColor.blue() * 0.114f;
        return QColor{((255 - bckgDelta) < threshold) ? (Qt::black) : (Qt::white)};
    }
}
