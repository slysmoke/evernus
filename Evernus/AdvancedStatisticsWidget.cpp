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
#include <QApplication>
#include <QRadioButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QClipboard>
#include <QTabWidget>
#include <QTableView>
#include <QTextEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>
#include <QSpinBox>
#include <QAction>
#include <QLabel>
#include <QFont>

#include "QtScriptSyntaxHighlighter.h"
#include "OrderScriptRepository.h"
#include "NumberFormatDelegate.h"
#include "CharacterRepository.h"
#include "RepositoryProvider.h"
#include "UISettings.h"

#include "AdvancedStatisticsWidget.h"

namespace Evernus
{
    AdvancedStatisticsWidget::AdvancedStatisticsWidget(const RepositoryProvider &repositoryProvider,
                                                       const EveDataProvider &dataProvider,
                                                       const ItemCostProvider &itemCostProvider,
                                                       QWidget *parent)
        : QWidget(parent)
        , mMarketOrderRepository(repositoryProvider.getMarketOrderRepository())
        , mOrderScriptRepository(repositoryProvider.getOrderScriptRepository())
        , mCharacterRepository(repositoryProvider.getCharacterRepository())
        , mAggrModel(mMarketOrderRepository, dataProvider)
        , mScriptModel(dataProvider, itemCostProvider)
    {
        auto mainLayout = new QVBoxLayout{this};

        mainLayout->addWidget(new QLabel{tr(
            "This tab allows you to create custom reports aggregating historic market order data."), this});

        auto configGroup = new QGroupBox{this};
        mainLayout->addWidget(configGroup);

        auto configLayout = new QVBoxLayout{configGroup};

        auto simpleConfigBtn = new QRadioButton{tr("Simple aggregation"), this};
        configLayout->addWidget(simpleConfigBtn);
        simpleConfigBtn->setChecked(true);

        auto simpleConfigWidget = new QWidget{this};
        configLayout->addWidget(simpleConfigWidget);
        connect(simpleConfigBtn, &QRadioButton::toggled, simpleConfigWidget, &QWidget::setVisible);

        auto simgpleConfigLayout = new QHBoxLayout{simpleConfigWidget};

        simgpleConfigLayout->addWidget(new QLabel{tr("Group by:"), this});

        mAggrGroupingColumnCombo = new QComboBox{this};
        simgpleConfigLayout->addWidget(mAggrGroupingColumnCombo);
        mAggrGroupingColumnCombo->addItem(tr("Type"), static_cast<int>(MarketOrderRepository::AggregateColumn::TypeId));
        mAggrGroupingColumnCombo->addItem(tr("Location"), static_cast<int>(MarketOrderRepository::AggregateColumn::LocationId));

        simgpleConfigLayout->addWidget(new QLabel{tr("Order by:"), this});

        mAggrOrderColumnCombo = new QComboBox{this};
        simgpleConfigLayout->addWidget(mAggrOrderColumnCombo);
        mAggrOrderColumnCombo->addItem(tr("Id"), static_cast<int>(MarketOrderRepository::AggregateOrderColumn::Id));
        mAggrOrderColumnCombo->addItem(tr("Count"), static_cast<int>(MarketOrderRepository::AggregateOrderColumn::Count));
        mAggrOrderColumnCombo->addItem(tr("Price"), static_cast<int>(MarketOrderRepository::AggregateOrderColumn::Price));
        mAggrOrderColumnCombo->addItem(tr("Volume"), static_cast<int>(MarketOrderRepository::AggregateOrderColumn::Volume));

        simgpleConfigLayout->addWidget(new QLabel{tr("Limit:"), this});

        mAggrLimitEdit = new QSpinBox{this};
        simgpleConfigLayout->addWidget(mAggrLimitEdit);
        mAggrLimitEdit->setValue(10);
        mAggrLimitEdit->setMaximum(10000);
        mAggrLimitEdit->setSpecialValueText(tr("none"));

        mAggrIncludeActiveBtn = new QCheckBox{tr("Include active"), this};
        simgpleConfigLayout->addWidget(mAggrIncludeActiveBtn);

        mAggrIncludeNotFulfilledBtn = new QCheckBox{tr("Include expired/cancelled"), this};
        simgpleConfigLayout->addWidget(mAggrIncludeNotFulfilledBtn);

        mAggrApplyBtn = new QPushButton{tr("Apply"), this};
        simgpleConfigLayout->addWidget(mAggrApplyBtn);
        mAggrApplyBtn->setDisabled(true);
        connect(mAggrApplyBtn, &QPushButton::clicked, this, &AdvancedStatisticsWidget::applyAggrFilter);

        simgpleConfigLayout->addStretch();

        auto scriptConfigBtn = new QRadioButton{tr("Script processing"), this};
        configLayout->addWidget(scriptConfigBtn);

        auto scriptConfigWidget = new QWidget{this};
        configLayout->addWidget(scriptConfigWidget);
        scriptConfigWidget->setVisible(false);
        connect(scriptConfigBtn, &QRadioButton::toggled, scriptConfigWidget, &QWidget::setVisible);

        auto scriptConfigLayout = new QHBoxLayout{scriptConfigWidget};

        QFont scriptFont{"Monospace"};
        scriptFont.setFixedPitch(true);
        scriptFont.setStyleHint(QFont::TypeWriter);

        mAggrScriptEdit = new QTextEdit{this};
        scriptConfigLayout->addWidget(mAggrScriptEdit, 1);
        mAggrScriptEdit->setPlaceholderText(tr("see the online help to learn how to use script processing"));
        mAggrScriptEdit->document()->setDefaultFont(scriptFont);

        new QtScriptSyntaxHighlighter{mAggrScriptEdit};

        auto scriptControlsLayout = new QVBoxLayout{};
        scriptConfigLayout->addLayout(scriptControlsLayout);

        mScriptApplyBtn = new QPushButton{tr("Apply"), this};
        scriptControlsLayout->addWidget(mScriptApplyBtn);
        mScriptApplyBtn->setDisabled(true);
        connect(mScriptApplyBtn, &QPushButton::clicked, this, &AdvancedStatisticsWidget::applyScript);

        auto saveScriptBtn = new QPushButton{tr("Save script..."), this};
        scriptControlsLayout->addWidget(saveScriptBtn);
        connect(saveScriptBtn, &QPushButton::clicked, this, &AdvancedStatisticsWidget::saveScript);

        auto loadScriptBtn = new QPushButton{tr("Load script..."), this};
        scriptControlsLayout->addWidget(loadScriptBtn);
        connect(loadScriptBtn, &QPushButton::clicked, this, &AdvancedStatisticsWidget::loadScript);

        auto deleteScriptBtn = new QPushButton{tr("Delete script..."), this};
        scriptControlsLayout->addWidget(deleteScriptBtn);
        connect(deleteScriptBtn, &QPushButton::clicked, this, &AdvancedStatisticsWidget::deleteScript);

        auto scriptModeGroup = new QGroupBox{tr("Mode"), this};
        scriptControlsLayout->addWidget(scriptModeGroup);

        auto scriptModeGroupLayout = new QVBoxLayout{scriptModeGroup};

        mScriptForEachModeBtn = new QRadioButton{tr("For each"), this};
        scriptModeGroupLayout->addWidget(mScriptForEachModeBtn);
        mScriptForEachModeBtn->setChecked(true);

        auto scriptAggrgateModeBtn = new QRadioButton{tr("Aggregate"), this};
        scriptModeGroupLayout->addWidget(scriptAggrgateModeBtn);

        scriptControlsLayout->addStretch();

        auto copyAct = new QAction{this};
        copyAct->setShortcuts(QKeySequence::Copy);
        connect(copyAct, &QAction::triggered, this, &AdvancedStatisticsWidget::copyAggrData);

        mAggrView = new QTableView{this};
        mainLayout->addWidget(mAggrView, 1);
        mAggrView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        mAggrView->addAction(copyAct);
        mAggrView->setItemDelegate(new NumberFormatDelegate{this});
    }

