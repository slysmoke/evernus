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
#include <atomic>
#include <limits>
#include <mutex>

#include <QtConcurrent>
#include <QtDebug>

#include <QFutureWatcher>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QApplication>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QSettings>
#include <QHash>

#include <boost/scope_exit.hpp>

#include "SovereigntyStructure.h"
#include "ESIInterfaceManager.h"
#include "EveDataProvider.h"
#include "NetworkSettings.h"
#include "ExternalOrder.h"
#include "ReplyTimeout.h"
#include "MiningLedger.h"
#include "Blueprint.h"
#include "Defines.h"

#include "ESIManager.h"

namespace Evernus
{
    const QString ESIManager::firstTimeCitadelOrderImportKey = "import/firstTimeCitadelOrderImport";

    bool ESIManager::mFirstTimeCitadelOrderImport = true;

    ESIManager::ESIManager(const EveDataProvider &dataProvider,
                           ESIInterfaceManager &interfaceManager,
                           QObject *parent)
        : QObject{parent}
        , mDataProvider{dataProvider}
        , mInterfaceManager{interfaceManager}
    {
        QSettings settings;
        mFirstTimeCitadelOrderImport = settings.value(firstTimeCitadelOrderImportKey, mFirstTimeCitadelOrderImport).toBool();
    }

    void ESIManager::fetchMarketOrders(uint regionId,
                                       EveType::IdType typeId,
                                       const MarketOrderCallback &callback) const
    {
        qDebug() << "Started market order import at" << QDateTime::currentDateTime();
        getInterface().fetchMarketOrders(regionId, typeId, getMarketOrderCallback(regionId, callback));
    }

