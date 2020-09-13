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
#include <ctime>

#include <QtDebug>

#include <QNetworkInterface>
#include <QDesktopServices>
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QScrollArea>
#include <QClipboard>
#include <QStatusBar>
#include <QTabWidget>
#include <QSettings>
#include <QMenuBar>
#include <QTabBar>
#include <QLabel>

#ifdef Q_OS_WIN
#   include <sys/utime.h>
#   include <QWinTaskbarButton>
#else
#   include <utime.h>
#endif

#include "WalletTransactionsWidget.h"
#include "CharacterManagerDialog.h"
#include "NewCharacterController.h"
#include "MarketAnalysisWidget.h"
#include "CitadelManagerDialog.h"
#include "WalletJournalWidget.h"
#include "CharacterRepository.h"
#include "MarketBrowserWidget.h"
#include "ItemHistoriesWidget.h"
#include "ESIInterfaceManager.h"
#include "RepositoryProvider.h"
#include "ActiveTasksDialog.h"
#include "PreferencesDialog.h"
#include "MarketOrderWidget.h"
#include "MarginToolDialog.h"
#include "StatisticsWidget.h"
#include "CharacterWidget.h"
#include "CustomFPCDialog.h"
#include "EveDataProvider.h"
#include "IndustryWidget.h"
#include "ContractWidget.h"
#include "ItemCostWidget.h"
#include "ImportSettings.h"
#include "ClickableLabel.h"
#include "MenuBarWidget.h"
#include "PriceSettings.h"
#include "SSOMessageBox.h"
#include "SyncSettings.h"
#include "HttpSettings.h"
#include "AssetsWidget.h"
#include "LMeveWidget.h"
#include "AboutDialog.h"
#include "SyncDialog.h"
#include "UISettings.h"
#include "Updater.h"

#include "MainWindow.h"

namespace Evernus
{
    const QString MainWindow::settingsMaximizedKey = QStringLiteral("mainWindow/maximized");
    const QString MainWindow::settingsPosKey = QStringLiteral("mainWindow/pos");
    const QString MainWindow::settingsSizeKey = QStringLiteral("mainWindow/size");

    const QString MainWindow::redirectDomain = "evernus.anver.ee";

    MainWindow::MainWindow(const RepositoryProvider &repositoryProvider,
                           MarketOrderProvider &charOrderProvider,
                           MarketOrderProvider &corpOrderProvider,
                           AssetProvider &charAssetProvider,
                           AssetProvider &corpAssetProvider,
                           const ContractProvider &charContractProvider,
                           const ContractProvider &corpContractProvider,
                           EveDataProvider &eveDataProvider,
                           const CacheTimerProvider &cacheTimerProvider,
                           ItemCostProvider &itemCostProvider,
                           const LMeveDataProvider &lMeveDataProvider,
                           ESIInterfaceManager &interfaceManager,
                           TaskManager &taskManager,
                           QWidget *parent,
                           Qt::WindowFlags flags)
        : QMainWindow{parent, flags}
        , mRepositoryProvider{repositoryProvider}
        , mItemCostProvider{itemCostProvider}
        , mEveDataProvider{eveDataProvider}
        , mCitadelAccessCache{interfaceManager.getCitadelAccessCache()}
        , mTrayIcon{new QSystemTrayIcon{QIcon{QStringLiteral(":/images/main-icon.png")}, this}}
        , mStatusActiveTasksThrobber{QStringLiteral(":/images/loader.gif")}
        , mStatusActiveTasksDonePixmap{QStringLiteral(":/images/tick.png")}
        , mClientId{interfaceManager.getClientId()}
        , mClientSecret{interfaceManager.getClientSecret()}
    {
        readSettings();
        createStatusBar();
        createMenu();
        createMainView(charOrderProvider,
                       corpOrderProvider,
                       charAssetProvider,
                       corpAssetProvider,
                       charContractProvider,
                       corpContractProvider,
                       lMeveDataProvider,
                       cacheTimerProvider,
                       interfaceManager,
                       taskManager);

        connect(mTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::activateTrayIcon);
        connect(&mAutoImportTimer, &QTimer::timeout, this, &MainWindow::refreshAll);

        setWindowIcon(QIcon{":/images/main-icon.png"});

        setUpAutoImportTimer();

        mStatusActiveTasksThrobber.start();
    }

    void MainWindow::showAsSaved()
    {
        if (mShowMaximized)
            showMaximized();
        else
            show();
    }

    void MainWindow::showActiveTasks()
    {
        if (mActiveTasksDialog == nullptr)
        {
#ifdef Q_OS_WIN
            if (mTaskbarButton == nullptr)
            {
                mTaskbarButton = new QWinTaskbarButton{this};
                mTaskbarButton->setWindow(windowHandle());
            }

            mActiveTasksDialog = new ActiveTasksDialog{*mTaskbarButton, this};
#else
            mActiveTasksDialog = new ActiveTasksDialog{this};
#endif
            connect(this, &MainWindow::newTaskInfoAdded, mActiveTasksDialog, &ActiveTasksDialog::addNewTaskInfo);
            connect(this, &MainWindow::newSubTaskInfoAdded, mActiveTasksDialog, &ActiveTasksDialog::addNewSubTaskInfo);
            connect(this, &MainWindow::taskInfoChanged, mActiveTasksDialog, &ActiveTasksDialog::setTaskInfo);
            connect(this, &MainWindow::taskEnded, mActiveTasksDialog, &ActiveTasksDialog::endTask);
            connect(mActiveTasksDialog, &ActiveTasksDialog::taskCountChanged, this, &MainWindow::updateTasksStatus);
            mActiveTasksDialog->setAttribute(Qt::WA_ShowWithoutActivating);
            mActiveTasksDialog->show();
        }
        else
        {
            mActiveTasksDialog->show();
        }
    }

    void MainWindow::showCharacterManagement()
    {
        if (mCharacterManagerDialog == nullptr)
        {
            mCharacterManagerDialog = new CharacterManagerDialog{mRepositoryProvider.getCharacterRepository(), this};
            connect(this, &MainWindow::charactersChanged, mCharacterManagerDialog, &CharacterManagerDialog::updateCharacters);
            connect(mCharacterManagerDialog, &CharacterManagerDialog::charactersChanged,
                    mMenuWidget, &MenuBarWidget::refreshCharacters);
            connect(mCharacterManagerDialog, &CharacterManagerDialog::charactersChanged,
                    this, &MainWindow::updateCharacters);
            connect(mCharacterManagerDialog, &CharacterManagerDialog::addCharacter,
                    this, &MainWindow::addCharacter);
        }

        mCharacterManagerDialog->exec();
    }

