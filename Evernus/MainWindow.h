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

#include <unordered_map>

#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QPointer>
#include <QPixmap>
#include <QTimer>
#include <QMovie>

#include "ExternalOrderImporter.h"
#include "FPCController.h"
#include "SSOAuthDialog.h"
#include "Character.h"

class QSystemTrayIcon;
class QTabWidget;
class QLabel;
class QUrl;

#ifdef Q_OS_WIN
class QWinTaskbarButton;
#endif

namespace Evernus
{
    class CharacterManagerDialog;
    class MarketOrderProvider;
    class ESIInterfaceManager;
    class MarketBrowserWidget;
    class WalletJournalEntry;
    class CacheTimerProvider;
    class RepositoryProvider;
    class CitadelAccessCache;
    class ActiveTasksDialog;
    class LMeveDataProvider;
    class MarginToolDialog;
    class ItemCostProvider;
    class ContractProvider;
    class EveDataProvider;
    class ClickableLabel;
    class MenuBarWidget;
    class AssetProvider;
    class TaskManager;
    class Key;

    class MainWindow
        : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(const RepositoryProvider &repositoryProvider,
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
                   QWidget *parent = nullptr,
                   Qt::WindowFlags flags = 0);
        virtual ~MainWindow() = default;

        void showAsSaved();

    signals:
        void refreshCharacters();
        void refreshCitadels();

        void citadelsChanged();
        void citadelsEdited();
        void charactersChanged();
        void characterAssetsChanged();
        void externalOrdersChanged();
        void externalOrdersChangedWithMarketOrders();
        void characterWalletJournalChanged();
        void characterWalletTransactionsChanged();
        void characterMarketOrdersChanged();
        void characterContractsChanged();
        void characterMiningLedgerChanged();
        void corpWalletJournalChanged();
        void corpWalletTransactionsChanged();
        void corpMarketOrdersChanged();
        void corpContractsChanged();
        void corpAssetsChanged();
        void itemCostsChanged();
        void itemVolumeChanged();
        void lMeveTasksChanged();

        void snapshotsTaken();

        void preferencesChanged();

        void newTaskInfoAdded(uint taskId, const QString &description);
        void newSubTaskInfoAdded(uint taskId, uint parentTask, const QString &description);
        void taskInfoChanged(uint taskId, const QString &text);
        void taskEnded(uint taskId, const QString &error);

        void importCharacter(Character::IdType id);
        void importCharacterAssets(Character::IdType id);
        void importCharacterContracts(Character::IdType id);
        void importCharacterWalletJournal(Character::IdType id);
        void importCharacterWalletTransactions(Character::IdType id);
        void importCharacterMarketOrdersFromAPI(Character::IdType id);
        void importCharacterMarketOrdersFromLogs(Character::IdType id);
        void importCharacterMiningLedger(Character::IdType id);
        void importCorpAssets(Character::IdType id);
        void importCorpContracts(Character::IdType id);
        void importCorpWalletJournal(Character::IdType id);
        void importCorpWalletTransactions(Character::IdType id);
        void importCorpMarketOrdersFromAPI(Character::IdType id);
        void importCorpMarketOrdersFromLogs(Character::IdType id);

        void importExternalOrdersFromWeb(Character::IdType id, const TypeLocationPairs &target);
        void importExternalOrdersFromFile(Character::IdType id, const TypeLocationPairs &target);

        void importFromMentat();

        void syncLMeve(Character::IdType id);

        void setDestinationInEve(quint64 locationId, Character::IdType charId);
        void openMarketInEve(EveType::IdType typeId, Character::IdType charId);

        void updateExternalOrders(const std::vector<ExternalOrder> &orders);

        void clearCorpWalletData();
        void clearRefreshTokens();

        void makeValueSnapshots(Character::IdType id);

        void gotSSOAuthorizationCode(Character::IdType charId, const QByteArray &code);
        void ssoAuthCanceled(Character::IdType charId);
        void authorizedCharacter(Character::IdType id, const QString &accessToken, const QString &refreshToken);

    public slots:
        void showActiveTasks();
        void showCharacterManagement();
        void showPreferences();
        void showMarginTool();
        void showCustomFPC();
        void showCitadelManager();
        void showAbout();
        void openHelp();
        void checkForUpdates();
        void showColumnHelp();

