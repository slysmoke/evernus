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
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "LanguageComboBox.h"

#include "LanguageSelectDialog.h"

namespace Evernus
{
    LanguageSelectDialog::LanguageSelectDialog(QWidget *parent)
        : QDialog(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto languageLayout = new QHBoxLayout{};
        mainLayout->addLayout(languageLayout);

        languageLayout->addWidget(new QLabel{tr("Language:"), this});

        mLanguageCombo = new LanguageComboBox{this};
        languageLayout->addWidget(mLanguageCombo);

        auto buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
        mainLayout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &LanguageSelectDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &LanguageSelectDialog::reject);

        setWindowTitle(tr("Select Language"));
    }

    QString LanguageSelectDialog::getSelectedLanguage() const
    {
        return mLanguageCombo->currentData().toString();
    }
}
