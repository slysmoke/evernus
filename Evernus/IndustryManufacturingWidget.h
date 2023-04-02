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

#include <vector>

#include <QQuickWindow>
#include <QVariant>
#include <QWidget>
#include <QString>

#include "IndustryManufacturingSetupController.h"
#include "IndustryManufacturingSetupModel.h"
#include "IndustryManufacturingSetup.h"
#include "MarketOrderDataFetcher.h"
#include "TaskConstants.h"
#include "ESIManager.h"
#include "Character.h"

class QQuickWidget;
class QProgressBar;
class QPushButton;
class QByteArray;
class QComboBox;
class QLabel;

namespace Evernus
{
    class IndustryManufacturingSetupRepository;
    class DontSaveImportedOrdersCheckBox;
    class RegionStationPresetRepository;
    class TradeableTypesTreeView;
    class MarketGroupRepository;
    class CharacterRepository;
    class EveTypeRepository;
    class PriceTypeComboBox;
    class ItemCostProvider;
    class EveDataProvider;
    class RegionComboBox;
    class AssetProvider;
    class ExternalOrder;
    class TaskManager;

    class IndustryManufacturingWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        IndustryManufacturingWidget(const EveDataProvider &dataProvider,
                                    const RegionStationPresetRepository &regionStationPresetRepository,
                                    const EveTypeRepository &typeRepo,
                                    const MarketGroupRepository &groupRepo,
                                    const CharacterRepository &characterRepo,
                                    const IndustryManufacturingSetupRepository &setupRepo,
                                    ESIInterfaceManager &interfaceManager,
                                    TaskManager &taskManager,
                                    const AssetProvider &assetProvider,
                                    const ItemCostProvider &costProvider,
                                    QWidget *parent = nullptr);
        IndustryManufacturingWidget(const IndustryManufacturingWidget &) = default;
        IndustryManufacturingWidget(IndustryManufacturingWidget &&) = default;
        virtual ~IndustryManufacturingWidget() = default;

        void refreshAssets();

        IndustryManufacturingWidget &operator =(const IndustryManufacturingWidget &) = default;
        IndustryManufacturingWidget &operator =(IndustryManufacturingWidget &&) = default;

    signals:
        void updateExternalOrders(const std::vector<ExternalOrder> &orders);
        void setupRefreshChanged(bool started);
        void showInEve(EveType::IdType id);
        void showExternalOrders(EveType::IdType id);

    public slots:
        void handleNewPreferences();
        void setCharacter(Character::IdType id);

    private slots:
        void refreshTypes();
        void importData();

        void showSceneGraphError(QQuickWindow::SceneGraphError error, const QString &message);

        void updateOrderTask(const QString &text);
        void endOrderTask(const MarketOrderDataFetcher::OrderResultType &orders, const QString &error);

        void setPriceTypes();

        void loadSetup();
        void saveSetup();

        void showBoM();
        void updateSummary();
        void importCharacterBlueprints();
        void importCorporationBlueprints();

    private:
        const EveDataProvider &mDataProvider;
        TaskManager &mTaskManager;
        const CharacterRepository &mCharacterRepo;
        const IndustryManufacturingSetupRepository &mSetupRepo;

        RegionComboBox *mSrcRegionCombo = nullptr;
        RegionComboBox *mDstRegionCombo = nullptr;

        PriceTypeComboBox *mSrcPriceTypeCombo = nullptr;
        PriceTypeComboBox *mDstPriceTypeCombo = nullptr;

        QComboBox *mFacilityTypeCombo = nullptr;
        QComboBox *mFacilitySizeCombo = nullptr;
        QComboBox *mSecurityStatusCombo = nullptr;
        QComboBox *mMaterialRigCombo = nullptr;
        QComboBox *mTimeRigCombo = nullptr;

        DontSaveImportedOrdersCheckBox *mDontSaveBtn = nullptr;

        QProgressBar *mViewResetProgress = nullptr;

        QQuickWidget *mManufacturingView = nullptr;

        TradeableTypesTreeView *mTypeView = nullptr;

        QPushButton *mImportCorpBlueprintsBtn = nullptr;

        QLabel *mTotalCostLabel = nullptr;
        QLabel *mTotalProfitLabel = nullptr;
        QLabel *mMinTimeLabel = nullptr;
        QLabel *mISKPerHLabel = nullptr;
        QLabel *mSystemCostIndexLabel = nullptr;

        quint64 mSrcStation = 0;
        quint64 mDstStation = 0;

        Character::IdType mCharacterId = Character::invalidId;
        quint64 mCorpId = 0;

        IndustryManufacturingSetup mSetup{mDataProvider};
        IndustryManufacturingSetupModel mSetupModel;
        IndustryManufacturingSetupController mSetupController{mSetupModel};

        MarketOrderDataFetcher mDataFetcher;
        ESIManager mESIManager;

        uint mOrderSubtask = TaskConstants::invalidTask;
        uint mMarketPricesSubtask = TaskConstants::invalidTask;
        uint mCostIndicesSubtask = TaskConstants::invalidTask;
        uint mBlueprintImportSubtask = TaskConstants::invalidTask;

        QString mLastLoadedSetup;

        void changeStation(quint64 &destination, const QVariantList &path, const QString &settingName);

        void toggleFacilityCombos();
        bool shouldEnableFacilityCombos() const;

        void setQmlSettings();

        void connectToSetupRefresh(QWidget &widget);

        void toggleCorpImportButton();

        template<class Fetcher>
        void importBlueprints(Fetcher fetcher);
    };
}
