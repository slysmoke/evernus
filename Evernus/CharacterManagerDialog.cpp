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
#include <unordered_set>
#include <memory>

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSqlRecord>
#include <QGroupBox>
#include <QTreeView>
#include <QLabel>

#include "CharacterRepository.h"
#include "Repository.h"

#include "CharacterManagerDialog.h"

namespace Evernus
{
    CharacterManagerDialog::CharacterManagerDialog(const CharacterRepository &characterRepository,
                                                   QWidget *parent)
        : QDialog{parent}
        , mCharacterRepository{characterRepository}
        , mCharacterModel{mCharacterRepository}
    {
        auto mainLayout = new QVBoxLayout{this};

        auto charGroup = new QGroupBox{tr("Available characters"), this};
        mainLayout->addWidget(charGroup);

        auto groupLayout = new QVBoxLayout{charGroup};

        mCharacterModelProxy.setSourceModel(&mCharacterModel);

        mCharacterView = new QTreeView{this};
        groupLayout->addWidget(mCharacterView);
        mCharacterView->setModel(&mCharacterModelProxy);
        mCharacterView->setSortingEnabled(true);
        connect(mCharacterView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &CharacterManagerDialog::selectCharacter);

        auto btnLayout = new QHBoxLayout{};
        mainLayout->addLayout(btnLayout);

        const auto addBtn = new QPushButton{QIcon{":/images/add.png"}, tr("Add"), this};
        btnLayout->addWidget(addBtn);
        connect(addBtn, &QPushButton::clicked, this, &CharacterManagerDialog::addCharacter);

        mRemoveCharacterBtn = new QPushButton{QIcon{":/images/delete.png"}, tr("Remove"), this};
        btnLayout->addWidget(mRemoveCharacterBtn);
        mRemoveCharacterBtn->setDisabled(true);
        connect(mRemoveCharacterBtn, &QPushButton::clicked, this, &CharacterManagerDialog::removeCharacter);

        auto btnBox = new QDialogButtonBox{QDialogButtonBox::Close, this};
        mainLayout->addWidget(btnBox);
        connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        connect(&mCharacterModel, &CharacterModel::dataChanged,
                this, &CharacterManagerDialog::charactersChanged);

        setWindowTitle(tr("Character Manager"));
    }

    void CharacterManagerDialog::updateCharacters()
    {
        mCharacterModel.reset();
    }

    void CharacterManagerDialog::removeCharacter()
    {
        mRemoveCharacterBtn->setEnabled(false);

        mCharacterModelProxy.removeRow(mCharacterView->currentIndex().row());
        mCharacterModelProxy.invalidate();

        emit charactersChanged();
    }

    void CharacterManagerDialog::selectCharacter(const QItemSelection &selected)
    {
        mRemoveCharacterBtn->setEnabled(!selected.isEmpty());
    }
}
