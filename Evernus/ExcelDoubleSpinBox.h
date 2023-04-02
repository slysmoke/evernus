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

#include <QDoubleSpinBox>

namespace Evernus
{
    class ExcelDoubleSpinBox
        : public QDoubleSpinBox
    {
    public:
        using QDoubleSpinBox::QDoubleSpinBox;
        ExcelDoubleSpinBox(const ExcelDoubleSpinBox &) = default;
        ExcelDoubleSpinBox(ExcelDoubleSpinBox &&) = default;
        virtual ~ExcelDoubleSpinBox() = default;

        virtual QValidator::State validate(QString &input, int &pos) const override;

        ExcelDoubleSpinBox &operator =(const ExcelDoubleSpinBox &) = default;
        ExcelDoubleSpinBox &operator =(ExcelDoubleSpinBox &&) = default;
    };
}
