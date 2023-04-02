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

#include <QSortFilterProxyModel>
#include <QSharedPointer>
#include <QWidget>

#include "MarketOrderPerformanceModel.h"
#include "TypePerformanceModel.h"
#include "Character.h"

class QCPAxisTickerText;
class QCustomPlot;
class QCheckBox;
class QCPBars;

namespace Evernus
{
    class WalletTransactionRepository;
    class CharacterRepository;
    class AdjustableTableView;
    class DateRangeWidget;
    class EveDataProvider;

    class ReportsWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        ReportsWidget(const RepositoryProvider &repositoryProvider,
                      const EveDataProvider &dataProvider,
                      QWidget *parent = nullptr);
        ReportsWidget(const ReportsWidget &) = default;
        ReportsWidget(ReportsWidget &&) = default;
        virtual ~ReportsWidget() = default;

        void setCharacter(Character::IdType id);
        void handleNewPreferences();

        ReportsWidget &operator =(const ReportsWidget &) = default;
        ReportsWidget &operator =(ReportsWidget &&) = default;

    private slots:
        void recalculateData();

    private:
        const WalletTransactionRepository &mWalletTransactionRepository;
        const WalletTransactionRepository &mCorpWalletTransactionRepository;
        const CharacterRepository &mCharacterRepository;

        const EveDataProvider &mDataProvider;

        DateRangeWidget *mDateRangeEdit = nullptr;
        QCheckBox *mCombineBtn = nullptr;
        QCheckBox *mCombineWithCorpBtn = nullptr;

        TypePerformanceModel mTypePerformanceModel;
        QSortFilterProxyModel mTypePerformanceProxy;

        MarketOrderPerformanceModel mMarketOrderPerformanceModel;
        QSortFilterProxyModel mMarketOrderPerformanceProxy;

        AdjustableTableView *mBestItemsView = nullptr;
        AdjustableTableView *mFastestOrdersView = nullptr;

        QSharedPointer<QCPAxisTickerText> mStationNamesTicker;

        QCustomPlot *mStationProfitPlot = nullptr;
        QCPBars *mStationProfitGraph = nullptr;
        QCPBars *mStationCostGraph = nullptr;
        QCPBars *mStationVolumeGraph = nullptr;

        Character::IdType mCharacterId = Character::invalidId;

        void recalculateBestItems(bool combineCharacters, bool combineCorp);
        void recalculateFastestOrders(bool combineCharacters, bool combineCorp);
        void recalculateTotalProfit(bool combineCharacters, bool combineCorp);

        void applyGraphFormats();
    };
}