    void MainWindow::showPreferences()
    {
        PreferencesDialog dlg{mEveDataProvider, mRepositoryProvider.getCharacterRepository().getDatabase(), this};
        connect(&dlg, &PreferencesDialog::clearCorpWalletData, this, &MainWindow::clearCorpWalletData);
        connect(&dlg, &PreferencesDialog::clearRefreshTokens, this, &MainWindow::clearRefreshTokens);

        dlg.exec();

        setUpAutoImportTimer();
        mFPCController.handleNewPreferences();

        emit preferencesChanged();
    }

    void MainWindow::showMarginTool()
    {
        QSettings settings;
        if (settings.value(UISettings::minimizeByMarginToolKey, UISettings::minimizeByMarginToolDefault).toBool())
            showMinimized();

        if (mMarginToolDialog.isNull())
        {
            mMarginToolDialog = new MarginToolDialog{mRepositoryProvider.getCharacterRepository(),
                                                     mItemCostProvider,
                                                     mEveDataProvider};
            mMarginToolDialog->setCharacter(mCurrentCharacterId);
            connect(mMenuWidget, &MenuBarWidget::currentCharacterChanged, mMarginToolDialog, &MarginToolDialog::setCharacter);
            connect(mMarginToolDialog, &MarginToolDialog::hidden, this, &MainWindow::showNormal);
            connect(mMarginToolDialog, &MarginToolDialog::quit, this, &MainWindow::close);
            connect(this, &MainWindow::preferencesChanged, mMarginToolDialog, &MarginToolDialog::handleNewPreferences);
        }

        mMarginToolDialog->showNormal();
        mMarginToolDialog->activateWindow();
    }

    void MainWindow::showCustomFPC()
    {
        CustomFPCDialog dlg{this};
        mFPCController.changeExecutor(&dlg);
        connect(&dlg, &CustomFPCDialog::showInEve, this, [=](const auto id) {
            this->showInEve(id, Character::invalidId);
        });

        dlg.exec();
    }

    void MainWindow::showCitadelManager()
    {
        CitadelManagerDialog dlg{mEveDataProvider,
                                 mRepositoryProvider.getCitadelRepository(),
                                 mCitadelAccessCache,
                                 mCurrentCharacterId,
                                 this};
        connect(this, &MainWindow::citadelsChanged, &dlg, &CitadelManagerDialog::refreshCitadels);
        connect(&dlg, &CitadelManagerDialog::citadelsChanged, this, &MainWindow::citadelsEdited);

        if (dlg.exec() == QDialog::Accepted)
            emit citadelsEdited();
    }

    void MainWindow::showAbout()
    {
        AboutDialog dlg{this};
        dlg.exec();
    }

    void MainWindow::openHelp()
    {
        QDesktopServices::openUrl(QUrl{"http://evernus.com/help"});
    }

    void MainWindow::checkForUpdates()
    {
        Updater::getInstance().checkForUpdates(false);
    }

    void MainWindow::showColumnHelp()
    {
        QMessageBox::information(this, tr("Evernus"), tr(
            "You can show/hide table columns via right-click menu. Columns can also be moved around via dragging."));
    }

    void MainWindow::addNewTaskInfo(uint taskId, const QString &description)
    {
        showActiveTasks();
        emit newTaskInfoAdded(taskId, description);
    }

    void MainWindow::updateTasksStatus(size_t remaining)
    {
        if (remaining == 0)
        {
            mStatusActiveTasksBtn->setPixmap(mStatusActiveTasksDonePixmap);
            mStatusActiveTasksBtn->setToolTip(tr("No active tasks"));
        }
        else
        {
            mStatusActiveTasksBtn->setMovie(&mStatusActiveTasksThrobber);
            mStatusActiveTasksBtn->setToolTip(tr("Active tasks: %1").arg(remaining));
        }
    }

    void MainWindow::updateIskData()
    {
        if (mCurrentCharacterId != Character::invalidId)
        {
            const auto character = mRepositoryProvider.getCharacterRepository().find(mCurrentCharacterId);
            mStatusWalletLabel->setText(tr("Wallet: <strong>%1</strong>")
                .arg(character->getISKPresentation()));
        }
        else
        {
            mStatusWalletLabel->setText(QString{});
        }
    }

    void MainWindow::updateCharacters()
    {
        refreshCharactersMenu();
        emit charactersChanged();
    }

    void MainWindow::setCharacter(Character::IdType id)
    {
        mCurrentCharacterId = id;
        updateIskData();
        updateCurrentTab(mMainTabs->currentIndex());
    }

    void MainWindow::refreshAssets()
    {
        refreshData<&MainWindow::importCharacterAssets, &MainWindow::importCorpAssets>();
    }

    void MainWindow::refreshContracts()
    {
        refreshData<&MainWindow::importCharacterContracts, &MainWindow::importCorpContracts>();
    }

    void MainWindow::refreshWalletJournal()
    {
        refreshData<&MainWindow::importCharacterWalletJournal, &MainWindow::importCorpWalletJournal>();
    }

    void MainWindow::refreshWalletTransactions()
    {
        refreshData<&MainWindow::importCharacterWalletTransactions, &MainWindow::importCorpWalletTransactions>();
    }

    void MainWindow::refreshMarketOrdersFromAPI()
    {
        refreshData<&MainWindow::importCharacterMarketOrdersFromAPI, &MainWindow::importCorpMarketOrdersFromAPI>();
    }

    void MainWindow::refreshMarketOrdersFromLogs()
    {
        refreshData<&MainWindow::importCharacterMarketOrdersFromLogs, &MainWindow::importCorpMarketOrdersFromLogs>();
    }

    void MainWindow::refreshMiningLedger()
    {
        QSettings settings;
        if (settings.value(ImportSettings::importAllCharactersKey, ImportSettings::importAllCharactersDefault).toBool())
        {
            enumerateEnabledCharacters([=](auto id) {
                emit importCharacterMiningLedger(id);
            });
        }
        else if (mCurrentCharacterId != Character::invalidId)
        {
            emit importCharacterMiningLedger(mCurrentCharacterId);
        }
    }

