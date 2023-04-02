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

#include <unordered_set>
#include <functional>
#include <memory>

#include <boost/functional/hash.hpp>

#include <QApplication>
#include <QTranslator>
#include <QFuture>

#include "CharacterCorporationCombinedMarketOrderProvider.h"
#include "CorpMarketOrderValueSnapshotRepository.h"
#include "IndustryManufacturingSetupRepository.h"
#include "MarketOrderValueSnapshotRepository.h"
#include "CorpAssetValueSnapshotRepository.h"
#include "MainDatabaseConnectionProvider.h"
#include "ExternalOrderImporterRegistry.h"
#include "RegionStationPresetRepository.h"
#include "EveDatabaseConnectionProvider.h"
#include "AssetValueSnapshotRepository.h"
#include "WalletJournalEntryRepository.h"
#include "CorpWalletSnapshotRepository.h"
#include "WalletTransactionRepository.h"
#include "CachingMarketOrderProvider.h"
#include "LocationBookmarkRepository.h"
#include "RegionTypePresetRepository.h"
#include "WalletSnapshotRepository.h"
#include "ExternalOrderRepository.h"
#include "CachingContractProvider.h"
#include "EveDataManagerProvider.h"
#include "FavoriteItemRepository.h"
#include "CachingEveDataProvider.h"
#include "ContractItemRepository.h"
#include "MiningLedgerRepository.h"
#include "ExternalOrderImporter.h"
#include "MarketGroupRepository.h"
#include "UpdateTimerRepository.h"
#include "OrderScriptRepository.h"
#include "qxthttpsessionmanager.h"
#include "CachingAssetProvider.h"
#include "CacheTimerRepository.h"
#include "FilterTextRepository.h"
#include "CharacterRepository.h"
#include "LMeveTaskRepository.h"
#include "AssetListRepository.h"
#include "MetaGroupRepository.h"
#include "ESIInterfaceManager.h"
#include "ItemCostRepository.h"
#include "CacheTimerProvider.h"
#include "RepositoryProvider.h"
#include "ContractRepository.h"
#include "WalletTransactions.h"
#include "CitadelRepository.h"
#include "EveTypeRepository.h"
#include "LMeveDataProvider.h"
#include "ItemCostProvider.h"
#include "LMeveAPIManager.h"
#include "CitadelManager.h"
#include "ItemRepository.h"
#include "TaskConstants.h"
#include "WalletJournal.h"
#include "ESIManager.h"
#include "TaskManager.h"
#include "Contracts.h"

#include "qxtsmtp.h"

class QSplashScreen;
class QUrl;

