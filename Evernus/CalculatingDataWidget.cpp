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
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLabel>

#include "CalculatingDataWidget.h"

namespace Evernus
{
    CalculatingDataWidget::CalculatingDataWidget(QWidget *parent)
        : QWidget(parent)
    {
        const auto waitingLayout = new QVBoxLayout{this};
        waitingLayout->setAlignment(Qt::AlignCenter);

        const auto waitingLabel = new QLabel{tr("Calculating data..."), this};
        waitingLayout->addWidget(waitingLabel);
        waitingLabel->setAlignment(Qt::AlignCenter);

        const auto waitingProgress = new QProgressBar{this};
        waitingLayout->addWidget(waitingProgress);
        waitingProgress->setRange(0, 0);
    }
}
