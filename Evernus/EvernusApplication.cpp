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
#include <unordered_set>
#include <stdexcept>
#include <algorithm>
#include <future>

#include <boost/range/algorithm/transform.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/throw_exception.hpp>

#include <QSslConfiguration>
#include <QFutureWatcher>
#include <QSplashScreen>
#include <QNetworkProxy>
#include <QMessageBox>
#include <QFileDialog>
#include <QThreadPool>
#include <QSettings>
#include <QDir>
#include <QSet>

#if Q_CC_MSVC
#   pragma warning(push)
#   pragma warning(disable : 4996)
#endif

#include <QtConcurrent>

#if Q_CC_MSVC
#   pragma warning(pop)
#endif

#include <QtDebug>

#include "StandardExceptionQtWrapperException.h"
#include "ExternalOrderImporterNames.h"
#include "LanguageSelectDialog.h"
#include "SovereigntyStructure.h"
#include "StatisticsSettings.h"
#include "UpdaterSettings.h"
#include "NetworkSettings.h"
#include "ImportSettings.h"
#include "WalletSettings.h"
#include "PriceSettings.h"
#include "OrderSettings.h"
#include "PathSettings.h"
#include "HttpSettings.h"
#include "SyncSettings.h"
#include "SimpleCrypt.h"
#include "HttpService.h"
#include "SyncDialog.h"
#include "UISettings.h"
#include "DbSettings.h"
#include "Blueprint.h"
#include "PathUtils.h"
#include "SSOUtils.h"
#include "Updater.h"

#include "qxtmailmessage.h"

#include "EvernusApplication.h"

namespace Evernus
{
    const QString EvernusApplication::versionKey = "version";

    EvernusApplication::EvernusApplication(int &argc,
                                           char *argv[],
                                           QString clientId,
                                           QString clientSecret,
                                           const QString &forcedVersion,
                                           bool dontUpdate)
        : QApplication{argc, argv}
        , ExternalOrderImporterRegistry{}
        , CacheTimerProvider{}
        , ItemCostProvider{}
        , RepositoryProvider{}
        , LMeveDataProvider{}
        , TaskManager{}
        , EveDataManagerProvider{}
    {
        QSettings settings;

        auto lang = settings.value(UISettings::languageKey).toString();
        if (lang.isEmpty())
        {
            lang = QLocale{}.name();
            updateTranslator(lang);

            LanguageSelectDialog dlg;
            if (dlg.exec() == QDialog::Accepted)
            {
                lang = dlg.getSelectedLanguage();
                settings.setValue(UISettings::languageKey, lang);
                updateTranslator(lang);
            }
            else
            {
                settings.setValue(UISettings::languageKey, lang);
            }
        }
        else
        {
            updateTranslator(lang);
        }

        if (!forcedVersion.isEmpty())
            setApplicationVersion(forcedVersion);

        setProxySettings();

#ifdef EVERNUS_DROPBOX_ENABLED
        if (settings.value(SyncSettings::enabledOnStartupKey, SyncSettings::enabledOnStartupDefault).toBool())
        {
            SyncDialog syncDlg{SyncDialog::Mode::Download};
            syncDlg.exec();
        }
#endif

        QSplashScreen splash{QPixmap{":/images/splash.png"}};
        splash.show();
        showSplashMessage(tr("Loading..."), splash);

        showSplashMessage(tr("Creating databases..."), splash);
        createDb();

        showSplashMessage(tr("Creating schemas..."), splash);
        createDbSchema();

        showSplashMessage(tr("Creating data providers..."), splash);

        mCharacterAssetProvider = std::make_unique<CachingAssetProvider>(*mCharacterRepository,
                                                                         *mAssetListRepository,
                                                                         *mItemRepository);
        mCorpAssetProvider = std::make_unique<CachingAssetProvider>(*mCharacterRepository,
                                                                    *mCorpAssetListRepository,
                                                                    *mItemRepository);

        mCharacterOrderProvider = std::make_unique<CachingMarketOrderProvider>(*mMarketOrderRepository);
        connect(mCharacterOrderProvider.get(), &CachingMarketOrderProvider::orderChanged,
                this, &EvernusApplication::scheduleMarketOrderChange);

        mCorpOrderProvider = std::make_unique<CachingMarketOrderProvider>(*mCorpMarketOrderRepository);
        connect(mCorpOrderProvider.get(), &CachingMarketOrderProvider::orderChanged,
                this, &EvernusApplication::scheduleCorpMarketOrderChange);

        mCombinedOrderProvider
            = std::make_unique<CharacterCorporationCombinedMarketOrderProvider>(*mCharacterOrderProvider, *mCorpOrderProvider);

        mCharacterContractProvider = std::make_unique<CachingContractProvider>(*mContractRepository);
        mCorpContractProvider = std::make_unique<CachingContractProvider>(*mCorpContractRepository);

        mDataProvider = std::make_unique<CachingEveDataProvider>(*mEveTypeRepository,
                                                                 *mMetaGroupRepository,
                                                                 *mExternalOrderRepository,
                                                                 *mMarketOrderRepository,
                                                                 *mCorpMarketOrderRepository,
                                                                 *mMarketGroupRepository,
                                                                 *mCitadelRepository,
                                                                 *this,
                                                                 mEveDatabaseConnectionProvider);

        mESIInterfaceManager = std::make_unique<ESIInterfaceManager>(std::move(clientId),
                                                                     std::move(clientSecret),
                                                                     getCharacterRepository(),
                                                                     *mDataProvider);

        showSplashMessage(tr("Precaching timers..."), splash);
        precacheCacheTimers();
        precacheUpdateTimers();

        showSplashMessage(tr("Precaching jump map..."), splash);
        mDataProvider->precacheJumpMap();

        showSplashMessage(tr("Clearing old wallet entries..."), splash);
        deleteOldWalletEntries();

        showSplashMessage(tr("Clearing old market orders..."), splash);
        deleteOldMarketOrders();

        showSplashMessage(tr("Setting up HTTP service..."), splash);
        auto httpService = new HttpService{*mCombinedOrderProvider,
                                           *mCorpOrderProvider,
                                           *mDataProvider,
                                           *mCharacterRepository,
                                           *this,
                                           *this,
                                           &mHttpSessionManager,
                                           this};

        mHttpSessionManager.setPort(settings.value(HttpSettings::portKey, HttpSettings::portDefault).value<quint16>());
        mHttpSessionManager.setStaticContentService(httpService);
        mHttpSessionManager.setConnector(QxtHttpSessionManager::HttpServer);

        if (settings.value(HttpSettings::enabledKey, HttpSettings::enabledDefault).toBool())
            mHttpSessionManager.start();

        showSplashMessage(tr("Updating..."), splash);

        if (dontUpdate)
        {
            Updater::getInstance().updateDatabaseVersion(mMainDatabaseConnectionProvider.getConnection());
        }
        else
        {
            Updater::getInstance().performVersionMigration(*this,
                                                           *mDataProvider,
                                                           mESIInterfaceManager->getCitadelAccessCache());
        }

        showSplashMessage(tr("Loading..."), splash);

        settings.setValue(versionKey, applicationVersion());

        connect(&mSmtp, QOverload<const QByteArray &>::of(&QxtSmtp::authenticationFailed),
                this, &EvernusApplication::showSmtpError);
        connect(&mSmtp, QOverload<const QByteArray &>::of(&QxtSmtp::connectionFailed),
                this, &EvernusApplication::showSmtpError);
        connect(&mSmtp, QOverload<const QByteArray &>::of(&QxtSmtp::encryptionFailed),
                this, &EvernusApplication::showSmtpError);
        connect(&mSmtp, &QxtSmtp::finished, &mSmtp, &QxtSmtp::disconnectFromHost);
        setSmtpSettings();

        if (settings.value(UpdaterSettings::autoUpdateKey, UpdaterSettings::autoUpdateDefault).toBool())
            Updater::getInstance().checkForUpdates(true);

        connect(mESIInterfaceManager.get(), &ESIInterfaceManager::ssoAuthRequested, this, &EvernusApplication::ssoAuthRequested);

        mESIManager = std::make_unique<ESIManager>(*mDataProvider, *mESIInterfaceManager);
        connect(mESIManager.get(), &ESIManager::error, this, &EvernusApplication::ssoError);

        mDataProvider->precacheNames();
    }

    EvernusApplication::~EvernusApplication()
    {
        QThreadPool::globalInstance()->waitForDone();
    }

    void EvernusApplication::registerImporter(const std::string &name, std::unique_ptr<ExternalOrderImporter> &&importer)
    {
        Q_ASSERT(mExternalOrderImporters.find(name) == std::end(mExternalOrderImporters));

        connect(importer.get(), &ExternalOrderImporter::statusChanged, this, &EvernusApplication::showPriceImportStatus, Qt::QueuedConnection);
        connect(importer.get(), &ExternalOrderImporter::genericError, this, &EvernusApplication::ssoError, Qt::QueuedConnection);
        connect(importer.get(), &ExternalOrderImporter::externalOrdersChanged, this, &EvernusApplication::finishExternalOrderImport, Qt::QueuedConnection);
        mExternalOrderImporters.emplace(name, std::move(importer));
    }

    QDateTime EvernusApplication::getLocalCacheTimer(Character::IdType id, TimerType type) const
    {
        const auto charTimes = mCacheTimes.find(type);
        if (charTimes == std::end(mCacheTimes))
            return QDateTime::currentDateTime();

        const auto time = charTimes->second.find(id);
        return (time == std::end(charTimes->second)) ? (QDateTime::currentDateTime()) : (time->second.toLocalTime());
    }

    void EvernusApplication::setUtcCacheTimer(Character::IdType id, TimerType type, const QDateTime &dt)
    {
        if (!dt.isValid())
            return;

        mCacheTimes[type][id] = dt;

        if (type != TimerType::Character)
        {
            CacheTimer timer;
            timer.setCharacterId(id);
            timer.setType(type);
            timer.setCacheUntil(dt);

            mCacheTimerRepository->store(timer);
        }
    }

    QDateTime EvernusApplication::getLocalUpdateTimer(Character::IdType id, TimerType type) const
    {
        const auto charTimes = mUpdateTimes.find(type);
        if (charTimes == std::end(mUpdateTimes))
            return QDateTime{};

        const auto time = charTimes->second.find(id);
        return (time == std::end(charTimes->second)) ? (QDateTime{}) : (time->second.toLocalTime());
    }

    std::shared_ptr<ItemCost> EvernusApplication::fetchForCharacterAndType(Character::IdType characterId, EveType::IdType typeId) const
    {
        const auto it = mCharacterItemCostCache.find(std::make_pair(characterId, typeId));
        if (it != std::end(mCharacterItemCostCache))
            return it->second;

        ItemCostRepository::EntityPtr cost;

        try
        {
            cost = mItemCostRepository->fetchForCharacterAndType(characterId, typeId);
        }
        catch (const ItemCostRepository::NotFoundException &)
        {
            QSettings settings;
            if (settings.value(PriceSettings::shareCostsKey, PriceSettings::shareCostsDefault).toBool())
            {
                const auto it = mTypeItemCostCache.find(typeId);
                if (it != std::end(mTypeItemCostCache))
                {
                    mCharacterItemCostCache.emplace(std::make_pair(characterId, typeId), it->second);
                    return it->second;
                }

                try
                {
                    cost = mItemCostRepository->fetchLatestForType(typeId);
                }
                catch (const ItemCostRepository::NotFoundException &)
                {
                    cost = std::make_shared<ItemCost>();
                }

                mTypeItemCostCache.emplace(typeId, cost);
            }
            else
            {
                cost = std::make_shared<ItemCost>();
            }
        }

        mCharacterItemCostCache.emplace(std::make_pair(characterId, typeId), cost);
        return cost;
    }

    ItemCostProvider::CostList EvernusApplication::fetchForCharacter(Character::IdType characterId) const
    {
        return mItemCostRepository->fetchForCharacter(characterId);
    }