    void AdvancedStatisticsWidget::setCharacter(Character::IdType id)
    {
        mCharacterId = id;

        if (mCharacterId == Character::invalidId)
        {
            mAggrApplyBtn->setDisabled(true);
            mScriptApplyBtn->setDisabled(true);
        }
        else
        {
            mAggrApplyBtn->setDisabled(false);
            mScriptApplyBtn->setDisabled(false);
        }

        mAggrModel.clear();
        mScriptModel.clear();
    }

    void AdvancedStatisticsWidget::applyAggrFilter()
    {
        const auto limit = mAggrLimitEdit->value();
        mAggrModel.reset(mCharacterId,
                         static_cast<MarketOrderRepository::AggregateColumn>(mAggrGroupingColumnCombo->currentData().toInt()),
                         static_cast<MarketOrderRepository::AggregateOrderColumn>(mAggrOrderColumnCombo->currentData().toInt()),
                         (limit == 0) ? (-1) : (limit),
                         mAggrIncludeActiveBtn->isChecked(),
                         mAggrIncludeNotFulfilledBtn->isChecked());

        mAggrView->setModel(&mAggrModel);
    }

    void AdvancedStatisticsWidget::applyScript()
    {
        mScriptModel.reset(mMarketOrderRepository.fetchForCharacter(mCharacterId),
                           mAggrScriptEdit->toPlainText(),
                           (mScriptForEachModeBtn->isChecked()) ?
                           (ScriptOrderProcessingModel::Mode::ForEach) :
                           (ScriptOrderProcessingModel::Mode::Aggregate));
        mAggrView->setModel(&mScriptModel);
    }