    void MainWindow::refreshAll()
    {
        emit refreshCharacters();
        emit refreshCitadels();

        refreshWalletJournal();

        QSettings settings;

        const auto marketOrderSource = static_cast<ImportSettings::MarketOrderImportSource>(
            settings.value(ImportSettings::marketOrderImportSourceKey, static_cast<int>(ImportSettings::marketOrderImportSourceDefault)).toInt());
        if (marketOrderSource == ImportSettings::MarketOrderImportSource::Logs)
            refreshMarketOrdersFromLogs();
        else
            refreshMarketOrdersFromAPI();

        if (!settings.value(PriceSettings::autoAddCustomItemCostKey, PriceSettings::autoAddCustomItemCostDefault).toBool())
            refreshWalletTransactions();

        if (settings.value(ImportSettings::importAssetsKey, ImportSettings::importAssetsDefault).toBool())
            refreshAssets();
        if (settings.value(ImportSettings::importContractsKey, ImportSettings::importContractsDefault).toBool())
            refreshContracts();
        if (settings.value(ImportSettings::importMiningLedgerKey, ImportSettings::importMiningLedgerDefault).toBool())
            refreshMiningLedger();
    }

    void MainWindow::characterDataChanged()
    {
        mTabCharacterIds.clear();

        const auto tabIndex = mMainTabs->currentIndex();
        if (tabIndex != mCharacterTabIndex)
            updateCurrentTab(tabIndex);
    }

    void MainWindow::showSSOError(const QString &info)
    {
        SSOMessageBox::showMessage(info, this);
    }

    void MainWindow::requestSSOAuth(Character::IdType charId, const QUrl &url)
    {
        qInfo() << "Showing SSO auth for:" << charId << url;

        if (mAuthViews.find(charId) != std::end(mAuthViews))
            return;

        const auto authView = new SSOAuthDialog{url, this};
        mAuthViews[charId] = authView;

        QString charName;
        auto charNameFound = false;

        std::tie(charName, charNameFound) = getCharacterName(charId);

        if (charName.isEmpty())
            authView->setWindowTitle(tr("SSO authentication for unknown character: %1").arg(charId));
        else
            authView->setWindowTitle(getAuthWidowTitle(charName));

        authView->show();

        connect(authView, &SSOAuthDialog::acquiredCode, this, [=](const auto &code) {
            authView->disconnect(this);
            authView->deleteLater();

            mAuthViews.erase(charId);

            emit gotSSOAuthorizationCode(charId, code);
        });
        connect(authView, &SSOAuthDialog::aboutToClose, this, [=] {
            authView->deleteLater();
            emit ssoAuthCanceled(charId);
        });
        connect(authView, &SSOAuthDialog::destroyed, this, [=] {
            mAuthViews.erase(charId);
        });

        if (!charNameFound)
        {
            connect(&mEveDataProvider, &EveDataProvider::namesChanged, authView, [=] {
                authView->setWindowTitle(getAuthWidowTitle(mEveDataProvider.getGenericName(charId)));
            });
        }
    }

    void MainWindow::addCharacter()
    {
        const auto controller = new NewCharacterController{mClientId, mClientSecret, this};
        connect(controller, &NewCharacterController::error, this, &MainWindow::showErrorMessage);
        connect(controller, &NewCharacterController::authorizedCharacter, this, &MainWindow::authorizedCharacter);
    }

    void MainWindow::updateCurrentTab(int index)
    {
        auto widget = mMainTabs->widget(index);
        if (widget != nullptr)
        {
            if (mCurrentCharacterId != mTabCharacterIds[index])
            {
                mTabCharacterIds[index] = mCurrentCharacterId;
                QMetaObject::invokeMethod(mTabWidgets[index], "setCharacter", Qt::AutoConnection, Q_ARG(Character::IdType, mCurrentCharacterId));
            }
#ifdef Q_OS_MAC
            statusBar()->repaint();
#endif
        }
    }

    void MainWindow::activateTrayIcon(QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
        {
            mTrayIcon->hide();
            showMaximized();
            activateWindow();
        }
    }

    void MainWindow::copyHTTPLink()
    {
        const auto addresses = QNetworkInterface::allAddresses();
        QString curAddress = "localhost";

        for (const auto &address : addresses)
        {
            if (!address.isLoopback() && address.protocol() == QAbstractSocket::IPv4Protocol)
            {
                curAddress = address.toString();
                break;
            }
        }

        QSettings settings;
        QApplication::clipboard()->setText(QString{"http://%1:%2"}
            .arg(curAddress)
            .arg(settings.value(HttpSettings::portKey, HttpSettings::portDefault).value<quint16>()));

        QMessageBox::information(this, tr("Evernus"), tr("HTTP link was copied to the clipboard."));
    }

    void MainWindow::showMarketBrowser(EveType::IdType typeId)
    {
        mMainTabs->setCurrentIndex(mMarketBrowserTabIndex);
        mMarketBrowserTab->showOrdersForType(typeId);
    }

    void MainWindow::performSync()
    {
#ifdef EVERNUS_DROPBOX_ENABLED
        SyncDialog syncDlg{SyncDialog::Mode::Upload};
        syncDlg.exec();

        utimbuf buf;
        time(&buf.actime);
        buf.modtime = buf.actime;

        utime(mRepositoryProvider.getCharacterRepository().getDatabase().databaseName().toUtf8().constData(), &buf);
#endif
    }

    void MainWindow::showInEve(EveType::IdType typeId, Character::IdType ownerId)
    {
        if (ownerId != Character::invalidId)
            emit openMarketInEve(typeId, ownerId);
        else if (mCurrentCharacterId != Character::invalidId)
            emit openMarketInEve(typeId, mCurrentCharacterId);
    }

    void MainWindow::setWaypoint(quint64 locationId)
    {
        if (mCurrentCharacterId != Character::invalidId)
            emit setDestinationInEve(locationId, mCurrentCharacterId);
    }

    void MainWindow::showErrorMessage(const QString &error)
    {
        QMessageBox::warning(this, tr("Error"), error);
    }

    void MainWindow::changeEvent(QEvent *event)
    {
        QSettings settings;
        if ((event->type() == QEvent::WindowStateChange) &&
            settings.value(UISettings::minimizeToTrayKey, UISettings::minimizeToTrayDefault).toBool())
        {
            if (isMinimized())
            {
                mTrayIcon->show();
                QMetaObject::invokeMethod(this, "hide", Qt::QueuedConnection);
            }
            else
            {
                mTrayIcon->hide();
            }
        }
    }

    void MainWindow::closeEvent(QCloseEvent *event)
    {
        if (!mMarginToolDialog.isNull())
            mMarginToolDialog->close();

        writeSettings();

        QSettings settings;
        if (settings.value(SyncSettings::enabledOnStartupKey, SyncSettings::enabledOnStartupDefault).toBool() &&
            settings.value(SyncSettings::enabledOnShutdownKey, SyncSettings::enabledOnShutdownDefault).toBool() &&
            SyncDialog::performedSync())
        {
            performSync();
        }

        event->accept();
    }

