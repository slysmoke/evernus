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
#include <functional>
#include <optional>
#include <vector>
#include <memory>
#include <map>

#include <QDateTime>
#include <QString>
#include <QDate>

#include "IndustryCostIndices.h"
#include "MarketHistoryEntry.h"
#include "WalletJournalEntry.h"
#include "WalletTransactions.h"
#include "WalletTransaction.h"
#include "WalletJournal.h"
#include "ESIInterface.h"
#include "MarketOrders.h"
#include "MarketPrices.h"
#include "ContractItem.h"
#include "MarketOrder.h"
#include "Character.h"
#include "AssetList.h"
#include "Contracts.h"
#include "Contract.h"
#include "EveType.h"

class QJsonObject;
class QDateTime;

namespace Evernus
{
    struct SovereigntyStructure;
    class ESIInterfaceManager;
    class EveDataProvider;
    class ExternalOrder;
    class MiningLedger;
    class Blueprint;

    class ESIManager final
        : public QObject
    {
        Q_OBJECT

    public:
        template<class T>
        using Callback = std::function<void (T &&data, const QString &error, const QDateTime &expires)>;
        template<class T>
        using PesistentDataCallback = std::function<void (T &&data, const QString &error)>;
        using ExternalOrderList = std::vector<ExternalOrder>;
        using BlueprintList = std::vector<Blueprint>;
        using MiningLedgerList = std::vector<MiningLedger>;
        using SovereigntyStructureList = std::vector<SovereigntyStructure>;
        using ContractItemList = std::vector<ContractItem>;
        using MarketOrderCallback = Callback<ExternalOrderList>;
        using AssetCallback = Callback<AssetList>;
        using ContractCallback = Callback<Contracts>;
        using ContractItemCallback = Callback<ContractItemList>;
        using WalletJournalCallback = Callback<WalletJournal>;
        using WalletTransactionsCallback = Callback<WalletTransactions>;
        using MarketOrdersCallback = Callback<MarketOrders>;
        using BlueprintCallback = Callback<BlueprintList>;
        using HistoryMap = std::map<QDate, MarketHistoryEntry>;
        using NameMap = std::unordered_map<quint64, QString>;

        static const QString loginUrl;

        ESIManager(const EveDataProvider &dataProvider,
                   ESIInterfaceManager &interfaceManager,
                   QObject *parent = nullptr);
        ESIManager(const ESIManager &) = default;
        ESIManager(ESIManager &&) = default;
        virtual ~ESIManager() = default;

        void fetchMarketOrders(uint regionId,
                               EveType::IdType typeId,
                               const MarketOrderCallback &callback) const;
        void fetchMarketHistory(uint regionId,
                                EveType::IdType typeId,
                                const Callback<HistoryMap> &callback) const;
        void fetchMarketOrders(uint regionId, const MarketOrderCallback &callback) const;
        void fetchCitadelMarketOrders(quint64 citadelId,
                                      uint regionId,
                                      Character::IdType charId,
                                      const MarketOrderCallback &callback) const;
        void fetchCharacterAssets(Character::IdType charId, const AssetCallback &callback) const;
        void fetchCorporationAssets(Character::IdType charId, quint64 corpId, const AssetCallback &callback) const;
        void fetchCharacter(Character::IdType charId, const Callback<Character> &callback) const;
        void fetchRaces(const Callback<NameMap> &callback) const;
        void fetchBloodlines(const Callback<NameMap> &callback) const;
        void fetchAncestries(const Callback<NameMap> &callback) const;
        void fetchCharacterMarketOrders(Character::IdType charId, const MarketOrdersCallback &callback) const;
        void fetchCorporationMarketOrders(Character::IdType charId, quint64 corpId, const MarketOrdersCallback &callback) const;
        void fetchCharacterWalletJournal(Character::IdType charId,
                                         WalletJournalEntry::IdType tillId,
                                         const WalletJournalCallback &callback) const;
        void fetchCorporationWalletJournal(Character::IdType charId,
                                           quint64 corpId,
                                           int division,
                                           WalletJournalEntry::IdType tillId,
                                           const WalletJournalCallback &callback) const;
        void fetchCharacterWalletTransactions(Character::IdType charId,
                                              WalletTransaction::IdType tillId,
                                              const WalletTransactionsCallback &callback) const;
        void fetchCorporationWalletTransactions(Character::IdType charId,
                                                quint64 corpId,
                                                int division,
                                                WalletTransaction::IdType tillId,
                                                const WalletTransactionsCallback &callback) const;
        void fetchCharacterContracts(Character::IdType charId, const ContractCallback &callback) const;
        void fetchCharacterContractItems(Character::IdType charId, Contract::IdType contractId, const ContractItemCallback &callback) const;
        void fetchCorporationContracts(Character::IdType charId, quint64 corpId, const ContractCallback &callback) const;
        void fetchCorporationContractItems(Character::IdType charId, quint64 corpId, Contract::IdType contractId, const ContractItemCallback &callback) const;
        void fetchCharacterBlueprints(Character::IdType charId, const BlueprintCallback &callback) const;
        void fetchCorporationBlueprints(Character::IdType charId, quint64 corpId, const BlueprintCallback &callback) const;
        void fetchCharacterMiningLedger(Character::IdType charId, const Callback<MiningLedgerList> &callback) const;
        void fetchGenericName(quint64 id, const PesistentDataCallback<QString> &callback) const;
        void fetchGenericNames(const std::vector<quint64> &ids, const PesistentDataCallback<std::unordered_map<quint64, QString>> &callback) const;
        void fetchMarketPrices(const Callback<MarketPrices> &callback) const;
        void fetchIndustryCostIndices(const Callback<IndustryCostIndices> &callback) const;
        void fetchSovereigntyStructures(const Callback<SovereigntyStructureList> &callback) const;

        void openMarketDetails(EveType::IdType typeId, Character::IdType charId) const;

        void setDestination(quint64 locationId, Character::IdType charId) const;

        ESIManager &operator =(const ESIManager &) = default;
        ESIManager &operator =(ESIManager &&) = default;

    signals:
        void error(const QString &text) const;

    private:
        static const QString firstTimeCitadelOrderImportKey;

        static bool mFirstTimeCitadelOrderImport;

        const EveDataProvider &mDataProvider;

        ESIInterfaceManager &mInterfaceManager;

        void fetchCharacterWalletTransactions(Character::IdType charId,
                                              const std::optional<WalletTransaction::IdType> &fromId,
                                              WalletTransaction::IdType tillId,
                                              std::shared_ptr<WalletTransactions> &&transactions,
                                              const WalletTransactionsCallback &callback) const;
        void fetchCorporationWalletTransactions(Character::IdType charId,
                                                quint64 corpId,
                                                int division,
                                                const std::optional<WalletTransaction::IdType> &fromId,
                                                WalletTransaction::IdType tillId,
                                                std::shared_ptr<WalletTransactions> &&transactions,
                                                const WalletTransactionsCallback &callback) const;

        ExternalOrder getExternalOrderFromJson(const QJsonObject &object, uint regionId, const QDateTime &updateTime) const;
        ESIInterface::PaginatedCallback getMarketOrderCallback(uint regionId, const MarketOrderCallback &callback) const;
        ESIInterface::JsonCallback getMarketOrdersCallback(Character::IdType charId, const MarketOrdersCallback &callback) const;
        ESIInterface::PaginatedCallback getAssetListCallback(Character::IdType charId, const AssetCallback &callback) const;
        ESIInterface::JsonCallback getContractCallback(const ContractCallback &callback) const;
        ESIInterface::JsonCallback getContractItemCallback(Contract::IdType contractId, const ContractItemCallback &callback) const;

        ESIInterface::PaginatedCallback getWalletJournalCallback(Character::IdType charId,
                                                                 quint64 corpId,
                                                                 WalletJournalEntry::IdType tillId,
                                                                 const WalletJournalCallback &callback) const;

        template<class T>
        ESIInterface::JsonCallback getWalletTransactionsCallback(Character::IdType charId,
                                                                 quint64 corpId,
                                                                 WalletTransaction::IdType tillId,
                                                                 std::shared_ptr<WalletTransactions> &&transactions,
                                                                 const WalletTransactionsCallback &callback,
                                                                 T nextCallback) const;

        ESIInterface::PaginatedCallback getBlueprintCallback(const BlueprintCallback &callback) const;

        const ESIInterface &getInterface() const;

        static short getMarketOrderRangeFromString(const QString &range);
        static QDateTime getDateTimeFromString(const QString &value);

        static Contract::Type getContractTypeFromString(const QString &type);
        static Contract::Status getContractStatusFromString(const QString &status);
        static Contract::Availability getContractAvailabilityFromString(const QString &availability);
    };
}