    void ESIManager::fetchMarketHistory(uint regionId,
                                        EveType::IdType typeId,
                                        const Callback<HistoryMap> &callback) const
    {
        qDebug() << "Started history import at" << QDateTime::currentDateTime();

#if EVERNUS_CLANG_LAMBDA_CAPTURE_BUG
        getInterface().fetchMarketHistory(regionId, typeId, [=, callback = callback](auto &&data, const auto &error, const auto &expires) {
#else
        getInterface().fetchMarketHistory(regionId, typeId, [=](auto &&data, const auto &error, const auto &expires) {
#endif
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const std::function<std::pair<QDate, MarketHistoryEntry> (const QJsonValue &)> parseItem = [](const auto &item) {
                const auto itemObject = item.toObject();
                auto date = QDate::fromString(itemObject.value(QStringLiteral("date")).toString(), Qt::ISODate);

                MarketHistoryEntry entry;
                entry.mAvgPrice = itemObject.value(QStringLiteral("average")).toDouble();
                entry.mHighPrice = itemObject.value(QStringLiteral("highest")).toDouble();
                entry.mLowPrice = itemObject.value(QStringLiteral("lowest")).toDouble();
                entry.mOrders = itemObject.value(QStringLiteral("order_count")).toInt();
                entry.mVolume = itemObject.value(QStringLiteral("volume")).toDouble();

                return std::make_pair(std::move(date), std::move(entry));
            };

            const auto insertItem = [](auto &history, auto &item) {
                history.insert(std::move(item));
            };

            callback(QtConcurrent::blockingMappedReduced<HistoryMap>(data.array(), parseItem, insertItem), {}, expires);
        });
    }

    void ESIManager::fetchMarketOrders(uint regionId, const MarketOrderCallback &callback) const
    {
        qDebug() << "Started market order import at" << QDateTime::currentDateTime();
        getInterface().fetchMarketOrders(regionId, getMarketOrderCallback(regionId, callback));
    }

    void ESIManager::fetchCitadelMarketOrders(quint64 citadelId, uint regionId, Character::IdType charId, const MarketOrderCallback &callback) const
    {
        if (mFirstTimeCitadelOrderImport)
        {
            mFirstTimeCitadelOrderImport = false;

            QSettings settings;
            settings.setValue(firstTimeCitadelOrderImportKey, false);

            QMessageBox::information(nullptr, tr("Citadel order import"), tr(
                "Seems like you are importing citadel orders for the first time. CCP only allows importing orders from citadels you have access to. "
                "This means you need to authenticate yourself with Eve SSO, if you haven't done that already (please wait for the SSO window to open).\n\n"
                "Also, please note that due to large numbers of citadels in some regions, the import might take much longer. Remember you can toggle citadel import "
                "in the Preferences."
            ));
        }

        qDebug() << "Started citadel market order import at" << QDateTime::currentDateTime();
        getInterface().fetchCitadelMarketOrders(citadelId, charId, getMarketOrderCallback(regionId, callback));
    }

    void ESIManager::fetchCharacterAssets(Character::IdType charId, const AssetCallback &callback) const
    {
        getInterface().fetchCharacterAssets(charId, getAssetListCallback(charId, callback));
    }

    void ESIManager::fetchCorporationAssets(Character::IdType charId, quint64 corpId, const AssetCallback &callback) const
    {
        getInterface().fetchCorporationAssets(charId, corpId, getAssetListCallback(charId, callback));
    }

    void ESIManager::fetchCharacter(Character::IdType charId, const Callback<Character> &callback) const
    {
        getInterface().fetchCharacter(charId, [=](auto &&publicData, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            getInterface().fetchCharacterSkills(
                charId, [=, publicData = std::move(publicData)](auto &&skillData, const auto &error, const auto &expires) {
                    if (Q_UNLIKELY(!error.isEmpty()))
                    {
                        callback({}, error, expires);
                        return;
                    }

                    const auto publicDataObj = publicData.object();

                    getInterface().fetchCorporation(
                        publicDataObj.value(QStringLiteral("corporation_id")).toDouble(),
                        [=, publicDataObj = std::move(publicDataObj), skillData = std::move(skillData)](auto &&corpData, const auto &error, const auto &expires) {
                            if (Q_UNLIKELY(!error.isEmpty()))
                            {
                                callback({}, error, expires);
                                return;
                            }

                            getInterface().fetchCharacterWallet(
                                charId,
                                [=, corpData = std::move(corpData), publicDataObj = std::move(publicDataObj), skillData = std::move(skillData)](auto &&walletData, const auto &error, const auto &expires) {
                                    if (Q_UNLIKELY(!error.isEmpty()))
                                    {
                                        callback({}, error, expires);
                                        return;
                                    }

                                    const auto corpDataObj = corpData.object();

                                    Character character{charId};
                                    character.setName(publicDataObj.value(QStringLiteral("name")).toString());
                                    character.setCorporationName(corpDataObj.value(QStringLiteral("name")).toString());
                                    character.setCorporationId(publicDataObj.value(QStringLiteral("corporation_id")).toDouble());
                                    character.setRace(mDataProvider.getRaceName(publicDataObj.value(QStringLiteral("race_id")).toDouble()));
                                    character.setBloodline(mDataProvider.getBloodlineName(publicDataObj.value(QStringLiteral("bloodline_id")).toDouble()));
                                    character.setAncestry(mDataProvider.getAncestryName(publicDataObj.value(QStringLiteral("ancestry_id")).toDouble()));
                                    character.setGender(publicDataObj.value(QStringLiteral("gender")).toString());
                                    character.setISK(walletData.toDouble());

                                    CharacterData::OrderAmountSkills orderAmountSkills;
                                    CharacterData::TradeRangeSkills tradeRangeSkills;
                                    CharacterData::FeeSkills feeSkills;
                                    CharacterData::ContractSkills contractSkills;
                                    CharacterData::ReprocessingSkills reprocessingSkills;
                                    CharacterData::IndustrySkills industrySkills;

                                    const auto skillLevelProperty = QStringLiteral("active_skill_level");

                                    const auto skills = skillData.object().value(QStringLiteral("skills")).toArray();
                                    for (const auto &skill : skills)
                                    {
                                        const auto skillObj = skill.toObject();
                                        switch (skillObj.value(QStringLiteral("skill_id")).toInt()) {
                                        case 3443:
                                            orderAmountSkills.mTrade = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3444:
                                            orderAmountSkills.mRetail = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 16596:
                                            orderAmountSkills.mWholesale = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 18580:
                                            orderAmountSkills.mTycoon = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 16598:
                                            tradeRangeSkills.mMarketing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 16594:
                                            tradeRangeSkills.mProcurement = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 16595:
                                            tradeRangeSkills.mDaytrading = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3447:
                                            tradeRangeSkills.mVisibility = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 16622:
                                            feeSkills.mAccounting = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3446:
                                            feeSkills.mBrokerRelations = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 16597:
                                            feeSkills.mMarginTrading = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 25235:
                                            contractSkills.mContracting = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12180:
                                            reprocessingSkills.mArkonorProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12181:
                                            reprocessingSkills.mBistotProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12182:
                                            reprocessingSkills.mCrokiteProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12183:
                                            reprocessingSkills.mDarkOchreProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12185:
                                            reprocessingSkills.mHedbergiteProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12186:
                                            reprocessingSkills.mHemorphiteProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 18025:
                                            reprocessingSkills.mIceProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12187:
                                            reprocessingSkills.mJaspetProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12188:
                                            reprocessingSkills.mKerniteProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12189:
                                            reprocessingSkills.mMercoxitProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12190:
                                            reprocessingSkills.mOmberProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12191:
                                            reprocessingSkills.mPlagioclaseProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12192:
                                            reprocessingSkills.mPyroxeresProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3385:
                                            reprocessingSkills.mReprocessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3389:
                                            reprocessingSkills.mReprocessingEfficiency = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12193:
                                            reprocessingSkills.mScorditeProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12196:
                                            reprocessingSkills.mScrapmetalProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12194:
                                            reprocessingSkills.mSpodumainProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 12195:
                                            reprocessingSkills.mVeldsparProcessing = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3380:
                                            industrySkills.mIndustry = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3388:
                                            industrySkills.mAdvancedIndustry = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3398:
                                            industrySkills.mAdvancedLargeShipConstruction = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3397:
                                            industrySkills.mAdvancedMediumShipConstruction = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3395:
                                            industrySkills.mAdvancedSmallShipConstruction = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11444:
                                            industrySkills.mAmarrStarshipEngineering = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 3396:
                                            industrySkills.mAvancedIndustrialShipConstruction = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11454:
                                            industrySkills.mCaldariStarshipEngineering = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11448:
                                            industrySkills.mElectromagneticPhysics = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11453:
                                            industrySkills.mElectronicEngineering = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11450:
                                            industrySkills.mGallenteStarshipEngineering = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11446:
                                            industrySkills.mGravitonPhysics = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11433:
                                            industrySkills.mHighEnergyPhysics = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11443:
                                            industrySkills.mHydromagneticPhysics = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11447:
                                            industrySkills.mLaserPhysics = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11452:
                                            industrySkills.mMechanicalEngineering = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11445:
                                            industrySkills.mMinmatarStarshipEngineering = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11529:
                                            industrySkills.mMolecularEngineering = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11451:
                                            industrySkills.mNuclearPhysics = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11441:
                                            industrySkills.mPlasmaPhysics = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11455:
                                            industrySkills.mQuantumPhysics = skillObj.value(skillLevelProperty).toInt();
                                            break;
                                        case 11449:
                                            industrySkills.mRocketScience = skillObj.value(skillLevelProperty).toInt();
                                        }
                                    }

                                    character.setOrderAmountSkills(std::move(orderAmountSkills));
                                    character.setTradeRangeSkills(std::move(tradeRangeSkills));
                                    character.setFeeSkills(std::move(feeSkills));
                                    character.setContractSkills(std::move(contractSkills));
                                    character.setReprocessingSkills(std::move(reprocessingSkills));
                                    character.setIndustrySkills(std::move(industrySkills));

                                    callback(std::move(character), {}, expires);
                            });
                    });
            });
        });
    }

    void ESIManager::fetchRaces(const Callback<NameMap> &callback) const
    {
        getInterface().fetchRaces([=](auto &&data, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto races = data.array();

            NameMap names;
            names.reserve(races.size());

            for (const auto &race : races)
            {
                const auto raceObj = race.toObject();
                names.emplace(
                    static_cast<quint64>(raceObj.value(QStringLiteral("race_id")).toDouble()),
                    raceObj.value(QStringLiteral("name")).toString()
                );
            }

            callback(std::move(names), {}, expires);
        });
    }

    void ESIManager::fetchBloodlines(const Callback<NameMap> &callback) const
    {
        getInterface().fetchBloodlines([=](auto &&data, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto bloodlines = data.array();

            NameMap names;
            names.reserve(bloodlines.size());

            for (const auto &bloodline : bloodlines)
            {
                const auto bloodlineObj = bloodline.toObject();
                names.emplace(
                    static_cast<quint64>(bloodlineObj.value(QStringLiteral("bloodline_id")).toDouble()),
                    bloodlineObj.value(QStringLiteral("name")).toString()
                );
            }

            callback(std::move(names), {}, expires);
        });
    }

    void ESIManager::fetchAncestries(const Callback<NameMap> &callback) const
    {
        getInterface().fetchAncestries([=](auto &&data, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto ancestries = data.array();

            NameMap names;
            names.reserve(ancestries.size());

            for (const auto &ancestry : ancestries)
            {
                const auto ancestryObj = ancestry.toObject();
                names.emplace(
                    static_cast<quint64>(ancestryObj.value(QStringLiteral("id")).toDouble()),
                    ancestryObj.value(QStringLiteral("name")).toString()
                );
            }

            callback(std::move(names), {}, expires);
        });
    }

    void ESIManager::fetchCharacterMarketOrders(Character::IdType charId, const MarketOrdersCallback &callback) const
    {
        getInterface().fetchCharacterMarketOrders(charId, getMarketOrdersCallback(charId, callback));
    }

    void ESIManager::fetchCorporationMarketOrders(Character::IdType charId, quint64 corpId, const MarketOrdersCallback &callback) const
    {
        getInterface().fetchCorporationMarketOrders(charId, corpId, getMarketOrdersCallback(charId, callback));
    }

    void ESIManager::fetchCharacterWalletJournal(Character::IdType charId, WalletJournalEntry::IdType tillId, const WalletJournalCallback &callback) const
    {
        getInterface().fetchCharacterWalletJournal(
            charId,
            getWalletJournalCallback(charId, 0, tillId, callback)
        );
    }

    void ESIManager::fetchCorporationWalletJournal(Character::IdType charId,
                                                   quint64 corpId,
                                                   int division,
                                                   WalletJournalEntry::IdType tillId,
                                                   const WalletJournalCallback &callback) const
    {
        getInterface().fetchCorporationWalletJournal(
            charId,
            corpId,
            division,
            getWalletJournalCallback(charId, corpId, tillId, callback)
        );
    }

    void ESIManager::fetchCharacterWalletTransactions(Character::IdType charId,
                                                      WalletTransaction::IdType tillId,
                                                      const WalletTransactionsCallback &callback) const
    {
        fetchCharacterWalletTransactions(charId, std::nullopt, tillId, std::make_shared<WalletTransactions>(), callback);
    }

    void ESIManager::fetchCorporationWalletTransactions(Character::IdType charId,
                                                        quint64 corpId,
                                                        int division,
                                                        WalletTransaction::IdType tillId,
                                                        const WalletTransactionsCallback &callback) const
    {
        fetchCorporationWalletTransactions(charId, corpId, division, std::nullopt, tillId, std::make_shared<WalletTransactions>(), callback);
    }

    void ESIManager::fetchCharacterMiningLedger(Character::IdType charId, const Callback<MiningLedgerList> &callback) const
    {
        auto ledgerResult = std::make_shared<MiningLedgerList>();
        getInterface().fetchCharacterMiningLedger(charId, [=, ledgerResult = std::move(ledgerResult)]
                                                                 (auto &&data, auto atEnd, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto ledgerArray = data.array();
            const auto curSize = ledgerResult->size();
            ledgerResult->resize(curSize + ledgerArray.size());

            std::atomic_size_t nextIndex{curSize};

            QtConcurrent::blockingMap(ledgerArray, [&, charId](const auto &ledger) {
                const auto ledgerObj = ledger.toObject();

                auto date = QDate::fromString(ledgerObj.value(QStringLiteral("date")).toString(), Qt::ISODate);
                if (Q_UNLIKELY(!date.isValid()))
                    date = QDate::currentDate();

                auto &curLedger = (*ledgerResult)[nextIndex++];
                curLedger.setCharacterId(charId);
                curLedger.setDate(std::move(date));
                curLedger.setQuantity(ledgerObj.value(QStringLiteral("quantity")).toDouble());
                curLedger.setSolarSystemId(ledgerObj.value(QStringLiteral("solar_system_id")).toDouble());
                curLedger.setTypeId(ledgerObj.value(QStringLiteral("type_id")).toDouble());
            });

            if (atEnd)
                callback(std::move(*ledgerResult), {}, expires);
        });
    }

    void ESIManager::fetchGenericName(quint64 id, const PesistentDataCallback<QString> &callback) const
    {
        getInterface().fetchGenericName(id, [=](auto &&data, const auto &error) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error);
                return;
            }

            callback(std::move(data), error);
        });
    }

    void ESIManager::fetchGenericNames(const std::vector<quint64> &ids, const PesistentDataCallback<std::unordered_map<quint64, QString>> &callback) const
    {
        const auto maxPerRequest = 1000;

        struct SharedState
        {
            std::unordered_map<quint64, QString> mResult;
            QString mError;
            bool mEmittedError = false;
        };

        auto state = std::make_shared<SharedState>();
        auto current = 0u;

        const auto transformCallback = [=, totalSize = ids.size()](auto &&data, const auto &error) {
            if (state->mError.isEmpty() && !error.isEmpty())
                state->mError = error;

            if (!state->mError.isEmpty())
            {
                if (!state->mEmittedError)
                {
                    state->mEmittedError = true;
                    callback({}, state->mError);
                }

                return;
            }

            const auto names = data.array();

            state->mResult.reserve(state->mResult.size() + names.size());

            std::transform(std::begin(names), std::end(names), std::inserter(state->mResult, std::end(state->mResult)), [](const auto &name) {
                const auto nameObj = name.toObject();
                return std::make_pair(static_cast<quint64>(nameObj.value(QStringLiteral("id")).toDouble()), nameObj.value(QStringLiteral("name")).toString());
            });

            if (state->mResult.size() == totalSize)
                callback(std::move(state->mResult), {});
        };

        for (; current < ids.size() / maxPerRequest; ++current)
        {
            getInterface().fetchGenericNames(
                std::vector<quint64>(std::begin(ids) + current * maxPerRequest, std::min(std::end(ids), std::begin(ids) + (current + 1) * maxPerRequest)),
                transformCallback
            );
        }

        if (current < ids.size())
            getInterface().fetchGenericNames(std::vector<quint64>(std::begin(ids) + current * maxPerRequest, std::end(ids)), transformCallback);
    }

    void ESIManager::fetchCharacterContracts(Character::IdType charId, const ContractCallback &callback) const
    {
        getInterface().fetchCharacterContracts(charId, getContractCallback(callback));
    }

    void ESIManager::fetchCharacterContractItems(Character::IdType charId, Contract::IdType contractId, const ContractItemCallback &callback) const
    {
        getInterface().fetchCharacterContractItems(charId, contractId, getContractItemCallback(contractId, callback));
    }

    void ESIManager::fetchCorporationContracts(Character::IdType charId, quint64 corpId, const ContractCallback &callback) const
    {
        getInterface().fetchCorporationContracts(charId, corpId, getContractCallback(callback));
    }

    void ESIManager::fetchCorporationContractItems(Character::IdType charId, quint64 corpId, Contract::IdType contractId, const ContractItemCallback &callback) const
    {
        getInterface().fetchCorporationContractItems(charId, corpId, contractId, getContractItemCallback(contractId, callback));
    }

    void ESIManager::fetchCharacterBlueprints(Character::IdType charId, const BlueprintCallback &callback) const
    {
        getInterface().fetchCharacterBlueprints(charId, getBlueprintCallback(callback));
    }

    void ESIManager::fetchCorporationBlueprints(Character::IdType charId, quint64 corpId, const BlueprintCallback &callback) const
    {
        getInterface().fetchCorporationBlueprints(charId, corpId, getBlueprintCallback(callback));
    }

    void ESIManager::fetchMarketPrices(const Callback<MarketPrices> &callback) const
    {
        getInterface().fetchMarketPrices([=](auto &&data, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto prices = data.array();

            MarketPrices result;
            result.reserve(prices.size());

            for (const auto &price : prices)
            {
                const auto priceObj = price.toObject();
                result.emplace(static_cast<EveType::IdType>(priceObj.value(QStringLiteral("type_id")).toDouble()), TypePriceData{
                    priceObj.value(QStringLiteral("adjusted_price")).toDouble(),
                    priceObj.value(QStringLiteral("average_price")).toDouble()
                });
            }

            callback(std::move(result), {}, expires);
        });
    }

    void ESIManager::fetchIndustryCostIndices(const Callback<IndustryCostIndices> &callback) const
    {
        getInterface().fetchIndustryCostIndices([=](auto &&data, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto prices = data.array();

            IndustryCostIndices result;
            result.reserve(prices.size());

            std::mutex resultMutex;

            const QHash<QString, IndustryCostIndex::Activity> activityNameMap = {
                { QStringLiteral("none"), IndustryCostIndex::Activity::None },
                { QStringLiteral("manufacturing"), IndustryCostIndex::Activity::Manufacturing },
                { QStringLiteral("researching_technology"), IndustryCostIndex::Activity::ResearchingTechnology },
                { QStringLiteral("researching_time_efficiency"), IndustryCostIndex::Activity::ResearchingTimeEfficiency },
                { QStringLiteral("researching_material_efficiency"), IndustryCostIndex::Activity::ResearchingMaterialEfficiency },
                { QStringLiteral("copying"), IndustryCostIndex::Activity::Copying },
                { QStringLiteral("duplicating"), IndustryCostIndex::Activity::Duplicating },
                { QStringLiteral("invention"), IndustryCostIndex::Activity::Invention },
                { QStringLiteral("reverse_engineering"), IndustryCostIndex::Activity::ReverseEngineering },
            };

            QtConcurrent::blockingMap(
                prices,
                std::function<void (const QJsonValue &)>{[&](const auto &index) {
                    const auto indexObj = index.toObject();
                    const auto costs = indexObj.value(QStringLiteral("cost_indices")).toArray();

                    const uint systemId = indexObj.value(QStringLiteral("solar_system_id")).toDouble();

                    IndustryCostIndex::ActivityCostIndices indices;
                    indices.reserve(costs.size());

                    for (const auto &cost : costs)
                    {
                        const auto costObj = cost.toObject();
                        const auto activityStr = costObj.value(QStringLiteral("activity")).toString();
                        const auto value = costObj.value(QStringLiteral("cost_index")).toDouble();
                        const auto activity = activityNameMap.value(activityStr, IndustryCostIndex::Activity::None);

                        indices.emplace(activity, value);
                    }

                    std::lock_guard<std::mutex> lock{resultMutex};
                    result.emplace(systemId, std::move(indices));
                }}
            );

            callback(std::move(result), {}, expires);
        });
    }

    void ESIManager::fetchSovereigntyStructures(const Callback<SovereigntyStructureList> &callback) const
    {
        getInterface().fetchSovereigntyStructures([=](auto &&data, const auto &error, const auto &expires) {
            Q_UNUSED(expires);

            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto structures = data.array();

            SovereigntyStructureList result;
            result.reserve(structures.size());

            for (const auto &structure : structures)
            {
                const auto structureObj = structure.toObject();

                SovereigntyStructure parsedStructure;
                parsedStructure.mSolarSystemId =  structureObj.value(QStringLiteral("solar_system_id")).toDouble();
                parsedStructure.mStructureId =  structureObj.value(QStringLiteral("structure_id")).toDouble();
                parsedStructure.mTypeId =  structureObj.value(QStringLiteral("structure_type_id")).toDouble();

                result.emplace_back(parsedStructure);
            }

            callback(std::move(result), {}, expires);
        });
    }

    void ESIManager::openMarketDetails(EveType::IdType typeId, Character::IdType charId) const
    {
        getInterface().openMarketDetails(typeId, charId, [=](const auto &errorText) {
            emit error(errorText);
        });
    }

    void ESIManager::setDestination(quint64 locationId, Character::IdType charId) const
    {
        getInterface().setDestination(locationId, charId, [=](const auto &errorText) {
            emit error(errorText);
        });
    }

    void ESIManager::fetchCharacterWalletTransactions(Character::IdType charId,
                                                      const std::optional<WalletTransaction::IdType> &fromId,
                                                      WalletTransaction::IdType tillId,
                                                      std::shared_ptr<WalletTransactions> &&transactions,
                                                      const WalletTransactionsCallback &callback) const
    {
        getInterface().fetchCharacterWalletTransactions(
            charId,
            fromId,
            getWalletTransactionsCallback(charId, 0, tillId, std::move(transactions), callback, [=](const auto &fromId, auto &&transactions) {
                fetchCharacterWalletTransactions(charId, fromId, tillId, std::move(transactions), callback);
            })
        );
    }

    void ESIManager::fetchCorporationWalletTransactions(Character::IdType charId,
                                                        quint64 corpId,
                                                        int division,
                                                        const std::optional<WalletTransaction::IdType> &fromId,
                                                        WalletTransaction::IdType tillId,
                                                        std::shared_ptr<WalletTransactions> &&transactions,
                                                        const WalletTransactionsCallback &callback) const
    {
        getInterface().fetchCorporationWalletTransactions(
            charId,
            corpId,
            division,
            fromId,
            getWalletTransactionsCallback(charId, corpId, tillId, std::move(transactions), callback, [=](const auto &fromId, auto &&transactions) {
                fetchCorporationWalletTransactions(charId, corpId, division, fromId, tillId, std::move(transactions), callback);
            })
        );
    }

    ExternalOrder ESIManager::getExternalOrderFromJson(const QJsonObject &object, uint regionId, const QDateTime &updateTime) const
    {
        const auto range = object.value(QStringLiteral("range")).toString();

        ExternalOrder order;

        order.setId(object.value(QStringLiteral("order_id")).toDouble()); // https://bugreports.qt.io/browse/QTBUG-28560
        order.setType((object.value(QStringLiteral("is_buy_order")).toBool()) ? (ExternalOrder::Type::Buy) : (ExternalOrder::Type::Sell));
        order.setTypeId(object.value(QStringLiteral("type_id")).toDouble());
        order.setStationId(object.value(QStringLiteral("location_id")).toDouble());
        order.setRegionId(regionId);

        if (object.contains(QStringLiteral("system_id")))
            order.setSolarSystemId(object.value(QStringLiteral("system_id")).toDouble());
        else
            order.setSolarSystemId(mDataProvider.getStationSolarSystemId(order.getStationId()));

        if (range == "station")
            order.setRange(ExternalOrder::rangeStation);
        else if (range == "system")
            order.setRange(ExternalOrder::rangeSystem);
        else if (range == "region")
            order.setRange(ExternalOrder::rangeRegion);
        else
            order.setRange(range.toShort());

        order.setUpdateTime(updateTime);
        order.setPrice(object.value(QStringLiteral("price")).toDouble());
        order.setVolumeEntered(object.value(QStringLiteral("volume_total")).toInt());
        order.setVolumeRemaining(object.value(QStringLiteral("volume_remain")).toInt());
        order.setMinVolume(object.value(QStringLiteral("min_volume")).toInt());
        order.setIssued(getDateTimeFromString(object.value(QStringLiteral("issued")).toString()));
        order.setDuration(object.value(QStringLiteral("duration")).toInt());

        return order;
    }

    ESIInterface::PaginatedCallback ESIManager::getMarketOrderCallback(uint regionId, const MarketOrderCallback &callback) const
    {
        auto orders = std::make_shared<std::vector<ExternalOrder>>();
        return [=, orders = std::move(orders)](auto &&data, auto atEnd, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto items = data.array();
            const auto curSize = orders->size();
            orders->resize(curSize + items.size());

            const auto updateTime = QDateTime::currentDateTimeUtc();

            std::atomic_size_t nextIndex{curSize};

            const auto parseItem = [&](const auto &item) {
                (*orders)[nextIndex++] = getExternalOrderFromJson(item.toObject(), regionId, updateTime);
            };

            QtConcurrent::blockingMap(items, parseItem);

            if (atEnd)
                callback(std::move(*orders), {}, expires);
        };
    }

    ESIInterface::JsonCallback ESIManager::getMarketOrdersCallback(Character::IdType charId, const MarketOrdersCallback &callback) const
    {
        return [=](auto &&data, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto orderArray = data.array();
            MarketOrders orders(orderArray.size());

            std::atomic_size_t index{0};
            QtConcurrent::blockingMap(orderArray, [&, charId](const auto &order) {
                const auto orderObj = order.toObject();

                const auto issued = getDateTimeFromString(orderObj.value("issued").toString());

                auto &curOrder = orders[index++];
                curOrder.setId(orderObj.value(QStringLiteral("order_id")).toDouble());
                curOrder.setCharacterId(charId);
                curOrder.setStationId(orderObj.value(QStringLiteral("location_id")).toDouble());
                curOrder.setVolumeEntered(orderObj.value(QStringLiteral("volume_total")).toDouble());
                curOrder.setVolumeRemaining(orderObj.value(QStringLiteral("volume_remain")).toDouble());
                curOrder.setMinVolume(orderObj.value(QStringLiteral("min_volume")).toDouble());
                curOrder.setState(MarketOrder::State::Active);  // ESI returns only open orders
                curOrder.setTypeId(orderObj.value(QStringLiteral("type_id")).toDouble());
                curOrder.setRange(getMarketOrderRangeFromString(orderObj.value(QStringLiteral("range")).toString()));
                curOrder.setDuration(orderObj.value(QStringLiteral("duration")).toInt());
                curOrder.setEscrow(orderObj.value(QStringLiteral("escrow")).toDouble());
                curOrder.setPrice(orderObj.value(QStringLiteral("price")).toDouble());
                curOrder.setType((orderObj.value(QStringLiteral("is_buy_order")).toBool()) ? (MarketOrder::Type::Buy) : (MarketOrder::Type::Sell));
                curOrder.setIssued(issued);
                curOrder.setFirstSeen(issued);
            });

            callback(std::move(orders), {}, expires);
        };
    }

    ESIInterface::PaginatedCallback ESIManager::getAssetListCallback(Character::IdType charId, const AssetCallback &callback) const
    {
        struct AssetProcessingData
        {
            std::vector<AssetList::ItemType> mAllItems;
            std::unordered_map<Item::IdType, Item *> mItemMap;
        };

        auto allItems = std::make_shared<AssetProcessingData>();
        return [=, allItems = std::move(allItems)](auto &&data, auto atEnd, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto assets = data.array();

            allItems->mAllItems.reserve(allItems->mAllItems.size() + assets.size());
            allItems->mItemMap.reserve(allItems->mItemMap.size() + assets.size());

            for (const auto &itemObj : assets)
            {
                const auto item = itemObj.toObject();
                const int rawQuantity = item.value(QStringLiteral("quantity")).toDouble();

                auto newItem = std::make_unique<Item>(static_cast<Item::IdType>(item.value(QStringLiteral("item_id")).toDouble()));
                newItem->setLocationId(item.value(QStringLiteral("location_id")).toDouble());
                newItem->setTypeId(item.value(QStringLiteral("type_id")).toDouble());
                newItem->setRawQuantity(rawQuantity);
                newItem->setBPCFlag(item.value(QStringLiteral("is_blueprint_copy")).toBool());
                // https://forums.eveonline.com/t/esi-assets-blueprints-and-quantities/19345/4
                newItem->setQuantity((rawQuantity < 0) ? (1) : (rawQuantity));

                allItems->mItemMap.emplace(newItem->getId(), newItem.get());
                allItems->mAllItems.emplace_back(std::move(newItem));
            }

            if (atEnd)
            {
                AssetList list;
                list.setCharacterId(charId);

                // make tree
                for (auto &item : allItems->mAllItems)
                {
                    const auto parent = allItems->mItemMap.find(*item->getLocationId());
                    if (parent != std::end(allItems->mItemMap))
                    {
                        item->setLocationId({});
                        parent->second->addItem(std::move(item));
                    }
                    else
                    {
                        list.addItem(std::move(item));
                    }
                }

                callback(std::move(list), {}, expires);
            }
        };
    }

    ESIInterface::JsonCallback ESIManager::getContractCallback(const ContractCallback &callback) const
    {
        return [=](auto &&data, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto contracts = data.array();

            Contracts result;
            result.resize(contracts.size());

            std::atomic_size_t nextIndex{0};

            QtConcurrent::blockingMap(contracts, [&](const auto &contract) {
                const auto contractObj = contract.toObject();

                auto &curContract = result[nextIndex++];
                curContract.setId(contractObj.value(QStringLiteral("contract_id")).toDouble());
                curContract.setIssuerId(contractObj.value(QStringLiteral("issuer_id")).toDouble());
                curContract.setIssuerCorpId(contractObj.value(QStringLiteral("issuer_corporation_id")).toDouble());
                curContract.setAssigneeId(contractObj.value(QStringLiteral("assignee_id")).toDouble());
                curContract.setAcceptorId(contractObj.value(QStringLiteral("acceptor_id")).toDouble());
                curContract.setStartStationId(contractObj.value(QStringLiteral("start_location_id")).toDouble());
                curContract.setEndStationId(contractObj.value(QStringLiteral("end_location_id")).toDouble());
                curContract.setType(getContractTypeFromString(contractObj.value(QStringLiteral("type")).toString()));
                curContract.setStatus(getContractStatusFromString(contractObj.value(QStringLiteral("status")).toString()));
                curContract.setTitle(contractObj.value(QStringLiteral("title")).toString());
                curContract.setForCorp(contractObj.value(QStringLiteral("for_corporation")).toBool());
                curContract.setAvailability(getContractAvailabilityFromString(contractObj.value(QStringLiteral("availability")).toString()));
                curContract.setIssued(getDateTimeFromString(contractObj.value(QStringLiteral("date_issued")).toString()));
                curContract.setExpired(getDateTimeFromString(contractObj.value(QStringLiteral("date_expired")).toString()));
                curContract.setNumDays(contractObj.value(QStringLiteral("days_to_complete")).toDouble());
                curContract.setPrice(contractObj.value(QStringLiteral("price")).toDouble());
                curContract.setReward(contractObj.value(QStringLiteral("reward")).toDouble());
                curContract.setCollateral(contractObj.value(QStringLiteral("collateral")).toDouble());
                curContract.setBuyout(contractObj.value(QStringLiteral("buyout")).toDouble());
                curContract.setVolume(contractObj.value(QStringLiteral("volume")).toDouble());

                if (contractObj.contains(QStringLiteral("date_accepted")))
                    curContract.setAccepted(getDateTimeFromString(contractObj.value(QStringLiteral("date_accepted")).toString()));
                if (contractObj.contains(QStringLiteral("date_completed")))
                    curContract.setCompleted(getDateTimeFromString(contractObj.value(QStringLiteral("date_completed")).toString()));
            });

            callback(std::move(result), {}, expires);
        };
    }

    ESIInterface::JsonCallback ESIManager::getContractItemCallback(Contract::IdType contractId, const ContractItemCallback &callback) const
    {
        return [=](auto &&data, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto itemList = data.array();

            ContractItemList result;
            result.reserve(itemList.size());

            for (const auto &item : itemList)
            {
                const auto itemObj = item.toObject();

                ContractItem resultItem{static_cast<ContractItem::IdType>(itemObj.value(QStringLiteral("record_id")).toDouble())};
                resultItem.setContractId(contractId);
                resultItem.setIncluded(itemObj.value(QStringLiteral("is_included")).toBool());
                resultItem.setQuantity(itemObj.value(QStringLiteral("quantity")).toDouble());
                resultItem.setTypeId(itemObj.value(QStringLiteral("type_id")).toDouble());

                result.emplace_back(std::move(resultItem));
            }

            callback(std::move(result), error, expires);
        };
    }

    ESIInterface::PaginatedCallback ESIManager::getWalletJournalCallback(Character::IdType charId,
                                                                         quint64 corpId,
                                                                         WalletJournalEntry::IdType tillId,
                                                                         const WalletJournalCallback &callback) const
    {
        auto journal = std::make_shared<WalletJournal>();
        return [=](auto &&data, auto atEnd, const auto &error, const auto &expires) mutable {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto array = data.array();
            std::mutex resultMutex;

            QtConcurrent::blockingMap(
                array,
                [&](const auto &value) {
                    const auto entryObj = value.toObject();

                    WalletJournalEntry entry{static_cast<WalletJournalEntry::IdType>(entryObj.value(QStringLiteral("id")).toDouble())};

                    const auto id = entry.getId();
                    if (id > tillId)
                    {
                        entry.setCharacterId(charId);
                        entry.setCorporationId(corpId);
                        entry.setTimestamp(getDateTimeFromString(entryObj.value(QStringLiteral("date")).toString()));
                        entry.setRefType(entryObj.value(QStringLiteral("ref_type")).toString());

                        if (entryObj.contains(QStringLiteral("first_party_id")))
                            entry.setFirstPartyId(entryObj.value(QStringLiteral("first_party_id")).toDouble());
                        if (entryObj.contains(QStringLiteral("second_party_id")))
                            entry.setSecondPartyId(entryObj.value(QStringLiteral("second_party_id")).toDouble());

                        entry.setReason(entryObj.value(QStringLiteral("reason")).toString());

                        if (entryObj.contains(QStringLiteral("amount")))
                            entry.setAmount(entryObj.value(QStringLiteral("amount")).toDouble());
                        if (entryObj.contains(QStringLiteral("balance")))
                            entry.setBalance(entryObj.value(QStringLiteral("balance")).toDouble());
                        if (entryObj.contains(QStringLiteral("tax_reciever_id")))
                            entry.setTaxReceiverId(entryObj.value(QStringLiteral("tax_reciever_id")).toDouble());
                        if (entryObj.contains(QStringLiteral("tax")))
                            entry.setTaxAmount(entryObj.value(QStringLiteral("tax")).toDouble());
                        if (entryObj.contains(QStringLiteral("context_id")))
                            entry.setContextId(entryObj.value(QStringLiteral("context_id")).toDouble());
                        if (entryObj.contains(QStringLiteral("context_id_type")))
                            entry.setContextIdType(entryObj.value(QStringLiteral("context_id_type")).toString());

                        std::lock_guard<std::mutex> lock{resultMutex};
                        journal->emplace(std::move(entry));
                    }
                }
            );

            if (atEnd)
                callback(std::move(*journal), {}, expires);
        };
    }

    template<class T>
    ESIInterface::JsonCallback ESIManager::getWalletTransactionsCallback(Character::IdType charId,
                                                                         quint64 corpId,
                                                                         WalletTransaction::IdType tillId,
                                                                         std::shared_ptr<WalletTransactions> &&transactions,
                                                                         const WalletTransactionsCallback &callback,
                                                                         T nextCallback) const
    {
        return [=, transactions = std::move(transactions), nextCallback = std::move(nextCallback)](auto &&data, const auto &error, const auto &expires) mutable {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            auto nextFromId = std::numeric_limits<WalletTransaction::IdType>::max();

            std::mutex resultMutex;

            auto transactionsArray = data.array();
            std::atomic_bool reachedEnd{transactionsArray.isEmpty()};

            QtConcurrent::blockingMap(
                transactionsArray,
                [&](const auto &value) {
                    const auto transactionObj = value.toObject();

                    WalletTransaction transaction{
                        static_cast<WalletTransaction::IdType>(transactionObj.value(QStringLiteral("transaction_id")).toDouble())
                    };

                    const auto id = transaction.getId();
                    if (id > tillId)
                    {
                        transaction.setCharacterId(charId);
                        transaction.setCorporationId(corpId);
                        transaction.setTimestamp(getDateTimeFromString(transactionObj.value(QStringLiteral("date")).toString()));
                        transaction.setQuantity(transactionObj.value(QStringLiteral("quantity")).toDouble());
                        transaction.setTypeId(transactionObj.value(QStringLiteral("type_id")).toDouble());
                        transaction.setPrice(transactionObj.value(QStringLiteral("unit_price")).toDouble());
                        transaction.setClientId(transactionObj.value(QStringLiteral("client_id")).toDouble());
                        transaction.setLocationId(transactionObj.value(QStringLiteral("location_id")).toDouble());
                        transaction.setType(
                            (transactionObj.value(QStringLiteral("is_buy")).toBool()) ?
                            (WalletTransaction::Type::Buy) :
                            (WalletTransaction::Type::Sell)
                        );
                        transaction.setJournalId(transactionObj.value(QStringLiteral("journal_ref_id")).toDouble());

                        {
                            std::lock_guard<std::mutex> lock{resultMutex};

                            transactions->emplace(std::move(transaction));
                            if (nextFromId > id)
                                nextFromId = id;
                        }
                    }
                    else
                    {
                        reachedEnd = true;
                    }
                }
            );

            if (reachedEnd)
                callback(std::move(*transactions), {}, expires);
            else
                nextCallback(nextFromId - 1, std::move(transactions));
        };
    }

    ESIInterface::PaginatedCallback ESIManager::getBlueprintCallback(const BlueprintCallback &callback) const
    {
        auto blueprints = std::make_shared<BlueprintList>();
        return [=, blueprints = std::move(blueprints)](auto &&data, auto atEnd, const auto &error, const auto &expires) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error, expires);
                return;
            }

            const auto blueprintList = data.array();

            blueprints->resize(blueprints->size() + blueprintList.size());
            std::atomic_size_t currentIndex{0};

            const QHash<QString, Blueprint::Location> locationNameMap = {
                { QStringLiteral("AutoFit"), Blueprint::Location::AutoFit },
                { QStringLiteral("Cargo"), Blueprint::Location::Cargo },
                { QStringLiteral("CorpseBay"), Blueprint::Location::CorpseBay },
                { QStringLiteral("DroneBay"), Blueprint::Location::DroneBay },
                { QStringLiteral("FleetHangar"), Blueprint::Location::FleetHangar },
                { QStringLiteral("Deliveries"), Blueprint::Location::Deliveries },
                { QStringLiteral("HiddenModifiers"), Blueprint::Location::HiddenModifiers },
                { QStringLiteral("Hangar"), Blueprint::Location::Hangar },
                { QStringLiteral("HangarAll"), Blueprint::Location::HangarAll },
                { QStringLiteral("LoSlot0"), Blueprint::Location::LoSlot0 },
                { QStringLiteral("LoSlot1"), Blueprint::Location::LoSlot1 },
                { QStringLiteral("LoSlot2"), Blueprint::Location::LoSlot2 },
                { QStringLiteral("LoSlot3"), Blueprint::Location::LoSlot3 },
                { QStringLiteral("LoSlot4"), Blueprint::Location::LoSlot4 },
                { QStringLiteral("LoSlot5"), Blueprint::Location::LoSlot5 },
                { QStringLiteral("LoSlot6"), Blueprint::Location::LoSlot6 },
                { QStringLiteral("LoSlot7"), Blueprint::Location::LoSlot7 },
                { QStringLiteral("MedSlot0"), Blueprint::Location::MedSlot0 },
                { QStringLiteral("MedSlot1"), Blueprint::Location::MedSlot1 },
                { QStringLiteral("MedSlot2"), Blueprint::Location::MedSlot2 },
                { QStringLiteral("MedSlot3"), Blueprint::Location::MedSlot3 },
                { QStringLiteral("MedSlot4"), Blueprint::Location::MedSlot4 },
                { QStringLiteral("MedSlot5"), Blueprint::Location::MedSlot5 },
                { QStringLiteral("MedSlot6"), Blueprint::Location::MedSlot6 },
                { QStringLiteral("MedSlot7"), Blueprint::Location::MedSlot7 },
                { QStringLiteral("HiSlot0"), Blueprint::Location::HiSlot0 },
                { QStringLiteral("HiSlot1"), Blueprint::Location::HiSlot1 },
                { QStringLiteral("HiSlot2"), Blueprint::Location::HiSlot2 },
                { QStringLiteral("HiSlot3"), Blueprint::Location::HiSlot3 },
                { QStringLiteral("HiSlot4"), Blueprint::Location::HiSlot4 },
                { QStringLiteral("HiSlot5"), Blueprint::Location::HiSlot5 },
                { QStringLiteral("HiSlot6"), Blueprint::Location::HiSlot6 },
                { QStringLiteral("HiSlot7"), Blueprint::Location::HiSlot7 },
                { QStringLiteral("AssetSafety"), Blueprint::Location::AssetSafety },
                { QStringLiteral("Locked"), Blueprint::Location::Locked },
                { QStringLiteral("Unlocked"), Blueprint::Location::Unlocked },
                { QStringLiteral("Implant"), Blueprint::Location::Implant },
                { QStringLiteral("QuafeBay"), Blueprint::Location::QuafeBay },
                { QStringLiteral("RigSlot0"), Blueprint::Location::RigSlot0 },
                { QStringLiteral("RigSlot1"), Blueprint::Location::RigSlot1 },
                { QStringLiteral("RigSlot2"), Blueprint::Location::RigSlot2 },
                { QStringLiteral("RigSlot3"), Blueprint::Location::RigSlot3 },
                { QStringLiteral("RigSlot4"), Blueprint::Location::RigSlot4 },
                { QStringLiteral("RigSlot5"), Blueprint::Location::RigSlot5 },
                { QStringLiteral("RigSlot6"), Blueprint::Location::RigSlot6 },
                { QStringLiteral("RigSlot7"), Blueprint::Location::RigSlot7 },
                { QStringLiteral("ShipHangar"), Blueprint::Location::ShipHangar },
                { QStringLiteral("SpecializedFuelBay"), Blueprint::Location::SpecializedFuelBay },
                { QStringLiteral("SpecializedOreHold"), Blueprint::Location::SpecializedOreHold },
                { QStringLiteral("SpecializedGasHold"), Blueprint::Location::SpecializedGasHold },
                { QStringLiteral("SpecializedMineralHold"), Blueprint::Location::SpecializedMineralHold },
                { QStringLiteral("SpecializedSalvageHold"), Blueprint::Location::SpecializedSalvageHold },
                { QStringLiteral("SpecializedShipHold"), Blueprint::Location::SpecializedShipHold },
                { QStringLiteral("SpecializedSmallShipHold"), Blueprint::Location::SpecializedSmallShipHold },
                { QStringLiteral("SpecializedMediumShipHold"), Blueprint::Location::SpecializedMediumShipHold },
                { QStringLiteral("SpecializedLargeShipHold"), Blueprint::Location::SpecializedLargeShipHold },
                { QStringLiteral("SpecializedIndustrialShipHold"), Blueprint::Location::SpecializedIndustrialShipHold },
                { QStringLiteral("SpecializedAmmoHold"), Blueprint::Location::SpecializedAmmoHold },
                { QStringLiteral("SpecializedCommandCenterHold"), Blueprint::Location::SpecializedCommandCenterHold },
                { QStringLiteral("SpecializedPlanetaryCommoditiesHold"), Blueprint::Location::SpecializedPlanetaryCommoditiesHold },
                { QStringLiteral("SpecializedMaterialBay"), Blueprint::Location::SpecializedMaterialBay },
                { QStringLiteral("SubSystemSlot0"), Blueprint::Location::SubSystemSlot0 },
                { QStringLiteral("SubSystemSlot1"), Blueprint::Location::SubSystemSlot1 },
                { QStringLiteral("SubSystemSlot2"), Blueprint::Location::SubSystemSlot2 },
                { QStringLiteral("SubSystemSlot3"), Blueprint::Location::SubSystemSlot3 },
                { QStringLiteral("SubSystemSlot4"), Blueprint::Location::SubSystemSlot4 },
                { QStringLiteral("SubSystemSlot5"), Blueprint::Location::SubSystemSlot5 },
                { QStringLiteral("SubSystemSlot6"), Blueprint::Location::SubSystemSlot6 },
                { QStringLiteral("SubSystemSlot7"), Blueprint::Location::SubSystemSlot7 },
                { QStringLiteral("FighterBay"), Blueprint::Location::FighterBay },
                { QStringLiteral("FighterTube0"), Blueprint::Location::FighterTube0 },
                { QStringLiteral("FighterTube1"), Blueprint::Location::FighterTube1 },
                { QStringLiteral("FighterTube2"), Blueprint::Location::FighterTube2 },
                { QStringLiteral("FighterTube3"), Blueprint::Location::FighterTube3 },
                { QStringLiteral("FighterTube4"), Blueprint::Location::FighterTube4 },
                { QStringLiteral("Module"), Blueprint::Location::Module },
                { QStringLiteral("FrigateEscapeBay"), Blueprint::Location::FrigateEscapeBay },
                
            };

            QtConcurrent::blockingMap(blueprintList, [&](const auto &blueprint) {
                const auto blueprintObj = blueprint.toObject();

                auto &curResult = (*blueprints)[currentIndex++];
                curResult.setId(blueprintObj.value(QStringLiteral("item_id")).toDouble());
                curResult.setLocationId(blueprintObj.value(QStringLiteral("location_id")).toDouble());
                curResult.setMaterialEfficiency(blueprintObj.value(QStringLiteral("material_efficiency")).toDouble());
                curResult.setTimeEfficiency(blueprintObj.value(QStringLiteral("time_efficiency")).toDouble());
                curResult.setQuantity(blueprintObj.value(QStringLiteral("quantity")).toInt());
                curResult.setRuns(blueprintObj.value(QStringLiteral("runs")).toInt());
                curResult.setTypeId(blueprintObj.value(QStringLiteral("type_id")).toDouble());

                const auto location = blueprintObj.value(QStringLiteral("location_flag")).toString();
                if (locationNameMap.contains(location))
                    curResult.setLocation(locationNameMap[location]);
            });

            if (atEnd)
                callback(std::move(*blueprints), {}, expires);
        };
    }

    const ESIInterface &ESIManager::getInterface() const
    {
        return mInterfaceManager.getInterface();
    }

    short ESIManager::getMarketOrderRangeFromString(const QString &range)
    {
        static const QHash<QString, short> ranges = {
            { QStringLiteral("station"), MarketOrder::rangeStation },
            { QStringLiteral("region"), MarketOrder::rangeRegion },
            { QStringLiteral("solarsystem"), MarketOrder::rangeSystem },
        };

        // assume unknown is active, since there shouldn't be non-open orders returned anyway
        return (ranges.contains(range)) ? (ranges[range]) : (range.toShort());
    }

    QDateTime ESIManager::getDateTimeFromString(const QString &value)
    {
        auto dt = QDateTime::fromString(value, Qt::ISODate);
        if (Q_UNLIKELY(!dt.isValid()))
            dt = QDateTime::currentDateTimeUtc();   // just to be safe
        else
            dt.setTimeSpec(Qt::UTC);

        return dt;
    }

    Contract::Type ESIManager::getContractTypeFromString(const QString &type)
    {
        if (type == "item_exchange")
            return Contract::Type::ItemExchange;
        if (type == "auction")
            return Contract::Type::Auction;
        if (type == "courier")
            return Contract::Type::Courier;
        if (type == "loan")
            return Contract::Type::Loan;

        return Contract::Type::Unknown;
    }

    Contract::Status ESIManager::getContractStatusFromString(const QString &status)
    {
        if (status == "in_progress")
            return Contract::Status::InProgress;
        if (status == "finished_issuer")
            return Contract::Status::CompletedByIssuer;
        if (status == "finished_contractor")
            return Contract::Status::CompletedByContractor;
        if (status == "finished")
            return Contract::Status::Completed;
        if (status == "cancelled")
            return Contract::Status::Cancelled;
        if (status == "rejected")
            return Contract::Status::Rejected;
        if (status == "failed")
            return Contract::Status::Failed;
        if (status == "deleted")
            return Contract::Status::Deleted;
        if (status == "reversed")
            return Contract::Status::Reversed;

        return Contract::Status::Outstanding;
    }

    Contract::Availability ESIManager::getContractAvailabilityFromString(const QString &availability)
    {
        if (availability == "public")
            return Contract::Availability::Public;
        if (availability == "corporation")
            return Contract::Availability::Corporation;
        if (availability == "alliance")
            return Contract::Availability::Alliance;

        return Contract::Availability::Private;
    }
}
