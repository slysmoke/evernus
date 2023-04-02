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

#include <functional>

#include <QWidget>
#include <QTimer>

#include "Character.h"

namespace Evernus
{
    class WarningBarWidget;
    class ButtonWithTimer;

    class CharacterBoundWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        using TimeGetter = std::function<QDateTime (Character::IdType)>;

        CharacterBoundWidget(const TimeGetter &apiTimeGetter,
                             const TimeGetter &updateTimeGetter,
                             const QString &updateAgeSettingsKey,
                             QWidget *parent = nullptr);
        virtual ~CharacterBoundWidget() = default;

    signals:
        void importFromAPI(Character::IdType id);

    public slots:
        void refreshImportTimer();

        void setCharacter(Character::IdType id);

    private slots:
        void requestUpdate();
        void updateTimerTick();

    protected:
        ButtonWithTimer &getAPIImportButton() const noexcept;
        WarningBarWidget &getWarningBarWidget() const noexcept;
        Character::IdType getCharacterId() const noexcept;

    private:
        TimeGetter mAPITimeGetter, mUpdateTimeGetter;
        QString mUpdateAgeSettingsKey;

        QTimer mUpdateTimer;

        ButtonWithTimer *mImportBtn = nullptr;
        WarningBarWidget *mWarningBarWidget = nullptr;

        Character::IdType mCharacterId = Character::invalidId;

        virtual void handleNewCharacter(Character::IdType id) = 0;
    };
}
