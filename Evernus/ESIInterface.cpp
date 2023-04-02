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
#include <algorithm>

#include <QtDebug>

#include <QCoreApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QJsonArray>
#include <QUrlQuery>
#include <QThread>
#include <QUrl>

#include "ESIInterfaceErrorLimiter.h"
#include "CitadelAccessCache.h"
#include "NetworkSettings.h"
#include "CallbackEvent.h"
#include "ReplyTimeout.h"
#include "ESIOAuth.h"
#include "ESIUrls.h"

#include "ESIInterface.h"

namespace Evernus
{
    struct ESIInterface::PaginatedContext
    {
        uint mFetchedPages = 0;
    };

    ESIInterface::ErrorInfo::operator QString() const
    {
        return QStringLiteral("%1 (SSO: %2)").arg(mMessage).arg(mSSOStatus);
    }

    template<>
    struct ESIInterface::TaggedInvoke<ESIInterface::JsonTag>
    {
        template<class T>
        static inline void invoke(const QByteArray &data, const QNetworkReply &reply, const T &callback)
        {
            callback(QJsonDocument::fromJson(data), QString{}, getExpireTime(reply));
        }

        template<class T>
        static inline void invoke(const QString &error, const QNetworkReply &reply, const T &callback)
        {
            callback(QJsonDocument{}, error, getExpireTime(reply));
        }

        template<class T>
        static inline void invoke(const QString &error, const T &callback)
        {
            callback(QJsonDocument{}, error, QDateTime{});
        }
    };

    template<>
    struct ESIInterface::TaggedInvoke<ESIInterface::PaginatedJsonTag>
    {
        template<class T>
        static inline void invoke(const QByteArray &data, const QNetworkReply &reply, const T &callback)
        {
            callback(QJsonDocument::fromJson(data), QString{}, getExpireTime(reply), getPageCount(reply));
        }

        template<class T>
        static inline void invoke(const QString &error, const QNetworkReply &reply, const T &callback)
        {
            callback(QJsonDocument{}, error, getExpireTime(reply), getPageCount(reply));
        }

        template<class T>
        static inline void invoke(const QString &error, const T &callback)
        {
            callback(QJsonDocument{}, error, QDateTime{}, 1u);
        }
    };

    template<>
    struct ESIInterface::TaggedInvoke<ESIInterface::StringTag>
    {
        template<class T>
        static inline void invoke(const QByteArray &data, const QNetworkReply &reply, const T &callback)
        {
            callback(QString::fromUtf8(data), QString{}, getExpireTime(reply));
        }

        template<class T>
        static inline void invoke(const QString &error, const QNetworkReply &reply, const T &callback)
        {
            callback(QString{}, error, getExpireTime(reply));
        }

        template<class T>
        static inline void invoke(const QString &error, const T &callback)
        {
            callback(QString{}, error, QDateTime{});
        }
    };

    ESIInterface::ESIInterface(CitadelAccessCache &citadelAccessCache,
                               ESIInterfaceErrorLimiter &errorLimiter,
                               ESIOAuth &oauth,
                               QObject *parent)
        : QObject{parent}
        , mCitadelAccessCache{citadelAccessCache}
        , mErrorLimiter{errorLimiter}
        , mOAuth{oauth}
    {
        QSettings settings;
        mLogReplies = settings.value(NetworkSettings::logESIRepliesKey, mLogReplies).toBool();
    }

    void ESIInterface::fetchMarketOrders(uint regionId, EveType::IdType typeId, const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching market orders for" << regionId << "and" << typeId;
        fetchPaginatedData(QStringLiteral("/v1/markets/%1/orders/").arg(regionId), { { QStringLiteral("type_id"), typeId } }, 1, callback, std::make_shared<PaginatedContext>());
    }

    void ESIInterface::fetchMarketOrders(uint regionId, const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching whole market for" << regionId;
        fetchPaginatedData(QStringLiteral("/v1/markets/%1/orders/").arg(regionId), {}, 1, callback, std::make_shared<PaginatedContext>());
    }

