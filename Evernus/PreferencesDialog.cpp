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
#include <vector>

#include <QDialogButtonBox>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeWidget>

#include "CharacterImportPreferencesWidget.h"
#include "IndustryImportPreferencesWidget.h"
#include "ContractImportPreferencesWidget.h"
#include "AssetsImportPreferencesWidget.h"
#include "ImportSourcePreferencesWidget.h"
#include "CorpImportPreferencesWidget.h"
#include "StatisticsPreferencesWidget.h"
#include "NetworkPreferencesWidget.h"
#include "GeneralPreferencesWidget.h"
#include "WalletPreferencesWidget.h"
#include "ImportPreferencesWidget.h"
#include "OrderPreferencesWidget.h"
#include "LMevePreferencesWidget.h"
#include "PricePreferencesWidget.h"
#include "SoundPreferencesWidget.h"
#include "SyncPreferencesWidget.h"
#include "HttpPreferencesWidget.h"
#include "PathPreferencesWidget.h"

#include "PreferencesDialog.h"

namespace Evernus
{
    PreferencesDialog::PreferencesDialog(const EveDataProvider &dataProvider, const QSqlDatabase &db, QWidget *parent)
        : QDialog{parent}
    {
        auto mainLayout = new QVBoxLayout{this};

        auto preferencesLayout = new QHBoxLayout{};
        mainLayout->addLayout(preferencesLayout);

        auto categoryTree = new QTreeWidget{this};
        preferencesLayout->addWidget(categoryTree);
        categoryTree->setHeaderHidden(true);
        categoryTree->setMaximumWidth(150);
        connect(categoryTree, &QTreeWidget::currentItemChanged, this, &PreferencesDialog::setCurrentPage);

        mPreferencesStack = new QStackedWidget{this};
        preferencesLayout->addWidget(mPreferencesStack, 1);

        const auto networkPreferences = new NetworkPreferencesWidget{this};
        connect(networkPreferences, &NetworkPreferencesWidget::clearRefreshTokens,
                this, &PreferencesDialog::clearRefreshTokens);

        std::vector<std::pair<QString, QWidget *>> categories;
        categories.emplace_back(std::make_pair(tr("General"), new GeneralPreferencesWidget{db, this}));
        categories.emplace_back(std::make_pair(tr("Paths"), new PathPreferencesWidget{this}));
        categories.emplace_back(std::make_pair(tr("Sounds"), new SoundPreferencesWidget{this}));
        categories.emplace_back(std::make_pair(tr("Prices"), new PricePreferencesWidget{this}));
        categories.emplace_back(std::make_pair(tr("Orders"), new OrderPreferencesWidget{dataProvider, this}));
        categories.emplace_back(std::make_pair(tr("Statistics"), new StatisticsPreferencesWidget{this}));
        categories.emplace_back(std::make_pair(tr("Network"), networkPreferences));
        categories.emplace_back(std::make_pair(tr("Synchronization"), new SyncPreferencesWidget{this}));
        categories.emplace_back(std::make_pair(tr("Web Service"), new HttpPreferencesWidget{this}));
        categories.emplace_back(std::make_pair(tr("Wallet"), new WalletPreferencesWidget{this}));
        categories.emplace_back(std::make_pair(tr("LMeve"), new LMevePreferencesWidget{this}));

        for (auto i = 0u; i < categories.size(); ++i)
        {
            auto item = new QTreeWidgetItem{categoryTree, QStringList{categories[i].first}};
            item->setData(0, Qt::UserRole, i);

            if (i == 0)
                categoryTree->setCurrentItem(item);
        }

        for (auto &category : categories)
        {
            connect(this, SIGNAL(settingsInvalidated()), category.second, SLOT(applySettings()));
            mPreferencesStack->addWidget(category.second);
        }

        auto importItem = new QTreeWidgetItem{categoryTree, QStringList{tr("Import")}};
        importItem->setExpanded(true);

        std::vector<std::pair<QString, QWidget *>> importCategories;
        importCategories.emplace_back(std::make_pair(tr("Character"), new CharacterImportPreferencesWidget{this}));
        importCategories.emplace_back(std::make_pair(tr("Assets"), new AssetsImportPreferencesWidget{this}));
        importCategories.emplace_back(std::make_pair(tr("Contracts"), new ContractImportPreferencesWidget{this}));
        importCategories.emplace_back(std::make_pair(tr("Industry"), new IndustryImportPreferencesWidget{this}));

        auto page = new CorpImportPreferencesWidget{this};
        importCategories.emplace_back(std::make_pair(tr("Corporation"), page));
        connect(page, &CorpImportPreferencesWidget::clearCorpWalletData, this, &PreferencesDialog::clearCorpWalletData);

        importCategories.emplace_back(std::make_pair(tr("Source"), new ImportSourcePreferencesWidget{this}));

        for (auto i = 0u; i < importCategories.size(); ++i)
        {
            auto item = new QTreeWidgetItem{importItem, QStringList{importCategories[i].first}};
            item->setData(0, Qt::UserRole, i + static_cast<int>(categories.size()));
        }

        for (auto &category : importCategories)
        {
            connect(this, SIGNAL(settingsInvalidated()), category.second, SLOT(applySettings()));
            mPreferencesStack->addWidget(category.second);
        }

        auto importPreferencesWidget = new ImportPreferencesWidget{this};
        connect(this, &PreferencesDialog::settingsInvalidated, importPreferencesWidget, &ImportPreferencesWidget::applySettings);

        mPreferencesStack->addWidget(importPreferencesWidget);
        importItem->setData(0, Qt::UserRole, static_cast<int>(categories.size() + importCategories.size()));

        auto buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
        mainLayout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        setWindowTitle(tr("Preferences"));
    }

    void PreferencesDialog::accept()
    {
        emit settingsInvalidated();

        QDialog::accept();
    }

    void PreferencesDialog::setCurrentPage(QTreeWidgetItem *current, QTreeWidgetItem *previous)
    {
        if (current == nullptr)
            mPreferencesStack->setCurrentIndex(-1);
        else
            mPreferencesStack->setCurrentIndex(current->data(0, Qt::UserRole).toInt());
    }
}
