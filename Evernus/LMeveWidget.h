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
#include <QWidget>

#include "ExternalOrderImporter.h"
#include "LMeveTaskModel.h"
#include "Character.h"

class QPushButton;

namespace Evernus
{
    class CacheTimerProvider;
    class LMeveDataProvider;
    class ItemCostProvider;
    class ButtonWithTimer;
    class StyledTreeView;
    class StationView;

    class LMeveWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        LMeveWidget(const CacheTimerProvider &cacheTimerProvider,
                    const EveDataProvider &dataProvider,
                    const LMeveDataProvider &lMeveDataProvider,
                    const ItemCostProvider &costProvider,
                    const CharacterRepository &characterRepository,
                    QWidget *parent = nullptr);
        virtual ~LMeveWidget() = default;

    signals:
        void syncLMeve(Character::IdType id);

        void openPreferences();

        void importPricesFromWeb(Character::IdType id, const TypeLocationPairs &target);
        void importPricesFromFile(Character::IdType id, const TypeLocationPairs &target);

    public slots:
        void setCharacter(Character::IdType id);
        void updateData();

    private slots:
        void setStationId(quint64 id);

        void prepareItemImportFromWeb();
        void prepareItemImportFromFile();

    private:
        const CacheTimerProvider &mCacheTimerProvider;
        const LMeveDataProvider &mLMeveDataProvider;

        ButtonWithTimer *mSyncBtn = nullptr;
        StyledTreeView *mTaskView = nullptr;
        StationView *mStationView = nullptr;
        QPushButton *mImportBtn = nullptr;

        Character::IdType mCharacterId = Character::invalidId;

        LMeveTaskModel mTaskModel;
        QSortFilterProxyModel mTaskProxy;

        QWidget *createTaskTab(const EveDataProvider &dataProvider);

        void refreshImportTimer();

        TypeLocationPairs getImportTarget() const;
    };
}