    void EvernusApplication::setForCharacterAndType(Character::IdType characterId, EveType::IdType typeId, double value)
    {
        auto cost = std::make_shared<ItemCost>();
        cost->setCharacterId(characterId);
        cost->setTypeId(typeId);
        cost->setCost(value);

        mItemCostRepository->store(*cost);

        mCharacterItemCostCache[std::make_pair(characterId, typeId)] = cost;
        mTypeItemCostCache[typeId] = cost;

        if (!mItemCostUpdateScheduled)
        {
            mItemCostUpdateScheduled = true;
            QMetaObject::invokeMethod(this, "emitNewItemCosts", Qt::QueuedConnection);
        }
    }

    std::shared_ptr<ItemCost> EvernusApplication::findItemCost(ItemCost::IdType id) const
    {
        return mItemCostRepository->find(id);
    }

    void EvernusApplication::removeItemCost(ItemCost::IdType id) const
    {
        mItemCostRepository->remove(id);
        mCharacterItemCostCache.clear();
        mTypeItemCostCache.clear();

        emit itemCostsChanged();
    }

    void EvernusApplication::storeItemCost(ItemCost &cost) const
    {
        mItemCostRepository->store(cost);

        auto sharedCost = std::make_shared<ItemCost>(cost);

        mCharacterItemCostCache[std::make_pair(cost.getCharacterId(), cost.getTypeId())] = sharedCost;
        mTypeItemCostCache[cost.getTypeId()] = sharedCost;

        emit itemCostsChanged();
    }

    void EvernusApplication::removeAllItemCosts(Character::IdType characterId) const
    {
        mItemCostRepository->removeForCharacter(characterId);
        mCharacterItemCostCache.clear();
        mTypeItemCostCache.clear();

        emit itemCostsChanged();
    }

    const CharacterRepository &EvernusApplication::getCharacterRepository() const noexcept
    {
        return *mCharacterRepository;
    }

    const WalletSnapshotRepository &EvernusApplication::getWalletSnapshotRepository() const noexcept
    {
        return *mWalletSnapshotRepository;
    }

    const CorpWalletSnapshotRepository &EvernusApplication::getCorpWalletSnapshotRepository() const noexcept
    {
        return *mCorpWalletSnapshotRepository;
    }

    const AssetValueSnapshotRepository &EvernusApplication::getAssetValueSnapshotRepository() const noexcept
    {
        return *mAssetValueSnapshotRepository;
    }

    const CorpAssetValueSnapshotRepository &EvernusApplication::getCorpAssetValueSnapshotRepository() const noexcept
    {
        return *mCorpAssetValueSnapshotRepository;
    }

    const WalletJournalEntryRepository &EvernusApplication::getWalletJournalEntryRepository() const noexcept
    {
        return *mWalletJournalEntryRepository;
    }

    const WalletTransactionRepository &EvernusApplication::getWalletTransactionRepository() const noexcept
    {
        return *mWalletTransactionRepository;
    }

    const MarketOrderRepository &EvernusApplication::getMarketOrderRepository() const noexcept
    {
        return *mMarketOrderRepository;
    }

    const WalletJournalEntryRepository &EvernusApplication::getCorpWalletJournalEntryRepository() const noexcept
    {
        return *mCorpWalletJournalEntryRepository;
    }

    const WalletTransactionRepository &EvernusApplication::getCorpWalletTransactionRepository() const noexcept
    {
        return *mCorpWalletTransactionRepository;
    }

    const MarketOrderRepository &EvernusApplication::getCorpMarketOrderRepository() const noexcept
    {
        return *mCorpMarketOrderRepository;
    }

    const ItemCostRepository &EvernusApplication::getItemCostRepository() const noexcept
    {
        return *mItemCostRepository;
    }

    const MarketOrderValueSnapshotRepository &EvernusApplication::getMarketOrderValueSnapshotRepository() const noexcept
    {
        return *mMarketOrderValueSnapshotRepository;
    }

    const CorpMarketOrderValueSnapshotRepository &EvernusApplication::getCorpMarketOrderValueSnapshotRepository() const noexcept
    {
        return *mCorpMarketOrderValueSnapshotRepository;
    }

    const FilterTextRepository &EvernusApplication::getFilterTextRepository() const noexcept
    {
        return *mFilterTextRepository;
    }

    const OrderScriptRepository &EvernusApplication::getOrderScriptRepository() const noexcept
    {
        return *mOrderScriptRepository;
    }

    const FavoriteItemRepository &EvernusApplication::getFavoriteItemRepository() const noexcept
    {
        return *mFavoriteItemRepository;
    }

    const LocationBookmarkRepository &EvernusApplication::getLocationBookmarkRepository() const noexcept
    {
        return *mLocationBookmarkRepository;
    }

    const ExternalOrderRepository &EvernusApplication::getExternalOrderRepository() const noexcept
    {
        return *mExternalOrderRepository;
    }

    const EveTypeRepository &EvernusApplication::getEveTypeRepository() const noexcept
    {
        return *mEveTypeRepository;
    }

    const MarketGroupRepository &EvernusApplication::getMarketGroupRepository() const noexcept
    {
        return *mMarketGroupRepository;
    }

    const CacheTimerRepository &EvernusApplication::getCacheTimerRepository() const noexcept
    {
        return *mCacheTimerRepository;
    }

    const UpdateTimerRepository &EvernusApplication::getUpdateTimerRepository() const noexcept
    {
        return *mUpdateTimerRepository;
    }

    const ItemRepository &EvernusApplication::getItemRepository() const noexcept
    {
        return *mItemRepository;
    }

    const CitadelRepository &EvernusApplication::getCitadelRepository() const noexcept
    {
        return *mCitadelRepository;
    }

    const RegionTypePresetRepository &EvernusApplication::getRegionTypePresetRepository() const noexcept
    {
        return *mRegionTypePresetRepository;
    }

    const ItemRepository &EvernusApplication::getCorpItemRepository() const noexcept
    {
        return *mCorpItemRepository;
    }

    const RegionStationPresetRepository &EvernusApplication::getRegionStationPresetRepository() const noexcept
    {
        return *mRegionStationPresetRepository;
    }

    const IndustryManufacturingSetupRepository &EvernusApplication::getIndustryManufacturingSetupRepository() const noexcept
    {
        return *mIndustryManufacturingSetupRepository;
    }

    const MiningLedgerRepository &EvernusApplication::getMiningLedgerRepository() const noexcept
    {
        return *mMiningLedgerRepository;
    }

    std::vector<std::shared_ptr<LMeveTask>> EvernusApplication::getTasks(Character::IdType characterId) const
    {
        const auto it = mLMeveTaskCache.find(characterId);
        if (it != std::end(mLMeveTaskCache))
            return it->second;

        return mLMeveTaskCache.emplace(characterId, mLMeveTaskRepository->fetchForCharacter(characterId)).first->second;
    }

    uint EvernusApplication::startTask(const QString &description)
    {
        QMetaObject::invokeMethod(this,
                                  "taskStarted",
                                  Qt::QueuedConnection,
                                  Q_ARG(uint, mTaskId),
                                  Q_ARG(QString, description));
        return mTaskId++;
    }

    uint EvernusApplication::startTask(uint parentTask, const QString &description)
    {
        if (parentTask == TaskConstants::invalidTask)
            return startTask(description);

        QMetaObject::invokeMethod(this,
                                  "taskStarted",
                                  Qt::QueuedConnection,
                                  Q_ARG(uint, mTaskId),
                                  Q_ARG(uint, parentTask),
                                  Q_ARG(QString, description));
        return mTaskId++;
    }

    void EvernusApplication::updateTask(uint taskId, const QString &description)
    {
        QMetaObject::invokeMethod(this,
                                  "taskInfoChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(uint, taskId),
                                  Q_ARG(QString, description));
    }

    void EvernusApplication::endTask(uint taskId, const QString &error)
    {
        QMetaObject::invokeMethod(this,
                                  "taskEnded",
                                  Qt::QueuedConnection,
                                  Q_ARG(uint, taskId),
                                  Q_ARG(QString, error));
    }

    const ESIManager &EvernusApplication::getESIManager() const
    {
        Q_ASSERT(mESIManager);
        return *mESIManager;
    }

    ESIInterfaceManager &EvernusApplication::getESIInterfaceManager() noexcept
    {
        return *mESIInterfaceManager;
    }

    MarketOrderProvider &EvernusApplication::getMarketOrderProvider() const noexcept
    {
        return *mCombinedOrderProvider;
    }

    MarketOrderProvider &EvernusApplication::getCorpMarketOrderProvider() const noexcept
    {
        return *mCorpOrderProvider;
    }

    const ContractProvider &EvernusApplication::getContractProvider() const noexcept
    {
        return *mCharacterContractProvider;
    }

    const ContractProvider &EvernusApplication::getCorpContractProvider() const noexcept
    {
        return *mCorpContractProvider;
    }

    AssetProvider &EvernusApplication::getAssetProvider() const noexcept
    {
        return *mCharacterAssetProvider;
    }

    AssetProvider &EvernusApplication::getCorpAssetProvider() const noexcept
    {
        return *mCorpAssetProvider;
    }

    EveDataProvider &EvernusApplication::getDataProvider() noexcept
    {
        return *mDataProvider;
    }

    void EvernusApplication::refreshCharacters()
    {
        qDebug() << "Refreshing characters...";

        const auto characters = mCharacterRepository->fetchAll();
        for (const auto &character : characters)
        {
            Q_ASSERT(character);

            const auto task = startTask(tr("Fetching character %1...").arg(character->getId()));
            processEvents(QEventLoop::ExcludeUserInputEvents);

            importCharacter(character->getId(), task);
        }
    }

    void EvernusApplication::refreshCharacter(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing character: " << id;

        const auto charSubtask = startTask(parentTask, getCharacterImportMessage(id));

        if (!checkImportAndEndTask(id, TimerType::Character, charSubtask))
            return;

        importCharacter(id, charSubtask);
    }

