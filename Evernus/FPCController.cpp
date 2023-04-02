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
#include <QSettings>
#include <QtDebug>
#include <QUrl>

#include "PriceSettings.h"
#include "SoundSettings.h"

#include "FPCController.h"

namespace Evernus
{
    FPCController::FPCController(QObject *parent)
        : QObject{parent}
    {
        mCopySound.setSource(QUrl{"qrc:/sounds/click.wav"});

        handleNewPreferences();

        connect(&mForwardShortcut, &QxtGlobalShortcut::activated, this, &FPCController::triggerForward);
        connect(&mBackwardShortcut, &QxtGlobalShortcut::activated, this, &FPCController::triggerBackward);
    }

    void FPCController::handleNewPreferences()
    {
        QSettings settings;

        mForwardShortcut.unsetShortcut();
        mBackwardShortcut.unsetShortcut();
        if (settings.value(PriceSettings::fpcKey, PriceSettings::fpcDefault).toBool())
        {
            auto shortcut = QKeySequence::fromString(settings.value(PriceSettings::fpcForwardShortcutKey).toString());
            if (!shortcut.isEmpty())
                mForwardShortcut.setShortcut(shortcut);

            shortcut = QKeySequence::fromString(settings.value(PriceSettings::fpcBackwardShortcutKey).toString());
            if (!shortcut.isEmpty())
                mBackwardShortcut.setShortcut(shortcut);
        }

        mCopySound.setMuted(!settings.value(SoundSettings::fpcSoundKey, SoundSettings::fpcSoundDefault).toBool());
    }

    void FPCController::changeExecutor(QObject *executor)
    {
        if (executor == mExecutor)
            return;

        qDebug() << "Changing FPC executor to:" << executor;

        if (!mExecutor.isNull())
        {
            disconnect(mExecutor, SLOT(executeFPC()));
            disconnect(mExecutor, SLOT(executeBackwardFPC()));
        }

        mExecutor = executor;

        if (!mExecutor.isNull())
        {
            connect(this, SIGNAL(executeForward()), mExecutor, SLOT(executeFPC()));
            connect(this, SIGNAL(executeBackward()), mExecutor, SLOT(executeBackwardFPC()));
        }
    }

    void FPCController::triggerForward()
    {
        if (mExecutor != nullptr)
        {
            qDebug() << "FPC triggered.";
            emit executeForward();
            mCopySound.play();
        }
    }

    void FPCController::triggerBackward()
    {
        if (mExecutor != nullptr)
        {
            qDebug() << "FPC triggered.";
            emit executeBackward();
            mCopySound.play();
        }
    }
}
