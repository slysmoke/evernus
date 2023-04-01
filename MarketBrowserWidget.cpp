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
#include <QSortFilterProxyModel>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QWidgetAction>
#include <QRadioButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QTabWidget>
#include <QCheckBox>
#include <QSqlQuery>
#include <QListView>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QMenu>
#include <QFont>

#include "LocationBookmarkSelectDialog.h"
#include "LocationBookmarkRepository.h"
#include "ExternalOrderRepository.h"
#include "FavoriteItemRepository.h"
#include "RegionTypeSelectDialog.h"
#include "MarketOrderRepository.h"
#include "DeviationSourceWidget.h"
#include "ItemTypeSelectDialog.h"
#include "ExternalOrderView.h"
#include "MarketOrderView.h"
#include "EveDataProvider.h"
#include "DatabaseUtils.h"
#include "SSOMessageBox.h"
#include "TaskManager.h"
#include "Defines.h"

#include "MarketBrowserWidget.h"

namespace Evernus
{
    MarketBrowserWidget::MarketBrowserWidget(const ExternalOrderRepository &externalOrderRepo,
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
                                             QWidget *parent)
        : QWidget{parent}
        , mExternalOrderRepo{externalOrderRepo}
        , mOrderRepo{orderRepo}
        , mCorpOrderRepo{corpOrderRepo}
        , mFavoriteItemRepo{favoriteItemRepo}
        , mLocationBookmarkRepo{locationBookmarkRepo}
        , mTypeRepo{typeRepo}
        , mGroupRepo{groupRepo}
        , mRegionTypePresetRepo{regionTypePresetRepo}
        , mDataProvider{dataProvider}
        , mTaskManager{taskManager}
        , mNameModel{mDataProvider}
        , mOrderNameModel{mDataProvider}
        , mFavoriteNameModel{mDataProvider}
        , mExternalOrderSellModel{mDataProvider, mExternalOrderRepo, characterRepo, orderProvider, corpOrderProvider, costProvider}
        , mExternalOrderBuyModel{mDataProvider, mExternalOrderRepo, characterRepo, orderProvider, corpOrderProvider, costProvider}
        , mDataFetcher{mDataProvider, interfaceManager}
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolbarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolbarLayout);

        auto importBtn = new QPushButton{QIcon{QStringLiteral(":/images/world.png")}, tr("Import prices from Web"), this};
        toolbarLayout->addWidget(importBtn);
        importBtn->setFlat(true);
        connect(importBtn, &QPushButton::clicked, this, &MarketBrowserWidget::prepareItemImportFromWeb);

        importBtn = new QPushButton{QIcon{QStringLiteral(":/images/page_refresh.png")}, tr("Import prices from logs"), this};
        toolbarLayout->addWidget(importBtn);
        importBtn->setFlat(true);
        connect(importBtn, &QPushButton::clicked, this, &MarketBrowserWidget::prepareItemImportFromFile);

        auto filterBtn = new QPushButton{QIcon{QStringLiteral(":/images/flag_blue.png")}, tr("Filter"), this};
        toolbarLayout->addWidget(filterBtn);
        filterBtn->setFlat(true);
        filterBtn->setCheckable(true);

        auto deviationWidget = new DeviationSourceWidget{this};
        mExternalOrderSellModel.changeDeviationSource(deviationWidget->getCurrentType(), deviationWidget->getCurrentValue());
        mExternalOrderBuyModel.changeDeviationSource(deviationWidget->getCurrentType(), deviationWidget->getCurrentValue());
        connect(deviationWidget, &DeviationSourceWidget::sourceChanged, this, &MarketBrowserWidget::changeDeviationSource);

        auto deviationAction = new QWidgetAction{this};
        deviationAction->setDefaultWidget(deviationWidget);

        auto deviationMenu = new QMenu{this};
        deviationMenu->addAction(deviationAction);

        mDeviationBtn = new QPushButton{QIcon{QStringLiteral(":/images/tag.png")}, getDeviationButtonText(deviationWidget->getCurrentType()), this};
        toolbarLayout->addWidget(mDeviationBtn);
        mDeviationBtn->setFlat(true);
        mDeviationBtn->setMenu(deviationMenu);

        auto cleanupMenu = new QMenu{this};
        cleanupMenu->addAction(tr("Clean all orders"), this, &MarketBrowserWidget::cleanAllOrders);
        cleanupMenu->addAction(tr("Clean for selected type"), this, &MarketBrowserWidget::cleanCurrentType);

        auto cleanupBtn = new QPushButton{QIcon{QStringLiteral(":/images/cross.png")}, tr("Cleanup  "), this};
        toolbarLayout->addWidget(cleanupBtn);
        cleanupBtn->setFlat(true);
        cleanupBtn->setMenu(cleanupMenu);

        toolbarLayout->addStretch();

        auto mainViewLayout = new QHBoxLayout{};
        mainLayout->addLayout(mainViewLayout, 1);

        auto navigatorGroup = new QGroupBox{tr("Navigator"), this};
        mainViewLayout->addWidget(navigatorGroup);

        auto navigatorGroupLayout = new QVBoxLayout{navigatorGroup};

        auto navigationLayout = new QHBoxLayout{};
        navigatorGroupLayout->addLayout(navigationLayout);

        mBackBtn = new QPushButton{QIcon{QStringLiteral(":/images/arrow_left.png")}, tr("Back"), this};
        navigationLayout->addWidget(mBackBtn);
        mBackBtn->setFlat(true);
        mBackBtn->setDisabled(true);
        connect(mBackBtn, &QPushButton::clicked, this, &MarketBrowserWidget::stepBack);

        mForwardBtn = new QPushButton{QIcon{QStringLiteral(":/images/arrow_right.png")}, tr("Forward"), this};
        navigationLayout->addWidget(mForwardBtn);
        mForwardBtn->setFlat(true);
        mForwardBtn->setDisabled(true);
        connect(mForwardBtn, &QPushButton::clicked, this, &MarketBrowserWidget::stepForward);

        mBookmarksMenu = new QMenu{this};

        auto bookmarksBtn = new QPushButton{QIcon{QStringLiteral(":/images/star.png")}, tr("Bookmarks"), this};
        navigationLayout->addWidget(bookmarksBtn);
        bookmarksBtn->setFlat(true);
        bookmarksBtn->setMenu(mBookmarksMenu);

        auto groupingGroup = new QGroupBox{tr("Grouping"), this};
        navigatorGroupLayout->addWidget(groupingGroup);

        auto groupingLayout = new QHBoxLayout{groupingGroup};

        auto groupingBtn = new QRadioButton{tr("None"), this};
        groupingLayout->addWidget(groupingBtn);
        groupingBtn->setChecked(true);
        connect(groupingBtn, &QRadioButton::toggled, this, [this](bool checked) {
            if (checked)
                setGrouping(ExternalOrderModel::Grouping::None);
        });

        groupingBtn = new QRadioButton{tr("Station"), this};
        groupingLayout->addWidget(groupingBtn);
        connect(groupingBtn, &QRadioButton::toggled, this, [this](bool checked) {
            if (checked)
                setGrouping(ExternalOrderModel::Grouping::Station);
        });

        groupingBtn = new QRadioButton{tr("System"), this};
        groupingLayout->addWidget(groupingBtn);
        connect(groupingBtn, &QRadioButton::toggled, this, [this](bool checked) {
            if (checked)
                setGrouping(ExternalOrderModel::Grouping::System);
        });

        groupingBtn = new QRadioButton{tr("Region"), this};
        groupingLayout->addWidget(groupingBtn);
        connect(groupingBtn, &QRadioButton::toggled, this, [this](bool checked) {
            if (checked)
                setGrouping(ExternalOrderModel::Grouping::Region);
        });

        mItemTabs = new QTabWidget{this};
        navigatorGroupLayout->addWidget(mItemTabs);

        mItemTabs->addTab(createItemNameListTab(mNameModel, mKnownItemList), tr("Name"));

        mKnownItemList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(mKnownItemList, &QListView::customContextMenuRequested, this, &MarketBrowserWidget::showItemContextMenu);

        mItemTabs->addTab(createItemNameListTab(mOrderNameModel, mOrderItemList), tr("My orders"));

        mOrderItemList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(mOrderItemList, &QListView::customContextMenuRequested, this, &MarketBrowserWidget::showItemContextMenu);

        auto favoriteToolbarLayout = new QHBoxLayout{};

        mItemTabs->addTab(createItemNameListTab(mFavoriteNameModel, mFavoriteItemList, favoriteToolbarLayout), tr("Favorite"));

        auto addFavoriteBtn = new QPushButton{QIcon{QStringLiteral(":/images/add.png")}, tr("Add..."), this};
        favoriteToolbarLayout->addWidget(addFavoriteBtn);
        addFavoriteBtn->setFlat(true);
        connect(addFavoriteBtn, &QPushButton::clicked, this, &MarketBrowserWidget::addFavoriteItem);

        mRemoveFavoriteItemBtn = new QPushButton{QIcon{QStringLiteral(":/images/delete.png")}, tr("Remove"), this};
        favoriteToolbarLayout->addWidget(mRemoveFavoriteItemBtn);
        mRemoveFavoriteItemBtn->setFlat(true);
        mRemoveFavoriteItemBtn->setDisabled(true);
        connect(mRemoveFavoriteItemBtn, &QPushButton::clicked, this, &MarketBrowserWidget::removeFavoriteItem);

        favoriteToolbarLayout->addStretch();

        connect(mFavoriteItemList->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &MarketBrowserWidget::selectFavoriteItem);

        auto filterLabel = new QLabel{tr("Regions [<a href='#'>all</a>]"), this};
        navigatorGroupLayout->addWidget(filterLabel);
        connect(filterLabel, &QLabel::linkActivated, this, [this] {
            mRegionList->setCurrentRow(0);
        });

        mRegionList = new QListWidget{this};
        navigatorGroupLayout->addWidget(mRegionList);
        connect(mRegionList, &QListWidget::currentItemChanged, this, &MarketBrowserWidget::selectRegion);

        filterLabel = new QLabel{tr("Solar systems [<a href='#'>all</a>]"), this};
        navigatorGroupLayout->addWidget(filterLabel);
        connect(filterLabel, &QLabel::linkActivated, this, [this] {
            mSolarSystemList->setCurrentRow(0);
        });

        mSolarSystemList = new QListWidget{this};
        navigatorGroupLayout->addWidget(mSolarSystemList);
        connect(mSolarSystemList, &QListWidget::currentItemChanged, this, &MarketBrowserWidget::selectSolarSystem);

        filterLabel = new QLabel{tr("Stations [<a href='#'>all</a>]"), this};
        navigatorGroupLayout->addWidget(filterLabel);
        connect(filterLabel, &QLabel::linkActivated, this, [this] {
            mStationList->setCurrentRow(0);
        });

        mStationList = new QListWidget{this};
        navigatorGroupLayout->addWidget(mStationList);
        connect(mStationList, &QListWidget::currentItemChanged, this, &MarketBrowserWidget::selectStation);

        auto orderLayout = new QVBoxLayout{};
        mainViewLayout->addLayout(orderLayout, 1);

        mFilterGroup = new QGroupBox{tr("Filter"), this};
        orderLayout->addWidget(mFilterGroup);
        mFilterGroup->setVisible(false);
        connect(filterBtn, &QPushButton::toggled, mFilterGroup, &QGroupBox::setVisible);

        auto filterLayout = new QGridLayout{mFilterGroup};

        filterLayout->addWidget(new QLabel{tr("Min. price:"), this}, 0, 0);

        mMinPriceFilter = new QDoubleSpinBox{this};
        filterLayout->addWidget(mMinPriceFilter, 0, 1);
        mMinPriceFilter->setSpecialValueText(tr("any"));
        mMinPriceFilter->setMaximum(1000000000000.);
        mMinPriceFilter->setSuffix("ISK");

        filterLayout->addWidget(new QLabel{tr("Max. price:"), this}, 1, 0);

        mMaxPriceFilter = new QDoubleSpinBox{this};
        filterLayout->addWidget(mMaxPriceFilter, 1, 1);
        mMaxPriceFilter->setSpecialValueText(tr("any"));
        mMaxPriceFilter->setMaximum(1000000000000.);
        mMaxPriceFilter->setSuffix("ISK");

        filterLayout->addWidget(new QLabel{tr("Min. volume:"), this}, 0, 2);

        mMinVolumeFilter = new QSpinBox{this};
        filterLayout->addWidget(mMinVolumeFilter, 0, 3);
        mMinVolumeFilter->setSpecialValueText(tr("any"));
        mMinVolumeFilter->setMaximum(1000000000);

        filterLayout->addWidget(new QLabel{tr("Max. volume:"), this}, 1, 2);

        mMaxVolumeFilter = new QSpinBox{this};
        filterLayout->addWidget(mMaxVolumeFilter, 1, 3);
        mMaxVolumeFilter->setSpecialValueText(tr("any"));
        mMaxVolumeFilter->setMaximum(1000000000);

        filterLayout->addWidget(new QLabel{tr("Security status:"), this}, 0, 4);

        auto securityStatusFilterLayout = new QHBoxLayout{};
        filterLayout->addLayout(securityStatusFilterLayout, 1, 4);

        mNullSecFilter = new QCheckBox{tr("-1.0 - 0.0"), this};
        securityStatusFilterLayout->addWidget(mNullSecFilter);
        mNullSecFilter->setChecked(true);
        mNullSecFilter->setStyleSheet("color: red;");

        mLowSecFilter = new QCheckBox{tr("0.1 - 0.4"), this};
        securityStatusFilterLayout->addWidget(mLowSecFilter);
        mLowSecFilter->setChecked(true);
        mLowSecFilter->setStyleSheet("color: orange;");

        mHighSecFilter = new QCheckBox{tr("0.5 - 1.0"), this};
        securityStatusFilterLayout->addWidget(mHighSecFilter);
        mHighSecFilter->setChecked(true);
        mHighSecFilter->setStyleSheet("color: green;");

        securityStatusFilterLayout->addStretch();

        auto applyFilterBtn = new QPushButton{tr("Apply"), this};
        filterLayout->addWidget(applyFilterBtn, 0, 5);
        connect(applyFilterBtn, &QPushButton::clicked, this, &MarketBrowserWidget::applyFilter);

        auto resetFilterBtn = new QPushButton{tr("Reset"), this};
        filterLayout->addWidget(resetFilterBtn, 1, 5);
        connect(resetFilterBtn, &QPushButton::clicked, this, &MarketBrowserWidget::resetFilter);

        QFont font;
        font.setBold(true);

        mInfoLabel = new QLabel{tr("select an item"), this};
        orderLayout->addWidget(mInfoLabel);
        mInfoLabel->setFont(font);

        auto sellGroup = new QGroupBox{tr("Sell orders"), this};
        orderLayout->addWidget(sellGroup);

        auto sellLayout = new QVBoxLayout{sellGroup};

        mSellView = new ExternalOrderView{costProvider, mDataProvider, "externalOrderSellView", this};
        sellLayout->addWidget(mSellView);
        mSellView->setModel(&mExternalOrderSellModel);

        auto setSellDeviationValueAct = new QAction{QIcon{":/images/tag.png"}, tr("Set as deviation reference"), this};
        connect(setSellDeviationValueAct, &QAction::triggered, [deviationWidget, this] {
            const auto index = mSellView->currentIndex();
            if (index.isValid())
                deviationWidget->setDeviationValue(mExternalOrderSellModel.getPrice(index));
        });
        mSellView->addTreeViewAction(setSellDeviationValueAct);

        auto buyGroup = new QGroupBox{tr("Buy orders"), this};
        orderLayout->addWidget(buyGroup);

        auto buyLayout = new QVBoxLayout{buyGroup};

        mBuyView = new ExternalOrderView{costProvider, mDataProvider, "externalOrderBuyView", this};
        buyLayout->addWidget(mBuyView);
        mBuyView->setModel(&mExternalOrderBuyModel);

        auto setBuyDeviationValueAct = new QAction{QIcon{":/images/tag.png"}, tr("Set as deviation reference"), this};
        connect(setBuyDeviationValueAct, &QAction::triggered, [deviationWidget, this] {
            const auto index = mBuyView->currentIndex();
            if (index.isValid())
                deviationWidget->setDeviationValue(mExternalOrderBuyModel.getPrice(index));
        });
        mBuyView->addTreeViewAction(setBuyDeviationValueAct);

        connect(&mDataFetcher, &MarketOrderDataFetcher::orderStatusUpdated,
                this, &MarketBrowserWidget::updateOrderTask);
        connect(&mDataFetcher, &MarketOrderDataFetcher::orderImportEnded,
                this, &MarketBrowserWidget::endOrderTask);
        connect(&mDataFetcher, &MarketOrderDataFetcher::genericError,
                this, [=](const auto &text) {
            SSOMessageBox::showMessage(text, this);
        });
    }

    void MarketBrowserWidget::setCharacter(Character::IdType id)
    {
        std::call_once(mKnownNamesFlag, [=] {
            fillBookmarksMenu();
            fillKnownItemNames();
            fillFavoriteItemNames();
            fillOrderItemNames();
            fillRegions();
        });

        mCharacterId = id;

        mExternalOrderSellModel.setCharacter(mCharacterId);
        mExternalOrderBuyModel.setCharacter(mCharacterId);
        mSellView->setCharacterId(mCharacterId);
        mBuyView->setCharacterId(mCharacterId);
    }

    void MarketBrowserWidget::updateData()
    {
        fillRegions();
        fillKnownItemNames();

        mNavigationStack.clear();
        mNagivationPointer = std::begin(mNavigationStack);

        updateNavigationButtons();
    }

    void MarketBrowserWidget::fillOrderItemNames()
    {
        QSqlQuery query{mOrderRepo.getDatabase()};
        query.prepare(QString{"SELECT DISTINCT ids.type_id FROM ("
            "SELECT type_id FROM %1 WHERE state = ? "
            "UNION "
            "SELECT type_id FROM %2 WHERE state = ?"
        ") ids"}.arg(mOrderRepo.getTableName()).arg(mCorpOrderRepo.getTableName()));

        query.addBindValue(static_cast<int>(MarketOrder::State::Active));
        query.addBindValue(static_cast<int>(MarketOrder::State::Active));

        DatabaseUtils::execQuery(query);

        ItemNameModel::TypeList types;

        const auto size = query.size();
        if (size > 0)
            types.reserve(size);

        while (query.next())
            types.emplace_back(query.value(0).value<EveType::IdType>());

        mOrderNameModel.setTypes(std::move(types));
    }

    void MarketBrowserWidget::showOrdersForType(EveType::IdType typeId)
    {
        setTypeId(typeId);

        if (!mBlockNavigationChange)
        {
            saveNavigationState();
            mExternalOrderSellModel.reset();
            mExternalOrderBuyModel.reset();
        }
    }

    void MarketBrowserWidget::prepareItemImportFromWeb()
    {
        RegionTypeSelectDialog dlg{mDataProvider, mTypeRepo, mGroupRepo, mRegionTypePresetRepo, this};
        connect(&dlg, &RegionTypeSelectDialog::selected, this, &MarketBrowserWidget::importData, Qt::QueuedConnection);

        dlg.exec();
    }

    void MarketBrowserWidget::prepareItemImportFromFile()
    {
        emit importPricesFromFile(mCharacterId, getImportTarget());
    }

    void MarketBrowserWidget::selectRegion(QListWidgetItem *item)
    {
        mSolarSystemList->blockSignals(true);
        mSolarSystemList->clear();
        mSolarSystemList->blockSignals(false);

        if (item != nullptr)
        {
            const auto solarSystems = mExternalOrderRepo.fetchUniqueSolarSystems(item->data(Qt::UserRole).toUInt());
            for (const auto &solarSystem : solarSystems)
            {
                auto item = new QListWidgetItem{mDataProvider.getSolarSystemName(solarSystem), mSolarSystemList};
                item->setData(Qt::UserRole, solarSystem);
            }
        }

        mSolarSystemList->sortItems();

        auto allItem = new QListWidgetItem{tr("(all)")};
        mSolarSystemList->insertItem(0, allItem);
        mSolarSystemList->setCurrentItem(allItem);
    }

    void MarketBrowserWidget::selectSolarSystem(QListWidgetItem *item)
    {
        mStationList->blockSignals(true);
        mStationList->clear();
        mStationList->blockSignals(false);

        if (item != nullptr)
        {
            std::vector<quint64> stations;

            if (item->data(Qt::UserRole).toULongLong() == 0)
            {
                const auto regionItem = mRegionList->currentItem();
                if (regionItem == nullptr || regionItem->data(Qt::UserRole).toUInt() == 0)
                    stations = mExternalOrderRepo.fetchUniqueStations();
                else
                    stations = mExternalOrderRepo.fetchUniqueStationsByRegion(regionItem->data(Qt::UserRole).toUInt());
            }
            else
            {
                stations = mExternalOrderRepo.fetchUniqueStationsBySolarSystem(item->data(Qt::UserRole).toUInt());
            }

            for (const auto station : stations)
            {
                auto item = new QListWidgetItem{mDataProvider.getLocationName(station), mStationList};
                item->setData(Qt::UserRole, station);
            }
        }

        mStationList->sortItems();

        auto allItem = new QListWidgetItem{tr("(all)")};
        mStationList->insertItem(0, allItem);
        mStationList->setCurrentItem(allItem);
    }

    void MarketBrowserWidget::selectStation(QListWidgetItem *item)
    {
        if (item != nullptr)
        {
            const auto region = mRegionList->currentItem()->data(Qt::UserRole).toUInt();
            const auto solarSystem = mSolarSystemList->currentItem()->data(Qt::UserRole).toUInt();
            const auto station = item->data(Qt::UserRole).toULongLong();

            mExternalOrderSellModel.setRegionId(region);
            mExternalOrderSellModel.setSolarSystemId(solarSystem);
            mExternalOrderSellModel.setStationId(station);

            mExternalOrderBuyModel.setRegionId(region);
            mExternalOrderBuyModel.setSolarSystemId(solarSystem);
            mExternalOrderBuyModel.setStationId(station);

            mExternalOrderSellModel.reset();
            mExternalOrderBuyModel.reset();

            saveNavigationState();
        }
    }

    void MarketBrowserWidget::changeDeviationSource(ExternalOrderModel::DeviationSourceType type, double value)
    {
        mDeviationBtn->setText(getDeviationButtonText(type));
        mExternalOrderSellModel.changeDeviationSource(type, value);
        mExternalOrderBuyModel.changeDeviationSource(type, value);
    }

    void MarketBrowserWidget::stepBack()
    {
        Q_ASSERT(mNagivationPointer != std::begin(mNavigationStack));

        --mNagivationPointer;
        restoreNavigationState();

        updateNavigationButtons();
    }

    void MarketBrowserWidget::stepForward()
    {
        Q_ASSERT(mNagivationPointer != std::end(mNavigationStack));

        ++mNagivationPointer;
        restoreNavigationState();

        updateNavigationButtons();
    }

    void MarketBrowserWidget::addFavoriteItem()
    {
        ItemTypeSelectDialog dlg{mDataProvider, this};
        if (dlg.exec() == QDialog::Accepted)
        {
            const auto selected = dlg.getSelectedTypes();

            std::vector<FavoriteItem> items;
            items.reserve(selected.size());

            for (const auto id : selected)
                items.emplace_back(id);

            mFavoriteItemRepo.batchStore(items, true);

            fillFavoriteItemNames();
        }
    }

    void MarketBrowserWidget::addFavoriteItemFromAction()
    {
        const auto action = static_cast<const QAction *>(sender());

        FavoriteItem item{action->data().value<EveType::IdType>()};
        mFavoriteItemRepo.store(item);

        fillFavoriteItemNames();
    }

    void MarketBrowserWidget::removeFavoriteItem()
    {
        auto selectionModel = mFavoriteItemList->selectionModel();

        const auto index = selectionModel->currentIndex();
        if (!index.isValid())
            return;

        auto model = mFavoriteItemList->model();

        mFavoriteItemRepo.remove(model->data(index, Qt::UserRole).value<EveType::IdType>());
        model->removeRow(index.row(), QModelIndex{});
    }

    void MarketBrowserWidget::selectFavoriteItem(const QModelIndex &index)
    {
        mRemoveFavoriteItemBtn->setEnabled(index.isValid());
    }

    void MarketBrowserWidget::showItemContextMenu(const QPoint &pos)
    {
        const auto list = static_cast<const QListView *>(sender());
        const auto index = list->indexAt(pos);
        if (!index.isValid())
            return;

        QMenu menu;

        auto action = menu.addAction(tr("Add to favorites"), this, SLOT(addFavoriteItemFromAction()));
        action->setData(list->model()->data(index, Qt::UserRole));

        menu.exec(list->mapToGlobal(pos));
    }

    void MarketBrowserWidget::applyFilter()
    {
        ExternalOrderFilterProxyModel::SecurityStatuses security;
        if (mNullSecFilter->isChecked())
            security |= ExternalOrderFilterProxyModel::NullSec;
        if (mLowSecFilter->isChecked())
            security |= ExternalOrderFilterProxyModel::LowSec;
        if (mHighSecFilter->isChecked())
            security |= ExternalOrderFilterProxyModel::HighSec;

        const auto minPrice = mMinPriceFilter->value();
        const auto maxPrice = mMaxPriceFilter->value();
        const auto minVolume = mMinVolumeFilter->value();
        const auto maxVolume = mMaxVolumeFilter->value();

        mSellView->setFilter(minPrice, maxPrice, minVolume, maxVolume, security);
        mBuyView->setFilter(minPrice, maxPrice, minVolume, maxVolume, security);
    }

    void MarketBrowserWidget::resetFilter()
    {
        mMinPriceFilter->setValue(0.);
        mMaxPriceFilter->setValue(0.);
        mMinVolumeFilter->setValue(0);
        mMaxVolumeFilter->setValue(0);
        mNullSecFilter->setChecked(true);
        mLowSecFilter->setChecked(true);
        mHighSecFilter->setChecked(true);

        applyFilter();
    }

    void MarketBrowserWidget::addBookmark()
    {
        LocationBookmark bookmark;
        bookmark.setRegionId(mRegionList->currentItem()->data(Qt::UserRole).toUInt());
        bookmark.setSolarSystemId(mSolarSystemList->currentItem()->data(Qt::UserRole).toUInt());
        bookmark.setStationId(mStationList->currentItem()->data(Qt::UserRole).toUInt());

        mLocationBookmarkRepo.store(bookmark);

        fillBookmarksMenu();
    }

    void MarketBrowserWidget::removeBookmark()
    {
        LocationBookmarkSelectDialog dlg{mDataProvider, mLocationBookmarkRepo, this};
        if (dlg.exec() == QDialog::Accepted)
        {
            mLocationBookmarkRepo.remove(dlg.getSelectedBookmark());
            fillBookmarksMenu();
        }
    }

    void MarketBrowserWidget::selectBookmark()
    {
        const auto id = static_cast<const QAction *>(sender())->data().toUInt();

        try
        {
            const auto bookmark = mLocationBookmarkRepo.find(id);
            selectNagivationItems(bookmark->getRegionId(), bookmark->getSolarSystemId(), bookmark->getStationId());
        }
        catch (const LocationBookmarkRepository::NotFoundException &)
        {
        }
    }

    void MarketBrowserWidget::cleanAllOrders()
    {
        mDataProvider.clearExternalOrders();
        emit externalOrdersChanged();
    }

    void MarketBrowserWidget::cleanCurrentType()
    {
        mDataProvider.clearExternalOrdersForType(mExternalOrderSellModel.getTypeId());
        emit externalOrdersChanged();
    }

    void MarketBrowserWidget::importData(const TypeLocationPairs &pairs)
    {
        if (!mDataFetcher.hasPendingOrderRequests())
        {
            const auto mainTask = mTaskManager.startTask(tr("Importing data..."));

            mOrderSubtask = mTaskManager.startTask(mainTask, QStringLiteral("Making %1 order requests...").arg(pairs.size()));
        }

        mDataFetcher.importData(pairs, mCharacterId);
    }

    void MarketBrowserWidget::updateOrderTask(const QString &text)
    {
        mTaskManager.updateTask(mOrderSubtask, text);
    }

    void MarketBrowserWidget::endOrderTask(const MarketOrderDataFetcher::OrderResultType &orders, const QString &error)
    {
        Q_ASSERT(orders);

        if (error.isEmpty())
        {
            mTaskManager.updateTask(mOrderSubtask, tr("Saving %1 imported orders...").arg(orders->size()));
            emit updateExternalOrders(*orders);
        }

        mTaskManager.endTask(mOrderSubtask, error);
    }

    TypeLocationPairs MarketBrowserWidget::getImportTarget() const
    {
        TypeLocationPairs result;

        QSqlQuery query{mOrderRepo.getDatabase()};
        query.prepare(QString{"SELECT DISTINCT ids.type_id, ids.location_id FROM ("
            "SELECT type_id, location_id FROM %1 WHERE state = ? "
            "UNION "
            "SELECT type_id, location_id FROM %2 WHERE state = ? "
            "UNION "
            "SELECT type_id, location_id FROM %3"
        ") ids"}.arg(mOrderRepo.getTableName()).arg(mCorpOrderRepo.getTableName()).arg(mExternalOrderRepo.getTableName()));

        query.addBindValue(static_cast<int>(MarketOrder::State::Active));
        query.addBindValue(static_cast<int>(MarketOrder::State::Active));

        DatabaseUtils::execQuery(query);

        while (query.next())
            result.emplace(std::make_pair(query.value(0).value<EveType::IdType>(), query.value(1).toUInt()));

        return result;
    }

    void MarketBrowserWidget::fillRegions()
    {
        mBlockNavigationChange = true;

        mRegionList->blockSignals(true);
        mRegionList->clear();
        mRegionList->blockSignals(false);

        const auto regions = mExternalOrderRepo.fetchUniqueRegions();
        for (const auto &region : regions)
        {
            auto item = new QListWidgetItem{mDataProvider.getRegionName(region), mRegionList};
            item->setData(Qt::UserRole, region);
        }

        mRegionList->sortItems();

        auto allItem = new QListWidgetItem{tr("(all)")};
        mRegionList->insertItem(0, allItem);
        mRegionList->setCurrentItem(allItem);

        mBlockNavigationChange = false;
    }

    void MarketBrowserWidget::fillKnownItemNames()
    {
        mNameModel.setTypes(mExternalOrderRepo.fetchUniqueTypes());
    }

    void MarketBrowserWidget::fillFavoriteItemNames()
    {
        ItemNameModel::TypeList types;

        const auto items = mFavoriteItemRepo.fetchAll();
        types.reserve(items.size());

        for (const auto &item : items)
            types.emplace_back(item->getId());

        mFavoriteNameModel.setTypes(std::move(types));
    }

    void MarketBrowserWidget::fillBookmarksMenu()
    {
        mBookmarksMenu->clear();
        mBookmarksMenu->addAction(tr("Add bookmark"), this, SLOT(addBookmark()));
        mBookmarksMenu->addAction(tr("Remove bookmark..."), this, SLOT(removeBookmark()));
        mBookmarksMenu->addSeparator();

        auto bookmarks = mLocationBookmarkRepo.fetchAll();
        std::sort(std::begin(bookmarks), std::end(bookmarks), [this](const auto &a, const auto &b) {
            return a->toString(mDataProvider) < b->toString(mDataProvider);
        });

        for (const auto &bookmark : bookmarks)
            mBookmarksMenu->addAction(bookmark->toString(mDataProvider), this, SLOT(selectBookmark()))->setData(bookmark->getId());
    }

    QWidget *MarketBrowserWidget::createItemNameListTab(ItemNameModel &model, QListView *&view, QLayout *toolbarLayout)
    {
        auto knownItemsTab = new QWidget{this};
        auto knownItemsLayout = new QVBoxLayout{knownItemsTab};

        if (toolbarLayout != nullptr)
            knownItemsLayout->addLayout(toolbarLayout);

        auto knownItemsProxy = new QSortFilterProxyModel{this};
        knownItemsProxy->setSourceModel(&model);
        knownItemsProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        knownItemsProxy->sort(0);

        auto knowItemsFilterEdit = new QLineEdit{this};
        knownItemsLayout->addWidget(knowItemsFilterEdit);
        knowItemsFilterEdit->setClearButtonEnabled(true);
        knowItemsFilterEdit->setPlaceholderText(tr("type in wildcard"));
        connect(knowItemsFilterEdit, &QLineEdit::textChanged, knownItemsProxy, &QSortFilterProxyModel::setFilterWildcard);

        view = new QListView{this};
        knownItemsLayout->addWidget(view);
        view->setModel(knownItemsProxy);
        connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, [knownItemsProxy, this](const auto &index) {
            if (index.isValid())
            {
#if EVERNUS_VS_TEMPLATE_LAMBDA_HACK
                showOrdersForType(knownItemsProxy->data(index, Qt::UserRole).value<Evernus::EveType::IdType>());
#else
                showOrdersForType(knownItemsProxy->data(index, Qt::UserRole).template value<Evernus::EveType::IdType>());
#endif
            }
        });

        return knownItemsTab;
    }

    QString MarketBrowserWidget::getDeviationButtonText(ExternalOrderModel::DeviationSourceType type) const
    {
        switch (type) {
        case ExternalOrderModel::DeviationSourceType::Median:
            return tr("Deviation [median]  ");
        case ExternalOrderModel::DeviationSourceType::Best:
            return tr("Deviation [best price]  ");
        case ExternalOrderModel::DeviationSourceType::Cost:
            return tr("Deviation [custom cost]  ");
        case ExternalOrderModel::DeviationSourceType::Fixed:
            return tr("Deviation [fixed]  ");
        default:
            return tr("Deviation  ");
        }
    }

    void MarketBrowserWidget::updateNavigationButtons()
    {
        mBackBtn->setDisabled(mNagivationPointer == std::begin(mNavigationStack));
        mForwardBtn->setDisabled(mNagivationPointer == std::end(mNavigationStack) ||
                                 std::next(mNagivationPointer) == std::end(mNavigationStack));
    }

    void MarketBrowserWidget::saveNavigationState()
    {
        if (mBlockNavigationChange)
            return;

        const auto typeId = mExternalOrderSellModel.getTypeId();
        if (typeId == EveType::invalidId)
            return;

        if (mNagivationPointer != std::end(mNavigationStack))
            mNavigationStack.erase(std::next(mNagivationPointer), std::end(mNavigationStack));

        const auto regionItem = mRegionList->currentItem();
        if (regionItem == nullptr)
            return;

        const auto systemItem = mSolarSystemList->currentItem();
        if (systemItem == nullptr)
            return;

        const auto stationItem = mStationList->currentItem();
        if (stationItem == nullptr)
            return;

        NavigationState state;
        state.mTypeId = typeId;
        state.mRegionId = regionItem->data(Qt::UserRole).toUInt();
        state.mSolarSystemId = systemItem->data(Qt::UserRole).toUInt();
        state.mStationId = stationItem->data(Qt::UserRole).toUInt();

        mNavigationStack.emplace_back(std::move(state));
        mNagivationPointer = std::prev(std::end(mNavigationStack));

        updateNavigationButtons();
    }

    void MarketBrowserWidget::restoreNavigationState()
    {
        Q_ASSERT(mNagivationPointer != std::end(mNavigationStack));

        mBlockNavigationChange = true;

        const auto itemSearcher = [](auto typeId, auto &list) {
            const auto model = list.model();
            const auto rows = model->rowCount();

            auto selectionModel = list.selectionModel();
            selectionModel->clearSelection();

            for (auto i = 0; i < rows; ++i)
            {
                const auto index = model->index(i, 0);
                if (model->data(index, Qt::UserRole).toUInt() == typeId)
                {
                    selectionModel->setCurrentIndex(index, QItemSelectionModel::Select);
                    return true;
                }
            }

            return false;
        };

        auto found = false;

        const auto curTab = mItemTabs->currentIndex();
        if (curTab == knownItemsTab)
            found = itemSearcher(mNagivationPointer->mTypeId, *mKnownItemList);
        else if (curTab == orderItemsTab)
            found = itemSearcher(mNagivationPointer->mTypeId, *mOrderItemList);
        else
            found = itemSearcher(mNagivationPointer->mTypeId, *mFavoriteItemList);

        if (!found)
            setTypeId(mNagivationPointer->mTypeId);

        selectNagivationItems(mNagivationPointer->mRegionId, mNagivationPointer->mSolarSystemId, mNagivationPointer->mStationId);

        mBlockNavigationChange = false;
    }

    void MarketBrowserWidget::selectNagivationItems(uint regionId, uint solarSystemId, quint64 stationId)
    {
        const auto regions = mRegionList->count();
        for (auto i = 0; i < regions; ++i)
        {
            if (mRegionList->item(i)->data(Qt::UserRole).toUInt() == regionId)
            {
                mSolarSystemList->blockSignals(true);
                mRegionList->setCurrentRow(i);
                mSolarSystemList->blockSignals(false);
                break;
            }
        }

        const auto systems = mSolarSystemList->count();
        for (auto i = 0; i < systems; ++i)
        {
            if (mSolarSystemList->item(i)->data(Qt::UserRole).toUInt() == solarSystemId)
            {
                mStationList->blockSignals(true);
                mSolarSystemList->setCurrentRow(i);
                mStationList->blockSignals(false);
                break;
            }
        }

        const auto stations = mStationList->count();
        for (auto i = 0; i < stations; ++i)
        {
            if (mStationList->item(i)->data(Qt::UserRole).toULongLong() == stationId)
            {
                if (mStationList->currentRow() != i)
                {
                    mStationList->setCurrentRow(i);
                }
                else
                {
                    mExternalOrderSellModel.reset();
                    mExternalOrderBuyModel.reset();
                }

                break;
            }
        }
    }

    void MarketBrowserWidget::setTypeId(EveType::IdType typeId)
    {
        mExternalOrderSellModel.setTypeId(typeId);
        mExternalOrderBuyModel.setTypeId(typeId);
        mSellView->setTypeId(typeId);
        mBuyView->setTypeId(typeId);

        if (typeId != EveType::invalidId)
            mInfoLabel->setText(tr("%1 (%2m³)").arg(mDataProvider.getTypeName(typeId)).arg(locale().toString(mDataProvider.getTypeVolume(typeId), 'f', 2)));
        else
            mInfoLabel->setText(tr("select an item"));
    }

    void MarketBrowserWidget::setGrouping(ExternalOrderModel::Grouping grouping)
    {
        mExternalOrderSellModel.setGrouping(grouping);
        mExternalOrderBuyModel.setGrouping(grouping);
        mSellView->sortByPrice();
        mBuyView->sortByPrice();
    }
}
