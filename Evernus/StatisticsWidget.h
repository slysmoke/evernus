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

namespace Evernus
{
    class AdvancedStatisticsWidget;
    class OrderScriptRepository;
    class BasicStatisticsWidget;
    class CharacterRepository;
    class RepositoryProvider;
    class ItemCostProvider;
    class ReportsWidget;

    class StatisticsWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        StatisticsWidget(const RepositoryProvider &repositoryProvider,
                         const EveDataProvider &dataProvider,
                         const ItemCostProvider &itemCostProvider,
                         QWidget *parent = nullptr);
        virtual ~StatisticsWidget() = default;

    signals:
        void makeSnapshots();

    public slots:
        void setCharacter(Character::IdType id);
        void updateBalanceData();
        void updateJournalData();
        void updateTransactionData();
        void updateData();
        void handleNewPreferences();

    private:
        BasicStatisticsWidget *mBasicStatsWidget = nullptr;
        AdvancedStatisticsWidget *mAdvancedStatisticsWidget = nullptr;
        ReportsWidget *mReportsWidget = nullptr;
    };
}