    void MainWindow::readSettings()
    {
        QSettings settings;

        const auto pos = settings.value(settingsPosKey).toPoint();
        const auto size = settings.value(settingsSizeKey, QSize{600, 400}).toSize();

        resize(size);
        move(pos);

        mShowMaximized = settings.value(settingsMaximizedKey, false).toBool();
    }

    void MainWindow::writeSettings()
    {
        QSettings settings;
        settings.setValue(settingsPosKey, pos());
        settings.setValue(settingsSizeKey, size());
        settings.setValue(settingsMaximizedKey, isMaximized());
    }

    void MainWindow::createMenu()
    {
        auto bar = menuBar();

        auto fileMenu = bar->addMenu(tr("&File"));
        mCharactersMenu = fileMenu->addMenu(tr("Select character"));
        fileMenu->addAction(QIcon{":/images/user.png"}, tr("&Manage characters..."), this, &MainWindow::showCharacterManagement);
#ifdef Q_OS_OSX
        fileMenu->addAction(QIcon{":/images/wrench.png"}, tr("&Preferences..."), this, &MainWindow::showPreferences, QKeySequence::Preferences)->setMenuRole(QAction::PreferencesRole);
#else
        fileMenu->addAction(QIcon{":/images/wrench.png"}, tr("&Preferences..."), this, &MainWindow::showPreferences, Qt::CTRL + Qt::Key_O)->setMenuRole(QAction::PreferencesRole);
#endif
        fileMenu->addSeparator();
        fileMenu->addAction(tr("Import EVE Mentat order history..."), this, &MainWindow::importFromMentat);
        fileMenu->addSeparator();
        fileMenu->addAction(tr("E&xit"), this, &MainWindow::close, QKeySequence::Quit)->setMenuRole(QAction::QuitRole);

        auto toolsMenu = bar->addMenu(tr("&Tools"));
        toolsMenu->addAction(tr("Import citadels"), this, &MainWindow::refreshCitadels);
        toolsMenu->addAction(tr("Citadel manager"), this, &MainWindow::showCitadelManager);
        toolsMenu->addAction(QIcon{":/images/report.png"}, tr("Ma&rgin tool..."), this, &MainWindow::showMarginTool, Qt::CTRL + Qt::Key_M);
        toolsMenu->addAction(tr("Custom &Fast Price Copy"), this, &MainWindow::showCustomFPC);
        toolsMenu->addSeparator();
        toolsMenu->addAction(tr("Copy HTTP link"), this, &MainWindow::copyHTTPLink);
#ifdef EVERNUS_DROPBOX_ENABLED
        toolsMenu->addSeparator();
        toolsMenu->addAction(QIcon{":/images/arrow_refresh.png"}, tr("Upload data to cloud..."), this, &MainWindow::performSync);
#endif

        auto viewMenu = bar->addMenu(tr("&View"));
        mViewTabsMenu = viewMenu->addMenu(tr("Show/hide tabs"));
        viewMenu->addAction(tr("Show/hide table columns"), this, &MainWindow::showColumnHelp);

        auto action = viewMenu->addAction(tr("Always on top"));
        auto toggleTopmost = [action, this] {
            const auto alwaysOnTop = action->isChecked();

            QSettings settings;
            settings.setValue(UISettings::mainWindowAlwaysOnTopKey, alwaysOnTop);

            auto flags = windowFlags();
            if (alwaysOnTop)
                flags |= Qt::WindowStaysOnTopHint;
            else
                flags &= ~Qt::WindowStaysOnTopHint;

            setWindowFlags(flags);
            show();
        };

        QSettings settings;

        action->setCheckable(true);
        action->setChecked(
            settings.value(UISettings::mainWindowAlwaysOnTopKey, UISettings::mainWindowAlwaysOnTopDefault).toBool());
        connect(action, &QAction::triggered, this, toggleTopmost);

        auto helpMenu = bar->addMenu(tr("&Help"));
        helpMenu->addAction(QIcon{":/images/help.png"}, tr("&Online help..."), this, &MainWindow::openHelp);
        helpMenu->addAction(tr("Check for &updates"), this, SLOT(checkForUpdates()));
        helpMenu->addSeparator();
        helpMenu->addAction(tr("&About..."), this, &MainWindow::showAbout)->setMenuRole(QAction::AboutRole);

        mMenuWidget = new MenuBarWidget{mRepositoryProvider.getCharacterRepository(), this};
        bar->setCornerWidget(mMenuWidget);
        connect(this, &MainWindow::charactersChanged, mMenuWidget, &MenuBarWidget::refreshCharacters);
        connect(mMenuWidget, &MenuBarWidget::currentCharacterChanged, this, &MainWindow::setCharacter);
        connect(mMenuWidget, &MenuBarWidget::importAll, this, &MainWindow::refreshAll);

        refreshCharactersMenu();
        toggleTopmost();
    }

