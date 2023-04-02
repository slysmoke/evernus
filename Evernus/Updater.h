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

#include <functional>
#include <map>

#include <QNetworkAccessManager>
#include <QVersionNumber>

#include "Repository.h"

class QSqlDatabase;

namespace Evernus
{
    template<class T>
    class Repository;
    class CorpMarketOrderValueSnapshotRepository;
    class IndustryManufacturingSetupRepository;
    class MarketOrderValueSnapshotRepository;
    class WalletJournalEntryRepository;
    class WalletTransactionRepository;
    class RegionTypePresetRepository;
    class ExternalOrderRepository;
    class MarketOrderRepository;
    class UpdateTimerRepository;
    class CacheTimerRepository;
    class CharacterRepository;
    class RepositoryProvider;
    class CitadelAccessCache;
    class CitadelRepository;
    class EveDataProvider;
    class ItemRepository;
    class ExternalOrder;
    class Character;

    class Updater
        : public QObject
    {
        Q_OBJECT

    public:
        void performVersionMigration(const RepositoryProvider &repoProvider,
                                     const EveDataProvider &dataProvider,
                                     CitadelAccessCache &citadelAccessCache) const;
        void updateDatabaseVersion(const QSqlDatabase &db) const;

        static Updater &getInstance();

    public slots:
        void checkForUpdates(bool quiet) const;

    private:
        template<class Sig>
        using UpdateChain = std::map<QVersionNumber, Sig>;

        UpdateChain<std::function<void (CitadelAccessCache &)>> mCoreUpdateSteps;
        UpdateChain<std::function<void (const RepositoryProvider &)>> mDbUpdateSteps;

        mutable QNetworkAccessManager mAccessManager;
        mutable bool mCheckingForUpdates = false;

        Updater();
        virtual ~Updater() = default;

        void finishCheck(bool quiet) const;

        void updateCore(const QVersionNumber &prevVersion, CitadelAccessCache &citadelAccessCache) const;
        void updateDatabase(const QVersionNumber &prevVersion, const RepositoryProvider &provider) const;

        static void migrateDatabaseTo05(const CacheTimerRepository &cacheTimerRepo,
                                        const Repository<Character> &characterRepo,
                                        const MarketOrderRepository &characterOrderRepo,
                                        const MarketOrderRepository &corporationOrderRepo);
        static void migrateDatabaseTo18(const ExternalOrderRepository &externalOrderRepo);
        static void migrateDatabaseTo19(const Repository<Character> &characterRepo,
                                        const WalletJournalEntryRepository &walletJournalRepo,
                                        const WalletJournalEntryRepository &corpWalletJournalRepo,
                                        const WalletTransactionRepository &walletTransactionRepo,
                                        const WalletTransactionRepository &corpWalletTransactionRepo);
        static void migrateDatabaseTo111(const CacheTimerRepository &cacheTimerRepo,
                                         const UpdateTimerRepository &updateTimerRepo,
                                         const Repository<Character> &characterRepo);
        static void migrateDatabaseTo116(const MarketOrderValueSnapshotRepository &orderValueSnapshotRepo,
                                         const CorpMarketOrderValueSnapshotRepository &corpOrderValueSnapshotRepo);
        static void migrateDatabaseTo123(const ExternalOrderRepository &externalOrderRepo,
                                         const ItemRepository &itemRepo);
        static void migrateDatabaseTo127(const MarketOrderRepository &characterOrderRepo,
                                         const MarketOrderRepository &corporationOrderRepo);
        static void migrateDatabaseTo141(const Repository<Character> &characterRepo);
        static void migrateDatabaseTo145(const CharacterRepository &characterRepo,
                                         const MarketOrderRepository &characterOrderRepo,
                                         const MarketOrderRepository &corporationOrderRepo);
        static void migrateDatabaseTo147(const MarketOrderRepository &characterOrderRepo,
                                         const MarketOrderRepository &corporationOrderRepo);
        static void migrateDatabaseTo149(const CitadelRepository &citadelRepo);
        static void migrateDatabaseTo150(const CitadelRepository &citadelRepo);
        static void migrateDatabaseTo153(const ItemRepository &itemRepo);
        static void migrateDatabaseTo20(const Repository<Character> &characterRepo);
        static void migrateDatabaseTo22(const CitadelRepository &citadelRepo,
                                        const ItemRepository &corpItemRepo);
        static void migrateDatabaseTo23(const CitadelRepository &citadelRepo);
        static void migrateDatabaseTo26(const WalletJournalEntryRepository &walletJournalRepo,
                                        const WalletJournalEntryRepository &corpWalletJournalRepo,
                                        const Repository<Character> &characterRepo);
        static void migrateDatabaseTo211(const WalletTransactionRepository &walletTransactionRepo,
                                         const WalletTransactionRepository &corpWalletTransactionRepo,
                                         const Repository<Character> &characterRepo);
        static void migrateDatabaseTo216(const Repository<Character> &characterRepo);
        static void migrateDatabaseTo34(const WalletJournalEntryRepository &walletJournalRepo,
                                        const WalletJournalEntryRepository &corpWalletJournalRepo);
        static void migrateDatabaseTo36(const ItemRepository &itemRepo, const ItemRepository &corpItemRepo);

        static void migrateCoreTo03();
        static void migrateCoreTo113();
        static void migrateCoreTo130();
        static void migrateCoreTo136();
        static void migrateCoreTo23();
        static void migrateCoreTo27();
        static void migrateCoreTo214(CitadelAccessCache &citadelAccessCache);
        static void migrateCoreTo217();
        static void migrateCoreTo218();
        static void migrateCoreTo30();
        static void migrateCoreTo31();

        static void removeRefreshTokens();

        static void fixRegionTypePresets(const RegionTypePresetRepository &repo,
                                         const EveDataProvider &dataProvider);

        static QVersionNumber getSavedCoreVersion();
        static QVersionNumber getCurrentCoreVersion();
        static QVersionNumber getDbVersion(const QSqlDatabase &db, const QVersionNumber &defaultVersion);

        template<class T>
        static void safelyExecQuery(const Repository<T> &repo, const QString &query);
    };
}
