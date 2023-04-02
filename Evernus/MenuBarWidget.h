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

#include "Character.h"

class QComboBox;

namespace Evernus
{
    class CharacterRepository;

    class MenuBarWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit MenuBarWidget(const CharacterRepository &characterRepository, QWidget *parent = nullptr);
        virtual ~MenuBarWidget() = default;

    public slots:
        void refreshCharacters();
        void setCurrentCharacter(Character::IdType id);

    private slots:
        void changeCharacter(int index);

    signals:
        void currentCharacterChanged(Character::IdType id);

        void importAll();

    private:
        const CharacterRepository &mCharacterRepository;

        QComboBox *mCharacterCombo = nullptr;
    };
}
