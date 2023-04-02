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
#include <QResizeEvent>
#include <QSettings>

#include "SizeRememberingWidget.h"

namespace Evernus
{
    SizeRememberingWidget::SizeRememberingWidget(QString settingsSizeKey, QWidget *parent, Qt::WindowFlags flags)
        : QWidget{parent, flags}
        , mSettingsSizeKey{std::move(settingsSizeKey)}
    {
        QSettings settings;

        const auto size = settings.value(mSettingsSizeKey).toSize();
        if (size.isValid())
            resize(size);
    }

    void SizeRememberingWidget::resizeEvent(QResizeEvent *event)
    {
        Q_ASSERT(event != nullptr);

        QSettings settings;
        settings.setValue(mSettingsSizeKey, event->size());
    }
}
