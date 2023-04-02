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

#include "CharacterBoundWidget.h"
#include "WalletJournalModel.h"

class QSortFilterProxyModel;
class QCheckBox;

namespace Evernus
{
    class WalletJournalEntryRepository;
    class WalletEntryFilterWidget;
    class FilterTextRepository;
    class CharacterRepository;
    class CacheTimerProvider;
    class EveDataProvider;
    class StyledTreeView;

    class WalletJournalWidget
        : public CharacterBoundWidget
    {
        Q_OBJECT

    public:
        typedef WalletJournalModel::EntryType EntryType;

        WalletJournalWidget(const WalletJournalEntryRepository &journalRepo,
                            const CharacterRepository &characterRepository,
                            const FilterTextRepository &filterRepo,
                            const CacheTimerProvider &cacheTimerProvider,
                            const EveDataProvider &dataProvider,
                            bool corp,
                            QWidget *parent = nullptr);
        virtual ~WalletJournalWidget() = default;

    public slots:
        void updateData();

        void updateFilter(const QDate &from, const QDate &to, const QString &filter, int type);

    private:
        WalletJournalModel mModel;
        QSortFilterProxyModel *mFilterModel = nullptr;

        WalletEntryFilterWidget *mFilter = nullptr;
        QCheckBox *mCombineBtn = nullptr;

        StyledTreeView *mView = nullptr;

        virtual void handleNewCharacter(Character::IdType id) override;
    };
}
