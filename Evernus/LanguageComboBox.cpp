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
#include <QCoreApplication>
#include <QLocale>
#include <QDir>

#include "UISettings.h"

#include "LanguageComboBox.h"

namespace Evernus
{
    LanguageComboBox::LanguageComboBox(QWidget *parent)
        : QComboBox{parent}
    {
        const QLocale defaultLocale{"en_US"};

        addItem(QIcon{":/images/flags/en_US.png"}, defaultLocale.nativeLanguageName(), defaultLocale.name());

        const QDir transDir{QCoreApplication::applicationDirPath() + UISettings::translationPath};
        const auto translations = transDir.entryList(QStringList{"lang_*.qm"}, QDir::Files | QDir::Readable, QDir::Name);

        const auto curLocale = locale().name();

        for (const auto &translation : translations)
        {
            auto lang = translation;
            lang.truncate(lang.lastIndexOf('.'));
            lang.remove(0, lang.indexOf('_') + 1);

            addItem(QIcon{QString{":/images/flags/%1.png"}.arg(lang)},
                    QLocale{lang}.nativeLanguageName(),
                    lang);

            if (lang == curLocale)
                setCurrentIndex(count() - 1);
        }
    }
}
