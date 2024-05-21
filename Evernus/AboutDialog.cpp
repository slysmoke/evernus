// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

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
 *


 */
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>

#include "AboutDialog.h"

namespace Evernus
{
    AboutDialog::AboutDialog(QWidget *parent)
        : QDialog{parent}
    {
        auto mainLayout = new QVBoxLayout{this};

        auto textLayout = new QHBoxLayout{};
        mainLayout->addLayout(textLayout);
        auto icon = parent->windowIcon();

        auto iconLabel = new QLabel{this};
        textLayout->addWidget(iconLabel);
        iconLabel->setPixmap(icon.pixmap(icon.actualSize(QSize{64, 64})));

        const auto link = QStringLiteral("https://github.com/slysmoke/evernus");
        const auto oldForum = QStringLiteral("https://forums-archive.eveonline.com/default.aspx?g=posts&t=362779");
        const auto newForum = QStringLiteral("https://forums.eveonline.com/t/evernus-2-2-release-the-ultimate-market-tool/6031");

        auto aboutLabel = new QLabel{tr(
                                         "<strong>%1</strong><br />%2<br /><br />"
                                         "Current DEV <strong><a href='https://evewho.com/character/2113351129'>Sasha Winston</a></strong><br />"
                                         "Created by <strong><a href='http://evewho.com/pilot/Pete+Butcher'>Pete Butcher</a></strong><br />"
                                         "German translation by <strong><a href='http://evewho.com/pilot/Hel+O%27Ween'>Hel O'Ween</a></strong><br />"
                                         "Thanks to <strong><a href='https://www.twitch.tv/maximilon'>Robert Knox</a></strong> for testing and tips.<br />"
                                         "All donations are welcome :)<br /><br />"
                                         "<hr /><br />"

                                         "Twitter: <a href='http://twitter.com/evernusproject'>@evernusproject</a><br />"
                                         "Github: <a href='%3'>%3</a><br />"
                                         "New forum topic: <a href='%4'>%4</a><br />"
                                         "Old forum topic: <a href='%5'>%5</a>")
                                         .arg(QCoreApplication::applicationName())
                                         .arg(QCoreApplication::applicationVersion())
                                         .arg(link)
                                         .arg(newForum)
                                         .arg(oldForum),
                                     this};
        textLayout->addWidget(aboutLabel);
        aboutLabel->setTextFormat(Qt::RichText);
        aboutLabel->setOpenExternalLinks(true);

        const auto buttons = new QDialogButtonBox{QDialogButtonBox::Close, this};
        mainLayout->addWidget(buttons);
        buttons->setCenterButtons(true);
        connect(buttons, &QDialogButtonBox::rejected, this, &AboutDialog::close);

        setWindowTitle(tr("About"));
    }
}
