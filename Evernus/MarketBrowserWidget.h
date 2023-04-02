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

#include <mutex>
#include <list>

#include <QWidget>

#include "MarketOrderDataFetcher.h"
#include "ExternalOrderSellModel.h"
#include "ExternalOrderBuyModel.h"
#include "ItemNameModel.h"
#include "TaskConstants.h"
#include "Character.h"

class QListWidgetItem;
class QDoubleSpinBox;
class QListWidget;
class QPushButton;
class QTabWidget;
class QByteArray;
class QListView;
class QCheckBox;
class QGroupBox;
class QSpinBox;
class QLabel;
class QMenu;

namespace Evernus
{
    class RegionTypePresetRepository;
    class LocationBookmarkRepository;
    class ExternalOrderRepository;
    class FavoriteItemRepository;
    class MarketOrderRepository;
    class MarketGroupRepository;
    class ExternalOrderView;
    class EveTypeRepository;
    class ItemCostProvider;
    class EveDataProvider;
    class TaskManager;

    class MarketBrowserWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        MarketBrowserWidget(const ExternalOrderRepository &externalOrderRepo,
                            const MarketOrderRepository &orderRepo,
                            const MarketOrderRepository &corpOrderRepo,
                            const CharacterRepository &characterRepo,
                            const FavoriteItemRepository &favoriteItemRepo,
                            const LocationBookmarkRepository &locationBookmarkRepo,
                            const EveTypeRepository &typeRepo,
                            const MarketGroupRepository &groupRepo,
                            const RegionTypePresetRepository &regionTypePresetRepo,
                            const MarketOrderProvider &orderProvider,
                            const MarketOrderProvider &corpOrderProvider,
                            EveDataProvider &dataProvider,
                            ESIInterfaceManager &interfaceManager,
                            TaskManager &taskManager,
                            const ItemCostProvider &costProvider,
                            QWidget *parent = nullptr);
        virtual ~MarketBrowserWidget() = default;

    signals:
        void importPricesFromFile(Character::IdType id, const TypeLocationPairs &target);

        void externalOrdersChanged();
        void preferencesChanged();

        void updateExternalOrders(const std::vector<ExternalOrder> &orders);

    public slots:
        void setCharacter(Character::IdType id);

        void updateData();

        void fillOrderItemNames();

        void showOrdersForType(EveType::IdType typeId);

    private slots:
        void prepareItemImportFromWeb();
        void prepareItemImportFromFile();

        void selectRegion(QListWidgetItem *item);
        void selectSolarSystem(QListWidgetItem *item);
        void selectStation(QListWidgetItem *item);

        void changeDeviationSource(ExternalOrderModel::DeviationSourceType type, double value);

        void stepBack();
        void stepForward();

        void addFavoriteItem();
        void addFavoriteItemFromAction();
        void removeFavoriteItem();
        void selectFavoriteItem(const QModelIndex &index);

        void showItemContextMenu(const QPoint &pos);

        void applyFilter();
        void resetFilter();

        void addBookmark();
        void removeBookmark();
        void selectBookmark();

        void cleanAllOrders();
        void cleanCurrentType();

        void importData(const TypeLocationPairs &pairs);

        void updateOrderTask(const QString &text);
        void endOrderTask(const MarketOrderDataFetcher::OrderResultType &orders, const QString &error);

    private:
        struct NavigationState
        {
            EveType::IdType mTypeId = EveType::invalidId;
            uint mRegionId = 0;
            uint mSolarSystemId = 0;
            quint64 mStationId = 0;
        };

        typedef std::list<NavigationState> NavigationStack;

        static const auto knownItemsTab = 0;
        static const auto orderItemsTab = 1;
        static const auto favoriteItemsTab = 2;

        const ExternalOrderRepository &mExternalOrderRepo;
        const MarketOrderRepository &mOrderRepo;
        const MarketOrderRepository &mCorpOrderRepo;
        const FavoriteItemRepository &mFavoriteItemRepo;
        const LocationBookmarkRepository &mLocationBookmarkRepo;
        const EveTypeRepository &mTypeRepo;
        const MarketGroupRepository &mGroupRepo;
        const RegionTypePresetRepository &mRegionTypePresetRepo;
        EveDataProvider &mDataProvider;
        TaskManager &mTaskManager;

        ItemNameModel mNameModel;
        ItemNameModel mOrderNameModel;
        ItemNameModel mFavoriteNameModel;

        ExternalOrderSellModel mExternalOrderSellModel;
        ExternalOrderBuyModel mExternalOrderBuyModel;

        QPushButton *mDeviationBtn = nullptr;

        QPushButton *mBackBtn = nullptr;
        QPushButton *mForwardBtn = nullptr;
        QMenu *mBookmarksMenu = nullptr;

        QTabWidget *mItemTabs = nullptr;

        QListView *mKnownItemList = nullptr;
        QListView *mOrderItemList = nullptr;
        QListView *mFavoriteItemList = nullptr;

        QPushButton *mRemoveFavoriteItemBtn = nullptr;

        QListWidget *mRegionList = nullptr;
        QListWidget *mSolarSystemList = nullptr;
        QListWidget *mStationList = nullptr;

        QGroupBox *mFilterGroup = nullptr;
        QDoubleSpinBox *mMinPriceFilter = nullptr;
        QDoubleSpinBox *mMaxPriceFilter = nullptr;
        QSpinBox *mMinVolumeFilter = nullptr;
        QSpinBox *mMaxVolumeFilter = nullptr;
        QCheckBox *mNullSecFilter = nullptr;
        QCheckBox *mLowSecFilter = nullptr;
        QCheckBox *mHighSecFilter = nullptr;

        QLabel *mInfoLabel = nullptr;

        ExternalOrderView *mSellView = nullptr;
        ExternalOrderView *mBuyView = nullptr;

        NavigationStack mNavigationStack;
        NavigationStack::const_iterator mNagivationPointer = std::begin(mNavigationStack);
        bool mBlockNavigationChange = false;

        Character::IdType mCharacterId = Character::invalidId;

        MarketOrderDataFetcher mDataFetcher;

        uint mOrderSubtask = TaskConstants::invalidTask;

        std::once_flag mKnownNamesFlag;

        TypeLocationPairs getImportTarget() const;

        void fillRegions();
        void fillKnownItemNames();
        void fillFavoriteItemNames();
        void fillBookmarksMenu();

        QWidget *createItemNameListTab(ItemNameModel &model, QListView *&view, QLayout *toolbarLayout = nullptr);

        QString getDeviationButtonText(ExternalOrderModel::DeviationSourceType type) const;

        void updateNavigationButtons();
        void saveNavigationState();
        void restoreNavigationState();
        void selectNagivationItems(uint regionId, uint solarSystemId, quint64 stationId);

        void setTypeId(EveType::IdType typeId);

        void setGrouping(ExternalOrderModel::Grouping grouping);
    };
}
