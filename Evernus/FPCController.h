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

#include <QSoundEffect>
#include <QPointer>
#include <QObject>

#include "qxtglobalshortcut.h"

namespace Evernus
{
    class FPCController
        : public QObject
    {
        Q_OBJECT

    public:
        explicit FPCController(QObject *parent = nullptr);
        virtual ~FPCController() = default;

        void handleNewPreferences();

    signals:
        void executeForward();
        void executeBackward();

    public slots:
        void changeExecutor(QObject *executor);

    private slots:
        void triggerForward();
        void triggerBackward();

    private:
        QxtGlobalShortcut mForwardShortcut;
        QxtGlobalShortcut mBackwardShortcut;
        QSoundEffect mCopySound;

        QPointer<QObject> mExecutor;
    };
}
