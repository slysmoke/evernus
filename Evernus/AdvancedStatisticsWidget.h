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

#include "ScriptOrderProcessingModel.h"
#include "AggregatedStatisticsModel.h"
#include "Character.h"

class QRadioButton;
class QPushButton;
class QTableView;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QLabel;

namespace Evernus
{
    class OrderScriptRepository;
    class CharacterRepository;
    class RepositoryProvider;
    class ItemCostProvider;

    class AdvancedStatisticsWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        AdvancedStatisticsWidget(const RepositoryProvider &repositoryProvider,
                                 const EveDataProvider &dataProvider,
                                 const ItemCostProvider &itemCostProvider,
                                 QWidget *parent = nullptr);
        virtual ~AdvancedStatisticsWidget() = default;

    public slots:
        void setCharacter(Character::IdType id);

    private slots:
        void applyAggrFilter();
        void applyScript();

        void copyAggrData();

        void showScriptError(const QString &message);

        void saveScript();
        void loadScript();
        void deleteScript();

    private:
        const MarketOrderRepository &mMarketOrderRepository;
        const OrderScriptRepository &mOrderScriptRepository;
        const CharacterRepository &mCharacterRepository;

        QPushButton *mAggrApplyBtn = nullptr;
        QPushButton *mScriptApplyBtn = nullptr;
        QComboBox *mAggrGroupingColumnCombo = nullptr;
        QComboBox *mAggrOrderColumnCombo = nullptr;
        QSpinBox *mAggrLimitEdit = nullptr;
        QCheckBox *mAggrIncludeActiveBtn = nullptr;
        QCheckBox *mAggrIncludeNotFulfilledBtn = nullptr;
        QTextEdit *mAggrScriptEdit = nullptr;
        QRadioButton *mScriptForEachModeBtn = nullptr;
        QTableView *mAggrView = nullptr;

        Character::IdType mCharacterId = Character::invalidId;

        AggregatedStatisticsModel mAggrModel;
        ScriptOrderProcessingModel mScriptModel;

        QString mLastLoadedScript;
    };
}
