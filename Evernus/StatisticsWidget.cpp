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
#include <QVBoxLayout>
#include <QTabWidget>

#include "AdvancedStatisticsWidget.h"
#include "BasicStatisticsWidget.h"
#include "ReportsWidget.h"

#include "StatisticsWidget.h"

namespace Evernus
{
    StatisticsWidget::StatisticsWidget(const RepositoryProvider &repositoryProvider,
                                       const EveDataProvider &dataProvider,
                                       const ItemCostProvider &itemCostProvider,
                                       QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto tabs = new QTabWidget{this};
        mainLayout->addWidget(tabs);

        mBasicStatsWidget = new BasicStatisticsWidget{repositoryProvider, itemCostProvider, tabs};
        connect(mBasicStatsWidget, &BasicStatisticsWidget::makeSnapshots, this, &StatisticsWidget::makeSnapshots);

        mAdvancedStatisticsWidget = new AdvancedStatisticsWidget{repositoryProvider, dataProvider, itemCostProvider, tabs};

        mReportsWidget = new ReportsWidget{repositoryProvider, dataProvider, this};

        tabs->addTab(mBasicStatsWidget, tr("Basic"));
        tabs->addTab(mAdvancedStatisticsWidget, tr("Advanced"));
        tabs->addTab(mReportsWidget, tr("Reports"));
    }

    void StatisticsWidget::setCharacter(Character::IdType id)
    {
        qDebug() << "Switching statistics to" << id;

        mBasicStatsWidget->setCharacter(id);
        mAdvancedStatisticsWidget->setCharacter(id);
        mReportsWidget->setCharacter(id);
    }

    void StatisticsWidget::updateBalanceData()
    {
        mBasicStatsWidget->updateBalanceData();
    }

    void StatisticsWidget::updateJournalData()
    {
        mBasicStatsWidget->updateJournalData();
    }

    void StatisticsWidget::updateTransactionData()
    {
        mBasicStatsWidget->updateTransactionData();
    }

    void StatisticsWidget::updateData()
    {
        mBasicStatsWidget->updateData();
    }

    void StatisticsWidget::handleNewPreferences()
    {
        mBasicStatsWidget->handleNewPreferences();
        mReportsWidget->handleNewPreferences();
    }
}
