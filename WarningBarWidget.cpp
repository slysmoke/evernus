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
#include <QLabel>

#include "WarningBarWidget.h"

namespace Evernus
{
    WarningBarWidget::WarningBarWidget(QWidget *parent)
        : QFrame{parent}
    {
        auto mainLayout = new QHBoxLayout{this};

        auto iconLabel = new QLabel{this};
        mainLayout->addWidget(iconLabel);
        iconLabel->setPixmap(QPixmap{":/images/error.png"});

        mText = new QLabel{this};
        mainLayout->addWidget(mText, 1, Qt::AlignLeft | Qt::AlignVCenter);
        mText->setTextFormat(Qt::RichText);
        mText->setStyleSheet("color: black;");

        auto closeBtn = new QPushButton{QIcon{":/images/cross.png"}, QString{}, this};
        mainLayout->addWidget(closeBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
        connect(closeBtn, &QPushButton::clicked, this, &WarningBarWidget::hide);

        setStyleSheet(
            "QFrame {"
                "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #ff8, stop: 1 #fa4);"
                "border: 1px solid #aaa;"
                "border-radius: 3px;"
            "} "
            "QFrame > * {"
                "background: transparent;"
                "border: none;"
            "}");
    }

    void WarningBarWidget::setText(const QString &text)
    {
        mText->setText(text);
    }
}
