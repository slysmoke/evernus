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

#include <QWidget>

#include "Character.h"

class QPushButton;
class QCheckBox;
class QCPBars;
class QLabel;

namespace Evernus
{
    class CorpMarketOrderValueSnapshotRepository;
    class MarketOrderValueSnapshotRepository;
    class CorpAssetValueSnapshotRepository;
    class WalletJournalEntryRepository;
    class AssetValueSnapshotRepository;
    class CorpWalletSnapshotRepository;
    class WalletTransactionRepository;
    class WalletSnapshotRepository;
    class DateFilteredPlotWidget;
    class CharacterRepository;
    class RepositoryProvider;
    class ItemCostProvider;

    class BasicStatisticsWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        BasicStatisticsWidget(const RepositoryProvider &repositoryProvider,
                              const ItemCostProvider &itemCostProvider,
                              QWidget *parent = nullptr);
        virtual ~BasicStatisticsWidget() = default;

    signals:
        void makeSnapshots();

    public slots:
        void setCharacter(Character::IdType id);
        void updateBalanceData();
        void updateJournalData();
        void updateTransactionData();
        void updateData();
        void handleNewPreferences();

    private slots:
        void updateBalanceTooltip(QMouseEvent *event);

    private:
        enum
        {
            assetValueGraph,
            walletBalanceGraph,
            corpWalletBalanceGraph,
            corpAssetValueGraph,
            buyOrdersGraph,
            sellOrdersGraph,
            totalValueGraph,
        };

        const AssetValueSnapshotRepository &mAssetSnapshotRepository;
        const CorpAssetValueSnapshotRepository &mCorpAssetSnapshotRepository;
        const WalletSnapshotRepository &mWalletSnapshotRepository;
        const CorpWalletSnapshotRepository &mCorpWalletSnapshotRepository;
        const MarketOrderValueSnapshotRepository &mMarketOrderSnapshotRepository;
        const CorpMarketOrderValueSnapshotRepository &mCorpMarketOrderSnapshotRepository;
        const WalletJournalEntryRepository &mJournalRepository, &mCorpJournalRepository;
        const WalletTransactionRepository &mTransactionRepository, &mCorpTransactionRepository;
        const CharacterRepository &mCharacterRepository;

        DateFilteredPlotWidget *mBalancePlot = nullptr;
        DateFilteredPlotWidget *mJournalPlot = nullptr;
        DateFilteredPlotWidget *mTransactionPlot = nullptr;

        QCPBars *mIncomingPlot = nullptr;
        QCPBars *mOutgoingPlot = nullptr;

        QCPBars *mBuyPlot = nullptr;
        QCPBars *mSellPlot = nullptr;

        QCheckBox *mCombineStatsBtn = nullptr;
        QLabel *mJournalIncomeLabel = nullptr;
        QLabel *mJournalOutcomeLabel = nullptr;
        QLabel *mJournalBalanceLabel = nullptr;
        QLabel *mTransactionsIncomeLabel = nullptr;
        QLabel *mTransactionsOutcomeLabel = nullptr;
        QLabel *mTransactionsBalanceLabel = nullptr;

        Character::IdType mCharacterId = Character::invalidId;

        void updateGraphAndLegend();
        void updateGraphColors();

        DateFilteredPlotWidget *createPlot();

        static QCPBars *createBarPlot(DateFilteredPlotWidget *plot, const QString &name, Qt::GlobalColor color);
        static void createBarTicks(QVector<double> &ticks,
                                   QVector<double> &incomingTicks,
                                   QVector<double> &outgoingTicks,
                                   QVector<double> &incomingValues,
                                   QVector<double> &outgoingValues,
                                   const QHash<QDate, std::pair<double, double>> &values);
    };
}
