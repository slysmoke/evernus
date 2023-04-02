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
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSettings>

#include "CharacterRepository.h"
#include "UISettings.h"

#include "MenuBarWidget.h"

namespace Evernus
{
    MenuBarWidget::MenuBarWidget(const CharacterRepository &characterRepository, QWidget *parent)
        : QWidget{parent}
        , mCharacterRepository{characterRepository}
    {
        auto mainLayout = new QHBoxLayout{this};
        mainLayout->setContentsMargins(QMargins{});

        auto importBtn = new QPushButton{QIcon{":/images/arrow_refresh.png"}, tr("Import all"), this};
        mainLayout->addWidget(importBtn);
        importBtn->setFlat(true);
        connect(importBtn, &QPushButton::clicked, this, &MenuBarWidget::importAll);

        mCharacterCombo = new QComboBox{this};
        mainLayout->addWidget(mCharacterCombo);
        mCharacterCombo->setMinimumWidth(180);
        mCharacterCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        connect(mCharacterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &MenuBarWidget::changeCharacter,
                Qt::QueuedConnection);

        refreshCharacters();
    }

    void MenuBarWidget::setCurrentCharacter(Character::IdType id)
    {
        const auto index = mCharacterCombo->findData(id);
        mCharacterCombo->setCurrentIndex(index);
    }

    void MenuBarWidget::refreshCharacters()
    {
        const auto index = mCharacterCombo->currentIndex();
        auto prevId = Character::invalidId;
        auto emitPrevious = false;

        if (index == -1)
        {
            QSettings settings;
            const auto lastCharacter = settings.value(UISettings::lastCharacterKey);
            if (lastCharacter.isValid())
                prevId = lastCharacter.value<Character::IdType>();
        }
        else
        {
            prevId = mCharacterCombo->currentData().value<Character::IdType>();
        }

        mCharacterCombo->blockSignals(true);
        mCharacterCombo->clear();

        auto characters = mCharacterRepository.getEnabledQuery();

        while (characters.next())
        {
            const auto id = characters.value("id").value<Character::IdType>();

            mCharacterCombo->addItem(characters.value("name").toString(), id);
            if (id == prevId)
            {
                emitPrevious = true;
                mCharacterCombo->setCurrentIndex(mCharacterCombo->count() - 1);
            }
        }

        mCharacterCombo->blockSignals(false);

        if (!emitPrevious || index == -1)
            QMetaObject::invokeMethod(this, "changeCharacter", Qt::QueuedConnection, Q_ARG(int, mCharacterCombo->currentIndex()));
    }

    void MenuBarWidget::changeCharacter(int index)
    {
        const auto id = (index == -1) ? (Character::invalidId) : (mCharacterCombo->currentData().value<Character::IdType>());

        QSettings settings;
        settings.setValue(UISettings::lastCharacterKey, id);

        emit currentCharacterChanged(id);
    }
}
