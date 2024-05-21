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
#include <unordered_map>
#include <memory>

#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>
#include <QLabel>
#include <QHash>
#include <QFont>

#include "CorpMarketOrderValueSnapshotRepository.h"
#include "MarketOrderValueSnapshotRepository.h"
#include "CorpAssetValueSnapshotRepository.h"
#include "WalletJournalEntryRepository.h"
#include "AssetValueSnapshotRepository.h"
#include "CorpWalletSnapshotRepository.h"
#include "WalletTransactionRepository.h"
#include "WalletSnapshotRepository.h"
#include "DateFilteredPlotWidget.h"
#include "CharacterRepository.h"
#include "RepositoryProvider.h"
#include "StatisticsSettings.h"
#include "UISettings.h"
#include "TextUtils.h"

#include "qcustomplot.h"

#include "BasicStatisticsWidget.h"

namespace Evernus
{
    BasicStatisticsWidget::BasicStatisticsWidget(const RepositoryProvider &repositoryProvider,
                                                 const ItemCostProvider &itemCostProvider,
                                                 QWidget *parent)
        : QWidget(parent)
        , mAssetSnapshotRepository(repositoryProvider.getAssetValueSnapshotRepository())
        , mCorpAssetSnapshotRepository(repositoryProvider.getCorpAssetValueSnapshotRepository())
        , mWalletSnapshotRepository(repositoryProvider.getWalletSnapshotRepository())
        , mCorpWalletSnapshotRepository(repositoryProvider.getCorpWalletSnapshotRepository())
        , mMarketOrderSnapshotRepository(repositoryProvider.getMarketOrderValueSnapshotRepository())
        , mCorpMarketOrderSnapshotRepository(repositoryProvider.getCorpMarketOrderValueSnapshotRepository())
        , mJournalRepository(repositoryProvider.getWalletJournalEntryRepository())
        , mCorpJournalRepository(repositoryProvider.getCorpWalletJournalEntryRepository())
        , mTransactionRepository(repositoryProvider.getWalletTransactionRepository())
        , mCorpTransactionRepository(repositoryProvider.getCorpWalletTransactionRepository())
        , mCharacterRepository(repositoryProvider.getCharacterRepository())
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolBarLayout);

        QSettings settings;

        mCombineStatsBtn = new QCheckBox{tr("Combine statistics for all characters"), this};
        toolBarLayout->addWidget(mCombineStatsBtn);
        mCombineStatsBtn->setChecked(settings.value(StatisticsSettings::combineStatisticsKey, StatisticsSettings::combineStatisticsDefault).toBool());
        connect(mCombineStatsBtn, &QCheckBox::toggled, this, [=](bool checked) {
            QSettings settings;
            settings.setValue(StatisticsSettings::combineStatisticsKey, checked);

            updateData();
        });

        auto makeSnapshotBtn = new QPushButton{tr("Make snapshot"), this};
        toolBarLayout->addWidget(makeSnapshotBtn);
        connect(makeSnapshotBtn, &QPushButton::clicked, this, &BasicStatisticsWidget::makeSnapshots);

        toolBarLayout->addStretch();

        auto balanceGroup = new QGroupBox{tr("Balance"), this};
        mainLayout->addWidget(balanceGroup);

        auto balanceLayout = new QVBoxLayout{balanceGroup};

        mBalancePlot = createPlot();
        balanceLayout->addWidget(mBalancePlot);
        connect(mBalancePlot, &DateFilteredPlotWidget::filterChanged, this, &BasicStatisticsWidget::updateBalanceData);
        connect(mBalancePlot, &DateFilteredPlotWidget::mouseMove, this, &BasicStatisticsWidget::updateBalanceTooltip);

        auto journalGroup = new QGroupBox{tr("Wallet journal"), this};
        mainLayout->addWidget(journalGroup);

        auto journalLayout = new QVBoxLayout{journalGroup};

        mJournalPlot = createPlot();
        journalLayout->addWidget(mJournalPlot);
        connect(mJournalPlot, &DateFilteredPlotWidget::filterChanged, this, &BasicStatisticsWidget::updateJournalData);

        QFont font;
        font.setBold(true);

        auto journalSummaryLayout = new QHBoxLayout{};
        journalLayout->addLayout(journalSummaryLayout);

        journalSummaryLayout->addWidget(new QLabel{tr("Total income:"), this}, 0, Qt::AlignRight);

        mJournalIncomeLabel = new QLabel{"-", this};
        journalSummaryLayout->addWidget(mJournalIncomeLabel, 0, Qt::AlignLeft);
        mJournalIncomeLabel->setFont(font);

        journalSummaryLayout->addWidget(new QLabel{tr("Total cost:"), this}, 0, Qt::AlignRight);

        mJournalOutcomeLabel = new QLabel{"-", this};
        journalSummaryLayout->addWidget(mJournalOutcomeLabel, 0, Qt::AlignLeft);
        mJournalOutcomeLabel->setFont(font);

        journalSummaryLayout->addWidget(new QLabel{tr("Balance:"), this}, 0, Qt::AlignRight);

        mJournalBalanceLabel = new QLabel{"-", this};
        journalSummaryLayout->addWidget(mJournalBalanceLabel, 0, Qt::AlignLeft);
        mJournalBalanceLabel->setFont(font);

        journalSummaryLayout->addStretch();

        auto transactionGroup = new QGroupBox{tr("Wallet transactions"), this};
        mainLayout->addWidget(transactionGroup);

        auto transactionLayout = new QVBoxLayout{transactionGroup};

        mTransactionPlot = createPlot();
        transactionLayout->addWidget(mTransactionPlot);
        connect(mTransactionPlot, &DateFilteredPlotWidget::filterChanged, this, &BasicStatisticsWidget::updateTransactionData);

        auto transactionSummaryLayout = new QHBoxLayout{};
        transactionLayout->addLayout(transactionSummaryLayout);

        transactionSummaryLayout->addWidget(new QLabel{tr("Total income:"), this}, 0, Qt::AlignRight);

        mTransactionsIncomeLabel = new QLabel{"-", this};
        transactionSummaryLayout->addWidget(mTransactionsIncomeLabel, 0, Qt::AlignLeft);
        mTransactionsIncomeLabel->setFont(font);

        transactionSummaryLayout->addWidget(new QLabel{tr("Total cost:"), this}, 0, Qt::AlignRight);

        mTransactionsOutcomeLabel = new QLabel{"-", this};
        transactionSummaryLayout->addWidget(mTransactionsOutcomeLabel, 0, Qt::AlignLeft);
        mTransactionsOutcomeLabel->setFont(font);

        transactionSummaryLayout->addWidget(new QLabel{tr("Balance:"), this}, 0, Qt::AlignRight);

        mTransactionsBalanceLabel = new QLabel{"-", this};
        transactionSummaryLayout->addWidget(mTransactionsBalanceLabel, 0, Qt::AlignLeft);
        mTransactionsBalanceLabel->setFont(font);

        transactionSummaryLayout->addStretch();

        mainLayout->addStretch();

        updateGraphAndLegend();
    }

    void BasicStatisticsWidget::setCharacter(Character::IdType id)
    {
        mCharacterId = id;

        if (mCharacterId == Character::invalidId)
        {
            mBalancePlot->getPlot().clearPlottables();
            mJournalPlot->getPlot().clearPlottables();
            updateGraphAndLegend();
        }
        else
        {
            mBalancePlot->blockSignals(true);
            mJournalPlot->blockSignals(true);
            mTransactionPlot->blockSignals(true);

            const auto date = QDate::currentDate();

            mBalancePlot->setFrom(date.addDays(-7));
            mBalancePlot->setTo(date);
            mJournalPlot->setFrom(date.addDays(-7));
            mJournalPlot->setTo(date);
            mTransactionPlot->setFrom(date.addDays(-7));
            mTransactionPlot->setTo(date);

            updateData();

            mTransactionPlot->blockSignals(false);
            mJournalPlot->blockSignals(false);
            mBalancePlot->blockSignals(false);
        }
    }

    void BasicStatisticsWidget::updateBalanceData()
    {
        const QDateTime from{mBalancePlot->getFrom()};
        const QDateTime to{mBalancePlot->getTo().addDays(1)};

        const auto combineStats = mCombineStatsBtn->isChecked();
        const auto assetShots = (combineStats) ?
                                (mAssetSnapshotRepository.fetchRange(from.toUTC(), to.toUTC())) :
                                (mAssetSnapshotRepository.fetchRange(mCharacterId, from.toUTC(), to.toUTC()));
        const auto walletShots = (combineStats) ?
                                 (mWalletSnapshotRepository.fetchRange(from.toUTC(), to.toUTC())) :
                                 (mWalletSnapshotRepository.fetchRange(mCharacterId, from.toUTC(), to.toUTC()));
        const auto orderShots = (combineStats) ?
                                (mMarketOrderSnapshotRepository.fetchRange(from.toUTC(), to.toUTC())) :
                                (mMarketOrderSnapshotRepository.fetchRange(mCharacterId, from.toUTC(), to.toUTC()));

        auto assetGraph = mBalancePlot->getPlot().graph(assetValueGraph);
        auto walletGraph = mBalancePlot->getPlot().graph(walletBalanceGraph);
        auto corpWalletGraph = mBalancePlot->getPlot().graph(corpWalletBalanceGraph);
        auto corpAssetGraph = mBalancePlot->getPlot().graph(corpAssetValueGraph);
        auto buyGraph = mBalancePlot->getPlot().graph(buyOrdersGraph);
        auto sellGraph = mBalancePlot->getPlot().graph(sellOrdersGraph);
        auto sumGraph = mBalancePlot->getPlot().graph(totalValueGraph);

        QSharedPointer<QCPGraphDataContainer> assetValues{new QCPGraphDataContainer{}};
        QSharedPointer<QCPGraphDataContainer> walletValues{new QCPGraphDataContainer{}};
        QSharedPointer<QCPGraphDataContainer> corpWalletValues{new QCPGraphDataContainer{}};
        QSharedPointer<QCPGraphDataContainer> corpAssetValues{new QCPGraphDataContainer{}};
        QSharedPointer<QCPGraphDataContainer> buyValues{new QCPGraphDataContainer{}};
        QSharedPointer<QCPGraphDataContainer> sellValues{new QCPGraphDataContainer{}};

        QCPGraphDataContainer buyAndSellValues;

        auto yMax = -1.;

        const auto convertTimestamp = [](const auto &shot) {
            return shot->getTimestamp().toMSecsSinceEpoch() / 1000.;
        };

        const auto dataInserter = [&convertTimestamp](auto &values, const auto &range) {
            for (const auto &shot : range)
                values.add(QCPGraphData{convertTimestamp(shot), shot->getBalance()});
        };

        QSharedPointer<QCPGraphDataContainer> sumData{new QCPGraphDataContainer{}};
        const auto merger = [&yMax](const auto &values1, const auto &values2) -> QCPGraphDataContainer {
            QCPGraphDataContainer result;

            auto value1It = values1.constBegin();
            auto value2It = values2.constBegin();
            auto prevValue1 = 0.;
            auto prevValue2 = 0.;

            const auto lerp = [](double a, double b, double t) {
                return a + (b - a) * t;
            };

            while (value1It != values1.constEnd() || value2It != values2.constEnd())
            {
                if (value1It == values1.constEnd())
                {
                    const auto value = prevValue1 + value2It->value;
                    if (value > yMax)
                        yMax = value;

                    result.add(QCPGraphData{value2It->key, value});

                    ++value2It;
                }
                else if (value2It == values2.constEnd())
                {
                    const auto value = prevValue2 + value1It->value;
                    if (value > yMax)
                        yMax = value;

                    result.add(QCPGraphData{value1It->key, value});

                    ++value1It;
                }
                else
                {
                    if (value1It->key < value2It->key)
                    {
                        const auto prevTick = (value2It == values2.constBegin()) ? (values1.constBegin()->key) : (std::prev(value2It)->key);
                        const auto div = value2It->key - prevTick;
                        while (value1It->key < value2It->key && value1It != values1.constEnd())
                        {
                            prevValue1 = value1It->value;

                            const auto value = lerp(prevValue2, value2It->value, (value1It->key - prevTick) / div) + prevValue1;
                            if (value > yMax)
                                yMax = value;

                            result.add(QCPGraphData{value1It->key, value});

                            ++value1It;
                        }
                    }
                    else if (value1It->key > value2It->key)
                    {
                        const auto prevTick = (value1It == values1.constBegin()) ? (values2.constBegin()->key) : (std::prev(value1It)->key);
                        const auto div = value1It->key - prevTick;
                        while (value1It->key > value2It->key && value2It != values2.constEnd())
                        {
                            prevValue2 = value2It->value;

                            const auto value = prevValue2 + lerp(prevValue1, value1It->value, (value2It->key - prevTick) / div);
                            if (value > yMax)
                                yMax = value;

                            result.add(QCPGraphData{value2It->key, value});

                            ++value2It;
                        }
                    }
                    else
                    {
                        prevValue2 = value2It->value;
                        prevValue1 = value1It->value;

                        const auto value = prevValue2 + prevValue1;
                        if (value > yMax)
                            yMax = value;

                        result.add(QCPGraphData{value1It->key, value});

                        ++value1It;
                        ++value2It;
                    }
                }
            }

            return result;
        };

        const auto combineMaps = [&merger](auto &target, const auto &characterValuesMap) {
            if (characterValuesMap.empty())
                return;

            target = std::begin(characterValuesMap)->second;

            for (auto it = std::next(std::begin(characterValuesMap)); it != std::end(characterValuesMap); ++it)
                target = merger(target, it->second);
        };

        const auto combineShots = [&convertTimestamp, &combineMaps](auto &target, const auto &snapshots) {
            std::unordered_map<Evernus::Character::IdType, QCPGraphDataContainer> map;
            for (const auto &snapshot : snapshots)
            {
                const auto secs = convertTimestamp(snapshot);
                map[snapshot->getCharacterId()].add(QCPGraphData{secs, snapshot->getBalance()});
            }

            combineMaps(target, map);
        };

        if (combineStats)
        {
            combineShots(*assetValues, assetShots);
            combineShots(*walletValues, walletShots);

            std::unordered_map<Character::IdType, QCPGraphDataContainer> buyMap, sellMap;
            for (const auto &order : orderShots)
            {
                const auto secs = convertTimestamp(order);
                buyMap[order->getCharacterId()].add(QCPGraphData{secs, order->getBuyValue()});
                sellMap[order->getCharacterId()].add(QCPGraphData{secs, order->getSellValue()});
            }

            combineMaps(*buyValues, buyMap);
            combineMaps(*sellValues, sellMap);

            for (auto bIt = std::begin(*buyValues), sIt = std::begin(*sellValues); bIt != std::end(*buyValues); ++bIt, ++sIt)
            {
                Q_ASSERT(bIt->key == sIt->key);
                buyAndSellValues.add(QCPGraphData{bIt->key, bIt->value + sIt->value});
            }
        }
        else
        {
            dataInserter(*assetValues, assetShots);
            dataInserter(*walletValues, walletShots);

            for (const auto &order : orderShots)
            {
                const auto secs = convertTimestamp(order);

                buyValues->add(QCPGraphData{secs, order->getBuyValue()});
                sellValues->add(QCPGraphData{secs, order->getSellValue()});

                buyAndSellValues.add(QCPGraphData{secs, order->getBuyValue() + order->getSellValue()});
            }
        }

        *sumData = merger(*assetValues, *walletValues);

        try
        {
            const auto corpId = mCharacterRepository.getCorporationId(mCharacterId);
            const auto walletShots = (combineStats) ?
                                     (mCorpWalletSnapshotRepository.fetchRange(from.toUTC(), to.toUTC())) :
                                     (mCorpWalletSnapshotRepository.fetchRange(corpId, from.toUTC(), to.toUTC()));
            const auto assetShots = (combineStats) ?
                                    (mCorpAssetSnapshotRepository.fetchRange(from.toUTC(), to.toUTC())) :
                                    (mCorpAssetSnapshotRepository.fetchRange(corpId, from.toUTC(), to.toUTC()));

            if (!walletShots.empty())
            {
                dataInserter(*corpWalletValues, walletShots);
                *sumData = merger(*sumData, *corpWalletValues);
            }
            if (!assetShots.empty())
            {
                dataInserter(*corpAssetValues, assetShots);
                *sumData = merger(*sumData, *corpAssetValues);
            }

            const auto corpOrderShots = mCorpMarketOrderSnapshotRepository.fetchRange(corpId, from.toUTC(), to.toUTC());
            if (!corpOrderShots.empty())
            {
                QCPGraphDataContainer corpBuyValues, corpSellValues, corpBuyAndSellValues;
                for (const auto &order : corpOrderShots)
                {
                    const auto secs = order->getTimestamp().toMSecsSinceEpoch() / 1000.;

                    corpBuyValues.add(QCPGraphData{secs, order->getBuyValue()});
                    corpSellValues.add(QCPGraphData{secs, order->getSellValue()});

                    corpBuyAndSellValues.add(QCPGraphData{secs, order->getBuyValue() + order->getSellValue()});
                }

                *buyValues = merger(*buyValues, corpBuyValues);
                *sellValues = merger(*sellValues, corpSellValues);
                buyAndSellValues = merger(buyAndSellValues, corpBuyAndSellValues);
            }
        }
        catch (const CharacterRepository::NotFoundException &)
        {
        }

        *sumData = merger(*sumData, buyAndSellValues);

        assetGraph->setData(assetValues);
        walletGraph->setData(walletValues);
        corpWalletGraph->setData(corpWalletValues);
        corpAssetGraph->setData(corpAssetValues);
        buyGraph->setData(buyValues);
        sellGraph->setData(sellValues);
        sumGraph->setData(sumData);

        mBalancePlot->getPlot().xAxis->rescale();
        mBalancePlot->getPlot().yAxis->setRange(0., yMax);
        mBalancePlot->getPlot().replot();
    }

    void BasicStatisticsWidget::updateJournalData()
    {
        const auto combineStats = mCombineStatsBtn->isChecked();
        const auto entries = (combineStats) ?
                             (mJournalRepository.fetchInRange(QDateTime{mJournalPlot->getFrom()},
                                                              QDateTime{mJournalPlot->getTo().addDays(1)},
                                                              WalletJournalEntryRepository::EntryType::All)) :
                             (mJournalRepository.fetchForCharacterInRange(mCharacterId,
                                                                          QDateTime{mJournalPlot->getFrom()},
                                                                          QDateTime{mJournalPlot->getTo().addDays(1)},
                                                                          WalletJournalEntryRepository::EntryType::All));

        auto totalIncome = 0., totalOutcome = 0.;

        QHash<QDate, std::pair<double, double>> values;
        const auto valueAdder = [&values, &totalIncome, &totalOutcome](const auto &entries) {
            for (const auto &entry : entries)
            {
                if (entry->isIgnored())
                    continue;

                auto &value = values[entry->getTimestamp().toLocalTime().date()];

                const auto amount = entry->getAmount();
                if (Q_UNLIKELY(!amount))
                    continue;

                if (*amount < 0.)
                {
                    totalOutcome -= *amount;
                    value.first -= *amount;
                }
                else
                {
                    totalIncome += *amount;
                    value.second += *amount;
                }
            }
        };

        valueAdder(entries);

        QSettings settings;
        if (settings.value(StatisticsSettings::combineCorpAndCharPlotsKey, StatisticsSettings::combineCorpAndCharPlotsDefault).toBool())
        {
            try
            {
                const auto corpId = mCharacterRepository.getCorporationId(mCharacterId);
                const auto entries = (combineStats) ?
                                     (mCorpJournalRepository.fetchInRange(QDateTime{mJournalPlot->getFrom()},
                                                                          QDateTime{mJournalPlot->getTo().addDays(1)},
                                                                          WalletJournalEntryRepository::EntryType::All)) :
                                     (mCorpJournalRepository.fetchForCorporationInRange(corpId,
                                                                                        QDateTime{mJournalPlot->getFrom()},
                                                                                        QDateTime{mJournalPlot->getTo().addDays(1)},
                                                                                        WalletJournalEntryRepository::EntryType::All));

                valueAdder(entries);
            }
            catch (const CharacterRepository::NotFoundException &)
            {
            }
        }

        QVector<double> ticks, incomingTicks, outgoingTicks, incomingValues, outgoingValues;
        createBarTicks(ticks, incomingTicks, outgoingTicks, incomingValues, outgoingValues, values);

        mIncomingPlot->setData(incomingTicks, incomingValues);
        mOutgoingPlot->setData(outgoingTicks, outgoingValues);

        mJournalPlot->getPlot().rescaleAxes();
        mJournalPlot->getPlot().replot();

        const auto loc = locale();

        mJournalIncomeLabel->setText(TextUtils::currencyToString(totalIncome, loc));
        mJournalOutcomeLabel->setText(TextUtils::currencyToString(totalOutcome, loc));
        mJournalBalanceLabel->setText(TextUtils::currencyToString(totalIncome - totalOutcome, loc));
    }

    void BasicStatisticsWidget::updateTransactionData()
    {
        const auto combineStats = mCombineStatsBtn->isChecked();
        const auto entries = (combineStats) ?
                             (mTransactionRepository.fetchInRange(QDateTime{mTransactionPlot->getFrom()},
                                                                  QDateTime{mTransactionPlot->getTo().addDays(1)},
                                                                  WalletTransactionRepository::EntryType::All)) :
                             (mTransactionRepository.fetchForCharacterInRange(mCharacterId,
                                                                              QDateTime{mTransactionPlot->getFrom()},
                                                                              QDateTime{mTransactionPlot->getTo().addDays(1)},
                                                                              WalletTransactionRepository::EntryType::All));

        auto totalIncome = 0., totalOutcome = 0.;

        QHash<QDate, std::pair<double, double>> values;
        const auto valueAdder = [&values, &totalIncome, &totalOutcome](const auto &entries) {
            for (const auto &entry : entries)
            {
                if (entry->isIgnored())
                    continue;

                auto &value = values[entry->getTimestamp().toLocalTime().date()];

                const auto amount = entry->getPrice() * entry->getQuantity();
                if (entry->getType() == Evernus::WalletTransaction::Type::Buy)
                {
                    totalOutcome += amount;
                    value.first += amount;
                }
                else
                {
                    totalIncome += amount;
                    value.second += amount;
                }
            }
        };

        valueAdder(entries);

        QSettings settings;
        if (settings.value(StatisticsSettings::combineCorpAndCharPlotsKey, StatisticsSettings::combineCorpAndCharPlotsDefault).toBool())
        {
            try
            {
                const auto corpId = mCharacterRepository.getCorporationId(mCharacterId);
                const auto entries = (combineStats) ?
                                     (mCorpTransactionRepository.fetchInRange(QDateTime{mJournalPlot->getFrom()},
                                                                              QDateTime{mJournalPlot->getTo().addDays(1)},
                                                                              WalletTransactionRepository::EntryType::All)) :
                                     (mCorpTransactionRepository.fetchForCorporationInRange(corpId,
                                                                                            QDateTime{mJournalPlot->getFrom()},
                                                                                            QDateTime{mJournalPlot->getTo().addDays(1)},
                                                                                            WalletTransactionRepository::EntryType::All));

                valueAdder(entries);
            }
            catch (const CharacterRepository::NotFoundException &)
            {
            }
        }

        QVector<double> ticks, incomingTicks, outgoingTicks, incomingValues, outgoingValues;
        createBarTicks(ticks, incomingTicks, outgoingTicks, incomingValues, outgoingValues, values);

        mSellPlot->setData(incomingTicks, incomingValues);
        mBuyPlot->setData(outgoingTicks, outgoingValues);

        mTransactionPlot->getPlot().rescaleAxes();
        mTransactionPlot->getPlot().replot();

        const auto loc = locale();

        mTransactionsIncomeLabel->setText(TextUtils::currencyToString(totalIncome, loc));
        mTransactionsOutcomeLabel->setText(TextUtils::currencyToString(totalOutcome, loc));
        mTransactionsBalanceLabel->setText(TextUtils::currencyToString(totalIncome - totalOutcome, loc));
    }

    void BasicStatisticsWidget::updateData()
    {
        updateBalanceData();
        updateJournalData();
        updateTransactionData();
    }

    void BasicStatisticsWidget::handleNewPreferences()
    {
        QSettings settings;
        const auto numberFormat
            = settings.value(UISettings::plotNumberFormatKey, UISettings::plotNumberFormatDefault).toString();
        const auto dateFormat
            = settings.value(UISettings::dateTimeFormatKey, mBalancePlot->getDateTimeFormat()).toString();

        mBalancePlot->getPlot().yAxis->setNumberFormat(numberFormat);
        mJournalPlot->getPlot().yAxis->setNumberFormat(numberFormat);
        mTransactionPlot->getPlot().yAxis->setNumberFormat(numberFormat);

        if (settings.value(UISettings::applyDateFormatToGraphsKey, UISettings::applyDateFormatToGraphsDefault).toBool())
        {
            mBalancePlot->setDateTimeFormat(dateFormat);
            mJournalPlot->setDateTimeFormat(dateFormat);
            mTransactionPlot->setDateTimeFormat(dateFormat);
        }

        updateJournalData();
        updateTransactionData();
        updateGraphColors();

        mBalancePlot->getPlot().replot();
        mJournalPlot->getPlot().replot();
        mTransactionPlot->getPlot().replot();
    }

    void BasicStatisticsWidget::updateBalanceTooltip(QMouseEvent *event)
    {
        auto getValue = [=](const QCPGraph *graph) {
            const auto x = graph->keyAxis()->pixelToCoord(event->pos().x());
            const auto data = graph->data();
            const auto b = data->findBegin(x);

            if (b == std::end(*data))
                return 0.;

            const auto a = (b->key == x || b == std::begin(*data)) ? (b) : (std::prev(b));
            const auto coeff = (qFuzzyCompare(a->key, b->key)) ? (1.) : (x - a->key) / (b->key - a->key);
            return b->value * coeff + a->value * (1 - coeff);
        };

        const auto sellOrdersValue = getValue(mBalancePlot->getPlot().graph(sellOrdersGraph));
        const auto buyOrdersValue = getValue(mBalancePlot->getPlot().graph(buyOrdersGraph));
        const auto corpWalletValue = getValue(mBalancePlot->getPlot().graph(corpWalletBalanceGraph));
        const auto walletValue = getValue(mBalancePlot->getPlot().graph(walletBalanceGraph));
        const auto assetValue = getValue(mBalancePlot->getPlot().graph(assetValueGraph));
        const auto corpAssetValue = getValue(mBalancePlot->getPlot().graph(corpAssetValueGraph));
        const auto totalValue = getValue(mBalancePlot->getPlot().graph(totalValueGraph));

        const auto loc = locale();
        mBalancePlot->setToolTip(
            tr("Assets: %1\nCorp. assets: %2\nWallet: %3\nCorp. wallet: %4\nBuy orders: %5\nSell orders: %6\nTotal: %7")
                .arg(TextUtils::currencyToString(assetValue, loc))
                .arg(TextUtils::currencyToString(corpAssetValue, loc))
                .arg(TextUtils::currencyToString(walletValue, loc))
                .arg(TextUtils::currencyToString(corpWalletValue, loc))
                .arg(TextUtils::currencyToString(buyOrdersValue, loc))
                .arg(TextUtils::currencyToString(sellOrdersValue, loc))
                .arg(TextUtils::currencyToString(totalValue, loc))
        );
    }

    void BasicStatisticsWidget::updateGraphAndLegend()
    {
        auto assetGraph = mBalancePlot->getPlot().addGraph();
        assetGraph->setName(tr("Asset value"));

        auto walletGraph = mBalancePlot->getPlot().addGraph();
        walletGraph->setName(tr("Wallet balance"));

        auto corpWalletGraph = mBalancePlot->getPlot().addGraph();
        corpWalletGraph->setName(tr("Corp. wallet balance"));

        auto corpAssetGraph = mBalancePlot->getPlot().addGraph();
        corpAssetGraph->setName(tr("Corp. asset value"));

        auto buyGraph = mBalancePlot->getPlot().addGraph();
        buyGraph->setName(tr("Buy order value"));

        auto sellGraph = mBalancePlot->getPlot().addGraph();
        sellGraph->setName(tr("Sell order value"));

        auto totalGraph = mBalancePlot->getPlot().addGraph();
        totalGraph->setName(tr("Total value"));

        mBalancePlot->getPlot().legend->setVisible(true);

        mIncomingPlot = createBarPlot(mJournalPlot, tr("Incoming"), Qt::green);
        mOutgoingPlot = createBarPlot(mJournalPlot, tr("Outgoing"), Qt::red);

        mSellPlot = createBarPlot(mTransactionPlot, tr("Sell"), Qt::green);
        mBuyPlot = createBarPlot(mTransactionPlot, tr("Buy"), Qt::red);

        mJournalPlot->getPlot().legend->setVisible(true);
        mTransactionPlot->getPlot().legend->setVisible(true);
        

        updateGraphColors();
    }

    void BasicStatisticsWidget::updateGraphColors()
    {
        const auto sellOrdersValue = mBalancePlot->getPlot().graph(sellOrdersGraph);
        const auto buyOrdersValue = mBalancePlot->getPlot().graph(buyOrdersGraph);
        const auto corpWalletValue = mBalancePlot->getPlot().graph(corpWalletBalanceGraph);
        const auto walletValue = mBalancePlot->getPlot().graph(walletBalanceGraph);
        const auto assetValue = mBalancePlot->getPlot().graph(assetValueGraph);
        const auto corpAssetValue = mBalancePlot->getPlot().graph(corpAssetValueGraph);
        const auto totalValue = mBalancePlot->getPlot().graph(totalValueGraph);

        QSettings settings;

        sellOrdersValue->setPen(
            settings.value(StatisticsSettings::statisticsSellOrderPlotColorKey, StatisticsSettings::statisticsSellOrderPlotColorDefault).value<QColor>());
        buyOrdersValue->setPen(
            settings.value(StatisticsSettings::statisticsBuyOrderPlotColorKey, StatisticsSettings::statisticsBuyOrderPlotColorDefault).value<QColor>());
        corpWalletValue->setPen(
            settings.value(StatisticsSettings::statisticsCorpWalletPlotColorKey, StatisticsSettings::statisticsCorpWalletPlotColorDefault).value<QColor>());
        walletValue->setPen(
            settings.value(StatisticsSettings::statisticsWalletPlotColorKey, StatisticsSettings::statisticsWalletPlotColorDefault).value<QColor>());
        assetValue->setPen(
            settings.value(StatisticsSettings::statisticsAssetPlotColorKey, StatisticsSettings::statisticsAssetPlotColorDefault).value<QColor>());
        corpAssetValue->setPen(
            settings.value(StatisticsSettings::statisticsCorpAssetPlotColorKey, StatisticsSettings::statisticsCorpAssetPlotColorDefault).value<QColor>());
        totalValue->setPen(
            settings.value(StatisticsSettings::statisticsTotalPlotColorKey, StatisticsSettings::statisticsTotalPlotColorDefault).value<QColor>());
    }

    DateFilteredPlotWidget *BasicStatisticsWidget::createPlot()
    {
        QSettings settings;
        auto plot = new DateFilteredPlotWidget{this};
        plot->getPlot().xAxis->grid()->setVisible(false);
        plot->getPlot().yAxis->setNumberPrecision(2);
        plot->getPlot().yAxis->setLabel("ISK");
        
        if (settings.value(UISettings::mDarkModeKey, UISettings::mDarkModeDefault).toBool())
        {

            QLinearGradient plotGradient;
            plot->getPlot().yAxis->setLabelColor(QColor(255, 255, 255));
            plot->getPlot().yAxis->setTickLabelColor(QColor(255, 255, 255));
            plot->getPlot().xAxis->setTickLabelColor(QColor(255, 255, 255));
            plotGradient.setStart(0, 0);
            plotGradient.setFinalStop(0, 350);
            plotGradient.setColorAt(0, QColor(80, 80, 80));
            plotGradient.setColorAt(1, QColor(50, 50, 50));
            plot->getPlot().setBackground(plotGradient);


        }


        
        plot->getPlot().yAxis->setNumberFormat(
            settings.value(UISettings::plotNumberFormatKey, UISettings::plotNumberFormatDefault).toString());

        if (settings.value(UISettings::applyDateFormatToGraphsKey, UISettings::applyDateFormatToGraphsDefault).toBool())
            plot->setDateTimeFormat(settings.value(UISettings::dateTimeFormatKey, plot->getDateTimeFormat()).toString());

        return plot;
    }

    QCPBars *BasicStatisticsWidget::createBarPlot(DateFilteredPlotWidget *plot, const QString &name, Qt::GlobalColor color)
    {
        auto graph = std::make_unique<QCPBars>(plot->getPlot().xAxis, plot->getPlot().yAxis);
        auto graphPtr = graph.get();
        graphPtr->setWidth(3600 * 6);
        graphPtr->setName(name);
        graphPtr->setPen(QPen{color});
        graphPtr->setBrush(color);
        graph.release();

        return graphPtr;
    }

    void BasicStatisticsWidget::createBarTicks(QVector<double> &ticks,
                                               QVector<double> &incomingTicks,
                                               QVector<double> &outgoingTicks,
                                               QVector<double> &incomingValues,
                                               QVector<double> &outgoingValues,
                                               const QHash<QDate, std::pair<double, double>> &values)
    {
        QHashIterator<QDate, std::pair<double, double>> it{values};
        while (it.hasNext())
        {
            it.next();

            const auto secs = QDateTime{it.key()}.toMSecsSinceEpoch() / 1000.;

            ticks << secs;

            incomingTicks << secs - 3600 * 3;
            outgoingTicks << secs + 3600 * 3;

            outgoingValues << it.value().first;
            incomingValues << it.value().second;
        }
    }
}