    void AdvancedStatisticsWidget::copyAggrData()
    {
        const auto indexes = mAggrView->selectionModel()->selectedIndexes();
        if (indexes.isEmpty())
            return;

        QSettings settings;
        const auto delim
            = settings.value(UISettings::columnDelimiterKey, UISettings::columnDelimiterDefault).value<char>();

        QString result;

        auto prevRow = indexes.first().row();
        for (const auto &index : indexes)
        {
            if (prevRow != index.row())
            {
                prevRow = index.row();
                result.append('\n');
            }

            result.append(mAggrModel.data(index).toString());
            result.append(delim);
        }

        QApplication::clipboard()->setText(result);
    }

    void AdvancedStatisticsWidget::showScriptError(const QString &message)
    {
        QMessageBox::warning(this, tr("Script error"), message);
    }

    void AdvancedStatisticsWidget::saveScript()
    {
        const auto name
            = QInputDialog::getText(this, tr("Save script"), tr("Enter script name:"), QLineEdit::Normal, mLastLoadedScript);
        if (!name.isEmpty())
        {
            mLastLoadedScript = name;

            OrderScript script{name};
            script.setCode(mAggrScriptEdit->toPlainText());

            mOrderScriptRepository.store(script);
        }
    }

    void AdvancedStatisticsWidget::loadScript()
    {
        const auto name
            = QInputDialog::getItem(this, tr("Load script"), tr("Select script:"), mOrderScriptRepository.getAllNames(), 0, false);
        if (!name.isEmpty())
        {
            try
            {
                mAggrScriptEdit->setPlainText(mOrderScriptRepository.find(name)->getCode());
                mLastLoadedScript = name;
            }
            catch (const OrderScriptRepository::NotFoundException &)
            {
            }
        }
    }

    void AdvancedStatisticsWidget::deleteScript()
    {
        const auto name
            = QInputDialog::getItem(this, tr("Delete script"), tr("Select script:"), mOrderScriptRepository.getAllNames(), 0, false);
        if (!name.isEmpty())
            mOrderScriptRepository.remove(name);
    }
}
