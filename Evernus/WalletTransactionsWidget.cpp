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
#include <QApplication>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QSettings>
#include <QCheckBox>
#include <QLabel>
#include <QtDebug>
#include <QFont>

#include "WalletEntryFilterWidget.h"
#include "WalletTransactionView.h"
#include "CacheTimerProvider.h"
#include "WarningBarWidget.h"
#include "ItemCostProvider.h"
#include "ButtonWithTimer.h"
#include "ImportSettings.h"
#include "PriceSettings.h"
#include "UISettings.h"
#include "FlowLayout.h"
#include "TextUtils.h"

#include "WalletTransactionsWidget.h"

namespace Evernus
{
    WalletTransactionsWidget::WalletTransactionsWidget(const WalletTransactionRepository &walletRepo,
                                                       const CharacterRepository &characterRepository,
                                                       const FilterTextRepository &filterRepo,
                                                       const CacheTimerProvider &cacheTimerProvider,
                                                       const EveDataProvider &dataProvider,
                                                       ItemCostProvider &itemCostProvider,
                                                       bool corp,
                                                       QWidget *parent)
        : CharacterBoundWidget(std::bind(&CacheTimerProvider::getLocalCacheTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpWalletTransactions) : (TimerType::WalletTransactions)),
                               std::bind(&CacheTimerProvider::getLocalUpdateTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpWalletTransactions) : (TimerType::WalletTransactions)),
                               ImportSettings::maxWalletAgeKey,
                               parent)
        , mModel(walletRepo, characterRepository, dataProvider, itemCostProvider, corp)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolBarLayout);

        auto &importBtn = getAPIImportButton();
        toolBarLayout->addWidget(&importBtn);

        mFilter = new WalletEntryFilterWidget{QStringList{} << tr("all") << tr("buy") << tr("sell"), filterRepo, this};
        toolBarLayout->addWidget(mFilter, 1);
        connect(mFilter, &WalletEntryFilterWidget::filterChanged, this, &WalletTransactionsWidget::updateFilter);

        QSettings settings;

        mCombineBtn = new QCheckBox{tr("Combine for all characters"), this};
        toolBarLayout->addWidget(mCombineBtn);
        mCombineBtn->setChecked(settings.value(UISettings::combineTransactionsKey, UISettings::combineTransactionsDefault).toBool());
        connect(mCombineBtn, &QCheckBox::toggled, this, [=](bool checked) {
            QSettings settings;
            settings.setValue(UISettings::combineTransactionsKey, checked);

            mModel.setCombineCharacters(checked);
        });

        auto &warningBar = getWarningBarWidget();
        mainLayout->addWidget(&warningBar);

        mFilterModel = new QSortFilterProxyModel{this};
        mFilterModel->setSortRole(Qt::UserRole);
        mFilterModel->setFilterKeyColumn(-1);
        mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        mFilterModel->setSourceModel(&mModel);

        mView = new WalletTransactionView{(corp) ? ("corpTransactionsView") : ("transactionsView"),
                                          itemCostProvider,
                                          characterRepository,
                                          this};
        mainLayout->addWidget(mView, 1);
        connect(mView, &WalletTransactionView::showInEve, this, &WalletTransactionsWidget::showInEve);
        mView->setModels(mFilterModel, &mModel);
        mView->sortByColumn(1, Qt::DescendingOrder);

        QFont font;
        font.setBold(true);

        auto infoLayout = new FlowLayout{};
        mainLayout->addLayout(infoLayout);

        infoLayout->addWidget(new QLabel{tr("Total transactions:"), this});

        mTotalTransactionsLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalTransactionsLabel);
        mTotalTransactionsLabel->setFont(font);

        infoLayout->addWidget(new QLabel{tr("Total quantity:"), this});

        mTotalQuantityLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalQuantityLabel);
        mTotalQuantityLabel->setFont(font);

        infoLayout->addWidget(new QLabel{tr("Total size:"), this});

        mTotalSizeLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalSizeLabel);
        mTotalSizeLabel->setFont(font);

        infoLayout->addWidget(new QLabel{tr("Total income:"), this});

        mTotalIncomeLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalIncomeLabel);
        mTotalIncomeLabel->setFont(font);
        mTotalIncomeLabel->setStyleSheet("color: green;");

        infoLayout->addWidget(new QLabel{tr("Total cost:"), this});

        mTotalCostLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalCostLabel);
        mTotalCostLabel->setFont(font);
        mTotalCostLabel->setStyleSheet("color: red;");

        infoLayout->addWidget(new QLabel{tr("Total balance:"), this});

        mTotalBalanceLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalBalanceLabel);
        mTotalBalanceLabel->setFont(font);

        infoLayout->addWidget(new QLabel{tr("Total profit based on costs:"), this});

        mTotalProfitLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalProfitLabel);
        mTotalProfitLabel->setFont(font);

        connect(&mModel, &WalletTransactionsModel::dataChanged, this, &WalletTransactionsWidget::updateInfo);
    }

    void WalletTransactionsWidget::updateData()
    {
        refreshImportTimer();
        mModel.reset();

        updateInfo();
    }

    void WalletTransactionsWidget::updateCharacters()
    {
        mView->updateCharacters();
    }

    void WalletTransactionsWidget::updateFilter(const QDate &from, const QDate &to, const QString &filter, int type)
    {
        mModel.setFilter(getCharacterId(), from, to, static_cast<EntryType>(type), mCombineBtn->isChecked());
        mFilterModel->setFilterWildcard(filter);

        updateInfo();
    }

    void WalletTransactionsWidget::handleNewCharacter(Character::IdType id)
    {
        qDebug() << "Switching wallet transactions to" << id;

        const auto tillDate = QDate::currentDate();
        const auto fromDate = tillDate.addDays(-7);

        mFilter->blockSignals(true);
        mFilter->setFilter(fromDate, tillDate, QString{}, static_cast<int>(EntryType::All));
        mFilter->blockSignals(false);

        mModel.setFilter(id, fromDate, tillDate, EntryType::All, mCombineBtn->isChecked());

        mView->setCharacter(id);
        mView->header()->resizeSections(QHeaderView::ResizeToContents);

        updateInfo();
    }

    void WalletTransactionsWidget::updateInfo()
    {
        auto curLocale = locale();

        const auto income = mModel.getTotalIncome();
        const auto cost = mModel.getTotalCost();
        const auto profit = mModel.getTotalProfit();

        const auto setColor = [](auto label, auto value) {
            if (value > 0.)
                label->setStyleSheet("color: green;");
            else
                label->setStyleSheet("color: red;");
        };

        mTotalTransactionsLabel->setText(curLocale.toString(mModel.rowCount()));
        mTotalQuantityLabel->setText(curLocale.toString(mModel.getTotalQuantity()));
        mTotalSizeLabel->setText(QString{"%1m³"}.arg(curLocale.toString(mModel.getTotalSize(), 'f', 2)));
        mTotalIncomeLabel->setText(TextUtils::currencyToString(income, curLocale));
        mTotalCostLabel->setText(TextUtils::currencyToString(cost, curLocale));
        mTotalBalanceLabel->setText(TextUtils::currencyToString(income - cost, curLocale));
        mTotalProfitLabel->setText(TextUtils::currencyToString(profit, curLocale));

        setColor(mTotalBalanceLabel, income - cost);
        setColor(mTotalProfitLabel, profit);
    }
}
