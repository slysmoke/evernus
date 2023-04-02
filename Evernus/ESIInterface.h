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
#include <mutex>

#include <optional>

#include <QSettings>
#include <QDateTime>
#include <QString>

#include "WalletJournalEntry.h"
#include "WalletTransaction.h"
#include "Character.h"
#include "Contract.h"
#include "EveType.h"

class QNetworkRequest;
class QJsonDocument;
class QNetworkReply;
class QUrlQuery;

namespace Evernus
{
    class ESIInterfaceErrorLimiter;
    class CitadelAccessCache;
    class ESIOAuth;

    class ESIInterface final
        : public QObject
    {
        Q_OBJECT

    public:
        template<class T>
        using PersistentCallback = std::function<void (T &&data, const QString &error)>;
        using JsonCallback = std::function<void (QJsonDocument &&data, const QString &error, const QDateTime &expires)>;
        using PaginatedCallback = std::function<void (QJsonDocument &&data, bool atEnd, const QString &error, const QDateTime &expires)>;
        using ErrorCallback = std::function<void (const QString &error)>;
        using StringCallback = std::function<void (QString &&data, const QString &error, const QDateTime &expires)>;  // https://bugreports.qt.io/browse/QTBUG-62502
        using PersistentStringCallback = PersistentCallback<QString>;
        using PersistentJsonCallback = PersistentCallback<QJsonDocument>;

        ESIInterface(CitadelAccessCache &citadelAccessCache,
                     ESIInterfaceErrorLimiter &errorLimiter,
                     ESIOAuth &oauth,
                     QObject *parent = nullptr);
        ESIInterface(const ESIInterface &) = default;
        ESIInterface(ESIInterface &&) = default;
        virtual ~ESIInterface() = default;

        void fetchMarketOrders(uint regionId, EveType::IdType typeId, const PaginatedCallback &callback) const;
        void fetchMarketOrders(uint regionId, const PaginatedCallback &callback) const;
        void fetchMarketHistory(uint regionId, EveType::IdType typeId, const JsonCallback &callback) const;
        void fetchCitadelMarketOrders(quint64 citadelId, Character::IdType charId, const PaginatedCallback &callback) const;
        void fetchCharacterAssets(Character::IdType charId, const PaginatedCallback &callback) const;
        void fetchCorporationAssets(Character::IdType charId, quint64 corpId, const PaginatedCallback &callback) const;
        void fetchCharacter(Character::IdType charId, const JsonCallback &callback) const;
        void fetchCharacterSkills(Character::IdType charId, const JsonCallback &callback) const;
        void fetchCorporation(quint64 corpId, const JsonCallback &callback) const;
        void fetchRaces(const JsonCallback &callback) const;
        void fetchBloodlines(const JsonCallback &callback) const;
        void fetchAncestries(const JsonCallback &callback) const;
        void fetchCharacterWallet(Character::IdType charId, const StringCallback &callback) const;
        void fetchCharacterMarketOrders(Character::IdType charId, const JsonCallback &callback) const;
        void fetchCorporationMarketOrders(Character::IdType charId, quint64 corpId, const JsonCallback &callback) const;
        void fetchCharacterWalletJournal(Character::IdType charId, const PaginatedCallback &callback) const;
        void fetchCorporationWalletJournal(Character::IdType charId,
                                           quint64 corpId,
                                           int division,
                                           const PaginatedCallback &callback) const;
        void fetchCharacterWalletTransactions(Character::IdType charId,
                                              const std::optional<WalletTransaction::IdType> &fromId,
                                              const JsonCallback &callback) const;
        void fetchCorporationWalletTransactions(Character::IdType charId,
                                                quint64 corpId,
                                                int division,
                                                const std::optional<WalletTransaction::IdType> &fromId,
                                                const JsonCallback &callback) const;
        void fetchCharacterContracts(Character::IdType charId, const JsonCallback &callback) const;
        void fetchCharacterContractItems(Character::IdType charId, Contract::IdType contractId, const JsonCallback &callback) const;
        void fetchCorporationContracts(Character::IdType charId, quint64 corpId, const JsonCallback &callback) const;
        void fetchCorporationContractItems(Character::IdType charId, quint64 corpId, Contract::IdType contractId, const JsonCallback &callback) const;
        void fetchCharacterBlueprints(Character::IdType charId, const PaginatedCallback &callback) const;
        void fetchCorporationBlueprints(Character::IdType charId, quint64 corpId, const PaginatedCallback &callback) const;
        void fetchCharacterMiningLedger(Character::IdType charId, const PaginatedCallback &callback) const;
        void fetchGenericName(quint64 id, const PersistentStringCallback &callback) const;
        void fetchGenericNames(const std::vector<quint64> &ids, const PersistentJsonCallback &callback) const;
        void fetchMarketPrices(const JsonCallback &callback) const;
        void fetchIndustryCostIndices(const JsonCallback &callback) const;
        void fetchSovereigntyStructures(const JsonCallback &callback) const;

        void openMarketDetails(EveType::IdType typeId, Character::IdType charId, const ErrorCallback &errorCallback) const;

        void setDestination(quint64 locationId, Character::IdType charId, const ErrorCallback &errorCallback) const;

        ESIInterface &operator =(const ESIInterface &) = default;
        ESIInterface &operator =(ESIInterface &&) = default;

    protected:
        virtual void customEvent(QEvent *event) override;

    private:
        struct ErrorInfo
        {
            QString mMessage;
            int mSSOStatus = 0;

            operator QString() const;
        };

        struct JsonTag {};
        struct PaginatedJsonTag {};
        struct StringTag {};

        template<class Tag>
        struct TaggedInvoke;

        struct PaginatedContext;

        static const int errorLimitCode = 420;
        static const int requestThrottledCode = 429;

        CitadelAccessCache &mCitadelAccessCache;
        ESIInterfaceErrorLimiter &mErrorLimiter;
        ESIOAuth &mOAuth;

        bool mLogReplies = false;

        mutable std::mutex mObjectStateMutex;

        QSettings mSettings;

        template<class T>
        void fetchPaginatedData(const QString &url, QVariantMap parameters, uint page, T &&continuation, const std::shared_ptr<PaginatedContext> &context) const;
        template<class T>
        void fetchPaginatedData(Character::IdType charId,
                                const QString &url,
                                uint page,
                                T &&continuation,
                                const std::shared_ptr<PaginatedContext> &context,
                                bool importingCitadels = false,
                                quint64 citadelId = 0) const;

        template<class T, class ResultTag = JsonTag>
        void get(const QString &url, const QVariantMap &parameters, const T &continuation, uint retries) const;
        template<class T, class ResultTag = JsonTag>
        void get(Character::IdType charId,
                 const QString &url,
                 const QVariantMap &parameters,
                 const T &continuation,
                 uint retries,
                 bool importingCitadels = false,
                 quint64 citadelId = 0) const;

        template<class T>
        void post(Character::IdType charId, const QString &url, const QVariant &data, T &&errorCallback) const;
        template<class T>
        void post(const QString &url, const QVariant &data, ErrorCallback errorCallback, T &&resultCallback) const;

        template<class T>
        void schedulePostErrorLimitRequest(T &&callback, const QNetworkReply &reply) const;

        uint getNumRetries() const;

        template<class T>
        void runNowOrLater(T callback) const;

        template<class T, class U>
        static auto createPaginatedCallback(uint page, T continuation, U fetchNext, std::shared_ptr<PaginatedContext> context);

        static ErrorInfo getError(const QByteArray &reply);
        static ErrorInfo getError(const QString &url, const QVariantMap &parameters, QNetworkReply &reply);
        static QDateTime getExpireTime(const QNetworkReply &reply);
        static uint getPageCount(const QNetworkReply &reply);

        static void showReplyDebugInfo(const QNetworkReply &reply);

        static bool shouldThrottle(int httpStatus);
    };
}