    void ESIInterface::fetchMarketHistory(uint regionId, EveType::IdType typeId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching market history for" << regionId << "and" << typeId;
        get(QStringLiteral("/v1/markets/%1/history/").arg(regionId), { { QStringLiteral("type_id"), typeId } }, callback, getNumRetries());
    }

    void ESIInterface::fetchCitadelMarketOrders(quint64 citadelId, Character::IdType charId, const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching orders from citadel" << citadelId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, true, tr("Cannot fetch citadels with no character selected."), {});
            return;
        }

        if (!mCitadelAccessCache.isAvailable(charId, citadelId))
        {
            qDebug() << "Citadel blacklisted:" << charId << citadelId;
            callback({}, true, {}, {});
            return;
        }

        fetchPaginatedData(charId, QStringLiteral("/v1/markets/structures/%1/").arg(citadelId), 1, callback, std::make_shared<PaginatedContext>(), true, citadelId);
    }

    void ESIInterface::fetchCharacterAssets(Character::IdType charId, const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching character assets for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, true, tr("Cannot fetch assets with no character selected."), {});
            return;
        }

        fetchPaginatedData(charId, QStringLiteral("/v5/characters/%1/assets/").arg(charId), 1, callback, std::make_shared<PaginatedContext>());
    }

    void ESIInterface::fetchCorporationAssets(Character::IdType charId, quint64 corpId, const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching corporation assets for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, true, tr("Cannot fetch assets with no character selected."), {});
            return;
        }

        fetchPaginatedData(charId, QStringLiteral("/v5/corporations/%1/assets/").arg(corpId), 1, callback, std::make_shared<PaginatedContext>());
    }

    void ESIInterface::fetchCharacter(Character::IdType charId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching character" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch character with no character selected."), {});
            return;
        }

        get(QStringLiteral("/v5/characters/%1/").arg(charId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCharacterSkills(Character::IdType charId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching character skills for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch character skills with no character selected."), {});
            return;
        }

        get(charId, QStringLiteral("/v4/characters/%1/skills/").arg(charId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCorporation(quint64 corpId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching corporation" << corpId;
        get(QStringLiteral("/v4/corporations/%1/").arg(corpId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchRaces(const JsonCallback &callback) const
    {
        qDebug() << "Fetching races";
        get(QStringLiteral("/v1/universe/races/"), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchBloodlines(const JsonCallback &callback) const
    {
        qDebug() << "Fetching bloodlines";
        get(QStringLiteral("/v1/universe/bloodlines/"), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchAncestries(const JsonCallback &callback) const
    {
        qDebug() << "Fetching ancestries";
        get(QStringLiteral("/v1/universe/ancestries/"), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCharacterWallet(Character::IdType charId, const StringCallback &callback) const
    {
        qDebug() << "Fetching character wallet for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch character wallet with no character selected."), {});
            return;
        }

        get<decltype(callback), StringTag>(charId, QStringLiteral("/v1/characters/%1/wallet/").arg(charId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCharacterMarketOrders(Character::IdType charId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching character market orders for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch character market orders with no character selected."), {});
            return;
        }

        get(charId, QStringLiteral("/v2/characters/%1/orders/").arg(charId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCorporationMarketOrders(Character::IdType charId, quint64 corpId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching corporation market orders for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch corporation market orders with no character selected."), {});
            return;
        }

        get(charId, QStringLiteral("/v3/corporations/%1/orders/").arg(corpId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCharacterWalletJournal(Character::IdType charId,
                                                   const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching character wallet journal for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, true, tr("Cannot fetch character wallet journal with no character selected."), {});
            return;
        }

        fetchPaginatedData(charId, QStringLiteral("/v6/characters/%1/wallet/journal/").arg(charId), 1, callback, std::make_shared<PaginatedContext>());
    }

    void ESIInterface::fetchCorporationWalletJournal(Character::IdType charId,
                                                     quint64 corpId,
                                                     int division,
                                                     const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching corporation wallet journal for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, true, tr("Cannot fetch corporation wallet journal with no character selected."), {});
            return;
        }

        fetchPaginatedData(charId, QStringLiteral("/v3/corporations/%1/wallets/%2/journal/").arg(corpId).arg(division), 1, callback, std::make_shared<PaginatedContext>());
    }

    void ESIInterface::fetchCharacterWalletTransactions(Character::IdType charId,
                                                        const std::optional<WalletTransaction::IdType> &fromId,
                                                        const JsonCallback &callback) const
    {
        qDebug() << "Fetching character wallet transactions for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch character wallet transactions with no character selected."), {});
            return;
        }

        QVariantMap paramteres;
        if (fromId)
            paramteres[QStringLiteral("from_id")] = *fromId;

        get(charId, QStringLiteral("/v1/characters/%1/wallet/transactions/").arg(charId), paramteres, callback, getNumRetries());
    }

    void ESIInterface::fetchCorporationWalletTransactions(Character::IdType charId,
                                                          quint64 corpId,
                                                          int division,
                                                          const std::optional<WalletJournalEntry::IdType> &fromId,
                                                          const JsonCallback &callback) const
    {
        qDebug() << "Fetching corporation wallet transactions for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch corporation wallet transactions with no character selected."), {});
            return;
        }

        QVariantMap paramteres;
        if (fromId)
            paramteres[QStringLiteral("from_id")] = *fromId;

        get(charId, QStringLiteral("/v1/corporations/%1/wallets/%2/transactions/").arg(corpId).arg(division), paramteres, callback, getNumRetries());
    }

    void ESIInterface::fetchCharacterContracts(Character::IdType charId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching character contracts for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch character contracts with no character selected."), {});
            return;
        }

        get(charId, QStringLiteral("/v1/characters/%1/contracts/").arg(charId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCharacterContractItems(Character::IdType charId, Contract::IdType contractId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching character contract items for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch character contract items with no character selected."), {});
            return;
        }

        get(charId, QStringLiteral("/v1/characters/%1/contracts/%2/items/").arg(charId).arg(contractId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCorporationContracts(Character::IdType charId, quint64 corpId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching corporation contracts for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch corporation contracts with no character selected."), {});
            return;
        }

        get(charId, QStringLiteral("/v1/corporations/%1/contracts/").arg(corpId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCorporationContractItems(Character::IdType charId, quint64 corpId, Contract::IdType contractId, const JsonCallback &callback) const
    {
        qDebug() << "Fetching corporation contract items for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, tr("Cannot fetch corporation contract items with no character selected."), {});
            return;
        }

        get(charId, QStringLiteral("/v1/corporations/%1/contracts/%2/items/").arg(corpId).arg(contractId), {}, callback, getNumRetries());
    }

    void ESIInterface::fetchCharacterBlueprints(Character::IdType charId, const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching character blueprints for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, true, tr("Cannot fetch character blueprints with no character selected."), {});
            return;
        }

        fetchPaginatedData(charId, QStringLiteral("/v2/characters/%1/blueprints/").arg(charId), 1, callback, std::make_shared<PaginatedContext>());
    }

    void ESIInterface::fetchCorporationBlueprints(Character::IdType charId, quint64 corpId, const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching corporation blueprints for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, true, tr("Cannot fetch corporation blueprints with no character selected."), {});
            return;
        }

        fetchPaginatedData(charId, QStringLiteral("/v2/corporations/%1/blueprints/").arg(corpId), 1, callback, std::make_shared<PaginatedContext>());
    }

    void ESIInterface::fetchCharacterMiningLedger(Character::IdType charId, const PaginatedCallback &callback) const
    {
        qDebug() << "Fetching character mining ledger for" << charId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            callback({}, true, tr("Cannot fetch character mining ledger with no character selected."), {});
            return;
        }

        fetchPaginatedData(charId, QStringLiteral("/v1/characters/%1/mining/").arg(charId), 1, callback, std::make_shared<PaginatedContext>());
    }

    void ESIInterface::fetchGenericName(quint64 id, const PersistentStringCallback &callback) const
    {
        qDebug() << "Fetching generic name:" << id;

        QVariantList idArray;
        idArray << id;

        post(QStringLiteral("/v2/universe/names/"),
             idArray,
             [=](const auto &error) {
                 callback({}, error);
             }, [=](const auto &data) {
                 const auto doc = QJsonDocument::fromJson(data);
                 const auto names = doc.array();

                 if (Q_LIKELY(names.size() > 0))
                     callback(names.first().toObject().value(QStringLiteral("name")).toString(), {});
                 else
                     callback({}, tr("Missing name data for: %1").arg(id));
             }
        );
    }

    void ESIInterface::fetchGenericNames(const std::vector<quint64> &ids, const PersistentJsonCallback &callback) const
    {
        qDebug() << "Fetching generic names:" << ids.size();

        QVariantList idArray;
        std::transform(std::begin(ids), std::end(ids), std::back_inserter(idArray), [](auto id) {
            return static_cast<double>(id);
        });

        post(QStringLiteral("/v2/universe/names/"),
             idArray,
             [=](const auto &error) {
                 callback({}, error);
             }, [=](const auto &data) {
                 callback(QJsonDocument::fromJson(data), {});
             }
        );
    }

    void ESIInterface::fetchMarketPrices(const JsonCallback &callback) const
    {
        qDebug() << "Fetching market prices.";
        get(QStringLiteral("/v1/markets/prices/"), {}, [=](auto &&data, const auto &error, const auto &expires) {
            callback(std::move(data), error, expires);
        }, getNumRetries());
    }

    void ESIInterface::fetchIndustryCostIndices(const JsonCallback &callback) const
    {
        qDebug() << "Fetching industry cost indices.";
        get(QStringLiteral("/v1/industry/systems/"), {}, [=](auto &&data, const auto &error, const auto &expires) {
            callback(std::move(data), error, expires);
        }, getNumRetries());
    }

    void ESIInterface::fetchSovereigntyStructures(const JsonCallback &callback) const
    {
        qDebug() << "Fetching sovereignty structures.";
        get(QStringLiteral("/v1/sovereignty/structures/"), {}, [=](auto &&data, const auto &error, const auto &expires) {
            callback(std::move(data), error, expires);
        }, getNumRetries());
    }

    void ESIInterface::openMarketDetails(EveType::IdType typeId, Character::IdType charId, const ErrorCallback &errorCallback) const
    {
        qDebug() << "Opening market details for" << typeId;

        if (Q_UNLIKELY(charId == Character::invalidId))
        {
            errorCallback(tr("Cannot open market window for invalid character. Check if there's a character associated with the item you wish to view."));
            return;
        }

        post(charId, QStringLiteral("/v1/ui/openwindow/marketdetails/?type_id=%1").arg(typeId), {}, std::move(errorCallback));
    }

    void ESIInterface::setDestination(quint64 locationId, Character::IdType charId, const ErrorCallback &errorCallback) const
    {
        qDebug() << "Setting destination:" << locationId;

        QVariantMap data;
        data[QStringLiteral("destination_id")] = locationId;
        data[QStringLiteral("add_to_beginning")] = QStringLiteral("false");
        data[QStringLiteral("clear_other_waypoints")] = QStringLiteral("true");

        post(charId, QStringLiteral("/v2/ui/autopilot/waypoint/"), data, std::move(errorCallback));
    }

    void ESIInterface::customEvent(QEvent *event)
    {
        Q_ASSERT(event != nullptr);

        if (event->type() == CallbackEvent::customType())
        {
            event->accept();
            static_cast<CallbackEvent *>(event)->execute();
        }
        else
        {
            event->ignore();
        }
    }

    template<class T, class U>
    auto ESIInterface::createPaginatedCallback(uint page, T continuation, U fetchNext, std::shared_ptr<PaginatedContext> context)
    {
        return [=, continuation = std::move(continuation), fetchNext = std::move(fetchNext), context = std::move(context)]
               (auto &&response, const auto &error, const auto &expires, auto pages) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                continuation({}, true, error, expires);
                return;
            }

            Q_ASSERT(context);
            ++context->mFetchedPages;

            if (pages > 0)
            {
                if (page == 1)
                {
                    qDebug() << "Got number of pages for paginated request:" << pages;

                    if (pages == 1)
                    {
                        continuation(std::move(response), true, QString{}, expires);
                    }
                    else
                    {
                        continuation(std::move(response), false, QString{}, expires);

                        for (auto nextPage = 2u; nextPage <= pages; ++nextPage)
                            fetchNext(nextPage);
                    }
                }
                else if (context->mFetchedPages >= pages)
                {
                    continuation(std::move(response), true, QString{}, expires);
                }
                else
                {
                    continuation(std::move(response), false, QString{}, expires);
                }
            }
            else
            {
                const auto array = response.array();
                if (array.isEmpty())
                {
                    continuation(std::move(response), true, QString{}, expires);
                }
                else
                {
                    continuation(std::move(response), false, QString{}, expires);
                    fetchNext(page + 1);
                }
            }
        };
    }

    template<class T>
    void ESIInterface::fetchPaginatedData(const QString &url, QVariantMap parameters, uint page, T &&continuation, const std::shared_ptr<PaginatedContext> &context) const
    {
        const auto callback = createPaginatedCallback(
            page,
            continuation,
            [=](auto nextPage) {
                fetchPaginatedData(url, parameters, nextPage, continuation, context);
            },
            context
        );

        parameters[QStringLiteral("page")] = page;
        get<decltype(callback), PaginatedJsonTag>(url, parameters, callback, getNumRetries());
    }

    template<class T>
    void ESIInterface::fetchPaginatedData(Character::IdType charId,
                                          const QString &url,
                                          uint page,
                                          T &&continuation,
                                          const std::shared_ptr<PaginatedContext> &context,
                                          bool importingCitadels,
                                          quint64 citadelId) const
    {
        const auto callback = createPaginatedCallback(
            page,
            continuation,
            [=](auto nextPage) {
                fetchPaginatedData(charId, url, nextPage, continuation, context, importingCitadels, citadelId);
            },
            context
        );

        get<decltype(callback), PaginatedJsonTag>(
            charId,
            url,
            { { QStringLiteral("page"), page } },
            callback,
            getNumRetries(),
            importingCitadels,
            citadelId
        );
    }

    template<class T, class ResultTag>
    void ESIInterface::get(const QString &url, const QVariantMap &parameters, const T &continuation, uint retries) const
    {
        runNowOrLater([=] {
            auto reply = mOAuth.get(ESIUrls::esiUrl + url, parameters);
            Q_ASSERT(reply != nullptr);

            qDebug() << "ESI request:" << reply << "" << url << ":" << parameters;
            qDebug() << "Retries" << retries;

            new ReplyTimeout{*reply};

            connect(reply, &QNetworkReply::finished, this, [=] {
                reply->deleteLater();

                showReplyDebugInfo(*reply);

                const auto error = reply->error();
                if (Q_UNLIKELY(error != QNetworkReply::NoError))
                {
                    const auto httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    const auto errorInfo = getError(url, parameters, *reply);

                    qWarning() << "Error for request" << reply << ":" << url << parameters << ":" << httpStatus << errorInfo;

                    if (shouldThrottle(httpStatus))  // error limit reached?
                    {
                        schedulePostErrorLimitRequest([=] {
                            get<T, ResultTag>(url, parameters, continuation, retries);
                        }, *reply);
                    }
                    else
                    {
                        if (retries > 0)
                            get<T, ResultTag>(url, parameters, continuation, retries - 1);
                        else
                            TaggedInvoke<ResultTag>::invoke(errorInfo, *reply, continuation);
                    }
                }
                else
                {
                    const auto data = reply->readAll();
                    if (mLogReplies)
                        qDebug() << reply << data;

                    TaggedInvoke<ResultTag>::invoke(data, *reply, continuation);
                }
            });
        });
    }

    template<class T, class ResultTag>
    void ESIInterface::get(Character::IdType charId,
                           const QString &url,
                           const QVariantMap &parameters,
                           const T &continuation,
                           uint retries,
                           bool importingCitadels,
                           quint64 citadelId) const
    {
        runNowOrLater([=] {
            mOAuth.get(charId, ESIUrls::esiUrl + url, parameters, [=](auto &reply) {
                qDebug() << "ESI request:" << url << ":" << parameters;
                qDebug() << "Retries" << retries;

                showReplyDebugInfo(reply);

                const auto error = reply.error();
                if (Q_UNLIKELY(error != QNetworkReply::NoError))
                {
                    const auto httpStatus = reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    const auto parsedError = getError(url, parameters, reply);

                    qWarning() << "Error for request:" << httpStatus << parsedError;

                    if (shouldThrottle(httpStatus))  // error limit reached?
                    {
                        schedulePostErrorLimitRequest([=] {
                            get<T, ResultTag>(charId, url, parameters, continuation, retries, importingCitadels, citadelId);
                        }, reply);
                    }
                    else
                    {
                        if (error == QNetworkReply::ContentAccessDenied)
                        {
                            if (importingCitadels && parsedError.mSSOStatus == 0)
                            {
                                if (citadelId != 0)
                                {
                                    qDebug() << "Blacklisting citadel:" << citadelId << charId;
                                    mCitadelAccessCache.blacklist(charId, citadelId);
                                }

                                TaggedInvoke<ResultTag>::invoke(QString{}, reply, continuation);
                            }
                            else
                            {
                                TaggedInvoke<ResultTag>::invoke(parsedError, reply, continuation);
                            }
                        }
                        else if (retries > 0)
                        {
                            get<T, ResultTag>(charId, url, parameters, continuation, retries - 1, importingCitadels, citadelId);
                        }
                        else
                        {
                            TaggedInvoke<ResultTag>::invoke(parsedError, reply, continuation);
                        }
                    }
                }
                else
                {
                    const auto data = reply.readAll();
                    if (mLogReplies)
                        qDebug() << url << data;

                    TaggedInvoke<ResultTag>::invoke(data, reply, continuation);
                }
            }, [=](const auto &error) {
                TaggedInvoke<ResultTag>::invoke(error, continuation);
            });
        });
    }

    template<class T>
    void ESIInterface::post(Character::IdType charId, const QString &url, const QVariant &data, T &&errorCallback) const
    {
        runNowOrLater([=] {
            mOAuth.post(charId, ESIUrls::esiUrl + url, data, [=](auto &reply) {
                qDebug() << "ESI request:" << url << ":" << data;

                showReplyDebugInfo(reply);

                const auto error = reply.error();
                if (Q_UNLIKELY(error != QNetworkReply::NoError))
                {
                    const auto httpStatus = reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    const auto parsedError = getError(url, {}, reply);

                    qWarning() << "Error for request:" << httpStatus << parsedError;

                    if (shouldThrottle(httpStatus))  // error limit reached?
                    {
                        schedulePostErrorLimitRequest([=] {
                            post(charId, url, data, std::move(errorCallback));
                        }, reply);
                    }
                    else
                    {
                        errorCallback(parsedError);
                    }
                }
                else
                {
                    const auto data = reply.readAll();
                    if (mLogReplies)
                        qDebug() << url << data;

                    const auto error = getError(data);
                    if (!error.mMessage.isEmpty())
                        errorCallback(error);
                }
            }, [=](const auto &error) {
                errorCallback(error);
            });
        });
    }

    template<class T>
    void ESIInterface::post(const QString &url, const QVariant &data, ErrorCallback errorCallback, T &&resultCallback) const
    {
        runNowOrLater([=] {
            auto reply = mOAuth.post(ESIUrls::esiUrl + url, data);
            Q_ASSERT(reply != nullptr);

            qDebug() << "ESI request" << reply << ":" << url << ":" << data;

            new ReplyTimeout{*reply};

            connect(reply, &QNetworkReply::finished, this, [=] {
                reply->deleteLater();

                showReplyDebugInfo(*reply);

                const auto error = reply->error();
                if (Q_UNLIKELY(error != QNetworkReply::NoError))
                {
                    const auto httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    const auto parsedError = getError(url, {}, *reply);

                    qWarning() << "Error for request" << reply << ":" << url << ":" << httpStatus << parsedError;

                    if (shouldThrottle(httpStatus))  // error limit reached?
                    {
                        schedulePostErrorLimitRequest([=] {
                            post(url, data, errorCallback, resultCallback);
                        }, *reply);
                    }
                    else
                    {
                        errorCallback(parsedError);
                    }
                }
                else
                {
                    const auto resultText = reply->readAll();
                    if (mLogReplies)
                        qDebug() << url << resultText;

                    const auto error = getError(resultText);
                    if (!error.mMessage.isEmpty())
                        errorCallback(error);
                    else
                        resultCallback(resultText);
                }
            });
        });
    }

    template<class T>
    void ESIInterface::schedulePostErrorLimitRequest(T &&callback, const QNetworkReply &reply) const
    {
        const auto esiLimitHeader = QByteArrayLiteral("X-Esi-Error-Limit-Reset");

        auto errorTimeout = 10u;

        if (reply.hasRawHeader(esiLimitHeader))
        {
            errorTimeout = reply.rawHeader(esiLimitHeader).toUInt();
        }
        else
        {
            const auto retryAfter = reply.rawHeader(QByteArrayLiteral("Retry-After"));
            const auto targetDate = QDateTime::fromString(retryAfter, Qt::RFC2822Date);
            if (targetDate.isValid())
                errorTimeout = QDateTime::currentDateTime().secsTo(targetDate);
            else
                errorTimeout = retryAfter.toUInt();
        }

        mErrorLimiter.addCallback(std::move(callback), std::chrono::seconds{errorTimeout});
    }

    uint ESIInterface::getNumRetries() const
    {
        return mSettings.value(NetworkSettings::maxRetriesKey, NetworkSettings::maxRetriesDefault).toUInt();
    }

    template<class T>
    void ESIInterface::runNowOrLater(T callback) const
    {
        if (thread() == QThread::currentThread())
        {
            callback();
        }
        else
        {
            std::lock_guard<std::mutex> lock{mObjectStateMutex};

            // this is actually a design flaw in Qt - postEvent() is supposed to be thread-safe and the receiver is synchronized inside
            // but!
            // this is nowhere stated and receiver is non-const, despite no visible state change, so we cannot simply depend on internal implementation
            // hence an artificial mutex and an artificial const_cast
            // because obviously nobody at Qt has heard of mutable mutexes
            QCoreApplication::postEvent(const_cast<ESIInterface *>(this), new CallbackEvent{std::move(callback)});
        }
    }

    ESIInterface::ErrorInfo ESIInterface::getError(const QByteArray &reply)
    {
        // try to get ESI error
        const auto error = QJsonDocument::fromJson(reply).object();
        return { error.value(QStringLiteral("error")).toString(), error.value(QStringLiteral("sso_status")).toInt() };
    }

    ESIInterface::ErrorInfo ESIInterface::getError(const QString &url, const QVariantMap &parameters, QNetworkReply &reply)
    {
        // try to get ESI error
        auto error = getError(reply.readAll());
        if (error.mMessage.isEmpty())
            error.mMessage = reply.errorString();

        QStringList query;
        for (auto param = std::begin(parameters); param != std::end(parameters); ++param)
            query << QStringLiteral("%1=%2").arg(param.key()).arg(param.value().toString());

        return { QStringLiteral("%1?%2: %3").arg(url).arg(query.join('&')).arg(error.mMessage), error.mSSOStatus };
    }

    QDateTime ESIInterface::getExpireTime(const QNetworkReply &reply)
    {
        return QDateTime::fromString(reply.rawHeader(QByteArrayLiteral("expires")), Qt::RFC2822Date);
    }

    uint ESIInterface::getPageCount(const QNetworkReply &reply)
    {
        return reply.rawHeader(QByteArrayLiteral("X-Pages")).toUInt();
    }

    void ESIInterface::showReplyDebugInfo(const QNetworkReply &reply)
    {
        qDebug() << "X-Esi-Ab-Test:" << reply.rawHeader(QByteArrayLiteral("X-Esi-Ab-Test"));
    }

    bool ESIInterface::shouldThrottle(int httpStatus)
    {
        return httpStatus == errorLimitCode || httpStatus == requestThrottledCode;
    }
}
