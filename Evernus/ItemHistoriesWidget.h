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

class QVBoxLayout;

namespace Evernus
{
    class WalletTransactionRepository;
    class EveDataProvider;

    class ItemHistoriesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        ItemHistoriesWidget(const WalletTransactionRepository &walletRepo,
                            const WalletTransactionRepository &corpWalletRepo,
                            const EveDataProvider &dataProvider,
                            QWidget *parent = nullptr);
        virtual ~ItemHistoriesWidget() = default;

    signals:
        void updateData();
        void handleNewPreferences();

        void characterChanged(Character::IdType id);

    public slots:
        void setCharacter(Character::IdType id);

    private slots:
        void addHistory();

    private:
        const WalletTransactionRepository &mWalletRepo;
        const WalletTransactionRepository &mCorpWalletRepo;
        const EveDataProvider &mDataProvider;

        QVBoxLayout *mMainLayout = nullptr;

        Character::IdType mCharacterId = Character::invalidId;
    };
}
