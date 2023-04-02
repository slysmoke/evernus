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

#include <QSortFilterProxyModel>
#include <QDialog>

#include "CharacterModel.h"

class QItemSelection;
class QPushButton;
class QTreeView;

namespace Evernus
{
    class CharacterRepository;
    class Character;

    class CharacterManagerDialog
        : public QDialog
    {
        Q_OBJECT

    public:
        explicit CharacterManagerDialog(const CharacterRepository &characterRepository,
                                        QWidget *parent = nullptr);
        virtual ~CharacterManagerDialog() = default;

    signals:
        void addCharacter();
        void charactersChanged();

    public slots:
        void updateCharacters();

    private slots:
        void removeCharacter();
        void selectCharacter(const QItemSelection &selected);

    private:
        const CharacterRepository &mCharacterRepository;

        CharacterModel mCharacterModel;
        QSortFilterProxyModel mCharacterModelProxy;

        QTreeView *mCharacterView = nullptr;

        QPushButton *mRemoveCharacterBtn = nullptr;
    };
}