    void MainWindow::createMainView(MarketOrderProvider &charOrderProvider,
                                    MarketOrderProvider &corpOrderProvider,
                                    AssetProvider &charAssetProvider,
                                    AssetProvider &corpAssetProvider,
                                    const ContractProvider &charContractProvider,
                                    const ContractProvider &corpContractProvider,
                                    const LMeveDataProvider &lMeveDataProvider,
                                    const CacheTimerProvider &cacheTimerProvider,
                                    ESIInterfaceManager &interfaceManager,
                                    TaskManager &taskManager)
    {
        mMainTabs = new QTabWidget{this};
        setCentralWidget(mMainTabs);
#ifdef Q_OS_OSX
        mMainTabs->tabBar()->setElideMode(Qt::ElideNone);
        mMainTabs->tabBar()->setUsesScrollButtons(true);
#endif
        mMainTabs->tabBar()->setStyleSheet("*::tab:disabled { width: 0; height: 0; margin: 0; padding: 0; border: none; }");
        connect(mMainTabs, &QTabWidget::currentChanged, this, &MainWindow::updateCurrentTab);

        auto charTab = new CharacterWidget{mRepositoryProvider.getCharacterRepository(),
                                           mRepositoryProvider.getMarketOrderRepository(),
                                           cacheTimerProvider,
                                           this};
        mCharacterTabIndex = addTab(charTab, tr("Character"), TabType::Character);
        connect(charTab, &CharacterWidget::importFromAPI, this, &MainWindow::importCharacter);
        connect(charTab, &CharacterWidget::importAll, this, &MainWindow::refreshAll);
        connect(charTab, &CharacterWidget::characterDataChanged, this, &MainWindow::characterDataChanged);
        connect(this, &MainWindow::charactersChanged, charTab, &CharacterWidget::updateData);
        connect(this, &MainWindow::characterAssetsChanged, charTab, &CharacterWidget::updateTimerList);
        connect(this, &MainWindow::characterWalletJournalChanged, charTab, &CharacterWidget::updateTimerList);
        connect(this, &MainWindow::characterWalletTransactionsChanged, charTab, &CharacterWidget::updateTimerList);
        connect(this, &MainWindow::characterMarketOrdersChanged, charTab, &CharacterWidget::updateMarketData);

        auto statsTab = new StatisticsWidget{mRepositoryProvider,
                                             mEveDataProvider,
                                             mItemCostProvider,
                                             this};
        addTab(statsTab, tr("Statistics"), TabType::Other);
        connect(statsTab, &StatisticsWidget::makeSnapshots, this, [=] {
            makeValueSnapshots(mCurrentCharacterId);
        });
        connect(this, &MainWindow::charactersChanged, statsTab, &StatisticsWidget::updateBalanceData);
        connect(this, &MainWindow::externalOrdersChanged, statsTab, &StatisticsWidget::updateBalanceData);
        connect(this, &MainWindow::characterAssetsChanged, statsTab, &StatisticsWidget::updateBalanceData);
        connect(this, &MainWindow::characterWalletJournalChanged, statsTab, &StatisticsWidget::updateBalanceData);
        connect(this, &MainWindow::characterWalletJournalChanged, statsTab, &StatisticsWidget::updateJournalData);
        connect(this, &MainWindow::corpWalletJournalChanged, statsTab, &StatisticsWidget::updateJournalData);
        connect(this, &MainWindow::corpWalletJournalChanged, statsTab, &StatisticsWidget::updateBalanceData);
        connect(this, &MainWindow::snapshotsTaken, statsTab, &StatisticsWidget::updateBalanceData);
        connect(this, &MainWindow::characterWalletTransactionsChanged, statsTab, &StatisticsWidget::updateTransactionData);
        connect(this, &MainWindow::corpWalletTransactionsChanged, statsTab, &StatisticsWidget::updateTransactionData);
        connect(this, &MainWindow::preferencesChanged, statsTab, &StatisticsWidget::handleNewPreferences);

        auto assetsTab = new AssetsWidget{charAssetProvider,
                                          mEveDataProvider,
                                          cacheTimerProvider,
                                          mRepositoryProvider.getFilterTextRepository(),
                                          false,
                                          this};
        addTab(assetsTab, tr("Assets"), TabType::Character);
        connect(assetsTab, &AssetsWidget::importFromAPI, this, &MainWindow::importCharacterAssets);
        connect(assetsTab, &AssetsWidget::importPricesFromWeb, this, &MainWindow::importExternalOrdersFromWeb);
        connect(assetsTab, &AssetsWidget::importPricesFromFile, this, &MainWindow::importExternalOrdersFromFile);
        connect(assetsTab, &AssetsWidget::setDestinationInEve, this, &MainWindow::setWaypoint);
        connect(assetsTab, &AssetsWidget::showInEve, this, &MainWindow::showInEve);
        connect(this, &MainWindow::citadelsChanged, assetsTab, &AssetsWidget::updateData);
        connect(this, &MainWindow::characterAssetsChanged, assetsTab, &AssetsWidget::updateData);
        connect(this, &MainWindow::externalOrdersChanged, assetsTab, &AssetsWidget::updateData);
        connect(this, &MainWindow::externalOrdersChangedWithMarketOrders, assetsTab, &AssetsWidget::updateData);
        connect(this, &MainWindow::itemVolumeChanged, assetsTab, &AssetsWidget::updateData);

        auto orderTab = new MarketOrderWidget{charOrderProvider,
                                              corpOrderProvider,
                                              cacheTimerProvider,
                                              mEveDataProvider,
                                              mItemCostProvider,
                                              mRepositoryProvider.getWalletTransactionRepository(),
                                              mRepositoryProvider.getCharacterRepository(),
                                              mRepositoryProvider.getFilterTextRepository(),
                                              mRepositoryProvider.getExternalOrderRepository(),
                                              false,
                                              this};
        addTab(orderTab, tr("Character orders"), TabType::Character);
        connect(orderTab, &MarketOrderWidget::importFromAPI, this, &MainWindow::importCharacterMarketOrdersFromAPI);
        connect(orderTab, &MarketOrderWidget::importFromLogs, this, &MainWindow::importCharacterMarketOrdersFromLogs);
        connect(orderTab, &MarketOrderWidget::importPricesFromWeb, this, &MainWindow::importExternalOrdersFromWeb);
        connect(orderTab, &MarketOrderWidget::importPricesFromFile, this, &MainWindow::importExternalOrdersFromFile);
        connect(orderTab, &MarketOrderWidget::openMarginTool, this, &MainWindow::showMarginTool);
        connect(orderTab, &MarketOrderWidget::showExternalOrders, this, &MainWindow::showMarketBrowser);
        connect(orderTab, &MarketOrderWidget::showInEve, this, &MainWindow::showInEve);
        connect(orderTab, &MarketOrderWidget::showExternalOrders, this, &MainWindow::showMarketBrowser);
        connect(orderTab, &MarketOrderWidget::fpcExecutorChanged, &mFPCController, &FPCController::changeExecutor);
        connect(this, &MainWindow::characterMarketOrdersChanged, orderTab, &MarketOrderWidget::updateData);
        connect(this, &MainWindow::corpMarketOrdersChanged, orderTab, &MarketOrderWidget::updateData);
        connect(this, &MainWindow::externalOrdersChanged, orderTab, &MarketOrderWidget::updateData);
        connect(this, &MainWindow::citadelsChanged, orderTab, &MarketOrderWidget::updateData);
        connect(this, &MainWindow::itemCostsChanged, orderTab, &MarketOrderWidget::updateData);
        connect(this, &MainWindow::charactersChanged, orderTab, &MarketOrderWidget::updateCharacters);

        auto journalTab = new WalletJournalWidget{mRepositoryProvider.getWalletJournalEntryRepository(),
                                                  mRepositoryProvider.getCharacterRepository(),
                                                  mRepositoryProvider.getFilterTextRepository(),
                                                  cacheTimerProvider,
                                                  mEveDataProvider,
                                                  false,
                                                  this};
        addTab(journalTab, tr("Character journal"), TabType::Character);
        connect(journalTab, &WalletJournalWidget::importFromAPI, this, &MainWindow::importCharacterWalletJournal);
        connect(this, &MainWindow::characterWalletJournalChanged, journalTab, &WalletJournalWidget::updateData);

        auto transactionsTab = new WalletTransactionsWidget{mRepositoryProvider.getWalletTransactionRepository(),
                                                            mRepositoryProvider.getCharacterRepository(),
                                                            mRepositoryProvider.getFilterTextRepository(),
                                                            cacheTimerProvider,
                                                            mEveDataProvider,
                                                            mItemCostProvider,
                                                            false,
                                                            this};
        addTab(transactionsTab, tr("Character transactions"), TabType::Character);
        connect(transactionsTab, &WalletTransactionsWidget::importFromAPI, this, &MainWindow::importCharacterWalletTransactions);
        connect(transactionsTab, &WalletTransactionsWidget::showInEve, this, &MainWindow::showInEve);
        connect(this, &MainWindow::characterWalletTransactionsChanged, transactionsTab, &WalletTransactionsWidget::updateData);
        connect(this, &MainWindow::charactersChanged, transactionsTab, &WalletTransactionsWidget::updateCharacters);

        auto contractsTab = new ContractWidget{cacheTimerProvider,
                                               mEveDataProvider,
                                               charContractProvider,
                                               mRepositoryProvider.getFilterTextRepository(),
                                               mRepositoryProvider.getCharacterRepository(),
                                               false,
                                               this};
        addTab(contractsTab, tr("Character contracts"), TabType::Character);
        connect(contractsTab, &ContractWidget::importFromAPI, this, &MainWindow::importCharacterContracts);
        connect(this, &MainWindow::characterContractsChanged, contractsTab, &ContractWidget::updateData);

        auto corpAssetsTab = new AssetsWidget{corpAssetProvider,
                                              mEveDataProvider,
                                              cacheTimerProvider,
                                              mRepositoryProvider.getFilterTextRepository(),
                                              true,
                                              this};
        addTab(corpAssetsTab, tr("Corporation assets"), TabType::Corp);
        connect(corpAssetsTab, &AssetsWidget::importFromAPI, this, &MainWindow::importCorpAssets);
        connect(corpAssetsTab, &AssetsWidget::importPricesFromWeb, this, &MainWindow::importExternalOrdersFromWeb);
        connect(corpAssetsTab, &AssetsWidget::importPricesFromFile, this, &MainWindow::importExternalOrdersFromFile);
        connect(corpAssetsTab, &AssetsWidget::setDestinationInEve, this, &MainWindow::setWaypoint);
        connect(corpAssetsTab, &AssetsWidget::showInEve, this, &MainWindow::showInEve);
        connect(this, &MainWindow::citadelsChanged, corpAssetsTab, &AssetsWidget::updateData);
        connect(this, &MainWindow::corpAssetsChanged, corpAssetsTab, &AssetsWidget::updateData);
        connect(this, &MainWindow::externalOrdersChanged, corpAssetsTab, &AssetsWidget::updateData);
        connect(this, &MainWindow::externalOrdersChangedWithMarketOrders, corpAssetsTab, &AssetsWidget::updateData);
        connect(this, &MainWindow::itemVolumeChanged, corpAssetsTab, &AssetsWidget::updateData);

        auto corpOrderTab = new MarketOrderWidget{corpOrderProvider,
                                                  corpOrderProvider,
                                                  cacheTimerProvider,
                                                  mEveDataProvider,
                                                  mItemCostProvider,
                                                  mRepositoryProvider.getCorpWalletTransactionRepository(),
                                                  mRepositoryProvider.getCharacterRepository(),
                                                  mRepositoryProvider.getFilterTextRepository(),
                                                  mRepositoryProvider.getExternalOrderRepository(),
                                                  true,
                                                  this};
        addTab(corpOrderTab, tr("Corporation orders"), TabType::Corp);
        connect(corpOrderTab, &MarketOrderWidget::importFromAPI, this, &MainWindow::importCorpMarketOrdersFromAPI);
        connect(corpOrderTab, &MarketOrderWidget::importFromLogs, this, &MainWindow::importCorpMarketOrdersFromLogs);
        connect(corpOrderTab, &MarketOrderWidget::importPricesFromWeb, this, &MainWindow::importExternalOrdersFromWeb);
        connect(corpOrderTab, &MarketOrderWidget::importPricesFromFile, this, &MainWindow::importExternalOrdersFromFile);
        connect(corpOrderTab, &MarketOrderWidget::openMarginTool, this, &MainWindow::showMarginTool);
        connect(corpOrderTab, &MarketOrderWidget::showExternalOrders, this, &MainWindow::showMarketBrowser);
        connect(corpOrderTab, &MarketOrderWidget::showInEve, this, &MainWindow::showInEve);
        connect(corpOrderTab, &MarketOrderWidget::showExternalOrders, this, &MainWindow::showMarketBrowser);
        connect(corpOrderTab, &MarketOrderWidget::fpcExecutorChanged, &mFPCController, &FPCController::changeExecutor);
        connect(this, &MainWindow::corpMarketOrdersChanged, corpOrderTab, &MarketOrderWidget::updateData);
        connect(this, &MainWindow::externalOrdersChanged, corpOrderTab, &MarketOrderWidget::updateData);
        connect(this, &MainWindow::citadelsChanged, corpOrderTab, &MarketOrderWidget::updateData);
        connect(this, &MainWindow::itemCostsChanged, corpOrderTab, &MarketOrderWidget::updateData);
        connect(this, &MainWindow::charactersChanged, corpOrderTab, &MarketOrderWidget::updateCharacters);

        auto corpJournalTab = new WalletJournalWidget{mRepositoryProvider.getCorpWalletJournalEntryRepository(),
                                                      mRepositoryProvider.getCharacterRepository(),
                                                      mRepositoryProvider.getFilterTextRepository(),
                                                      cacheTimerProvider,
                                                      mEveDataProvider,
                                                      true,
                                                      this};
        addTab(corpJournalTab, tr("Corporation journal"), TabType::Corp);
        connect(corpJournalTab, &WalletJournalWidget::importFromAPI, this, &MainWindow::importCorpWalletJournal);
        connect(this, &MainWindow::corpWalletJournalChanged, corpJournalTab, &WalletJournalWidget::updateData);

        auto corpTransactionsTab = new WalletTransactionsWidget{mRepositoryProvider.getCorpWalletTransactionRepository(),
                                                                mRepositoryProvider.getCharacterRepository(),
                                                                mRepositoryProvider.getFilterTextRepository(),
                                                                cacheTimerProvider,
                                                                mEveDataProvider,
                                                                mItemCostProvider,
                                                                true,
                                                                this};
        addTab(corpTransactionsTab, tr("Corporation transactions"), TabType::Corp);
        connect(corpTransactionsTab, &WalletTransactionsWidget::importFromAPI, this, &MainWindow::importCorpWalletTransactions);
        connect(corpTransactionsTab, &WalletTransactionsWidget::showInEve, this, &MainWindow::showInEve);
        connect(this, &MainWindow::corpWalletTransactionsChanged, corpTransactionsTab, &WalletTransactionsWidget::updateData);
        connect(this, &MainWindow::charactersChanged, corpTransactionsTab, &WalletTransactionsWidget::updateCharacters);

        auto corpContractsTab = new ContractWidget{cacheTimerProvider,
                                                   mEveDataProvider,
                                                   corpContractProvider,
                                                   mRepositoryProvider.getFilterTextRepository(),
                                                   mRepositoryProvider.getCharacterRepository(),
                                                   true,
                                                   this};
        addTab(corpContractsTab, tr("Corporation contracts"), TabType::Corp);
        connect(corpContractsTab, &ContractWidget::importFromAPI, this, &MainWindow::importCorpContracts);
        connect(this, &MainWindow::corpContractsChanged, corpContractsTab, &ContractWidget::updateData);

        auto itemHistoryTab = new ItemHistoriesWidget{mRepositoryProvider.getWalletTransactionRepository(),
                                                      mRepositoryProvider.getCorpWalletTransactionRepository(),
                                                      mEveDataProvider,
                                                      this};
        connect(this, &MainWindow::characterWalletTransactionsChanged, itemHistoryTab, &ItemHistoriesWidget::updateData);
        connect(this, &MainWindow::corpWalletTransactionsChanged, itemHistoryTab, &ItemHistoriesWidget::updateData);
        connect(this, &MainWindow::preferencesChanged, itemHistoryTab, &ItemHistoriesWidget::handleNewPreferences);
        addTab(itemHistoryTab, tr("Item history"), TabType::Other);

        mMarketBrowserTab = new MarketBrowserWidget{mRepositoryProvider.getExternalOrderRepository(),
                                                    mRepositoryProvider.getMarketOrderRepository(),
                                                    mRepositoryProvider.getCorpMarketOrderRepository(),
                                                    mRepositoryProvider.getCharacterRepository(),
                                                    mRepositoryProvider.getFavoriteItemRepository(),
                                                    mRepositoryProvider.getLocationBookmarkRepository(),
                                                    mRepositoryProvider.getEveTypeRepository(),
                                                    mRepositoryProvider.getMarketGroupRepository(),
                                                    mRepositoryProvider.getRegionTypePresetRepository(),
                                                    charOrderProvider,
                                                    corpOrderProvider,
                                                    mEveDataProvider,
                                                    interfaceManager,
                                                    taskManager,
                                                    mItemCostProvider,
                                                    this};
        mMarketBrowserTabIndex = addTab(mMarketBrowserTab, tr("Market browser"), TabType::Other);
        connect(mMarketBrowserTab, &MarketBrowserWidget::importPricesFromFile, this, &MainWindow::importExternalOrdersFromFile);
        connect(mMarketBrowserTab, &MarketBrowserWidget::externalOrdersChanged, this, &MainWindow::externalOrdersChanged);
        connect(mMarketBrowserTab, &MarketBrowserWidget::updateExternalOrders, this, &MainWindow::updateExternalOrders);
        connect(this, &MainWindow::characterMarketOrdersChanged, mMarketBrowserTab, &MarketBrowserWidget::fillOrderItemNames);
        connect(this, &MainWindow::corpMarketOrdersChanged, mMarketBrowserTab, &MarketBrowserWidget::fillOrderItemNames);
        connect(this, &MainWindow::externalOrdersChanged, mMarketBrowserTab, &MarketBrowserWidget::updateData);
        connect(this, &MainWindow::itemVolumeChanged, mMarketBrowserTab, &MarketBrowserWidget::updateData);
        connect(this, &MainWindow::preferencesChanged, mMarketBrowserTab, &MarketBrowserWidget::preferencesChanged);

        auto itemCostTab = new ItemCostWidget{mItemCostProvider, mEveDataProvider, this};
        addTab(itemCostTab, tr("Item costs"), TabType::Other);
        connect(this, &MainWindow::itemCostsChanged, itemCostTab, &ItemCostWidget::updateData);

        auto lmEveTab = new LMeveWidget{cacheTimerProvider,
                                        mEveDataProvider,
                                        lMeveDataProvider,
                                        mItemCostProvider,
                                        mRepositoryProvider.getCharacterRepository(),
                                        this};
        addTab(lmEveTab, tr("LMeve"), TabType::Other);
        connect(lmEveTab, &LMeveWidget::openPreferences, this, &MainWindow::showPreferences);
        connect(lmEveTab, &LMeveWidget::syncLMeve, this, &MainWindow::syncLMeve);
        connect(lmEveTab, &LMeveWidget::importPricesFromWeb, this, &MainWindow::importExternalOrdersFromWeb);
        connect(lmEveTab, &LMeveWidget::importPricesFromFile, this, &MainWindow::importExternalOrdersFromFile);
        connect(this, &MainWindow::externalOrdersChanged, lmEveTab, &LMeveWidget::updateData);
        connect(this, &MainWindow::lMeveTasksChanged, lmEveTab, &LMeveWidget::updateData);

        auto marketAnalysisTab = new MarketAnalysisWidget{mEveDataProvider,
                                                          interfaceManager,
                                                          taskManager,
                                                          mRepositoryProvider.getMarketOrderRepository(),
                                                          mRepositoryProvider.getCorpMarketOrderRepository(),
                                                          mRepositoryProvider.getEveTypeRepository(),
                                                          mRepositoryProvider.getMarketGroupRepository(),
                                                          mRepositoryProvider.getCharacterRepository(),
                                                          mRepositoryProvider.getRegionTypePresetRepository(),
                                                          mRepositoryProvider.getRegionStationPresetRepository(),
                                                          this};
        connect(marketAnalysisTab, &MarketAnalysisWidget::updateExternalOrders, this, &MainWindow::updateExternalOrders);
        connect(marketAnalysisTab, &MarketAnalysisWidget::showInEve, this, &MainWindow::showInEve);
        connect(this, &MainWindow::preferencesChanged, marketAnalysisTab, &MarketAnalysisWidget::preferencesChanged);
        addTab(marketAnalysisTab, tr("Market analysis"), TabType::Other);

        const auto industryTab = new IndustryWidget{mEveDataProvider,
                                                    mRepositoryProvider.getRegionStationPresetRepository(),
                                                    mRepositoryProvider.getEveTypeRepository(),
                                                    mRepositoryProvider.getMarketGroupRepository(),
                                                    mRepositoryProvider.getCharacterRepository(),
                                                    mRepositoryProvider.getIndustryManufacturingSetupRepository(),
                                                    mRepositoryProvider.getMiningLedgerRepository(),
                                                    interfaceManager,
                                                    taskManager,
                                                    charAssetProvider,
                                                    mItemCostProvider,
                                                    cacheTimerProvider,
                                                    this};
        addTab(industryTab, tr("Industry"), TabType::Other);
        connect(industryTab, &IndustryWidget::updateExternalOrders, this, &MainWindow::updateExternalOrders);
        connect(industryTab, &IndustryWidget::importMiningLedger, this, &MainWindow::importCharacterMiningLedger);
        connect(industryTab, &IndustryWidget::showExternalOrders, this, &MainWindow::showMarketBrowser);
        connect(industryTab, &IndustryWidget::showInEve, this, [=](auto typeId) {
            showInEve(typeId, Character::invalidId);
        });
        connect(this, &MainWindow::preferencesChanged, industryTab, &IndustryWidget::handleNewPreferences);
        connect(this, &MainWindow::characterAssetsChanged, industryTab, &IndustryWidget::refreshAssets);
        connect(this, &MainWindow::characterMiningLedgerChanged, industryTab, &IndustryWidget::refreshMiningLedger);

        QSettings settings;

        const auto actions = mViewTabsMenu->actions();
        for (auto i = 0; i < actions.size(); ++i)
        {
            if (!settings.value(QString{UISettings::tabShowStateKey}.arg(i), true).toBool())
                actions[i]->setChecked(false);
        }

        mViewTabsMenu->addSeparator();

        auto toggleDefaultTabs = [=](bool checked, TabType type) {
            for (const auto action : actions)
            {
                if (static_cast<TabType>(action->data().toInt()) == type)
                    action->setChecked(checked);
            }
        };

        auto action = mViewTabsMenu->addAction(tr("Toggle character tabs"));
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [=](bool checked) {
            toggleDefaultTabs(checked, TabType::Character);
        });

