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

class QCustomPlot;
class QComboBox;
class QCheckBox;
class QCPGraph;
class QCPBars;
class QLabel;

namespace Evernus
{
    class WalletTransactionRepository;
    class EveDataProvider;

    class ItemHistoryWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        ItemHistoryWidget(const WalletTransactionRepository &walletRepo,
                          const WalletTransactionRepository &corpWalletRepo,
                          const EveDataProvider &dataProvider,
                          QWidget *parent = nullptr);
        virtual ~ItemHistoryWidget() = default;

    public slots:
        void setCharacter(Character::IdType id);

        void updateData();
        void handleNewPreferences();

    private slots:
        void computeValues();

    private:
        const WalletTransactionRepository &mWalletRepo, &mCorpWalletRepo;
        const EveDataProvider &mDataProvider;

        Character::IdType mCharacterId = Character::invalidId;

        QComboBox *mItemTypeCombo = nullptr;
        QCheckBox *mAllCharactersBtn = nullptr;
        QCustomPlot *mPlot = nullptr;
        QLabel *mTotalIncomeLabel = nullptr;
        QLabel *mTotalOutcomeLabel = nullptr;
        QLabel *mTotalBalanceLabel = nullptr;
        QLabel *mTotalMarginLabel = nullptr;
        QLabel *mTotalVolumeLabel = nullptr;

        QCPGraph *mBalanceGraph = nullptr;
        QCPBars *mVolumeGraph = nullptr;

        double mTotalIncome = 0., mTotalOutcome = 0.;

        void setMarginColor();
        double getMargin() const noexcept;
    };
}
