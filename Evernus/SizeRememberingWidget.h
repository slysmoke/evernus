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

#include <QWidget>

namespace Evernus
{
    class SizeRememberingWidget
        : public QWidget
    {
    public:
        explicit SizeRememberingWidget(QString settingsSizeKey, QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
        SizeRememberingWidget(const SizeRememberingWidget &) = default;
        SizeRememberingWidget(SizeRememberingWidget &&) = default;
        virtual ~SizeRememberingWidget() = default;

        SizeRememberingWidget &operator =(const SizeRememberingWidget &) = default;
        SizeRememberingWidget &operator =(SizeRememberingWidget &&) = default;

    protected:
        virtual void resizeEvent(QResizeEvent *event) override;

    private:
        QString mSettingsSizeKey;
    };
}