        action = mViewTabsMenu->addAction(tr("Toggle corporation tabs"));
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [=](bool checked) {
            toggleDefaultTabs(checked, TabType::Corp);
        });
    }

    void MainWindow::createStatusBar()
    {
        mStatusWalletLabel = new QLabel{this};
        mStatusActiveTasksBtn = new ClickableLabel{this};
        connect(mStatusActiveTasksBtn, &ClickableLabel::clicked, this, &MainWindow::showActiveTasks);

        updateTasksStatus(0);

        auto bar = statusBar();
        bar->addPermanentWidget(mStatusActiveTasksBtn);
        bar->addPermanentWidget(mStatusWalletLabel);
    }

    QWidget *MainWindow::createMainViewTab(QWidget *content)
    {
        auto scroll = new QScrollArea{this};
        scroll->setWidgetResizable(true);
        scroll->setWidget(content);
        scroll->setFrameStyle(QFrame::NoFrame);

        return scroll;
    }

    int MainWindow::addTab(QWidget *widget, const QString &label, TabType type)
    {
        const auto index = mMainTabs->addTab(createMainViewTab(widget), label);
        mTabWidgets[index] = widget;

        auto action = mViewTabsMenu->addAction(label);
        action->setCheckable(true);
        action->setChecked(true);
        action->setData(static_cast<int>(type));
        connect(action, &QAction::toggled, this, [index, this](bool checked) {
            mMainTabs->setTabEnabled(index, checked);
            mMainTabs->setElideMode(mMainTabs->elideMode());    // hack to refresh layout, since there is no api to do that

            QSettings settings;
            settings.setValue(QString{UISettings::tabShowStateKey}.arg(index), checked);
        });

        return index;
    }

    void MainWindow::setUpAutoImportTimer()
    {
        QSettings settings;

        mAutoImportTimer.setInterval(
            settings.value(ImportSettings::autoImportTimeKey, ImportSettings::autoImportTimerDefault).toInt() * 1000 * 60);

        if (settings.value(ImportSettings::autoImportEnabledKey, ImportSettings::autoImportEnabledDefault).toBool())
            mAutoImportTimer.start();
        else
            mAutoImportTimer.stop();
    }

    void MainWindow::refreshCharactersMenu()
    {
        mCharactersMenu->clear();

        const auto &repo = mRepositoryProvider.getCharacterRepository();
        const auto idName = repo.getIdColumn();

        auto characters = repo.getEnabledQuery();
        while (characters.next())
        {
            const auto id = characters.value(idName).value<Character::IdType>();
            auto action = mCharactersMenu->addAction(characters.value("name").toString());

            connect(action, &QAction::triggered, this, [id, this] {
                mMenuWidget->setCurrentCharacter(id);
            });
        }
    }

    template<class T>
    void MainWindow::enumerateEnabledCharacters(T &&func) const
    {
        const auto &repo = mRepositoryProvider.getCharacterRepository();
        const auto idName = repo.getIdColumn();
        auto query = repo.getEnabledQuery();

        while (query.next())
            std::forward<T>(func)(query.value(idName).value<Character::IdType>());
    }

    template<void (MainWindow::* CharRefresh)(Character::IdType), void (MainWindow::* CorpRefresh)(Character::IdType)>
    void MainWindow::refreshData()
    {
        QSettings settings;
        if (settings.value(ImportSettings::importAllCharactersKey, ImportSettings::importAllCharactersDefault).toBool())
        {
            const auto corp = settings.value(ImportSettings::updateCorpDataKey).toBool();
            enumerateEnabledCharacters([corp, this](auto id) {
                if (corp)
                    emit (this->*CorpRefresh)(id);

                emit (this->*CharRefresh)(id);
            });
        }
        else if (mCurrentCharacterId != Character::invalidId)
        {
            emit (this->*CharRefresh)(mCurrentCharacterId);
        }
    }

    std::pair<QString, bool> MainWindow::getCharacterName(Character::IdType id) const
    {
        QString charName;
        auto nameFound = false;

        try
        {
            charName = mRepositoryProvider.getCharacterRepository().getName(id);
            nameFound = true;
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            nameFound = mEveDataProvider.hasGenericName(id);
            charName = mEveDataProvider.getGenericName(id);
        }

        return std::make_pair(charName, nameFound);
    }

    QString MainWindow::getAuthWidowTitle(const QString &charName)
    {
        return tr("SSO Authentication for character: %1").arg(charName);
    }
}
