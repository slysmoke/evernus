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
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSettings>
#include <QtDebug>

#include "WalletEntryFilterWidget.h"
#include "CacheTimerProvider.h"
#include "WarningBarWidget.h"
#include "ButtonWithTimer.h"
#include "StyledTreeView.h"
#include "ImportSettings.h"
#include "UISettings.h"

#include "WalletJournalWidget.h"

namespace Evernus
{
    WalletJournalWidget::WalletJournalWidget(const WalletJournalEntryRepository &journalRepo,
                                             const CharacterRepository &characterRepository,
                                             const FilterTextRepository &filterRepo,
                                             const CacheTimerProvider &cacheTimerProvider,
                                             const EveDataProvider &dataProvider,
                                             bool corp,
                                             QWidget *parent)
        : CharacterBoundWidget(std::bind(&CacheTimerProvider::getLocalCacheTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpWalletJournal) : (TimerType::WalletJournal)),
                               std::bind(&CacheTimerProvider::getLocalUpdateTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpWalletJournal) : (TimerType::WalletJournal)),
                               ImportSettings::maxWalletAgeKey,
                               parent)
        , mModel(journalRepo, characterRepository, dataProvider, corp)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolBarLayout);

        auto &importBtn = getAPIImportButton();
        toolBarLayout->addWidget(&importBtn);

        mFilter = new WalletEntryFilterWidget{QStringList{} << tr("all") << tr("incoming") << tr("outgoing"), filterRepo, this};
        toolBarLayout->addWidget(mFilter, 1);
        connect(mFilter, &WalletEntryFilterWidget::filterChanged, this, &WalletJournalWidget::updateFilter);

        QSettings settings;

        mCombineBtn = new QCheckBox{tr("Combine for all characters"), this};
        toolBarLayout->addWidget(mCombineBtn);
        mCombineBtn->setChecked(settings.value(UISettings::combineJournalKey, UISettings::combineJournalDefault).toBool());
        connect(mCombineBtn, &QCheckBox::toggled, this, [=](bool checked) {
            QSettings settings;
            settings.setValue(UISettings::combineJournalKey, checked);

            mModel.setCombineCharacters(checked);
        });

        auto &warningBar = getWarningBarWidget();
        mainLayout->addWidget(&warningBar);

        mFilterModel = new QSortFilterProxyModel{this};
        mFilterModel->setSortRole(Qt::UserRole);
        mFilterModel->setFilterKeyColumn(-1);
        mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        mFilterModel->setSourceModel(&mModel);

        mView = new StyledTreeView{(corp) ? (QStringLiteral("corpJournalView")) : (QStringLiteral("journalView")), this};
        mainLayout->addWidget(mView, 1);
        mView->setModel(mFilterModel);
        mView->sortByColumn(1, Qt::DescendingOrder);
    }

    void WalletJournalWidget::updateData()
    {
        refreshImportTimer();
        mModel.reset();
    }

    void WalletJournalWidget::updateFilter(const QDate &from, const QDate &to, const QString &filter, int type)
    {
        mModel.setFilter(getCharacterId(), from, to, static_cast<EntryType>(type), mCombineBtn->isChecked());
        mFilterModel->setFilterWildcard(filter);
    }

    void WalletJournalWidget::handleNewCharacter(Character::IdType id)
    {
        qDebug() << "Switching wallet journal to" << id;

        const auto tillDate = QDate::currentDate();
        const auto fromDate = tillDate.addDays(-7);

        mFilter->blockSignals(true);
        mFilter->setFilter(fromDate, tillDate, QString{}, static_cast<int>(EntryType::All));
        mFilter->blockSignals(false);

        mModel.setFilter(id, fromDate, tillDate, EntryType::All, mCombineBtn->isChecked());

        mView->header()->resizeSections(QHeaderView::ResizeToContents);
    }
}