        void addNewTaskInfo(uint taskId, const QString &description);
        void updateTasksStatus(size_t remaining);

        void updateIskData();
        void updateCharacters();

        void setCharacter(Character::IdType id);

        void refreshAssets();
        void refreshContracts();
        void refreshWalletJournal();
        void refreshWalletTransactions();
        void refreshMarketOrdersFromAPI();
        void refreshMarketOrdersFromLogs();
        void refreshMiningLedger();
        void refreshAll();

        void characterDataChanged();

        void showSSOError(const QString &info);
        void requestSSOAuth(Character::IdType charId, const QUrl &url);

    private slots:
        void addCharacter();

        void updateCurrentTab(int index);

        void activateTrayIcon(QSystemTrayIcon::ActivationReason reason);
        void copyHTTPLink();

        void showMarketBrowser(EveType::IdType typeId);

        void performSync();

        void showInEve(EveType::IdType typeId, Character::IdType ownerId);
        void setWaypoint(quint64 locationId);

        void showErrorMessage(const QString &error);

    protected:
        virtual void changeEvent(QEvent *event) override;
        virtual void closeEvent(QCloseEvent *event) override;

    private:
        enum class TabType
        {
            Other,
            Character,
            Corp,
        };

        static const QString settingsMaximizedKey;
        static const QString settingsPosKey;
        static const QString settingsSizeKey;

        static const QString redirectDomain;

        const RepositoryProvider &mRepositoryProvider;
        ItemCostProvider &mItemCostProvider;

        EveDataProvider &mEveDataProvider;

        CitadelAccessCache &mCitadelAccessCache;

#ifdef Q_OS_WIN
        QWinTaskbarButton *mTaskbarButton = nullptr;
#endif

        MenuBarWidget *mMenuWidget = nullptr;
        QTabWidget *mMainTabs = nullptr;
        QSystemTrayIcon *mTrayIcon = nullptr;

        int mCharacterTabIndex = 0;

        bool mShowMaximized = false;

        CharacterManagerDialog *mCharacterManagerDialog = nullptr;
        ActiveTasksDialog *mActiveTasksDialog = nullptr;
        QPointer<MarginToolDialog> mMarginToolDialog;

        QLabel *mStatusWalletLabel = nullptr;
        QMovie mStatusActiveTasksThrobber;
        QPixmap mStatusActiveTasksDonePixmap;
        ClickableLabel *mStatusActiveTasksBtn = nullptr;

        Character::IdType mCurrentCharacterId = Character::invalidId;

        std::unordered_map<int, Character::IdType> mTabCharacterIds;
        std::unordered_map<int, QWidget *> mTabWidgets;

        QMenu *mCharactersMenu = nullptr;
        QMenu *mViewTabsMenu = nullptr;

        QTimer mAutoImportTimer;

        MarketBrowserWidget *mMarketBrowserTab = nullptr;
        int mMarketBrowserTabIndex = -1;

        FPCController mFPCController;

        std::unordered_map<Character::IdType, SSOAuthDialog *> mAuthViews;

        QString mClientId;
        QString mClientSecret;

        void readSettings();
        void writeSettings();

        void createMenu();
        void createMainView(MarketOrderProvider &charOrderProvider,
                            MarketOrderProvider &corpOrderProvider,
                            AssetProvider &charAssetProvider,
                            AssetProvider &corpAssetProvider,
                            const ContractProvider &charContractProvider,
                            const ContractProvider &corpContractProvider,
                            const LMeveDataProvider &lMeveDataProvider,
                            const CacheTimerProvider &cacheTimerProvider,
                            ESIInterfaceManager &interfaceManager,
                            TaskManager &taskManager);
        void createStatusBar();

        QWidget *createMainViewTab(QWidget *content);
        int addTab(QWidget *widget, const QString &label, TabType type);

        void setUpAutoImportTimer();
        void refreshCharactersMenu();

        template<class T>
        void enumerateEnabledCharacters(T &&func) const;

        template<void (MainWindow::* CharRefresh)(Character::IdType), void (MainWindow::* CorpRefresh)(Character::IdType)>
        void refreshData();

        std::pair<QString, bool> getCharacterName(Character::IdType id) const;

        static QString getAuthWidowTitle(const QString &charName);
    };
}