    void EvernusApplication::refreshCharacterAssets(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing assets: " << id;

        const auto assetSubtask = startTask(parentTask, tr("Fetching assets for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!checkImportAndEndTask(id, TimerType::AssetList, assetSubtask))
            return;

        Q_ASSERT(mESIManager);

        markImport(id, TimerType::AssetList);
        mESIManager->fetchCharacterAssets(id, [=](auto &&assets, const auto &error, const auto &expires) {
            unmarkImport(id, TimerType::AssetList);

            if (!error.isEmpty())
            {
                emit taskEnded(assetSubtask, error);
                return;
            }

            setUtcCacheTimer(id, TimerType::AssetList, expires);
            updateCharacterAssets(id, assets);

            emit taskEnded(assetSubtask, error);
        });
    }

    void EvernusApplication::refreshCharacterContracts(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing contracts: " << id;

        const auto task = startTask(tr("Fetching contracts for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!checkImportAndEndTask(id, TimerType::Contracts, task))
            return;

        Q_ASSERT(mESIManager);

        try
        {
            markImport(id, TimerType::Contracts);
            mESIManager->fetchCharacterContracts(id, [=](auto &&data, const auto &error, const auto &expires) {
                unmarkImport(id, TimerType::Contracts);

                if (error.isEmpty())
                {
                    const auto it = std::remove_if(std::begin(data), std::end(data), [](const Contract &contract) {
                        return contract.isForCorp();
                    });

                    if (it != std::end(data))
                    {
                        Contracts corpContracts(std::make_move_iterator(it), std::make_move_iterator(std::end(data)));

                        data.erase(it, std::end(data));

                        const auto issuerCorpId = corpContracts.front().getIssuerCorpId();
                        const auto assigneeCorpId = corpContracts.front().getAssigneeId();

                        asyncBatchStore(*mCorpContractRepository, std::move(corpContracts), true, [=] {
                            mCharacterContractProvider->clearForCorporation(issuerCorpId);
                            mCorpContractProvider->clearForCorporation(issuerCorpId);
                            mCharacterContractProvider->clearForCorporation(assigneeCorpId);
                            mCorpContractProvider->clearForCorporation(assigneeCorpId);
                        });
                    }

                    asyncBatchStore(*mContractRepository, std::move(data), true, [=] {
                        mCharacterContractProvider->clearForCharacter(id);
                        mCorpContractProvider->clearForCharacter(id);

                        setUtcCacheTimer(id, TimerType::Contracts, expires);
                        saveUpdateTimer(TimerType::Contracts, mUpdateTimes[TimerType::Contracts], id);

                        handleIncomingCharacterContracts(data, id, task);
                    });
                }
                else
                {
                    emit taskEnded(task, error);
                }
            });
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            emit taskEnded(task, tr("Character not found!"));
        }
    }

    void EvernusApplication::refreshCharacterWalletJournal(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing wallet journal: " << id;

        const auto task = startTask(tr("Fetching wallet journal for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!checkImportAndEndTask(id, TimerType::WalletJournal, task))
            return;

        const auto maxId = mWalletJournalEntryRepository->getLatestEntryId(id);

        if (maxId == WalletJournalEntry::invalidId)
            emit taskInfoChanged(task, tr("Fetching wallet journal for character %1 (this may take a while)...").arg(id));

        Q_ASSERT(mESIManager);

        markImport(id, TimerType::WalletJournal);
        mESIManager->fetchCharacterWalletJournal(id, maxId, [=](auto &&data, const auto &error, const auto &expires) {
            unmarkImport(id, TimerType::WalletJournal);

            if (error.isEmpty())
            {
                setUtcCacheTimer(id, TimerType::WalletJournal, expires);
                updateCharacterWalletJournal(id, std::move(data), task);
            }
            else
            {
                emit taskEnded(task, error);
            }
        });
    }

    void EvernusApplication::refreshCharacterWalletTransactions(Character::IdType id, uint parentTask, bool force)
    {
        qDebug() << "Refreshing wallet transactions: " << id;

        const auto task = startTask(tr("Fetching wallet transactions for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!force && !checkImportAndEndTask(id, TimerType::WalletTransactions, task))
            return;

        const auto maxId = mWalletTransactionRepository->getLatestEntryId(id);

        if (maxId == WalletTransaction::invalidId)
            emit taskInfoChanged(task, tr("Fetching wallet transactions for character %1 (this may take a while)...").arg(id));

        Q_ASSERT(mESIManager);

        markImport(id, TimerType::WalletTransactions);
        mESIManager->fetchCharacterWalletTransactions(id, maxId, [=](auto &&data, const auto &error, const auto &expires) {
            unmarkImport(id, TimerType::WalletTransactions);

            if (error.isEmpty())
            {
                setUtcCacheTimer(id, TimerType::WalletTransactions, expires);
                updateCharacterWalletTransactions(id, std::move(data), task);
            }
            else
            {
                emit taskEnded(task, error);
            }
        });
    }

    void EvernusApplication::refreshCharacterMarketOrdersFromAPI(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing market orders from API: " << id;

        const auto task = startTask(tr("Fetching market orders for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!checkImportAndEndTask(id, TimerType::MarketOrders, task))
            return;

        Q_ASSERT(mESIManager);

        markImport(id, TimerType::MarketOrders);
        mESIManager->fetchCharacterMarketOrders(id, [=](auto &&data, const auto &error, const auto &expires) {
            unmarkImport(id, TimerType::MarketOrders);

            if (error.isEmpty())
            {
                setUtcCacheTimer(id, TimerType::MarketOrders, expires);
                importMarketOrders(id, data, false);

                emit characterMarketOrdersChanged();
                emit externalOrdersChangedWithMarketOrders();
            }

            emit taskEnded(task, error);
        });
    }

    void EvernusApplication::refreshCharacterMarketOrdersFromLogs(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing market orders from logs:" << id;

        const auto task = startTask(tr("Fetching market orders for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        importMarketOrdersFromLogs(id, task, false);

        emit taskEnded(task, QString{});
    }

    void EvernusApplication::refreshCharacterMiningLedger(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing mining ledger:" << id;

        const auto task = startTask(tr("Fetching mining ledger for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!checkImportAndEndTask(id, TimerType::MiningLedger, task))
            return;

        Q_ASSERT(mESIManager);

        markImport(id, TimerType::MiningLedger);
        mESIManager->fetchCharacterMiningLedger(id, [=](auto &&data, const auto &error, const auto &expires) {
            unmarkImport(id, TimerType::MiningLedger);

            if (error.isEmpty())
            {
                setUtcCacheTimer(id, TimerType::MiningLedger, expires);

                Q_ASSERT(mMiningLedgerRepository);

                // TODO: remove when https://github.com/ccpgames/esi-issues/issues/593 is done
                mMiningLedgerRepository->removeForCharacter(id);
                asyncBatchStore(*mMiningLedgerRepository, std::move(data), false, [=] {
                    saveUpdateTimer(Evernus::TimerType::MiningLedger, mUpdateTimes[Evernus::TimerType::MiningLedger], id);

                    emit characterMiningLedgerChanged();
                    emit taskEnded(task, {});
                });
            }
            else
            {
                emit taskEnded(task, error);
            }
        });
    }

    void EvernusApplication::refreshCorpAssets(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing corp assets:" << id;

        const auto assetSubtask = startTask(parentTask, tr("Fetching corporation assets for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!checkImportAndEndTask(id, TimerType::CorpAssetList, assetSubtask))
            return;

        Q_ASSERT(mESIManager);

        try
        {
            const auto corpId = mCharacterRepository->getCorporationId(id);

            markImport(id, TimerType::CorpAssetList);
            mESIManager->fetchCorporationAssets(id, corpId, [=](auto &&data, const auto &error, const auto &expires) {
                unmarkImport(id, TimerType::CorpAssetList);

                if (!error.isEmpty())
                {
                    emit taskEnded(assetSubtask, error);
                    return;
                }

                if (error.isEmpty())
                {
                    mCorpAssetListRepository->deleteForCharacter(id);
                    mCorpAssetProvider->setForCharacter(id, data);
                    mCorpAssetListRepository->store(data);

                    QSettings settings;

                    if (settings.value(ImportSettings::autoUpdateAssetValueKey, ImportSettings::autoUpdateAssetValueDefault).toBool() &&
                        settings.value(StatisticsSettings::automaticSnapshotsKey, StatisticsSettings::automaticSnapshotsKey).toBool())
                    {
                        computeCorpAssetListSellValueSnapshot(data);
                    }

                    setUtcCacheTimer(id, TimerType::CorpAssetList, expires);
                    saveUpdateTimer(Evernus::TimerType::CorpAssetList, mUpdateTimes[Evernus::TimerType::CorpAssetList], id);

                    emit corpAssetsChanged();
                }

                emit taskEnded(assetSubtask, error);
            });
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            emit taskEnded(assetSubtask, tr("Character not found!"));
        }
    }

    void EvernusApplication::refreshCorpContracts(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing corp contracts: " << id;

        const auto task = startTask(tr("Fetching corporation contracts for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!checkImportAndEndTask(id, TimerType::CorpContracts, task))
            return;

        Q_ASSERT(mESIManager);

        try
        {
            const auto corpId = mCharacterRepository->getCorporationId(id);

            markImport(id, TimerType::CorpContracts);
            mESIManager->fetchCorporationContracts(id, corpId, [=](auto &&data, const auto &error, const auto &expires) {
                unmarkImport(id, Evernus::TimerType::CorpContracts);

                if (error.isEmpty())
                {
                    asyncBatchStore(*mCorpContractRepository, data, true, [=] {
                        mCharacterContractProvider->clearForCorporation(corpId);
                        mCorpContractProvider->clearForCorporation(corpId);
                        mCharacterContractProvider->clearForCorporation(corpId);
                        mCorpContractProvider->clearForCorporation(corpId);

                        setUtcCacheTimer(id, TimerType::Contracts, expires);
                        saveUpdateTimer(TimerType::CorpContracts, mUpdateTimes[TimerType::CorpContracts], id);

                        handleIncomingCorpContracts(data, id, corpId, task);
                    });
                }
                else
                {
                    emit taskEnded(task, error);
                }
            });
        }
        catch (const CharacterRepository::NotFoundException &)
        {
        }
    }

    void EvernusApplication::refreshCorpWalletJournal(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing corp wallet journal: " << id;

        const auto task = startTask(tr("Fetching corporation wallet journal for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!checkImportAndEndTask(id, TimerType::CorpWalletJournal, task))
            return;

        Q_ASSERT(mESIManager);

        try
        {
            const auto corpId = mCharacterRepository->getCorporationId(id);
            const auto maxId = mCorpWalletJournalEntryRepository->getLatestEntryId(id);

            if (maxId == WalletJournalEntry::invalidId)
                emit taskInfoChanged(task, tr("Fetching corporation wallet journal for character %1 (this may take a while)...").arg(id));

            QSettings settings;
            const auto accountKey = settings.value(ImportSettings::corpWalletDivisionKey, ImportSettings::corpWalletDivisionDefault).toInt();

            markImport(id, TimerType::CorpWalletJournal);
            mESIManager->fetchCorporationWalletJournal(id, corpId, accountKey, maxId,
                                                       [=](auto &&data, const auto &error, const auto &expires) {
                unmarkImport(id, TimerType::CorpWalletJournal);

                if (error.isEmpty())
                {
                    QSettings settings;
                    if (settings.value(StatisticsSettings::automaticSnapshotsKey, StatisticsSettings::automaticSnapshotsDefault).toBool())
                    {
                        std::vector<CorpWalletSnapshot> snapshots;
                        snapshots.reserve(data.size());

                        QSet<QDateTime> usedSnapshots;

                        for (auto &entry : data)
                        {
                            const auto balance = entry.getBalance();

                            if (Q_UNLIKELY(!balance))
                                continue;

                            const auto timestamp = entry.getTimestamp();

                            if (!usedSnapshots.contains(timestamp))
                            {
                                CorpWalletSnapshot snapshot;
                                snapshot.setTimestamp(timestamp);
                                snapshot.setBalance(*balance);
                                snapshot.setCorporationId(entry.getCorporationId());

                                snapshots.emplace_back(std::move(snapshot));
                                usedSnapshots << timestamp;
                            }
                        }

                        asyncBatchStore(*mCorpWalletSnapshotRepository, std::move(snapshots), false);
                    }

                    asyncBatchStore(*mCorpWalletJournalEntryRepository, std::move(data), true, [=] {
                        setUtcCacheTimer(id, TimerType::CorpWalletJournal, expires);
                        saveUpdateTimer(TimerType::CorpWalletJournal, mUpdateTimes[TimerType::CorpWalletJournal], id);

                        emit corpWalletJournalChanged();
                        emit taskEnded(task, {});
                    });
                }
                else
                {
                    emit taskEnded(task, error);
                }
            });
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            emit taskEnded(task, tr("Character not found!"));
        }
    }

    void EvernusApplication::refreshCorpWalletTransactions(Character::IdType id, uint parentTask, bool force)
    {
        qDebug() << "Refreshing corp wallet transactions: " << id;

        const auto task = startTask(tr("Fetching corporation wallet transactions for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!force && !checkImportAndEndTask(id, TimerType::CorpWalletTransactions, task))
            return;

        Q_ASSERT(mESIManager);

        try
        {
            const auto maxId = mCorpWalletTransactionRepository->getLatestEntryId(id);
            const auto corpId = mCharacterRepository->getCorporationId(id);

            if (maxId == WalletTransaction::invalidId)
                emit taskInfoChanged(task, tr("Fetching corporation wallet transactions for character %1 (this may take a while)...").arg(id));

            QSettings settings;
            const auto accountKey = settings.value(ImportSettings::corpWalletDivisionKey, ImportSettings::corpWalletDivisionDefault).toInt();

            markImport(id, TimerType::CorpWalletTransactions);
            mESIManager->fetchCorporationWalletTransactions(id, corpId, accountKey, maxId,
                                                            [=](auto &&data, const auto &error, const auto &expires) {
                unmarkImport(id, TimerType::CorpWalletTransactions);

                if (error.isEmpty())
                {
                    asyncBatchStore(*mCorpWalletTransactionRepository, std::move(data), true, [=] {
                        setUtcCacheTimer(id, TimerType::CorpWalletTransactions, expires);
                        saveUpdateTimer(TimerType::CorpWalletTransactions, mUpdateTimes[TimerType::CorpWalletTransactions], id);

                        QSettings settings;
                        if (settings.value(PriceSettings::autoAddCustomItemCostKey, PriceSettings::autoAddCustomItemCostDefault).toBool() &&
                            !mPendingAutoCostOrders.empty())
                        {
                            computeAutoCosts(id,
                                             mCorpOrderProvider->getBuyOrdersForCorporation(corpId),
                                             std::bind(&WalletTransactionRepository::fetchForCorporationInRange,
                                                       mCorpWalletTransactionRepository.get(),
                                                       corpId,
                                                       std::placeholders::_1,
                                                       std::placeholders::_2,
                                                       WalletTransactionRepository::EntryType::Buy,
                                                       std::placeholders::_3));
                        }

                        emit corpWalletTransactionsChanged();
                        emit taskEnded(task, {});
                    });
                }
                else
                {
                    emit taskEnded(task, error);
                }
            });
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            emit taskEnded(task, tr("Character not found!"));
        }
    }

    void EvernusApplication::refreshCorpMarketOrdersFromAPI(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing corp market orders from API: " << id;

        const auto task = startTask(tr("Fetching corporation market orders for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (!checkImportAndEndTask(id, TimerType::CorpMarketOrders, task))
            return;

        Q_ASSERT(mESIManager);

        try
        {
            const auto corpId = mCharacterRepository->getCorporationId(id);

            markImport(id, TimerType::CorpMarketOrders);
            mESIManager->fetchCorporationMarketOrders(id, corpId, [=](auto &&data, const auto &error, const auto &expires) {
                unmarkImport(id, TimerType::CorpMarketOrders);

                if (error.isEmpty())
                {
                    setUtcCacheTimer(id, TimerType::CorpMarketOrders, expires);
                    importMarketOrders(id, data, true);

                    emit corpMarketOrdersChanged();
                    emit externalOrdersChangedWithMarketOrders();
                }

                emit taskEnded(task, error);
            });
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            emit taskEnded(task, tr("Character not found!"));
        }
    }

    void EvernusApplication::refreshCorpMarketOrdersFromLogs(Character::IdType id, uint parentTask)
    {
        qDebug() << "Refreshing corp market orders from logs: " << id;

        const auto task = startTask(tr("Fetching corporation market orders for character %1...").arg(id));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        importMarketOrdersFromLogs(id, task, true);

        emit taskEnded(task, QString{});
    }

    void EvernusApplication::refreshCitadels()
    {
        qDebug() << "Refreshing citadels...";

        const auto task = startTask(tr("Fetching citadels..."));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        mCitadelManager.fetchCitadels([=](auto &&citadels, const auto &error) {
            if (error.isEmpty())
            {
                mDataProvider->clearCitadelCache();

                asyncExecute([=, citadels = std::move(citadels)] {
                    QSettings settings;

                    mCitadelRepository->replace(std::move(citadels), settings.value(ImportSettings::clearExistingCitadelsKey, ImportSettings::clearExistingCitadelsDefault).toBool());
                    mExternalOrderRepository->fixMissingData(*mCitadelRepository);
                    emit citadelsChanged();
                });
            }

            emit taskEnded(task, error);
        });
    }

    void EvernusApplication::refreshAllExternalOrders(Character::IdType id)
    {
        TypeLocationPairs target;

        QSqlQuery query{mMainDatabaseConnectionProvider.getConnection()};
        query.prepare(QStringLiteral("SELECT DISTINCT ids.type_id, ids.location_id FROM ("
            "SELECT type_id, location_id FROM %1 WHERE state = ? "
            "UNION "
            "SELECT type_id, location_id FROM %2 WHERE state = ?"
        ") ids").arg(mMarketOrderRepository->getTableName()).arg(mCorpMarketOrderRepository->getTableName()));

        query.addBindValue(static_cast<int>(MarketOrder::State::Active));
        query.addBindValue(static_cast<int>(MarketOrder::State::Active));

        DatabaseUtils::execQuery(query);

        while (query.next())
            target.emplace(std::make_pair(query.value(0).value<EveType::IdType>(), query.value(1).toUInt()));

        QSettings settings;

        const auto source = static_cast<ImportSettings::PriceImportSource>(
            settings.value(ImportSettings::priceImportSourceKey, static_cast<int>(ImportSettings::priceImportSourceDefault)).toInt());
        switch (source) {
        case ImportSettings::PriceImportSource::Logs:
            refreshExternalOrdersFromFile(id, target);
            break;
        default:
            refreshExternalOrdersFromWeb(id, target);
        }
    }

    void EvernusApplication::refreshExternalOrdersFromWeb(Character::IdType id, const TypeLocationPairs &target)
    {
        importExternalOrders(ExternalOrderImporterNames::webImporter, id, target);
    }

    void EvernusApplication::refreshExternalOrdersFromFile(Character::IdType id, const TypeLocationPairs &target)
    {
        importExternalOrders(ExternalOrderImporterNames::logImporter, id, target);
    }

    void EvernusApplication::finishExternalOrderImport(const QString &info, const std::vector<ExternalOrder> &orders)
    {
        try
        {
            emit taskInfoChanged(mCurrentExternalOrderImportTask, tr("Saving %1 imported orders...").arg(orders.size()));

            updateExternalOrdersAndAssetValue(orders);
            finishExternalOrderImportTask(info);
        }
        catch (const std::exception &e)
        {
            finishExternalOrderImportTask(e.what());
        }
    }

    void EvernusApplication::updateExternalOrdersAndAssetValue(const std::vector<ExternalOrder> &orders)
    {
        auto watcher = new QFutureWatcher<void>{this};
        connect(watcher, &QFutureWatcher<void>::finished, this, [=] {
            QSettings settings;
            if (settings.value(ImportSettings::autoUpdateAssetValueKey, ImportSettings::autoUpdateAssetValueDefault).toBool())
            {
                const auto assets = mCharacterAssetProvider->fetchAllAssets();
                for (const auto &list : assets)
                    computeAssetListSellValueSnapshot(*list);
            }

            emit externalOrdersChanged();
        });

        watcher->setFuture(asyncExecute(std::bind(&CachingEveDataProvider::updateExternalOrders, mDataProvider.get(), orders)));
    }

    void EvernusApplication::handleNewPreferences()
    {
        QSettings settings;
        updateTranslator(settings.value(UISettings::languageKey).toString());

        setProxySettings();

        mHttpSessionManager.shutdown();
        mHttpSessionManager.setPort(settings.value(HttpSettings::portKey, HttpSettings::portDefault).value<quint16>());

        if (settings.value(HttpSettings::enabledKey, HttpSettings::enabledDefault).toBool())
            mHttpSessionManager.start();

        mCharacterItemCostCache.clear();
        mDataProvider->handleNewPreferences();

        setSmtpSettings();

        emit itemCostsChanged();
        emit itemVolumeChanged();
    }

    void EvernusApplication::importFromMentat()
    {
        const auto path = QFileDialog::getExistingDirectory(activeWindow(), tr("Select Mentat directory"));
        if (path.isEmpty())
            return;

        auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("mentat"));
        db.setDatabaseName(path + "/Storage/EVEMentat.dynamic.database");
        db.setConnectOptions("QSQLITE_OPEN_READONLY");

        if (!db.open())
        {
            QMessageBox::warning(activeWindow(), tr("Error"), tr("Error opening %1").arg(path + "/Storage/EVEMentat.dynamic.database"));
            return;
        }

        const auto task = startTask(tr("Importing order history..."));
        const auto charIds = mCharacterRepository->fetchAllIds();

        auto query = db.exec(
            QStringLiteral("SELECT * FROM mentatOrders o INNER JOIN mentatOrdersHistory h ON h.orderID = o.orderID WHERE o.orderState != 0 AND o.isCorp = 0"));

        std::vector<MarketOrder> orders;
        if (query.size() > 0)
            orders.reserve(query.size());

        while (query.next())
        {
            const auto charId = query.value(QStringLiteral("charID")).value<Character::IdType>();
            if (charIds.find(charId) == std::end(charIds))
                continue;

            auto issued = query.value(QStringLiteral("issued")).toDateTime();
            issued.setTimeSpec(Qt::UTC);

            auto lastSeen = query.value(QStringLiteral("referenceTime")).toDateTime();
            lastSeen.setTimeSpec(Qt::UTC);

            auto intState = query.value(QStringLiteral("orderState")).toInt();
            if (intState < static_cast<int>(MarketOrder::State::Active) || intState > static_cast<int>(MarketOrder::State::CharacterDeleted))
                intState = static_cast<int>(MarketOrder::State::Fulfilled);

            MarketOrder order{query.value(QStringLiteral("orderID")).value<MarketOrder::IdType>()};
            order.setAccountKey(query.value(QStringLiteral("accountID")).value<short>());
            order.setCharacterId(charId);
            order.setDuration(query.value(QStringLiteral("duration")).value<short>());
            order.setEscrow(query.value(QStringLiteral("escrow")).toDouble() / 100.);
            order.setFirstSeen(issued);
            order.setIssued(issued);
            order.setLastSeen(lastSeen);
            order.setStationId(query.value(QStringLiteral("stationID")).toULongLong());
            order.setMinVolume(query.value(QStringLiteral("minVolume")).toUInt());
            order.setPrice(query.value(QStringLiteral("price")).toDouble() / 100.);
            order.setRange(query.value(QStringLiteral("range")).value<short>());
            order.setState(static_cast<MarketOrder::State>(intState));
            order.setType((query.value(QStringLiteral("bid")).toBool()) ? (MarketOrder::Type::Buy) : (MarketOrder::Type::Sell));
            order.setTypeId(query.value(QStringLiteral("typeID")).value<EveType::IdType>());
            order.setVolumeEntered(query.value(QStringLiteral("volEntered")).toUInt());
            order.setVolumeRemaining(query.value(QStringLiteral("volRemaining")).toUInt());

            orders.emplace_back(std::move(order));

            emit taskInfoChanged(task, tr("Importing order history: %1 processed").arg(orders.size()));

            processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        emit taskInfoChanged(task, tr("Importing order history: storing %1 orders (this may take a while)").arg(orders.size()));

        try
        {
            asyncBatchStore(*mMarketOrderRepository, std::move(orders), true, [=] {
                emit taskEnded(task, {});

                mCharacterOrderProvider->clearArchived();
                emit characterMarketOrdersChanged();
            });
        }
        catch (const std::exception &e)
        {
            emit taskEnded(task, e.what());

            mCharacterOrderProvider->clearArchived();
            emit characterMarketOrdersChanged();
        }
    }

    void EvernusApplication::syncLMeve(Character::IdType id)
    {
        qDebug() << "Performing LMeve sync:" << id;

        const auto task = startTask(tr("Synchronizing with LMeve..."));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        if (getLocalCacheTimer(id, TimerType::LMeveTasks) > QDateTime::currentDateTime())
        {
            emit taskEnded(task, QString{});
            return;
        }

        mLMeveAPIManager.fetchTasks(id, [id, task, this](auto &&list, const auto &error) {
            if (error.isEmpty())
            {
                mLMeveTaskCache.erase(id);
                mLMeveTaskRepository->removeForCharacter(id);

                asyncBatchStore(*mLMeveTaskRepository, std::move(list), true, [=] {
                    setUtcCacheTimer(id, Evernus::TimerType::LMeveTasks, QDateTime::currentDateTimeUtc().addSecs(3600));

                    emit lMeveTasksChanged();
                    emit taskEnded(task, {});
                });
            }
            else
            {
                emit taskEnded(task, error);
            }
        });
    }

    void EvernusApplication::clearCorpWalletData()
    {
        mCorpWalletJournalEntryRepository->deleteAll();
        mCorpWalletTransactionRepository->deleteAll();

        emit corpWalletJournalChanged();
        emit corpWalletTransactionsChanged();
    }

    void EvernusApplication::clearCitadelCache()
    {
        mDataProvider->clearCitadelCache();
        emit citadelsChanged();
    }

    void EvernusApplication::clearRefreshTokens()
    {
        SSOUtils::clearRefreshTokens();
        mESIInterfaceManager->clearRefreshTokens();
    }

    void EvernusApplication::makeValueSnapshots(Character::IdType id)
    {
        qDebug() << "Making all value snapshots for" << id;

        const auto time = QDateTime::currentDateTimeUtc();

        auto assets = mCharacterAssetProvider->fetchAssetsForCharacter(id);
        if (assets)
            computeAssetListSellValueSnapshot(*assets);

        assets = mCorpAssetProvider->fetchAssetsForCharacter(id);
        if (assets)
            computeCorpAssetListSellValueSnapshot(*assets);

        const auto charData = mCharacterRepository->find(id);
        if (charData)
            createWalletSnapshot(id, charData->getISK());

        auto orderData = mMarketOrderRepository->getAggregatedData(id);

        MarketOrderValueSnapshot orderSnapshot;
        orderSnapshot.setTimestamp(time);
        orderSnapshot.setCharacterId(id);
        orderSnapshot.setBuyValue(orderData.mBuyData.mPriceSum);
        orderSnapshot.setSellValue(orderData.mSellData.mPriceSum);

        mMarketOrderValueSnapshotRepository->store(orderSnapshot);

        orderData = mCorpMarketOrderRepository->getAggregatedData(id);

        CorpMarketOrderValueSnapshot corpOrderSnapshot;
        corpOrderSnapshot.setTimestamp(time);
        corpOrderSnapshot.setCorporationId(mCharacterRepository->getCorporationId(id));
        corpOrderSnapshot.setBuyValue(orderData.mBuyData.mPriceSum);
        corpOrderSnapshot.setSellValue(orderData.mSellData.mPriceSum);

        mCorpMarketOrderValueSnapshotRepository->store(corpOrderSnapshot);

        emit snapshotsTaken();
    }

    void EvernusApplication::showInEve(EveType::IdType typeId, Character::IdType charId)
    {
        Q_ASSERT(mESIManager);
        mESIManager->openMarketDetails(typeId, charId);
    }

    void EvernusApplication::setDestinationInEve(quint64 locationId, Character::IdType charId)
    {
        Q_ASSERT(mESIManager);
        mESIManager->setDestination(locationId, charId);
    }

    void EvernusApplication::processSSOAuthorizationCode(Character::IdType charId, const QByteArray &code)
    {
        Q_ASSERT(mESIInterfaceManager);
        mESIInterfaceManager->processSSOAuthorizationCode(charId, code);
    }

    void EvernusApplication::cancelSsoAuth(Character::IdType charId)
    {
        Q_ASSERT(mESIInterfaceManager);
        mESIInterfaceManager->cancelSsoAuth(charId);
    }

    void EvernusApplication::processNewCharacter(Character::IdType id, const QString &accessToken, const QString &refreshToken)
    {
        Q_ASSERT(mESIInterfaceManager);
        mESIInterfaceManager->setTokens(id, accessToken, refreshToken);

        refreshCharacter(id, TaskConstants::invalidTask);
    }

    void EvernusApplication::scheduleCharacterUpdate()
    {
        if (mCharacterUpdateScheduled)
            return;

        mCharacterUpdateScheduled = true;
        QMetaObject::invokeMethod(this, "updateCharacters", Qt::QueuedConnection);
    }

    void EvernusApplication::updateCharacters()
    {
        mCharacterUpdateScheduled = false;
        emit charactersChanged();
    }

    void EvernusApplication::scheduleMarketOrderChange()
    {
        if (mMarketOrderUpdateScheduled)
            return;

        mMarketOrderUpdateScheduled = true;
        QMetaObject::invokeMethod(this, "updateCharacterMarketOrders", Qt::QueuedConnection);
    }

    void EvernusApplication::updateCharacterMarketOrders()
    {
        mMarketOrderUpdateScheduled = false;
        emit characterMarketOrdersChanged();
    }

    void EvernusApplication::scheduleCorpMarketOrderChange()
    {
        if (mCorpMarketOrderUpdateScheduled)
            return;

        mCorpMarketOrderUpdateScheduled = true;
        QMetaObject::invokeMethod(this, "updateCorpMarketOrders", Qt::QueuedConnection);
    }

    void EvernusApplication::updateCorpMarketOrders()
    {
        mCorpMarketOrderUpdateScheduled = false;
        emit corpMarketOrdersChanged();
    }

    void EvernusApplication::showPriceImportStatus(const QString &info)
    {
        Q_ASSERT(mCurrentExternalOrderImportTask != TaskConstants::invalidTask);
        emit taskInfoChanged(mCurrentExternalOrderImportTask, info);
    }

    void EvernusApplication::emitNewItemCosts()
    {
        mItemCostUpdateScheduled = false;
        emit itemCostsChanged();
    }

    void EvernusApplication::showSmtpError(const QByteArray &message)
    {
        QMessageBox::warning(activeWindow(), tr("SMTP Error"), tr("Error sending email: %1").arg(QString{message}));
    }

    void EvernusApplication::showMailError(int mailID, int errorCode, const QByteArray &message)
    {
        Q_UNUSED(mailID);
        Q_UNUSED(errorCode);

        QMessageBox::warning(activeWindow(), tr("Mail Error"), tr("Error sending email: %1").arg(QString{message}));
    }

    void EvernusApplication::updateTranslator(const QString &lang)
    {
        qDebug() << "Switching language to" << lang;

        const QLocale locale{lang};
        QLocale::setDefault(locale);

        removeTranslator(&mTranslator);
        removeTranslator(&mQtTranslator);

        mTranslator.load(locale, QStringLiteral("lang"), QStringLiteral("_"), applicationDirPath() + UISettings::translationPath);
        mQtTranslator.load(locale, QStringLiteral("qt"), QStringLiteral("_"), applicationDirPath() + UISettings::translationPath);
        mQtBaseTranslator.load(locale, QStringLiteral("qtbase"), QStringLiteral("_"), applicationDirPath() + UISettings::translationPath);
        mQtScriptTranslator.load(locale, QStringLiteral("qtscript"), QStringLiteral("_"), applicationDirPath() + UISettings::translationPath);

        installTranslator(&mQtScriptTranslator);
        installTranslator(&mQtBaseTranslator);
        installTranslator(&mQtTranslator);
        installTranslator(&mTranslator);
    }

    void EvernusApplication::createDb()
    {
        if (!QDir{}.mkpath(DatabaseUtils::getDbPath()))
            BOOST_THROW_EXCEPTION(std::runtime_error{QCoreApplication::translate("DatabaseUtils", "Error creating DB path!").toStdString()});

        mCharacterRepository.reset(new CharacterRepository{mMainDatabaseConnectionProvider});
        mItemRepository.reset(new ItemRepository{false, mMainDatabaseConnectionProvider});
        mCorpItemRepository.reset(new ItemRepository{true, mMainDatabaseConnectionProvider});
        mAssetListRepository.reset(new AssetListRepository{false, mMainDatabaseConnectionProvider, *mItemRepository});
        mCorpAssetListRepository.reset(new AssetListRepository{true, mMainDatabaseConnectionProvider, *mCorpItemRepository});
        mWalletSnapshotRepository.reset(new WalletSnapshotRepository{mMainDatabaseConnectionProvider});
        mCorpWalletSnapshotRepository.reset(new CorpWalletSnapshotRepository{mMainDatabaseConnectionProvider});
        mExternalOrderRepository.reset(new ExternalOrderRepository{mMainDatabaseConnectionProvider});
        mAssetValueSnapshotRepository.reset(new AssetValueSnapshotRepository{mMainDatabaseConnectionProvider});
        mCorpAssetValueSnapshotRepository.reset(new CorpAssetValueSnapshotRepository{mMainDatabaseConnectionProvider});
        mWalletJournalEntryRepository.reset(new WalletJournalEntryRepository{false, mMainDatabaseConnectionProvider});
        mCorpWalletJournalEntryRepository.reset(new WalletJournalEntryRepository{true, mMainDatabaseConnectionProvider});
        mCacheTimerRepository.reset(new CacheTimerRepository{mMainDatabaseConnectionProvider});
        mUpdateTimerRepository.reset(new UpdateTimerRepository{mMainDatabaseConnectionProvider});
        mWalletTransactionRepository.reset(new WalletTransactionRepository{false, mMainDatabaseConnectionProvider});
        mCorpWalletTransactionRepository.reset(new WalletTransactionRepository{true, mMainDatabaseConnectionProvider});
        mMarketOrderRepository.reset(new MarketOrderRepository{false, mMainDatabaseConnectionProvider});
        mCorpMarketOrderRepository.reset(new MarketOrderRepository{true, mMainDatabaseConnectionProvider});
        mItemCostRepository.reset(new ItemCostRepository{mMainDatabaseConnectionProvider});
        mMarketOrderValueSnapshotRepository.reset(new MarketOrderValueSnapshotRepository{mMainDatabaseConnectionProvider});
        mCorpMarketOrderValueSnapshotRepository.reset(new CorpMarketOrderValueSnapshotRepository{mMainDatabaseConnectionProvider});
        mFilterTextRepository.reset(new FilterTextRepository{mMainDatabaseConnectionProvider});
        mOrderScriptRepository.reset(new OrderScriptRepository{mMainDatabaseConnectionProvider});
        mFavoriteItemRepository.reset(new FavoriteItemRepository{mMainDatabaseConnectionProvider});
        mLocationBookmarkRepository.reset(new LocationBookmarkRepository{mMainDatabaseConnectionProvider});
        mContractItemRepository.reset(new ContractItemRepository{false, mMainDatabaseConnectionProvider});
        mCorpContractItemRepository.reset(new ContractItemRepository{true, mMainDatabaseConnectionProvider});
        mContractRepository.reset(new ContractRepository{*mContractItemRepository, false, mMainDatabaseConnectionProvider});
        mCorpContractRepository.reset(new ContractRepository{*mCorpContractItemRepository, true, mMainDatabaseConnectionProvider});
        mLMeveTaskRepository.reset(new LMeveTaskRepository{mMainDatabaseConnectionProvider});
        mEveTypeRepository.reset(new EveTypeRepository{mEveDatabaseConnectionProvider});
        mMarketGroupRepository.reset(new MarketGroupRepository{mEveDatabaseConnectionProvider});
        mMetaGroupRepository.reset(new MetaGroupRepository{mEveDatabaseConnectionProvider});
        mCitadelRepository.reset(new CitadelRepository{mMainDatabaseConnectionProvider});
        mRegionTypePresetRepository.reset(new RegionTypePresetRepository{mMainDatabaseConnectionProvider});
        mRegionStationPresetRepository.reset(new RegionStationPresetRepository{mMainDatabaseConnectionProvider});
        mIndustryManufacturingSetupRepository.reset(new IndustryManufacturingSetupRepository{mMainDatabaseConnectionProvider});
        mMiningLedgerRepository.reset(new MiningLedgerRepository{mMainDatabaseConnectionProvider});
    }

    void EvernusApplication::createDbSchema()
    {
        mCharacterRepository->create();
        mAssetListRepository->create(*mCharacterRepository);
        mCorpAssetListRepository->create(*mCharacterRepository);
        mItemRepository->create(*mAssetListRepository);
        mCorpItemRepository->create(*mCorpAssetListRepository);
        mWalletSnapshotRepository->create(*mCharacterRepository);
        mCorpWalletSnapshotRepository->create();
        mAssetValueSnapshotRepository->create(*mCharacterRepository);
        mCorpAssetValueSnapshotRepository->create();
        mWalletJournalEntryRepository->create(*mCharacterRepository);
        mCorpWalletJournalEntryRepository->create(*mCharacterRepository);
        mExternalOrderRepository->create();
        mCacheTimerRepository->create(*mCharacterRepository);
        mUpdateTimerRepository->create(*mCharacterRepository);
        mWalletTransactionRepository->create(*mCharacterRepository);
        mCorpWalletTransactionRepository->create(*mCharacterRepository);
        mMarketOrderRepository->create(*mCharacterRepository);
        mCorpMarketOrderRepository->create(*mCharacterRepository);
        mItemCostRepository->create(*mCharacterRepository);
        mMarketOrderValueSnapshotRepository->create(*mCharacterRepository);
        mCorpMarketOrderValueSnapshotRepository->create();
        mFilterTextRepository->create();
        mOrderScriptRepository->create();
        mFavoriteItemRepository->create();
        mLocationBookmarkRepository->create();
        mContractRepository->create();
        mCorpContractRepository->create();
        mContractItemRepository->create(*mContractRepository);
        mCorpContractItemRepository->create(*mCorpContractRepository);
        mLMeveTaskRepository->create(*mCharacterRepository);
        mCitadelRepository->create();
        mRegionTypePresetRepository->create();
        mRegionStationPresetRepository->create();
        mIndustryManufacturingSetupRepository->create();
        mMiningLedgerRepository->create(*mCharacterRepository);
    }

    void EvernusApplication::precacheCacheTimers()
    {
        const auto timers = mCacheTimerRepository->fetchAll();
        for (const auto &timer : timers)
            mCacheTimes[timer->getType()][timer->getCharacterId()] = timer->getCacheUntil();
    }

    void EvernusApplication::precacheUpdateTimers()
    {
        const auto timers = mUpdateTimerRepository->fetchAll();
        for (const auto &timer : timers)
            mUpdateTimes[timer->getType()][timer->getCharacterId()] = timer->getUpdateTime();
    }

    void EvernusApplication::deleteOldWalletEntries()
    {
        QSettings settings;

        if (settings.value(WalletSettings::deleteOldJournalKey, WalletSettings::deleteOldJournalDefault).toBool())
        {
            const auto journalDt = QDateTime::currentDateTimeUtc().addDays(
                -settings.value(WalletSettings::oldJournalDaysKey, WalletSettings::oldJournalDaysDefault).toInt());
            mWalletJournalEntryRepository->deleteOldEntries(journalDt);
        }

        if (settings.value(WalletSettings::deleteOldTransactionsKey, WalletSettings::deleteOldTransactionsDefault).toBool())
        {
            const auto transactionDt = QDateTime::currentDateTimeUtc().addDays(
                -settings.value(WalletSettings::oldJournalDaysKey, WalletSettings::oldJournalDaysDefault).toInt());
            mWalletTransactionRepository->deleteOldEntries(transactionDt);
        }
    }

    void EvernusApplication::deleteOldMarketOrders()
    {
        QSettings settings;
        if (settings.value(OrderSettings::deleteOldMarketOrdersKey, OrderSettings::deleteOldMarketOrdersDefault).toBool())
        {
            const auto oldDt = QDateTime::currentDateTimeUtc().addDays(
                -settings.value(OrderSettings::oldMarketOrderDaysKey, OrderSettings::oldMarketOrderDaysDefault).toInt());
            mMarketOrderRepository->deleteOldEntries(oldDt);
            mCorpMarketOrderRepository->deleteOldEntries(oldDt);
        }
    }

    void EvernusApplication::importCharacter(Character::IdType id, uint task)
    {
        Q_ASSERT(mESIManager);

        markImport(id, TimerType::Character);
        mESIManager->fetchCharacter(id, [=](auto &&data, const auto &error, const auto &expires) {
            Q_UNUSED(expires);

            unmarkImport(id, TimerType::Character);

            if (error.isEmpty())
                updateCharacter(data);

            emit taskEnded(task, error);
        });
    }

    void EvernusApplication::importExternalOrders(const std::string &importerName, Character::IdType id, const TypeLocationPairs &target)
    {
        if (mCurrentExternalOrderImportTask != TaskConstants::invalidTask)
            return;

        qDebug() << "Refreshing item prices using importer:" << importerName.c_str();

        const auto it = mExternalOrderImporters.find(importerName);
        Q_ASSERT(it != std::end(mExternalOrderImporters));

        mCurrentExternalOrderImportTask = startTask(tr("Importing item prices..."));
        processEvents(QEventLoop::ExcludeUserInputEvents);

        it->second->fetchExternalOrders(id, target);
    }

    void EvernusApplication::importMarketOrdersFromLogs(Character::IdType id, uint task, bool corp)
    {
        const auto logPath = PathUtils::getMarketLogsPath();
        if (logPath.isEmpty())
        {
            emit taskEnded(task, tr("Cannot determine market logs path!"));
            return;
        }

        QSettings settings;

        const auto wildcard = (corp) ?
                              (settings.value(PathSettings::corporationLogWildcardKey, PathSettings::corporationLogWildcardDefault).toString()) :
                              (settings.value(PathSettings::characterLogWildcardKey, PathSettings::characterLogWildcardDefault).toString());

        const QDir logDir{logPath};
        const auto logs = logDir.entryList(QStringList{wildcard}, QDir::Files | QDir::Readable, QDir::Time);
        if (logs.isEmpty())
        {
            emit taskEnded(task, tr("No market logs found!"));
            return;
        }

        for (const auto &log : logs)
        {
            qDebug() << "Parsing" << log;

            QFile file{logDir.filePath(log)};
            if (!file.open(QIODevice::ReadOnly))
            {
                emit taskEnded(task, tr("Could not open market log file!"));
                return;
            }

            file.readLine();

            auto characterFound = false;

            MarketOrders charOrders, corpOrders;
            while (!file.atEnd())
            {
                const QString line = file.readLine();
                const auto values = line.split(',');

                if (values.size() >= 24)
                {
                    const auto idColumn = 0;
                    const auto typeIdColumn = 1;
                    const auto characterIdColumn = 2;
                    const auto stationIdColumn = 8;
                    const auto rangeColumn = 10;
                    const auto typeColumn = 11;
                    const auto priceColumn = 12;
                    const auto volumeEnteredColumn = 13;
                    const auto volumeRemainingColumn = 14;
                    const auto issuedColumn = 16;
                    const auto stateColumn = 17;
                    const auto minVolumeColumn = 15;
                    const auto durationColumn = 18;
                    const auto isCorpColumn = 20;
                    const auto escrowColumn = 19;

                    const auto characterId = values[characterIdColumn].toULongLong();
                    if (!characterFound && !corp)
                    {
                        if (characterId != id)
                        {
                            file.close();
                            continue;
                        }

                        characterFound = true;
                    }

                    MarketOrder order{values[idColumn].toULongLong()};
                    order.setCharacterId(characterId);
                    order.setStationId(values[stationIdColumn].toULongLong());
                    order.setVolumeEntered(values[volumeEnteredColumn].toUInt());
                    order.setVolumeRemaining(static_cast<uint>(values[volumeRemainingColumn].toDouble()));
                    order.setMinVolume(values[minVolumeColumn].toUInt());
                    order.setDelta(order.getVolumeRemaining() - order.getVolumeEntered());
                    order.setState(static_cast<MarketOrder::State>(values[stateColumn].toInt()));
                    order.setTypeId(values[typeIdColumn].toUInt());
                    order.setRange(values[rangeColumn].toShort());
                    order.setDuration(values[durationColumn].toShort());
                    order.setEscrow(values[escrowColumn].toDouble());
                    order.setPrice(values[priceColumn].toDouble());
                    order.setType((values[typeColumn] == "True") ? (MarketOrder::Type::Buy) : (MarketOrder::Type::Sell));

                    auto issued = QDateTime::fromString(values[issuedColumn], QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
                    if (!issued.isValid())
                    {
                        issued = QDateTime::fromString(values[issuedColumn], QStringLiteral("yyyy-MM-dd"));
                        if (!issued.isValid())
                        {
                            // thank CCP
                            issued = QDateTime::currentDateTimeUtc();
                        }
                    }
                    issued.setTimeSpec(Qt::UTC);

                    order.setIssued(issued);
                    order.setFirstSeen(issued);

                    if (values[isCorpColumn] == "True")
                        corpOrders.emplace_back(order);
                    if (!corp && characterId == id)
                        charOrders.emplace_back(std::move(order));
                }
            }

            if (characterFound || corp)
            {
                if (settings.value(PathSettings::deleteLogsKey, PathSettings::deleteLogsDefault).toBool())
                    file.remove();

                if (!charOrders.empty())
                {
                    importMarketOrders(id, charOrders, false);
                    emit characterMarketOrdersChanged();
                }

                if (!corpOrders.empty())
                {
                    importMarketOrders(id, corpOrders, true);
                    emit corpMarketOrdersChanged();
                }

                emit externalOrdersChangedWithMarketOrders();
                break;
            }
        }
    }

    void EvernusApplication::importMarketOrders(Character::IdType id, MarketOrders &orders, bool corp)
    {
        try
        {
            const auto corpId = mCharacterRepository->getCorporationId(id);

            const auto &orderRepo = (corp) ? (*mCorpMarketOrderRepository) : (*mMarketOrderRepository);
            auto curStates = orderRepo.getOrderStates(id);

            if (corp)
            {
                mCorpOrderProvider->clearOrdersForCharacter(id);
                mCorpOrderProvider->clearOrdersForCorporation(corpId);
            }
            else
            {
                mCharacterOrderProvider->clearOrdersForCharacter(id);
                mCharacterOrderProvider->clearOrdersForCorporation(corpId);
            }

            mCombinedOrderProvider->clearOrdersForCharacter(id);

            QSettings settings;
            const auto autoSetCosts = settings.value(PriceSettings::autoAddCustomItemCostKey, PriceSettings::autoAddCustomItemCostDefault).toBool();
            const auto makeSnapshot = settings.value(StatisticsSettings::automaticSnapshotsKey, StatisticsSettings::automaticSnapshotsDefault).toBool();
            const auto makeCorpSnapshot = makeSnapshot && settings.value(ImportSettings::makeCorpSnapshotsKey, ImportSettings::makeCorpSnapshotsDefault).toBool();
            const auto emailNotification = settings.value(ImportSettings::autoImportEnabledKey, ImportSettings::autoImportEnabledDefault).toBool() &&
                                           settings.value(ImportSettings::emailNotificationsEnabledKey, ImportSettings::emailNotificationsEnabledDefault).toBool();
            const auto defaultCustomStation = settings.value(OrderSettings::defaultCustomStationKey).toUInt();

            struct EmailOrderInfo
            {
                EveType::IdType mTypeId;
                uint mVolumeEntered;
                MarketOrder::State mState;
            };

            std::vector<EmailOrderInfo> emailOrders;

            const auto curDt = QDateTime::currentDateTimeUtc();

            for (auto &order : orders)
            {
                const auto cIt = curStates.find(order.getId());
                if (cIt != std::end(curStates))
                {
                    order.setDelta(order.getVolumeRemaining() - cIt->second.mVolumeRemaining);
                    order.setFirstSeen(cIt->second.mFirstSeen);
                    order.setCustomStationId(cIt->second.mCustomStation);
                    order.setNotes(cIt->second.mNotes);
                    order.setColorTag(cIt->second.mColorTag);

                    if (order.getState() != MarketOrder::State::Active)
                    {
                        if (!cIt->second.mLastSeen.isNull())
                        {
                            order.setLastSeen(cIt->second.mLastSeen);
                        }
                        else
                        {
                            order.setLastSeen(std::min(curDt, order.getIssued().addDays(order.getDuration())));

                            if (emailNotification)
                            {
                                EmailOrderInfo info;
                                info.mTypeId = order.getTypeId();
                                info.mVolumeEntered = order.getVolumeEntered();
                                info.mState = order.getState();

                                emailOrders.emplace_back(std::move(info));
                            }
                        }
                    }

                    curStates.erase(cIt);
                }
                else
                {
                    order.setDelta(order.getVolumeRemaining() - order.getVolumeEntered());
                    if (defaultCustomStation != 0)
                        order.setCustomStationId(defaultCustomStation);
                }

                order.setCorporationId(corpId); // TODO: move up after 0.5

                if (autoSetCosts && order.getType() == MarketOrder::Type::Buy && order.getDelta() != 0 && order.getState() == MarketOrder::State::Fulfilled)
                    mPendingAutoCostOrders.emplace(order.getId());
            }

            std::vector<MarketOrder::IdType> toArchive, toFulfill;
            toArchive.reserve(curStates.size());
            toFulfill.reserve(curStates.size());

            for (const auto &order : curStates)
            {
                if (order.second.mLastSeen.isNull() || order.second.mDelta != 0)
                {
                    if (order.second.mExpiry < curDt)
                    {
                        toArchive.emplace_back(order.first);
                    }
                    else
                    {
                        toFulfill.emplace_back(order.first);
                        mPendingAutoCostOrders.emplace(order.first);
                    }
                }
            }

            if (!toArchive.empty())
                orderRepo.archive(toArchive);
            if (!toFulfill.empty())
                orderRepo.fulfill(toFulfill);

            auto future = asyncBatchStore(orderRepo, orders, true);

            if (!corp)
            {
                if (makeSnapshot)
                {
                    MarketOrderValueSnapshot snapshot;
                    snapshot.setTimestamp(QDateTime::currentDateTimeUtc());
                    snapshot.setCharacterId(id);

                    double buy = 0., sell = 0.;
                    for (const auto &order : orders)
                    {
                        if (order.getState() != MarketOrder::State::Active)
                            continue;

                        if (order.getType() == MarketOrder::Type::Buy)
                            buy += order.getEscrow();
                        else
                            sell += order.getPrice() * order.getVolumeRemaining();
                    }

                    snapshot.setBuyValue(buy);
                    snapshot.setSellValue(sell);

                    mMarketOrderValueSnapshotRepository->store(snapshot);
                }
            }
            else if (makeCorpSnapshot)
            {
                CorpMarketOrderValueSnapshot snapshot;
                snapshot.setTimestamp(QDateTime::currentDateTimeUtc());
                snapshot.setCorporationId(corpId);

                double buy = 0., sell = 0.;
                for (const auto &order : orders)
                {
                    if (order.getState() != MarketOrder::State::Active)
                        continue;

                    if (order.getType() == MarketOrder::Type::Buy)
                        buy += order.getEscrow();
                    else
                        sell += order.getPrice() * order.getVolumeRemaining();
                }

                snapshot.setBuyValue(buy);
                snapshot.setSellValue(sell);

                mCorpMarketOrderValueSnapshotRepository->store(snapshot);
            }

            mDataProvider->clearExternalOrderCaches();

            if (corp)
                saveUpdateTimer(TimerType::CorpMarketOrders, mUpdateTimes[TimerType::CorpMarketOrders], id);
            else
                saveUpdateTimer(TimerType::MarketOrders, mUpdateTimes[TimerType::MarketOrders], id);

            if (emailNotification && !emailOrders.empty())
            {
                try
                {
                    if (static_cast<ImportSettings::SmtpConnectionSecurity>(settings.value(
                        ImportSettings::smtpConnectionSecurityKey).toInt()) == ImportSettings::SmtpConnectionSecurity::TLS)
                    {
                        mSmtp.connectToSecureHost(settings.value(ImportSettings::smtpHostKey, ImportSettings::smtpHostDefault).toString());
                    }
                    else
                    {
                        mSmtp.connectToHost(settings.value(ImportSettings::smtpHostKey, ImportSettings::smtpHostDefault).toString());
                    }

                    QxtMailMessage message{tr("Evernus"), settings.value(ImportSettings::emailNotificationAddressKey).toString()};
                    message.setSubject(tr("[Evernus] Market orders fulfilled"));

                    QLocale locale;

                    QString body{tr("The following orders have changed their status:\n\n")};
                    for (const auto &order : emailOrders)
                    {
                        body.append(tr("    %1 x%2 [%3]\n")
                            .arg(mDataProvider->getTypeName(order.mTypeId))
                            .arg(locale.toString(order.mVolumeEntered))
                            .arg(MarketOrder::stateToString(order.mState)));
                    }

                    message.setBody(body);

                    mSmtp.send(message);
                }
                catch (...)
                {
                    mSmtp.disconnectFromHost();
                    throw;
                }
            }

            future.waitForFinished();

            if (autoSetCosts)
            {
                if (corp)
                    refreshCorpWalletTransactions(id, TaskConstants::invalidTask, true);
                else
                    refreshCharacterWalletTransactions(id, TaskConstants::invalidTask, true);
            }

            if (settings.value(PriceSettings::refreshPricesWithOrdersKey).toBool())
                refreshAllExternalOrders(id);
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            QMessageBox::warning(activeWindow(), tr("Evernus"), tr("Couldn't find character for order import!"));
        }
    }

    void EvernusApplication::finishExternalOrderImportTask(const QString &info)
    {
         const auto task = mCurrentExternalOrderImportTask;
         mCurrentExternalOrderImportTask = TaskConstants::invalidTask;

         emit taskEnded(task, info);
    }

    void EvernusApplication::computeAssetListSellValueSnapshot(const AssetList &list) const
    {
        try
        {
            AssetValueSnapshot snapshot;
            snapshot.setTimestamp(QDateTime::currentDateTimeUtc());
            snapshot.setBalance(getTotalAssetListValue(list));
            snapshot.setCharacterId(list.getCharacterId());

            mAssetValueSnapshotRepository->store(snapshot);
        }
        catch (const ExternalOrderRepository::NotFoundException &)
        {
        }
    }

    void EvernusApplication::computeCorpAssetListSellValueSnapshot(const AssetList &list) const
    {
        try
        {
            CorpAssetValueSnapshot snapshot;
            snapshot.setTimestamp(QDateTime::currentDateTimeUtc());
            snapshot.setBalance(getTotalAssetListValue(list));
            snapshot.setCorporationId(mCharacterRepository->getCorporationId(list.getCharacterId()));

            mCorpAssetValueSnapshotRepository->store(snapshot);
        }
        catch (const ExternalOrderRepository::NotFoundException &)
        {
        }
        catch (const CharacterRepository::NotFoundException &)
        {
        }
    }

    void EvernusApplication::updateCharacterAssets(Character::IdType id, AssetList &list)
    {
        mItemRepository->fillCustomValues(list);
        mAssetListRepository->deleteForCharacter(id);
        mCharacterAssetProvider->setForCharacter(id, list);
        mAssetListRepository->store(list);

        QSettings settings;

        if (settings.value(Evernus::ImportSettings::autoUpdateAssetValueKey, Evernus::ImportSettings::autoUpdateAssetValueDefault).toBool() &&
            settings.value(StatisticsSettings::automaticSnapshotsKey, StatisticsSettings::automaticSnapshotsKey).toBool())
        {
            computeAssetListSellValueSnapshot(list);
        }

        saveUpdateTimer(TimerType::AssetList, mUpdateTimes[TimerType::AssetList], id);

        emit characterAssetsChanged();
    }

    void EvernusApplication::updateCharacter(Character &character)
    {
        QSettings settings;

        const auto charId = character.getId();

        try
        {
            const auto prevData = mCharacterRepository->find(charId);
            Q_ASSERT(prevData);

            if (!settings.value(ImportSettings::importSkillsKey, ImportSettings::importSkillsDefault).toBool())
            {
                character.setOrderAmountSkills(prevData->getOrderAmountSkills());
                character.setTradeRangeSkills(prevData->getTradeRangeSkills());
                character.setFeeSkills(prevData->getFeeSkills());
                character.setContractSkills(prevData->getContractSkills());
            }

            character.setCorpStanding(prevData->getCorpStanding());
            character.setFactionStanding(prevData->getFactionStanding());
            character.setEnabled(prevData->isEnabled());
            character.setBuyBrokersFee(prevData->getBuyBrokersFee());
            character.setSellBrokersFee(prevData->getSellBrokersFee());
        }
        catch (const CharacterRepository::NotFoundException &)
        {
        }

        const auto db = mMainDatabaseConnectionProvider.getConnection();

        db.exec(QStringLiteral("PRAGMA foreign_keys = OFF;"));
        mCharacterRepository->store(character);
        db.exec(QStringLiteral("PRAGMA foreign_keys = ON;"));

        if (settings.value(StatisticsSettings::automaticSnapshotsKey, StatisticsSettings::automaticSnapshotsDefault).toBool())
            createWalletSnapshot(charId, character.getISK());

        saveUpdateTimer(TimerType::Character, mUpdateTimes[TimerType::Character], charId);

        QMetaObject::invokeMethod(this, "scheduleCharacterUpdate", Qt::QueuedConnection);

        const auto cacheTimer = mCacheTimes[TimerType::Character][charId];
        if (cacheTimer.isValid())
        {
            Evernus::CacheTimer timer;
            timer.setCharacterId(charId);
            timer.setType(TimerType::Character);
            timer.setCacheUntil(cacheTimer);

            mCacheTimerRepository->store(timer);
        }
    }

    void EvernusApplication::updateCharacterWalletJournal(Character::IdType id, WalletJournal data, uint task)
    {
        QSettings settings;
        if (settings.value(StatisticsSettings::automaticSnapshotsKey, StatisticsSettings::automaticSnapshotsDefault).toBool())
        {
            std::vector<WalletSnapshot> snapshots;
            snapshots.reserve(data.size());

            QSet<QDateTime> usedSnapshots;

            for (auto &entry : data)
            {
                const auto balance = entry.getBalance();

                if (Q_UNLIKELY(!balance))
                    continue;

                const auto timestamp = entry.getTimestamp();

                if (!usedSnapshots.contains(timestamp))
                {
                    WalletSnapshot snapshot;
                    snapshot.setTimestamp(timestamp);
                    snapshot.setBalance(*balance);
                    snapshot.setCharacterId(entry.getCharacterId());

                    snapshots.emplace_back(std::move(snapshot));
                    usedSnapshots << timestamp;
                }
            }

            asyncBatchStore(*mWalletSnapshotRepository, std::move(snapshots), false);
        }

        asyncBatchStore(*mWalletJournalEntryRepository, std::move(data), true, [=] {
            saveUpdateTimer(TimerType::WalletJournal, mUpdateTimes[TimerType::WalletJournal], id);

            emit characterWalletJournalChanged();
            emit taskEnded(task, {});
        });
    }

    void EvernusApplication::updateCharacterWalletTransactions(Character::IdType id, WalletTransactions data, uint task)
    {
        asyncBatchStore(*mWalletTransactionRepository, std::move(data), true, [=] {
            saveUpdateTimer(TimerType::WalletTransactions, mUpdateTimes[TimerType::WalletTransactions], id);

            QSettings settings;
            if (settings.value(PriceSettings::autoAddCustomItemCostKey, PriceSettings::autoAddCustomItemCostDefault).toBool() &&
                !mPendingAutoCostOrders.empty())
            {
                computeAutoCosts(id,
                                 mCharacterOrderProvider->getBuyOrders(id),
                                 std::bind(&WalletTransactionRepository::fetchForCharacterInRange,
                                           mWalletTransactionRepository.get(),
                                           id,
                                           std::placeholders::_1,
                                           std::placeholders::_2,
                                           WalletTransactionRepository::EntryType::Buy,
                                           std::placeholders::_3));
            }

            emit characterWalletTransactionsChanged();
            emit taskEnded(task, {});
        });
    }

    double EvernusApplication::getTotalAssetListValue(const AssetList &list) const
    {
        QSettings settings;
        const auto customLocationId = (settings.value(ImportSettings::useCustomAssetStationKey, ImportSettings::useCustomAssetStationDefault).toBool()) ?
                                      (EveDataProvider::getStationIdFromPath(settings.value(ImportSettings::customAssetStationKey).toList())) :
                                      (0);

        auto value = 0.;
        for (const auto &item : list)
        {
            const auto locationId = (customLocationId != 0) ? (customLocationId) : (item->getLocationId());
            if (!locationId)
                continue;

            value += getTotalItemSellValue(*item, *locationId);
        }

        return value;
    }

    double EvernusApplication::getTotalItemSellValue(const Item &item, quint64 locationId) const
    {
        // return 0 for BPC
        if (item.isBPC())
            return 0.;

        const auto customValue = item.getCustomValue();
        if (customValue)
            return *customValue;

        QSettings settings;
        const auto throwOnUnavailable
            = settings.value(ImportSettings::updateOnlyFullAssetValueKey, ImportSettings::updateOnlyFullAssetValueDefault).toBool();

        auto price
            = mDataProvider->getTypeSellPrice(item.getTypeId(), locationId, !throwOnUnavailable)->getPrice() * item.getQuantity();
        for (const auto &child : item)
            price += getTotalItemSellValue(*child, locationId);

        return price;
    }

    void EvernusApplication::saveUpdateTimer(TimerType timer, CharacterTimerMap &map, Character::IdType characterId) const
    {
        const auto time = QDateTime::currentDateTimeUtc();

        map[characterId] = time;

        UpdateTimer storedTimer;
        storedTimer.setCharacterId(characterId);
        storedTimer.setType(timer);
        storedTimer.setUpdateTime(time);

        mUpdateTimerRepository->store(storedTimer);
    }

    void EvernusApplication::computeAutoCosts(Character::IdType characterId,
                                              const MarketOrderProvider::OrderList &orders,
                                              const TransactionFetcher &transFetcher)
    {
        struct ItemCostData
        {
            uint mQuantity;
            double mPrice;
        };

        std::unordered_map<EveType::IdType, ItemCostData> newItemCosts;

        for (const auto &order : orders)
        {
            const auto it = mPendingAutoCostOrders.find(order->getId());
            if (it == std::end(mPendingAutoCostOrders))
                continue;

            const auto lastSeen = std::min(QDateTime::currentDateTimeUtc(), order->getIssued().addDays(order->getDuration()));
            const auto transactions = transFetcher(order->getFirstSeen(), lastSeen, order->getTypeId());

            if (!transactions.empty())
            {
                // we have a choice - either assume now transactions will ever be present, or assume they will arrive within this session
                // it's safer to go with the latter
                auto &cost = newItemCosts[order->getTypeId()];
                for (const auto &transaction : transactions)
                {
                    cost.mQuantity += transaction->getQuantity();
                    cost.mPrice += transaction->getQuantity() * transaction->getPrice();
                }

                mPendingAutoCostOrders.erase(it);
            }
        }


        for (const auto &cost : newItemCosts)
        {
            if (cost.second.mQuantity > 0)
                setForCharacterAndType(characterId, cost.first, cost.second.mPrice / cost.second.mQuantity);
        }
    }

    void EvernusApplication::setSmtpSettings()
    {
        SimpleCrypt crypt{ImportSettings::smtpCryptKey};
        QSettings settings;

        mSmtp.setUsername(settings.value(ImportSettings::smtpUserKey).toByteArray());
        mSmtp.setPassword(crypt.decryptToByteArray(settings.value(ImportSettings::smtpPasswordKey).toString()));
        mSmtp.setStartTlsDisabled(static_cast<ImportSettings::SmtpConnectionSecurity>(
            settings.value(ImportSettings::smtpConnectionSecurityKey).toInt()) != ImportSettings::SmtpConnectionSecurity::STARTTLS);
    }

    void EvernusApplication::createWalletSnapshot(Character::IdType characterId, double balance)
    {
        WalletSnapshot snapshot;
        snapshot.setTimestamp(QDateTime::currentDateTimeUtc());
        snapshot.setBalance(balance);
        snapshot.setCharacterId(characterId);

        mWalletSnapshotRepository->store(snapshot);
    }

    bool EvernusApplication::shouldImport(Character::IdType id, TimerType type) const
    {
        if (mPendingImports.find(std::make_pair(id, type)) != std::end(mPendingImports))
            return false;

        QSettings settings;
        if (!settings.value(ImportSettings::ignoreCachedImportKey, ImportSettings::ignoreCachedImportDefault).toBool())
            return true;

        return getLocalCacheTimer(id, type) <= QDateTime::currentDateTime();
    }

    bool EvernusApplication::checkImportAndEndTask(Character::IdType id, TimerType type, uint task)
    {
        const auto ret = shouldImport(id, type);
        if (!ret)
            QMetaObject::invokeMethod(this, "taskEnded", Qt::QueuedConnection, Q_ARG(uint, task), Q_ARG(QString, QString{}));

        return ret;
    }

    void EvernusApplication::markImport(Character::IdType id, TimerType type)
    {
        mPendingImports.emplace(id, type);
    }

    void EvernusApplication::unmarkImport(Character::IdType id, TimerType type)
    {
        mPendingImports.erase(std::make_pair(id, type));
    }

    void EvernusApplication::handleIncomingCharacterContracts(const Contracts &data,
                                                              Character::IdType id,
                                                              uint task)
    {
        if (data.empty())
        {
            emit characterContractsChanged();
            emit taskEnded(task, {});
        }
        else
        {
            for (const auto &contract : data)
            {
                if (contract.getType() == Contract::Type::Courier)
                    continue;

                ++mPendingCharacterContractItemRequests;

                Q_ASSERT(mESIManager);
                Q_ASSERT(mContractItemRepository);

                const auto subTask = startTask(task, tr("Fetching character contract items for contract %1...").arg(contract.getId()));
                mESIManager->fetchCharacterContractItems(id, contract.getId(), [=](auto &&data, const auto &error, const auto &expires) {
                    Q_UNUSED(expires);

                    --mPendingCharacterContractItemRequests;

                    if (error.isEmpty())
                    {
                        mPendingCharacterContractItems.reserve(mPendingCharacterContractItems.size() + data.size());
                        mPendingCharacterContractItems.insert(std::end(mPendingCharacterContractItems),
                                                              std::make_move_iterator(std::begin(data)),
                                                              std::make_move_iterator(std::end(data)));
                    }

                    if (mPendingCharacterContractItemRequests == 0)
                    {
                        asyncBatchStore(*mContractItemRepository, std::move(mPendingCharacterContractItems), true, [=] {
                            emit characterContractsChanged();
                            emit taskEnded(subTask, {});
                        });
                    }
                    else
                    {
                        emit taskEnded(subTask, error);
                    }
                });
            }

            if (mPendingCharacterContractItemRequests == 0)
            {
                emit characterContractsChanged();
                emit taskEnded(task, {});
            }
        }
    }

    void EvernusApplication::handleIncomingCorpContracts(const Contracts &data,
                                                         Character::IdType id,
                                                         quint64 corpId,
                                                         uint task)
    {
        if (data.empty())
        {
            emit corpContractsChanged();
            emit taskEnded(task, {});
        }
        else
        {
            for (const auto &contract : data)
            {
                if (contract.getType() == Contract::Type::Courier)
                    continue;

                ++mPendingCorpContractItemRequests;

                Q_ASSERT(mESIManager);
                Q_ASSERT(mCorpContractItemRepository);

                const auto subTask = startTask(task, tr("Fetching corporation contract items for contract %1...").arg(contract.getId()));
                mESIManager->fetchCorporationContractItems(id, corpId, contract.getId(), [=](auto &&data, const auto &error, const auto &expires) {
                    Q_UNUSED(expires);

                    --mPendingCorpContractItemRequests;

                    if (error.isEmpty())
                    {
                        mPendingCorpContractItems.reserve(mPendingCorpContractItems.size() + data.size());
                        mPendingCorpContractItems.insert(std::end(mPendingCorpContractItems),
                                                         std::make_move_iterator(std::begin(data)),
                                                         std::make_move_iterator(std::end(data)));
                    }

                    if (mPendingCorpContractItemRequests == 0)
                    {
                        asyncBatchStore(*mCorpContractItemRepository, std::move(mPendingCorpContractItems), true, [=] {
                            emit corpContractsChanged();
                            emit taskEnded(subTask, {});
                        });
                    }
                    else
                    {
                        emit taskEnded(subTask, error);
                    }
                });
            }

            if (mPendingCorpContractItemRequests == 0)
            {
                emit corpContractsChanged();
                emit taskEnded(task, {});
            }
        }
    }

    template<class T, class Data>
    QFuture<void> EvernusApplication::asyncBatchStore(const T &repo, Data data, bool hasId)
    {
        return asyncExecute(std::bind(&T::template batchStore<Data>, &repo, std::move(data), hasId, true));
    }

    template<class T, class Data, class Callback>
    void EvernusApplication::asyncBatchStore(const T &repo, Data data, bool hasId, Callback callback)
    {
        auto watcher = new QFutureWatcher<void>{this};
        connect(watcher, &QFutureWatcher<void>::finished, this, callback);
        connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
        connect(watcher, &QFutureWatcher<void>::canceled, this, [=] {
            watcher->waitForFinished(); // rethrow exception, if present
        });

        watcher->setFuture(asyncBatchStore(repo, std::move(data), hasId));
    }

    template<class Func>
    QFuture<void> EvernusApplication::asyncExecute(Func func)
    {
        qDebug() << "Starting async task...";
        return QtConcurrent::run([=] {
            // Qt is not smart enough to handle standard exceptions
            try
            {
                func();
            }
            catch (...)
            {
                throw StandardExceptionQtWrapperException{std::current_exception()};
            }
        });
    }

    void EvernusApplication::fetchStationTypeIds()
    {
        // get all ids from group 15, and hope it never changes...
        QSqlQuery query{QStringLiteral("SELECT typeID FROM invTypes WHERE groupID = 15"), mEveDatabaseConnectionProvider.getConnection()};
        while (query.next())
            mStationGroupTypeIds.emplace(query.value(0).value<EveType::IdType>());
    }

    void EvernusApplication::showSplashMessage(const QString &message, QSplashScreen &splash)
    {
        splash.showMessage(message, Qt::AlignBottom | Qt::AlignRight, Qt::white);
        processEvents();
    }

    QString EvernusApplication::getCharacterImportMessage(Character::IdType id)
    {
        return tr("Fetching character %1...").arg(id);
    }

    void EvernusApplication::setProxySettings()
    {
        QSettings settings;
        if (settings.value(NetworkSettings::useProxyKey, NetworkSettings::useProxyDefault).toBool())
        {
            SimpleCrypt crypt{NetworkSettings::cryptKey};

            const auto proxyType = settings.value(NetworkSettings::proxyTypeKey).toInt();
            const auto proxyHost = settings.value(NetworkSettings::proxyHostKey).toString();
            const quint16 proxyPort = settings.value(NetworkSettings::proxyPortKey).toUInt();
            const auto proxyUser = settings.value(NetworkSettings::proxyUserKey).toString();
            const auto proxyPassword = crypt.decryptToString(settings.value(NetworkSettings::proxyPasswordKey).toString());

            QNetworkProxy proxy{static_cast<QNetworkProxy::ProxyType>(proxyType), proxyHost, proxyPort, proxyUser, proxyPassword};
            QNetworkProxy::setApplicationProxy(proxy);
        }
        else
        {
            QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
        }
    }
}
