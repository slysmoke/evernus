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
#include <QPushButton>
#include <QGroupBox>
#include <QIcon>

#include "ItemHistoryWidget.h"

#include "ItemHistoriesWidget.h"

namespace Evernus
{
    ItemHistoriesWidget::ItemHistoriesWidget(const WalletTransactionRepository &walletRepo,
                                             const WalletTransactionRepository &corpWalletRepo,
                                             const EveDataProvider &dataProvider,
                                             QWidget *parent)
        : QWidget(parent)
        , mWalletRepo(walletRepo)
        , mCorpWalletRepo(corpWalletRepo)
        , mDataProvider(dataProvider)
        , mMainLayout(new QVBoxLayout{this})
    {
        auto addBtn = new QPushButton{tr("Add new history"), this};
        mMainLayout->addWidget(addBtn);
        addBtn->setFlat(true);
        addBtn->setIcon(QIcon{":/images/add.png"});
        connect(addBtn, &QPushButton::clicked, this, &ItemHistoriesWidget::addHistory);

        mMainLayout->addStretch();
    }

    void ItemHistoriesWidget::setCharacter(Character::IdType id)
    {
        mCharacterId = id;
        emit characterChanged(mCharacterId);
    }

    void ItemHistoriesWidget::addHistory()
    {
        auto group = new QGroupBox{this};
        mMainLayout->insertWidget(mMainLayout->count() - 2, group);

        auto groupLayout = new QVBoxLayout{group};

        auto historyWidget = new ItemHistoryWidget{mWalletRepo, mCorpWalletRepo, mDataProvider, group};
        groupLayout->addWidget(historyWidget);
        historyWidget->setCharacter(mCharacterId);
        connect(this, &ItemHistoriesWidget::characterChanged, historyWidget, &ItemHistoryWidget::setCharacter);
        connect(this, &ItemHistoriesWidget::updateData, historyWidget, &ItemHistoryWidget::updateData);
        connect(this, &ItemHistoriesWidget::handleNewPreferences, historyWidget, &ItemHistoryWidget::handleNewPreferences);

        auto removeBtn = new QPushButton{tr("Remove"), group};
        groupLayout->addWidget(removeBtn);
        removeBtn->setFlat(true);
        removeBtn->setIcon(QIcon{":/images/delete.png"});
        connect(removeBtn, &QPushButton::clicked, group, &QGroupBox::deleteLater);
    }
}