namespace Evernus
{
    class EvernusApplication
        : public QApplication
        , public ExternalOrderImporterRegistry
        , public CacheTimerProvider
        , public ItemCostProvider
        , public RepositoryProvider
        , public LMeveDataProvider
        , public TaskManager
        , public EveDataManagerProvider
    {
        Q_OBJECT

    public:
        static const QString versionKey;

        EvernusApplication(int &argc,
                           char *argv[],
                           QString clientId,
                           QString clientSecret,
                           const QString &forcedVersion,
                           bool dontUpdate);
        virtual ~EvernusApplication();

        virtual void registerImporter(const std::string &name, std::unique_ptr<ExternalOrderImporter> &&importer) override;

        virtual QDateTime getLocalCacheTimer(Character::IdType id, TimerType type) const override;
        virtual void setUtcCacheTimer(Character::IdType id, TimerType type, const QDateTime &dt) override;

        virtual QDateTime getLocalUpdateTimer(Character::IdType id, TimerType type) const override;

        virtual std::shared_ptr<ItemCost> fetchForCharacterAndType(Character::IdType characterId, EveType::IdType typeId) const override;
        virtual CostList fetchForCharacter(Character::IdType characterId) const override;
        virtual void setForCharacterAndType(Character::IdType characterId, EveType::IdType typeId, double value) override;

        virtual std::shared_ptr<ItemCost> findItemCost(ItemCost::IdType id) const override;
        virtual void removeItemCost(ItemCost::IdType id) const override;
        virtual void storeItemCost(ItemCost &cost) const override;
        virtual void removeAllItemCosts(Character::IdType characterId) const override;

        virtual const CharacterRepository &getCharacterRepository() const noexcept override;
        virtual const WalletSnapshotRepository &getWalletSnapshotRepository() const noexcept override;
        virtual const CorpWalletSnapshotRepository &getCorpWalletSnapshotRepository() const noexcept override;
        virtual const AssetValueSnapshotRepository &getAssetValueSnapshotRepository() const noexcept override;
        virtual const CorpAssetValueSnapshotRepository &getCorpAssetValueSnapshotRepository() const noexcept override;
        virtual const WalletJournalEntryRepository &getWalletJournalEntryRepository() const noexcept override;
        virtual const WalletTransactionRepository &getWalletTransactionRepository() const noexcept override;
        virtual const MarketOrderRepository &getMarketOrderRepository() const noexcept override;
        virtual const WalletJournalEntryRepository &getCorpWalletJournalEntryRepository() const noexcept override;
        virtual const WalletTransactionRepository &getCorpWalletTransactionRepository() const noexcept override;
        virtual const MarketOrderRepository &getCorpMarketOrderRepository() const noexcept override;
        virtual const ItemCostRepository &getItemCostRepository() const noexcept override;
        virtual const MarketOrderValueSnapshotRepository &getMarketOrderValueSnapshotRepository() const noexcept override;
        virtual const CorpMarketOrderValueSnapshotRepository &getCorpMarketOrderValueSnapshotRepository() const noexcept override;
        virtual const FilterTextRepository &getFilterTextRepository() const noexcept override;
        virtual const OrderScriptRepository &getOrderScriptRepository() const noexcept override;
        virtual const FavoriteItemRepository &getFavoriteItemRepository() const noexcept override;
        virtual const LocationBookmarkRepository &getLocationBookmarkRepository() const noexcept override;
        virtual const ExternalOrderRepository &getExternalOrderRepository() const noexcept override;
        virtual const EveTypeRepository &getEveTypeRepository() const noexcept override;
        virtual const MarketGroupRepository &getMarketGroupRepository() const noexcept override;
        virtual const CacheTimerRepository &getCacheTimerRepository() const noexcept override;
        virtual const UpdateTimerRepository &getUpdateTimerRepository() const noexcept override;
        virtual const ItemRepository &getItemRepository() const noexcept override;
        virtual const CitadelRepository &getCitadelRepository() const noexcept override;
        virtual const RegionTypePresetRepository &getRegionTypePresetRepository() const noexcept override;
        virtual const ItemRepository &getCorpItemRepository() const noexcept override;
        virtual const RegionStationPresetRepository &getRegionStationPresetRepository() const noexcept override;
        virtual const IndustryManufacturingSetupRepository &getIndustryManufacturingSetupRepository() const noexcept override;
        virtual const MiningLedgerRepository &getMiningLedgerRepository() const noexcept override;

        virtual std::vector<std::shared_ptr<LMeveTask>> getTasks(Character::IdType characterId) const override;

        virtual uint startTask(const QString &description) override;
        virtual uint startTask(uint parentTask, const QString &description) override;
        virtual void updateTask(uint taskId, const QString &description) override;
        virtual void endTask(uint taskId, const QString &error = QString{}) override;

        virtual const ESIManager &getESIManager() const override;

        ESIInterfaceManager &getESIInterfaceManager() noexcept;

        MarketOrderProvider &getMarketOrderProvider() const noexcept;
        MarketOrderProvider &getCorpMarketOrderProvider() const noexcept;

        const ContractProvider &getContractProvider() const noexcept;
        const ContractProvider &getCorpContractProvider() const noexcept;

        AssetProvider &getAssetProvider() const noexcept;
        AssetProvider &getCorpAssetProvider() const noexcept;

        EveDataProvider &getDataProvider() noexcept;

    signals:
        void taskStarted(uint taskId, const QString &description);
        void taskStarted(uint taskId, uint parentTask, const QString &description);
        void taskInfoChanged(uint taskId, const QString &text);
        void taskEnded(uint taskId, const QString &error);

        void citadelsChanged();
        void charactersChanged();
        void characterAssetsChanged();
        void externalOrdersChanged();
        void externalOrdersChangedWithMarketOrders();
        void characterWalletJournalChanged();
        void characterWalletTransactionsChanged();
        void characterMarketOrdersChanged();
        void characterContractsChanged();
        void characterMiningLedgerChanged();
        void corpAssetsChanged();
        void corpWalletJournalChanged();
        void corpWalletTransactionsChanged();
        void corpMarketOrdersChanged();
        void corpContractsChanged();
        void itemCostsChanged() const;
        void itemVolumeChanged();
        void lMeveTasksChanged();

        void snapshotsTaken();

        void openMarginTool();

        void ssoError(const QString &info);

        void ssoAuthRequested(Character::IdType charId, const QUrl &url);

    public slots:
        void refreshCharacters();
        void refreshCharacter(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCharacterAssets(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCharacterContracts(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCharacterWalletJournal(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCharacterWalletTransactions(Character::IdType id, uint parentTask = TaskConstants::invalidTask, bool force = false);
        void refreshCharacterMarketOrdersFromAPI(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCharacterMarketOrdersFromLogs(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCharacterMiningLedger(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCorpAssets(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCorpContracts(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCorpWalletJournal(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCorpWalletTransactions(Character::IdType id, uint parentTask = TaskConstants::invalidTask, bool force = false);
        void refreshCorpMarketOrdersFromAPI(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCorpMarketOrdersFromLogs(Character::IdType id, uint parentTask = TaskConstants::invalidTask);
        void refreshCitadels();
        void refreshAllExternalOrders(Character::IdType id);
        void refreshExternalOrdersFromWeb(Character::IdType id, const TypeLocationPairs &target);
        void refreshExternalOrdersFromFile(Character::IdType id, const TypeLocationPairs &target);

        void finishExternalOrderImport(const QString &info, const std::vector<ExternalOrder> &orders);
        void updateExternalOrdersAndAssetValue(const std::vector<ExternalOrder> &orders);

        void handleNewPreferences();

        void importFromMentat();

        void syncLMeve(Character::IdType id);

        void clearCorpWalletData();
        void clearCitadelCache();
        void clearRefreshTokens();

        void makeValueSnapshots(Character::IdType id);

        void showInEve(EveType::IdType typeId, Character::IdType charId);
        void setDestinationInEve(quint64 locationId, Character::IdType charId);

        void processSSOAuthorizationCode(Character::IdType charId, const QByteArray &code);
        void cancelSsoAuth(Character::IdType charId);
        void processNewCharacter(Character::IdType id, const QString &accessToken, const QString &refreshToken);

    private slots:
        void scheduleCharacterUpdate();
        void updateCharacters();

        void scheduleMarketOrderChange();
        void updateCharacterMarketOrders();

        void scheduleCorpMarketOrderChange();
        void updateCorpMarketOrders();

        void showPriceImportStatus(const QString &info);

        void emitNewItemCosts();

        void showSmtpError(const QByteArray &message);
        void showMailError(int mailID, int errorCode, const QByteArray &message);

    private:
        using CharacterTypePair = std::pair<Character::IdType, EveType::IdType>;
        using CharacterTimerMap = std::unordered_map<Character::IdType, QDateTime>;
        using TypedCharacterTimerMap = std::unordered_map<TimerType, CharacterTimerMap>;
        using TransactionFetcher = std::function<WalletTransactionRepository::EntityList (const QDateTime &, const QDateTime &, EveType::IdType)>;

        MainDatabaseConnectionProvider mMainDatabaseConnectionProvider;
        EveDatabaseConnectionProvider mEveDatabaseConnectionProvider;

        std::unique_ptr<CharacterRepository> mCharacterRepository;
        std::unique_ptr<ItemRepository> mItemRepository, mCorpItemRepository;
        std::unique_ptr<AssetListRepository> mAssetListRepository, mCorpAssetListRepository;
        std::unique_ptr<WalletSnapshotRepository> mWalletSnapshotRepository;
        std::unique_ptr<CorpWalletSnapshotRepository> mCorpWalletSnapshotRepository;
        std::unique_ptr<ExternalOrderRepository> mExternalOrderRepository;
        std::unique_ptr<AssetValueSnapshotRepository> mAssetValueSnapshotRepository;
        std::unique_ptr<CorpAssetValueSnapshotRepository> mCorpAssetValueSnapshotRepository;
        std::unique_ptr<WalletJournalEntryRepository> mWalletJournalEntryRepository, mCorpWalletJournalEntryRepository;
        std::unique_ptr<CacheTimerRepository> mCacheTimerRepository;
        std::unique_ptr<UpdateTimerRepository> mUpdateTimerRepository;
        std::unique_ptr<WalletTransactionRepository> mWalletTransactionRepository, mCorpWalletTransactionRepository;
        std::unique_ptr<MarketOrderRepository> mMarketOrderRepository, mCorpMarketOrderRepository;
        std::unique_ptr<ItemCostRepository> mItemCostRepository;
        std::unique_ptr<MarketOrderValueSnapshotRepository> mMarketOrderValueSnapshotRepository;
        std::unique_ptr<CorpMarketOrderValueSnapshotRepository> mCorpMarketOrderValueSnapshotRepository;
        std::unique_ptr<FilterTextRepository> mFilterTextRepository;
        std::unique_ptr<OrderScriptRepository> mOrderScriptRepository;
        std::unique_ptr<FavoriteItemRepository> mFavoriteItemRepository;
        std::unique_ptr<LocationBookmarkRepository> mLocationBookmarkRepository;
        std::unique_ptr<ContractItemRepository> mContractItemRepository, mCorpContractItemRepository;
        std::unique_ptr<ContractRepository> mContractRepository, mCorpContractRepository;
        std::unique_ptr<EveTypeRepository> mEveTypeRepository;
        std::unique_ptr<LMeveTaskRepository> mLMeveTaskRepository;
        std::unique_ptr<MarketGroupRepository> mMarketGroupRepository;
        std::unique_ptr<MetaGroupRepository> mMetaGroupRepository;
        std::unique_ptr<CitadelRepository> mCitadelRepository;
        std::unique_ptr<RegionTypePresetRepository> mRegionTypePresetRepository;
        std::unique_ptr<RegionStationPresetRepository> mRegionStationPresetRepository;
        std::unique_ptr<IndustryManufacturingSetupRepository> mIndustryManufacturingSetupRepository;
        std::unique_ptr<MiningLedgerRepository> mMiningLedgerRepository;

        std::unique_ptr<ESIInterfaceManager> mESIInterfaceManager;

        LMeveAPIManager mLMeveAPIManager;
        CitadelManager mCitadelManager;

        uint mTaskId = TaskConstants::invalidTask + 1;
        uint mCurrentExternalOrderImportTask = TaskConstants::invalidTask;

        bool mCharacterUpdateScheduled = false;
        bool mItemCostUpdateScheduled = false;
        bool mMarketOrderUpdateScheduled = false;
        bool mCorpMarketOrderUpdateScheduled = false;

        std::unordered_map<std::string, ImporterPtr> mExternalOrderImporters;

        std::unique_ptr<CachingAssetProvider> mCharacterAssetProvider, mCorpAssetProvider;

        TypedCharacterTimerMap mCacheTimes;
        TypedCharacterTimerMap mUpdateTimes;

        std::unordered_set<std::pair<Character::IdType, TimerType>, boost::hash<std::pair<Character::IdType, TimerType>>>
        mPendingImports;

        std::unique_ptr<CachingMarketOrderProvider> mCharacterOrderProvider, mCorpOrderProvider;
        std::unique_ptr<CharacterCorporationCombinedMarketOrderProvider> mCombinedOrderProvider;

        std::unique_ptr<CachingContractProvider> mCharacterContractProvider, mCorpContractProvider;

        mutable std::unordered_map<CharacterTypePair, ItemCostRepository::EntityPtr, boost::hash<CharacterTypePair>>
        mCharacterItemCostCache;
        mutable std::unordered_map<EveType::IdType, ItemCostRepository::EntityPtr> mTypeItemCostCache;

        QTranslator mTranslator, mQtTranslator, mQtBaseTranslator, mQtScriptTranslator;

        QxtHttpSessionManager mHttpSessionManager;
        QxtSmtp mSmtp;

        std::unordered_set<MarketOrder::IdType> mPendingAutoCostOrders;

        std::unique_ptr<CachingEveDataProvider> mDataProvider;

        mutable std::unordered_map<Character::IdType, LMeveTaskRepository::EntityList> mLMeveTaskCache;

        std::unique_ptr<ESIManager> mESIManager;

        std::unordered_set<EveType::IdType> mStationGroupTypeIds;

        std::size_t mPendingCharacterContractItemRequests = 0;
        std::size_t mPendingCorpContractItemRequests = 0;

        std::vector<ContractItem> mPendingCharacterContractItems;
        std::vector<ContractItem> mPendingCorpContractItems;

        void updateTranslator(const QString &lang);

        void createDb();
        void createDbSchema();
        void precacheCacheTimers();
        void precacheUpdateTimers();
        void deleteOldWalletEntries();
        void deleteOldMarketOrders();

        void importCharacter(Character::IdType id, uint task);
        void importExternalOrders(const std::string &importerName, Character::IdType id, const TypeLocationPairs &target);
        void importMarketOrdersFromLogs(Character::IdType id, uint task, bool corp);
        void importMarketOrders(Character::IdType id, MarketOrders &orders, bool corp);

        void finishExternalOrderImportTask(const QString &info);

        void computeAssetListSellValueSnapshot(const AssetList &list) const;
        void computeCorpAssetListSellValueSnapshot(const AssetList &list) const;

        void updateCharacterAssets(Character::IdType id, AssetList &list);
        void updateCharacter(Character &character);
        void updateCharacterWalletJournal(Character::IdType id, WalletJournal data, uint task);
        void updateCharacterWalletTransactions(Character::IdType id, WalletTransactions data, uint task);

        double getTotalAssetListValue(const AssetList &list) const;
        double getTotalItemSellValue(const Item &item, quint64 locationId) const;

        void saveUpdateTimer(TimerType timer, CharacterTimerMap &map, Character::IdType characterId) const;

        void computeAutoCosts(Character::IdType characterId,
                              const MarketOrderProvider::OrderList &orders,
                              const TransactionFetcher &transFetcher);

        void setSmtpSettings();

        void createWalletSnapshot(Character::IdType characterId, double balance);

        bool shouldImport(Character::IdType id, TimerType type) const;
        bool checkImportAndEndTask(Character::IdType id, TimerType type, uint task);

        void markImport(Character::IdType id, TimerType type);
        void unmarkImport(Character::IdType id, TimerType type);

        void handleIncomingCharacterContracts(const Contracts &data,
                                              Character::IdType id,
                                              uint task);
        void handleIncomingCorpContracts(const Contracts &data,
                                         Character::IdType id,
                                         quint64 corpId,
                                         uint task);

        template<class T, class Data>
        QFuture<void> asyncBatchStore(const T &repo, Data data, bool hasId);
        template<class T, class Data, class Callback>
        void asyncBatchStore(const T &repo, Data data, bool hasId, Callback callback);

        template<class Func>
        QFuture<void> asyncExecute(Func func);

        void fetchStationTypeIds();

        static void showSplashMessage(const QString &message, QSplashScreen &splash);
        static QString getCharacterImportMessage(Character::IdType id);

        static void setProxySettings();
    };
}
