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
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTabWidget>
#include <QGroupBox>
#include <QSettings>
#include <QLabel>
#include <QtDebug>
#include <QMenu>

#include "CacheTimerProvider.h"
#include "LMeveDataProvider.h"
#include "ButtonWithTimer.h"
#include "StyledTreeView.h"
#include "LMeveSettings.h"
#include "StationView.h"

#include "LMeveWidget.h"

namespace Evernus
{
    LMeveWidget::LMeveWidget(const CacheTimerProvider &cacheTimerProvider,
                             const EveDataProvider &dataProvider,
                             const LMeveDataProvider &lMeveDataProvider,
                             const ItemCostProvider &costProvider,
                             const CharacterRepository &characterRepository,
                             QWidget *parent)
        : QWidget(parent)
        , mCacheTimerProvider(cacheTimerProvider)
        , mLMeveDataProvider(lMeveDataProvider)
        , mTaskModel(dataProvider, costProvider, characterRepository)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolbarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolbarLayout);

        mSyncBtn = new ButtonWithTimer{tr("Synchronize"), this};
        toolbarLayout->addWidget(mSyncBtn);
        mSyncBtn->setEnabled(false);
        connect(mSyncBtn, &ButtonWithTimer::clicked, this, [this] {
            Q_ASSERT(mCharacterId != Character::invalidId);
            emit syncLMeve(mCharacterId);
        });

        auto infoLabel = new QLabel{tr("Before synchronizing, enter LMeve url and key in the <a href='#'>Preferences</a>."), this};
        toolbarLayout->addWidget(infoLabel);
        connect(infoLabel, &QLabel::linkActivated, this, &LMeveWidget::openPreferences);

        toolbarLayout->addStretch();

        auto tabs = new QTabWidget{this};
        mainLayout->addWidget(tabs, 1);

        tabs->addTab(createTaskTab(dataProvider), tr("Tasks"));

        mTaskProxy.setSortRole(Qt::UserRole);
    }

    void LMeveWidget::setCharacter(Character::IdType id)
    {
        qDebug() << "Switching LMeve to" << id;

        mCharacterId = id;
        mTaskModel.setCharacterId(mCharacterId);
        updateData();
    }

    void LMeveWidget::updateData()
    {
        refreshImportTimer();
        mTaskModel.setTasks(mLMeveDataProvider.getTasks(mCharacterId));
        mTaskView->header()->resizeSections(QHeaderView::ResizeToContents);
    }

    void LMeveWidget::prepareItemImportFromWeb()
    {
        emit importPricesFromWeb(mCharacterId, getImportTarget());
    }

    void LMeveWidget::prepareItemImportFromFile()
    {
        emit importPricesFromFile(mCharacterId, getImportTarget());
    }

    void LMeveWidget::setStationId(quint64 id)
    {
        if (id != 0)
        {
            mImportBtn->setEnabled(true);
            mTaskModel.setStationId(id);

            QSettings settings;
            settings.setValue(LMeveSettings::sellStationKey, mStationView->getSelectedPath());
        }
        else
        {
            mImportBtn->setEnabled(false);
        }
    }

    QWidget *LMeveWidget::createTaskTab(const EveDataProvider &dataProvider)
    {
        auto container = new QWidget{this};
        auto containerLayout = new QHBoxLayout{container};

        mTaskView = new StyledTreeView{QStringLiteral("lmeve-tasks"), this};
        containerLayout->addWidget(mTaskView, 1);
        mTaskView->setModel(&mTaskProxy);
        mTaskView->setRootIsDecorated(false);

        auto stationGroup = new QGroupBox{tr("Sell station"), this};
        containerLayout->addWidget(stationGroup);

        auto stationLayout = new QVBoxLayout{stationGroup};

        mStationView = new StationView{dataProvider, this};
        stationLayout->addWidget(mStationView, 1);
        mStationView->setMaximumWidth(260);
        connect(mStationView, &StationView::stationChanged, this, &LMeveWidget::setStationId);

        auto importMenu = new QMenu{this};

        importMenu->addAction(QIcon{":/images/world.png"}, tr("Import prices from Web"), this, &LMeveWidget::prepareItemImportFromWeb);
        importMenu->addAction(QIcon{":/images/page_refresh.png"}, tr("Import prices from logs"), this, &LMeveWidget::prepareItemImportFromFile);

        mImportBtn = new QPushButton{QIcon{":/images/arrow_refresh_small.png"}, tr("Import prices  "), this};
        stationLayout->addWidget(mImportBtn);
        mImportBtn->setFlat(true);
        mImportBtn->setMenu(importMenu);
        mImportBtn->setEnabled(false);

        mTaskProxy.setSourceModel(&mTaskModel);

        QSettings settings;

        const auto path = settings.value(LMeveSettings::sellStationKey).toList();
        mStationView->selectPath(path);

        return container;
    }

    void LMeveWidget::refreshImportTimer()
    {
        if (mCharacterId == Character::invalidId)
        {
            mSyncBtn->setDisabled(true);
            mSyncBtn->stopTimer();
        }
        else
        {
            mSyncBtn->setEnabled(true);
            mSyncBtn->setTimer(mCacheTimerProvider.getLocalCacheTimer(mCharacterId, TimerType::LMeveTasks));
        }
    }

    TypeLocationPairs LMeveWidget::getImportTarget() const
    {
        TypeLocationPairs result;

        const auto stationId = mTaskModel.getStationId();
        if (stationId != 0)
        {
            const auto &tasks = mTaskModel.getTasks();
            for (const auto &task : tasks)
                result.emplace(std::make_pair(task->getTypeId(), stationId));
        }

        return result;
    }
}
