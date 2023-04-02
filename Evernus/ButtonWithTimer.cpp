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
#include "TextUtils.h"

#include "ButtonWithTimer.h"

namespace Evernus
{
    ButtonWithTimer::ButtonWithTimer(const QString &text, QWidget *parent)
        : QPushButton{QIcon{QStringLiteral(":/images/arrow_refresh.png")}, text, parent}
        , mOrigText{text}
    {
        setFlat(true);

        connect(&mTimer, &QTimer::timeout, this, &ButtonWithTimer::tick);
    }

    void ButtonWithTimer::setTimer(const QDateTime &endTime)
    {
        mEndTime = endTime;
        mTimer.start(1000);
        tick();
    }

    void ButtonWithTimer::stopTimer()
    {
        mTimer.stop();
        setText(mOrigText);
    }

    void ButtonWithTimer::tick()
    {
        const auto curTime = QDateTime::currentDateTime();
        if (curTime >= mEndTime)
        {
            stopTimer();
        }
        else
        {
            const auto delta = curTime.secsTo(mEndTime);
            setText(QStringLiteral("%1 (%2)")
                .arg(mOrigText)
                .arg(TextUtils::durationToString(std::chrono::seconds{delta}))
            );
        }
    }
}
