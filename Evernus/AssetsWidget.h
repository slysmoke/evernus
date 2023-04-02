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

#include "ExternalOrderImporter.h"
#include "CharacterBoundWidget.h"
#include "LeafFilterProxyModel.h"
#include "AggregatedAssetModel.h"
#include "AssetModel.h"
#include "Character.h"
#include "Item.h"

class QItemSelection;
class QRadioButton;
class QLabel;

namespace Evernus
{
    class FilterTextRepository;
    class AdjustableTableView;
    class CacheTimerProvider;
    class EveDataProvider;
    class StyledTreeView;
    class AssetProvider;
    class StationView;
    class AssetList;

    class AssetsWidget
        : public CharacterBoundWidget
    {
        Q_OBJECT

    public:
        AssetsWidget(AssetProvider &assetProvider,
                     const EveDataProvider &dataProvider,
                     const CacheTimerProvider &cacheTimerProvider,
                     const FilterTextRepository &filterRepo,
                     bool corp,
                     QWidget *parent = nullptr);
        virtual ~AssetsWidget() = default;

    signals:
        void importPricesFromWeb(Character::IdType id, const TypeLocationPairs &target);
        void importPricesFromFile(Character::IdType id, const TypeLocationPairs &target);

        void setDestinationInEve(quint64 locationId);
        void showInEve(EveType::IdType id, Character::IdType charId);

    public slots:
        void updateData();

    private slots:
        void prepareItemImportFromWeb();
        void prepareItemImportFromFile();

        void applyWildcard(const QString &text);

        void setCustomStation(quint64 id);
        void setCustomValue();
        void clearCustomValue();

        void setDestinationForCurrent();
        void showInEveForCurrent();

        void handleSelection(const QItemSelection &selected);

        void setCombine(int state);

    private:
        AssetProvider &mAssetProvider;

        QRadioButton *mUseAssetStationBtn = nullptr;
        StyledTreeView *mAssetView = nullptr;
        AdjustableTableView *mAggregatedView = nullptr;
        StationView *mStationView = nullptr;
        QLabel *mInfoLabel = nullptr;

        AssetModel mInventoryModel;
        LeafFilterProxyModel mInventoryModelProxy;

        AggregatedAssetModel mAggregatedModel;
        LeafFilterProxyModel mAggregatedModelProxy;

        quint64 mCustomStationId = 0;

        QAction *mSetDestinationAct = nullptr;
        QAction *mShowInEveAct = nullptr;
        QAction *mSetCustomValueAct = nullptr;
        QAction *mClearCustomValueAct = nullptr;

        virtual void handleNewCharacter(Character::IdType id) override;

        void resetModel();
        void setNewInfo();

        TypeLocationPairs getImportTarget() const;

        QModelIndex getCurrentIndex() const;

        static void buildImportTarget(TypeLocationPairs &target, const Item &item, quint64 locationId);
    };
}
