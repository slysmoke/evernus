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
#include <QStringBuilder>
#include <QStandardPaths>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QListWidget>
#include <QSettings>
#include <QCheckBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QAction>
#include <QLabel>
#include <QFont>
#include <QFile>
#include <QDir>
#include <QUrl>

#include "MarketOrderRepository.h"
#include "CharacterRepository.h"
#include "CacheTimerProvider.h"
#include "WarningBarWidget.h"
#include "ButtonWithTimer.h"
#include "ImportSettings.h"
#include "TextUtils.h"

#include "CharacterWidget.h"

namespace Evernus
{
    const char * const CharacterWidget::skillFieldProperty = "field";
    const char * const CharacterWidget::downloadIdProperty = "downloadId";

    const QString CharacterWidget::defaultPortrait = ":/images/generic-portrait.jpg";

    CharacterWidget::CharacterWidget(const CharacterRepository &characterRepository,
                                     const MarketOrderRepository &marketOrderRepository,
                                     const CacheTimerProvider &cacheTimerProvider,
                                     QWidget *parent)
        : CharacterBoundWidget(std::bind(&CacheTimerProvider::getLocalCacheTimer, &cacheTimerProvider, std::placeholders::_1, TimerType::Character),
                               std::bind(&CacheTimerProvider::getLocalUpdateTimer, &cacheTimerProvider, std::placeholders::_1, TimerType::Character),
                               ImportSettings::maxCharacterAgeKey,
                               parent)
        , mCharacterRepository(characterRepository)
        , mMarketOrderRepository(marketOrderRepository)
        , mCacheTimerProvider(cacheTimerProvider)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolBarLayout);

        auto &importBtn = getAPIImportButton();
        toolBarLayout->addWidget(&importBtn);

        toolBarLayout->addStretch();

        auto &warningBar = getWarningBarWidget();
        mainLayout->addWidget(&warningBar);

        auto characterLayout = new QHBoxLayout{};
        mainLayout->addLayout(characterLayout);

        auto infoGroup = new QGroupBox{tr("Character info"), this};
        characterLayout->addWidget(infoGroup, 1);

        auto infoLayout = new QHBoxLayout{infoGroup};

        auto downloadPortraitAction = new QAction{tr("Download portrait"), this};
        connect(downloadPortraitAction, &QAction::triggered, this, &CharacterWidget::downloadPortrait);

        mPortrait = new QLabel{this};
        infoLayout->addWidget(mPortrait);
        mPortrait->setPixmap(defaultPortrait);
        mPortrait->setContextMenuPolicy(Qt::ActionsContextMenu);
        mPortrait->addAction(downloadPortraitAction);

        auto backgroundLayout = new QVBoxLayout{};
        infoLayout->addLayout(backgroundLayout, 1);

        mNameLabel = new QLabel{this};
        backgroundLayout->addWidget(mNameLabel);

        QFont font;
        font.setBold(true);
        font.setPointSize(20);

        mNameLabel->setFont(font);

        mBackgroundLabel = new QLabel{this};
        backgroundLayout->addWidget(mBackgroundLabel);

        mCorporationLabel = new QLabel{this};
        backgroundLayout->addWidget(mCorporationLabel);

        mISKLabel = new QLabel{this};
        backgroundLayout->addWidget(mISKLabel);

        mAlphaClone = new QCheckBox{tr("Alpha Clone"), this};
        backgroundLayout->addWidget(mAlphaClone);
        connect(mAlphaClone, &QCheckBox::stateChanged, this, &CharacterWidget::setAlphaClone);

        mUpdateTimersGroup = new QGroupBox{tr("Data age"), this};
        characterLayout->addWidget(mUpdateTimersGroup);
        mUpdateTimersGroup->setVisible(false);

        auto timersLayout = new QVBoxLayout{mUpdateTimersGroup};

        mUpdateTimersList = new QListWidget{this};
        timersLayout->addWidget(mUpdateTimersList, 0, Qt::AlignTop | Qt::AlignHCenter);
        mUpdateTimersList->setMaximumHeight(100);

        auto importAllBtn = new QPushButton{QIcon{":/images/arrow_refresh.png"}, tr("Import all"), this};
        timersLayout->addWidget(importAllBtn);
        connect(importAllBtn, &QPushButton::clicked, this, &CharacterWidget::importAll);

        auto underInfoLayout = new QHBoxLayout{};
        mainLayout->addLayout(underInfoLayout);

        auto addInfoGroup = new QGroupBox{tr("Orders"), this};
        underInfoLayout->addWidget(addInfoGroup);

        auto addInfoLayout = new QGridLayout{addInfoGroup};

        addInfoLayout->addWidget(new QLabel{tr("Buy orders:"), this}, 0, 0, Qt::AlignRight);

        mBuyOrderCountLabel = new QLabel{this};
        addInfoLayout->addWidget(mBuyOrderCountLabel, 0, 1, Qt::AlignLeft);

        addInfoLayout->addWidget(new QLabel{tr("Volume:"), this}, 0, 2, Qt::AlignRight);

        mBuyOrderVolumeLabel = new QLabel{this};
        addInfoLayout->addWidget(mBuyOrderVolumeLabel, 0, 3, Qt::AlignLeft);

        mBuyOrderValueLabel = new QLabel{this};
        addInfoLayout->addWidget(mBuyOrderValueLabel, 0, 4);

        addInfoLayout->addWidget(new QLabel{tr("Sell orders:"), this}, 1, 0, Qt::AlignRight);

        mSellOrderCountLabel = new QLabel{this};
        addInfoLayout->addWidget(mSellOrderCountLabel, 1, 1, Qt::AlignLeft);

        addInfoLayout->addWidget(new QLabel{tr("Volume:"), this}, 1, 2, Qt::AlignRight);

        mSellOrderVolumeLabel = new QLabel{this};
        addInfoLayout->addWidget(mSellOrderVolumeLabel, 1, 3, Qt::AlignLeft);

        mSellOrderValueLabel = new QLabel{this};
        addInfoLayout->addWidget(mSellOrderValueLabel, 1, 4);

        addInfoLayout->addWidget(new QLabel{tr("Total:"), this}, 2, 0, Qt::AlignRight);

        mTotalOrderCountLabel = new QLabel{this};
        addInfoLayout->addWidget(mTotalOrderCountLabel, 2, 1, Qt::AlignLeft);

        addInfoLayout->addWidget(new QLabel{tr("Volume:"), this}, 2, 2, Qt::AlignRight);

        mTotalOrderVolumeLabel = new QLabel{this};
        addInfoLayout->addWidget(mTotalOrderVolumeLabel, 2, 3, Qt::AlignLeft);

        mTotalOrderValueLabel = new QLabel{this};
        addInfoLayout->addWidget(mTotalOrderValueLabel, 2, 4);

        auto standingsGroup = new QGroupBox{tr("Station owner standings"), this};
        underInfoLayout->addWidget(standingsGroup);

        auto standingsLayout = new QFormLayout{standingsGroup};

        mCorpStandingEdit = new QDoubleSpinBox{this};
        standingsLayout->addRow(tr("Corporation standing:"), mCorpStandingEdit);
        mCorpStandingEdit->setMinimum(-10.0);
        mCorpStandingEdit->setMaximum(10.0);
        mCorpStandingEdit->setSingleStep(0.01);
        connect(mCorpStandingEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                this, &CharacterWidget::setCorpStanding);

        mFactionStandingEdit = new QDoubleSpinBox{this};
        standingsLayout->addRow(tr("Faction standing:"), mFactionStandingEdit);
        mFactionStandingEdit->setMinimum(-10.0);
        mFactionStandingEdit->setMaximum(10.0);
        mFactionStandingEdit->setSingleStep(0.01);
        connect(mFactionStandingEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                this, &CharacterWidget::setFactionStanding);

        auto brokersFeeLayout = new QHBoxLayout{};
        standingsLayout->addRow(tr("Custom broker's fee:"), brokersFeeLayout);

        mBrokersFeeBtn = new QCheckBox{tr("Enable"), this};
        brokersFeeLayout->addWidget(mBrokersFeeBtn);
        connect(mBrokersFeeBtn, &QCheckBox::stateChanged, this, &CharacterWidget::setBrokersFee);

        brokersFeeLayout->addWidget(new QLabel{tr("Buy:"), this}, 0, Qt::AlignRight);

        mBuyBrokersFeeEdit = new QDoubleSpinBox{this};
        brokersFeeLayout->addWidget(mBuyBrokersFeeEdit);
        mBuyBrokersFeeEdit->setMaximum(100.);
        mBuyBrokersFeeEdit->setSingleStep(0.01);
        mBuyBrokersFeeEdit->setEnabled(false);
        mBuyBrokersFeeEdit->setSuffix(locale().percent());
        connect(mBuyBrokersFeeEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                this, &CharacterWidget::setBrokersFee);
        connect(mBrokersFeeBtn, &QCheckBox::stateChanged, mBuyBrokersFeeEdit, &QDoubleSpinBox::setEnabled);

        brokersFeeLayout->addWidget(new QLabel{tr("Sell:"), this}, 0, Qt::AlignRight);

        mSellBrokersFeeEdit = new QDoubleSpinBox{this};
        brokersFeeLayout->addWidget(mSellBrokersFeeEdit);
        mSellBrokersFeeEdit->setMaximum(100.);
        mSellBrokersFeeEdit->setSingleStep(0.01);
        mSellBrokersFeeEdit->setEnabled(false);
        mSellBrokersFeeEdit->setSuffix(locale().percent());
        connect(mSellBrokersFeeEdit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                this, &CharacterWidget::setBrokersFee);
        connect(mBrokersFeeBtn, &QCheckBox::stateChanged, mSellBrokersFeeEdit, &QDoubleSpinBox::setEnabled);

        auto tradeSkillsGroup = new QGroupBox{tr("Trade skills"), this};
        mainLayout->addWidget(tradeSkillsGroup);

        auto skillsLayout = new QHBoxLayout{tradeSkillsGroup};

        auto orderAmountGroup = new QGroupBox{tr("Order amount skills"), this};
        skillsLayout->addWidget(orderAmountGroup);

        auto orderAmountLayout = new QFormLayout{orderAmountGroup};

        orderAmountLayout->addRow(tr("Trade:"), createSkillEdit(mTradeSkillEdit, QStringLiteral("trade_skill")));
        orderAmountLayout->addRow(tr("Retail:"), createSkillEdit(mRetailSkillEdit, QStringLiteral("retail_skill")));
        orderAmountLayout->addRow(tr("Wholesale:"), createSkillEdit(mWholesaleSkillEdit, QStringLiteral("wholesale_skill")));
        orderAmountLayout->addRow(tr("Tycoon:"), createSkillEdit(mTycoonSkillEdit, QStringLiteral("tycoon_skill")));

        auto tradeRangeGroup = new QGroupBox{tr("Trade range skills"), this};
        skillsLayout->addWidget(tradeRangeGroup);

        auto tradeRangeLayout = new QFormLayout{tradeRangeGroup};

        tradeRangeLayout->addRow(tr("Marketing:"), createSkillEdit(mMarketingSkillEdit, QStringLiteral("marketing_skill")));
        tradeRangeLayout->addRow(tr("Procurement:"), createSkillEdit(mProcurementSkillEdit, QStringLiteral("procurement_skill")));
        tradeRangeLayout->addRow(tr("Daytrading:"), createSkillEdit(mDaytradingSkillEdit, QStringLiteral("daytrading_skill")));
        tradeRangeLayout->addRow(tr("Visibility:"), createSkillEdit(mVisibilitySkillEdit, QStringLiteral("visibility_skill")));

        auto feeGroup = new QGroupBox{tr("Fee skills"), this};
        skillsLayout->addWidget(feeGroup);

        auto feeLayout = new QFormLayout{feeGroup};

        feeLayout->addRow(tr("Accounting:"), createSkillEdit(mAccountingSkillEdit, QStringLiteral("accounting_skill")));
        feeLayout->addRow(tr("Broker relations:"), createSkillEdit(mBrokerRelationsSkillEdit, QStringLiteral("broker_relations_skill")));
        feeLayout->addRow(tr("Advanced Broker Relations:"), createSkillEdit(mMarginTradingSkillEdit, QStringLiteral("margin_trading_skill")));

        auto contractingGroup = new QGroupBox{tr("Contracting skills"), this};
        skillsLayout->addWidget(contractingGroup);

        auto contractingLayout = new QFormLayout{contractingGroup};

        contractingLayout->addRow(tr("Contracting:"), createSkillEdit(mContractingSkillEdit, QStringLiteral("contracting_skill")));
        contractingLayout->addRow(tr("Corporation contracting:"),
                                  createSkillEdit(mCorporationContractingSkillEdit, QStringLiteral("corporation_contracting_skill")));

        const auto reprocessingSkillsGroup = new QGroupBox{tr("Reprocessing skills"), this};
        mainLayout->addWidget(reprocessingSkillsGroup);

        const auto reprocessingSkillsGroupLayout = new QVBoxLayout{reprocessingSkillsGroup};

        const auto reprocessingImplantBonusLayout = new QHBoxLayout{};
        reprocessingSkillsGroupLayout->addLayout(reprocessingImplantBonusLayout);

        reprocessingImplantBonusLayout->addWidget(new QLabel{tr("Reprocessing implant bonus:"), this});

        mReprocessingImplantBonusEdit = new QDoubleSpinBox{this};
        reprocessingImplantBonusLayout->addWidget(mReprocessingImplantBonusEdit);
        mReprocessingImplantBonusEdit->setRange(0., 100.);
        mReprocessingImplantBonusEdit->setSingleStep(0.01);
        mReprocessingImplantBonusEdit->setSuffix(locale().percent());
        connect(mReprocessingImplantBonusEdit, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &CharacterWidget::setReprocessingImplantBonus);

        reprocessingImplantBonusLayout->addStretch();

        const auto reprocessingSkillsLayout = new QHBoxLayout{};
        reprocessingSkillsGroupLayout->addLayout(reprocessingSkillsLayout);

        const auto reprocessingSkills1Layout = new QFormLayout{};
        reprocessingSkillsLayout->addLayout(reprocessingSkills1Layout);

        reprocessingSkills1Layout->addRow(tr("Arkonor processing:"),
                                          createSkillEdit(mArkonorProcessingSkillEdit, QStringLiteral("arkonor_processing")));
        reprocessingSkills1Layout->addRow(tr("Bistot processing:"),
                                          createSkillEdit(mBistotProcessingSkillEdit, QStringLiteral("bistot_processing")));
        reprocessingSkills1Layout->addRow(tr("Crokite processing:"),
                                          createSkillEdit(mCrokiteProcessingSkillEdit, QStringLiteral("crokite_processing")));
        reprocessingSkills1Layout->addRow(tr("Dark Ochre processing:"),
                                          createSkillEdit(mDarkOchreProcessingSkillEdit, QStringLiteral("dark_ochre_processing")));
        reprocessingSkills1Layout->addRow(tr("Gneiss processing:"),
                                          createSkillEdit(mGneissProcessingSkillEdit, QStringLiteral("gneiss_processing")));

        const auto reprocessingSkills2Layout = new QFormLayout{};
        reprocessingSkillsLayout->addLayout(reprocessingSkills2Layout);

        reprocessingSkills2Layout->addRow(tr("Hedbergite processing:"),
                                          createSkillEdit(mHedbergiteProcessingSkillEdit, QStringLiteral("hedbergite_processing")));
        reprocessingSkills2Layout->addRow(tr("Hemorphite processing:"),
                                          createSkillEdit(mHemorphiteProcessingSkillEdit, QStringLiteral("hemorphite_processing")));
        reprocessingSkills2Layout->addRow(tr("Ice processing:"),
                                          createSkillEdit(mIceProcessingSkillEdit, QStringLiteral("ice_processing")));
        reprocessingSkills2Layout->addRow(tr("Jaspet processing:"),
                                          createSkillEdit(mJaspetProcessingSkillEdit, QStringLiteral("jaspet_processing")));
        reprocessingSkills2Layout->addRow(tr("Kernite processing:"),
                                          createSkillEdit(mKerniteProcessingSkillEdit, QStringLiteral("kernite_processing")));

        const auto reprocessingSkills3Layout = new QFormLayout{};
        reprocessingSkillsLayout->addLayout(reprocessingSkills3Layout);

        reprocessingSkills3Layout->addRow(tr("Mercoxit processing:"),
                                          createSkillEdit(mMercoxitProcessingSkillEdit, QStringLiteral("mercoxit_processing")));
        reprocessingSkills3Layout->addRow(tr("Omber processing:"),
                                          createSkillEdit(mOmberProcessingSkillEdit, QStringLiteral("omber_processing")));
        reprocessingSkills3Layout->addRow(tr("Plagioclase processing:"),
                                          createSkillEdit(mPlagioclaseProcessingSkillEdit, QStringLiteral("plagioclase_processing")));
        reprocessingSkills3Layout->addRow(tr("Pyroxeres processing:"),
                                          createSkillEdit(mPyroxeresProcessingSkillEdit, QStringLiteral("pyroxeres_processing")));
        reprocessingSkills3Layout->addRow(tr("Reprocessing:"),
                                          createSkillEdit(mReprocessingSkillEdit, QStringLiteral("reprocessing")));

        const auto reprocessingSkills4Layout = new QFormLayout{};
        reprocessingSkillsLayout->addLayout(reprocessingSkills4Layout);

        reprocessingSkills4Layout->addRow(tr("Reprocessing efficiency:"),
                                          createSkillEdit(mReprocessingEfficiencySkillEdit, QStringLiteral("reprocessing_efficiency")));
        reprocessingSkills4Layout->addRow(tr("Scordite processing:"),
                                          createSkillEdit(mScorditeProcessingSkillEdit, QStringLiteral("scordite_processing")));
        reprocessingSkills4Layout->addRow(tr("Scrapmetal processing:"),
                                          createSkillEdit(mScrapmetalProcessingSkillEdit, QStringLiteral("scrapmetal_processing")));
        reprocessingSkills4Layout->addRow(tr("Spodumain processing:"),
                                          createSkillEdit(mSpodumainProcessingSkillEdit, QStringLiteral("spodumain_processing")));
        reprocessingSkills4Layout->addRow(tr("Veldspar processing:"),
                                          createSkillEdit(mVeldsparProcessingSkillEdit, QStringLiteral("veldspar_processing")));

        const auto industrySkillsGroup = new QGroupBox{tr("Industry skills"), this};
        mainLayout->addWidget(industrySkillsGroup);

        const auto industrySkillsGroupLayout = new QVBoxLayout{industrySkillsGroup};

        const auto manufacturingTimeImplantBonusLayout = new QHBoxLayout{};
        industrySkillsGroupLayout->addLayout(manufacturingTimeImplantBonusLayout);

        manufacturingTimeImplantBonusLayout->addWidget(new QLabel{tr("Manufacturing time implant bonus:"), this});

        mManufacturingTimeImplantBonusEdit = new QDoubleSpinBox{this};
        manufacturingTimeImplantBonusLayout->addWidget(mManufacturingTimeImplantBonusEdit);
        mManufacturingTimeImplantBonusEdit->setRange(0., 100.);
        mManufacturingTimeImplantBonusEdit->setSuffix(locale().percent());
        connect(mManufacturingTimeImplantBonusEdit, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &CharacterWidget::setManufacturingTimeImplantBonus);

        manufacturingTimeImplantBonusLayout->addStretch();

        const auto industrySkillsLayout = new QHBoxLayout{};
        industrySkillsGroupLayout->addLayout(industrySkillsLayout);

        const auto industrySkills1Layout = new QFormLayout{};
        industrySkillsLayout->addLayout(industrySkills1Layout);

        industrySkills1Layout->addRow(tr("Industry:"),
                                      createSkillEdit(mIndustrySkillEdit, QStringLiteral("industry")));
        industrySkills1Layout->addRow(tr("Advanced industry:"),
                                      createSkillEdit(mAdvancedIndustrySkillEdit, QStringLiteral("advanced_industry")));
        industrySkills1Layout->addRow(tr("Advanced small ship construction:"),
                                      createSkillEdit(mAdvancedSmallShipConstructionSkillEdit, QStringLiteral("advanced_small_ship_construction")));
        industrySkills1Layout->addRow(tr("Advanced medium ship construction:"),
                                      createSkillEdit(mAdvancedMediumShipConstructionSkillEdit, QStringLiteral("advanced_medium_ship_construction")));
        industrySkills1Layout->addRow(tr("Advanced large ship construction:"),
                                      createSkillEdit(mAdvancedLargeShipConstructionSkillEdit, QStringLiteral("advanced_large_ship_construction")));
        industrySkills1Layout->addRow(tr("Avanced industrial ship construction:"),
                                      createSkillEdit(mAvancedIndustrialShipConstructionSkillEdit, QStringLiteral("avanced_industrial_ship_construction")));

        const auto industrySkills2Layout = new QFormLayout{};
        industrySkillsLayout->addLayout(industrySkills2Layout);

        industrySkills2Layout->addRow(tr("Amarr starship engineering:"),
                                      createSkillEdit(mAmarrStarshipEngineeringSkillEdit, QStringLiteral("amarr_starship_engineering")));
        industrySkills2Layout->addRow(tr("Caldari starship engineering:"),
                                      createSkillEdit(mCaldariStarshipEngineeringSkillEdit, QStringLiteral("caldari_starship_engineering")));
        industrySkills2Layout->addRow(tr("Gallente starship engineering:"),
                                      createSkillEdit(mGallenteStarshipEngineeringSkillEdit, QStringLiteral("gallente_starship_engineering")));
        industrySkills2Layout->addRow(tr("Minmatar starship engineering:"),
                                      createSkillEdit(mMinmatarStarshipEngineeringSkillEdit, QStringLiteral("minmatar_starship_engineering")));
        industrySkills2Layout->addRow(tr("Electromagnetic physics:"),
                                      createSkillEdit(mElectromagneticPhysicsSkillEdit, QStringLiteral("electromagnetic_physics")));
        industrySkills2Layout->addRow(tr("Electronic engineering:"),
                                      createSkillEdit(mElectronicEngineeringSkillEdit, QStringLiteral("electronic_engineering")));

        const auto industrySkills3Layout = new QFormLayout{};
        industrySkillsLayout->addLayout(industrySkills3Layout);

        industrySkills3Layout->addRow(tr("Graviton physics:"),
                                      createSkillEdit(mGravitonPhysicsSkillEdit, QStringLiteral("graviton_physics")));
        industrySkills3Layout->addRow(tr("High energy physics:"),
                                      createSkillEdit(mHighEnergyPhysicsSkillEdit, QStringLiteral("high_energy_physics")));
        industrySkills3Layout->addRow(tr("Hydromagnetic physics:"),
                                      createSkillEdit(mHydromagneticPhysicsSkillEdit, QStringLiteral("hydromagnetic_physics")));
        industrySkills3Layout->addRow(tr("Laser physics:"),
                                      createSkillEdit(mLaserPhysicsSkillEdit, QStringLiteral("laser_physics")));
        industrySkills3Layout->addRow(tr("Mechanical engineering:"),
                                      createSkillEdit(mMechanicalEngineeringSkillEdit, QStringLiteral("mechanical_engineering")));
        industrySkills3Layout->addRow(tr("Molecular engineering:"),
                                      createSkillEdit(mMolecularEngineeringSkillEdit, QStringLiteral("molecular_engineering")));

        const auto industrySkills4Layout = new QFormLayout{};
        industrySkillsLayout->addLayout(industrySkills4Layout);

        industrySkills4Layout->addRow(tr("Nuclear physics:"),
                                      createSkillEdit(mNuclearPhysicsSkillEdit, QStringLiteral("nuclear_physics")));
        industrySkills4Layout->addRow(tr("Plasma physics:"),
                                      createSkillEdit(mPlasmaPhysicsSkillEdit, QStringLiteral("plasma_physics")));
        industrySkills4Layout->addRow(tr("Quantum physics:"),
                                      createSkillEdit(mQuantumPhysicsSkillEdit, QStringLiteral("quantum_physics")));
        industrySkills4Layout->addRow(tr("Rocket science:"),
                                      createSkillEdit(mRocketScienceSkillEdit, QStringLiteral("rocket_science")));

        mainLayout->addStretch();

        connect(&mUpdateTimer, &QTimer::timeout, this, &CharacterWidget::updateTimerList);

        mUpdateTimer.start(1000 * 60);
    }

    void CharacterWidget::updateData()
    {
        refreshImportTimer();
        handleNewCharacter(getCharacterId());
    }

    void CharacterWidget::updateTimerList()
    {
        const auto id = getCharacterId();
        if (id == Character::invalidId)
        {
            mUpdateTimersGroup->hide();
            return;
        }

        const auto curLocale = locale();
        auto show = false;

        QSettings settings;
        const auto importContracts = settings.value(ImportSettings::importContractsKey, ImportSettings::importContractsDefault).toBool();

        mUpdateTimersList->clear();

        const auto checker = [&, this](TimerType timer, const QString &settingsKey, const QString &text) {
            const auto curTime
                = QDateTime::currentDateTime().addSecs(-60 * settings.value(settingsKey, Evernus::ImportSettings::importTimerDefault).toInt());
            const auto dt = mCacheTimerProvider.getLocalUpdateTimer(id, timer);

            if (dt < curTime)
            {
                show = true;
                new QListWidgetItem{QIcon{":/images/error.png"},
                                    text.arg((dt.isValid()) ? (TextUtils::dateTimeToString(dt, curLocale)) : (tr("never imported"))),
                                    mUpdateTimersList};
            }
        };

        checker(TimerType::Character, ImportSettings::maxCharacterAgeKey, tr("Character sheet: %1"));
        checker(TimerType::AssetList, ImportSettings::maxAssetListAgeKey, tr("Asset list: %1"));
        checker(TimerType::MarketOrders, ImportSettings::maxMarketOrdersAgeKey, tr("Market orders: %1"));
        checker(TimerType::WalletJournal, ImportSettings::maxWalletAgeKey, tr("Wallet journal: %1"));
        checker(TimerType::WalletTransactions, ImportSettings::maxWalletAgeKey, tr("Wallet transactions: %1"));
        checker(TimerType::CorpAssetList, ImportSettings::maxAssetListAgeKey, tr("Corp. asset list: %1"));
        checker(TimerType::CorpMarketOrders, ImportSettings::maxMarketOrdersAgeKey, tr("Corp. market orders: %1"));
        checker(TimerType::CorpWalletJournal, ImportSettings::maxWalletAgeKey, tr("Corp. wallet journal: %1"));
        checker(TimerType::CorpWalletTransactions, ImportSettings::maxWalletAgeKey, tr("Corp. wallet transactions: %1"));

        if (importContracts)
        {
            checker(TimerType::CorpContracts, ImportSettings::maxWalletAgeKey, tr("Corp. contracts: %1"));
            checker(TimerType::Contracts, ImportSettings::maxWalletAgeKey, tr("Contracts: %1"));
        }

        mUpdateTimersGroup->setVisible(show);
    }

    void CharacterWidget::updateMarketData()
    {
        try
        {
            const auto character = mCharacterRepository.find(getCharacterId());
            updateCharacterMarketData(character->getOrderAmountSkills());
        }
        catch (const CharacterRepository::NotFoundException &)
        {
        }

        updateTimerList();
    }

    void CharacterWidget::setCorpStanding(double value)
    {
        updateStanding("corp_standing", value);
    }

    void CharacterWidget::setFactionStanding(double value)
    {
        updateStanding("faction_standing", value);
    }

    void CharacterWidget::setBrokersFee()
    {
        const auto id = getCharacterId();
        Q_ASSERT(id != Character::invalidId);

        const auto enabled = mBrokersFeeBtn->isChecked();
        const auto buy = (enabled) ? (mBuyBrokersFeeEdit->value() / 100.) : (std::optional<double>{});
        const auto sell = (enabled) ? (mSellBrokersFeeEdit->value() / 100.) : (std::optional<double>{});

        mCharacterRepository.updateBrokersFee(id, buy, sell);

        emit characterDataChanged();
    }

    void CharacterWidget::setReprocessingImplantBonus(double value)
    {
        const auto id = getCharacterId();
        Q_ASSERT(id != Character::invalidId);

        mCharacterRepository.updateReprocessingImplantBonus(id, value);

        emit characterDataChanged();
    }

    void CharacterWidget::setManufacturingTimeImplantBonus(double value)
    {
        const auto id = getCharacterId();
        Q_ASSERT(id != Character::invalidId);

        mCharacterRepository.updateManufacturingTimeImplantBonus(id, value);

        emit characterDataChanged();
    }

    void CharacterWidget::setAlphaClone()
    {
        const auto id = getCharacterId();
        Q_ASSERT(id != Character::invalidId);

        mCharacterRepository.updateAlphaClone(id, mAlphaClone->isChecked());

        emit characterDataChanged();
    }

    void CharacterWidget::setSkillLevel(int level)
    {
        const auto id = getCharacterId();
        Q_ASSERT(id != Character::invalidId);

        const auto fieldName = sender()->property(skillFieldProperty).toString();
        mCharacterRepository.updateSkill(id, fieldName, level);

        emit characterDataChanged();
    }

    void CharacterWidget::handleNewCharacter(Character::IdType id)
    {
        qDebug() << "Switching character to" << id;

        mCorpStandingEdit->blockSignals(true);
        mFactionStandingEdit->blockSignals(true);
        mBrokersFeeBtn->blockSignals(true);
        mBuyBrokersFeeEdit->blockSignals(true);
        mSellBrokersFeeEdit->blockSignals(true);
        mTradeSkillEdit->blockSignals(true);
        mRetailSkillEdit->blockSignals(true);
        mWholesaleSkillEdit->blockSignals(true);
        mTycoonSkillEdit->blockSignals(true);
        mMarketingSkillEdit->blockSignals(true);
        mProcurementSkillEdit->blockSignals(true);
        mDaytradingSkillEdit->blockSignals(true);
        mVisibilitySkillEdit->blockSignals(true);
        mAccountingSkillEdit->blockSignals(true);
        mBrokerRelationsSkillEdit->blockSignals(true);
        mMarginTradingSkillEdit->blockSignals(true);
        mContractingSkillEdit->blockSignals(true);
        mCorporationContractingSkillEdit->blockSignals(true);
        mAlphaClone->blockSignals(true);

        if (id == Character::invalidId)
        {
            mNameLabel->setText(QString{});
            mBackgroundLabel->setText(QString{});
            mCorporationLabel->setText(QString{});
            mISKLabel->setText(QString{});
            mAlphaClone->setChecked(false);

            mBuyOrderCountLabel->clear();
            mSellOrderCountLabel->clear();
            mTotalOrderCountLabel->clear();

            mBuyOrderValueLabel->clear();
            mSellOrderValueLabel->clear();
            mTotalOrderValueLabel->clear();

            mBuyOrderVolumeLabel->clear();
            mSellOrderVolumeLabel->clear();
            mTotalOrderVolumeLabel->clear();

            mCorpStandingEdit->setValue(0.);
            mFactionStandingEdit->setValue(0.);

            mBrokersFeeBtn->setChecked(false);
            mBuyBrokersFeeEdit->setValue(0.);
            mBuyBrokersFeeEdit->setEnabled(false);
            mSellBrokersFeeEdit->setValue(0.);
            mSellBrokersFeeEdit->setEnabled(false);

            mTradeSkillEdit->setValue(0);
            mRetailSkillEdit->setValue(0);
            mWholesaleSkillEdit->setValue(0);
            mTycoonSkillEdit->setValue(0);
            mMarketingSkillEdit->setValue(0);
            mProcurementSkillEdit->setValue(0);
            mDaytradingSkillEdit->setValue(0);
            mVisibilitySkillEdit->setValue(0);
            mAccountingSkillEdit->setValue(0);
            mBrokerRelationsSkillEdit->setValue(0);
            mMarginTradingSkillEdit->setValue(0);
            mContractingSkillEdit->setValue(0);
            mCorporationContractingSkillEdit->setValue(0);

            mReprocessingImplantBonusEdit->setValue(0.);

            mArkonorProcessingSkillEdit->setValue(0);
            mBistotProcessingSkillEdit->setValue(0);
            mCrokiteProcessingSkillEdit->setValue(0);
            mDarkOchreProcessingSkillEdit->setValue(0);
            mGneissProcessingSkillEdit->setValue(0);
            mHedbergiteProcessingSkillEdit->setValue(0);
            mHemorphiteProcessingSkillEdit->setValue(0);
            mIceProcessingSkillEdit->setValue(0);
            mJaspetProcessingSkillEdit->setValue(0);
            mKerniteProcessingSkillEdit->setValue(0);
            mMercoxitProcessingSkillEdit->setValue(0);
            mOmberProcessingSkillEdit->setValue(0);
            mPlagioclaseProcessingSkillEdit->setValue(0);
            mPyroxeresProcessingSkillEdit->setValue(0);
            mReprocessingSkillEdit->setValue(0);
            mReprocessingEfficiencySkillEdit->setValue(0);
            mScorditeProcessingSkillEdit->setValue(0);
            mScrapmetalProcessingSkillEdit->setValue(0);
            mSpodumainProcessingSkillEdit->setValue(0);
            mVeldsparProcessingSkillEdit->setValue(0);

            mManufacturingTimeImplantBonusEdit->setValue(0.);

            mIndustrySkillEdit->setValue(0);
            mAdvancedIndustrySkillEdit->setValue(0);
            mAdvancedSmallShipConstructionSkillEdit->setValue(0);
            mAdvancedMediumShipConstructionSkillEdit->setValue(0);
            mAdvancedLargeShipConstructionSkillEdit->setValue(0);
            mAvancedIndustrialShipConstructionSkillEdit->setValue(0);
            mAmarrStarshipEngineeringSkillEdit->setValue(0);
            mCaldariStarshipEngineeringSkillEdit->setValue(0);
            mGallenteStarshipEngineeringSkillEdit->setValue(0);
            mMinmatarStarshipEngineeringSkillEdit->setValue(0);
            mElectromagneticPhysicsSkillEdit->setValue(0);
            mElectronicEngineeringSkillEdit->setValue(0);
            mGravitonPhysicsSkillEdit->setValue(0);
            mHighEnergyPhysicsSkillEdit->setValue(0);
            mHydromagneticPhysicsSkillEdit->setValue(0);
            mLaserPhysicsSkillEdit->setValue(0);
            mMechanicalEngineeringSkillEdit->setValue(0);
            mMolecularEngineeringSkillEdit->setValue(0);
            mNuclearPhysicsSkillEdit->setValue(0);
            mPlasmaPhysicsSkillEdit->setValue(0);
            mQuantumPhysicsSkillEdit->setValue(0);
            mRocketScienceSkillEdit->setValue(0);

            mPortrait->setPixmap(defaultPortrait);
        }
        else
        {
            try
            {
                const auto character = mCharacterRepository.find(id);

                const auto orderAmountSkills = character->getOrderAmountSkills();
                const auto tradeRangeSkills = character->getTradeRangeSkills();
                const auto feeSkills = character->getFeeSkills();
                const auto contractSkills = character->getContractSkills();
                const auto reprocessingSkills = character->getReprocessingSkills();
                const auto industrySkills = character->getIndustrySkills();

                mNameLabel->setText(character->getName());
                mBackgroundLabel->setText(QStringLiteral("%1 %2, %3, %4")
                    .arg(character->getGender())
                    .arg(character->getRace())
                    .arg(character->getBloodline())
                    .arg(character->getAncestry()));
                mCorporationLabel->setText(character->getCorporationName());
                mISKLabel->setText(character->getISKPresentation());
                mAlphaClone->setChecked(character->isAlphaClone());

                updateCharacterMarketData(orderAmountSkills);

                mCorpStandingEdit->setValue(character->getCorpStanding());
                mFactionStandingEdit->setValue(character->getFactionStanding());

                mBrokersFeeBtn->setChecked(false);

                auto setBrokersFeeEdit = [=](auto edit, const auto &brokersFee) {
                    if (brokersFee)
                    {
                        mBrokersFeeBtn->setChecked(true);
                        edit->setValue(*brokersFee * 100.);
                        edit->setEnabled(true);
                    }
                    else
                    {
                        edit->setValue(0.);
                        edit->setEnabled(false);
                    }
                };

                setBrokersFeeEdit(mBuyBrokersFeeEdit, character->getBuyBrokersFee());
                setBrokersFeeEdit(mSellBrokersFeeEdit, character->getSellBrokersFee());

                mTradeSkillEdit->setValue(orderAmountSkills.mTrade);
                mRetailSkillEdit->setValue(orderAmountSkills.mRetail);
                mWholesaleSkillEdit->setValue(orderAmountSkills.mWholesale);
                mTycoonSkillEdit->setValue(orderAmountSkills.mTycoon);
                mMarketingSkillEdit->setValue(tradeRangeSkills.mMarketing);
                mProcurementSkillEdit->setValue(tradeRangeSkills.mProcurement);
                mDaytradingSkillEdit->setValue(tradeRangeSkills.mDaytrading);
                mVisibilitySkillEdit->setValue(tradeRangeSkills.mVisibility);
                mAccountingSkillEdit->setValue(feeSkills.mAccounting);
                mBrokerRelationsSkillEdit->setValue(feeSkills.mBrokerRelations);
                mMarginTradingSkillEdit->setValue(feeSkills.mMarginTrading);
                mContractingSkillEdit->setValue(contractSkills.mContracting);
                mCorporationContractingSkillEdit->setValue(contractSkills.mCorporationContracting);

                mReprocessingImplantBonusEdit->setValue(character->getReprocessingImplantBonus());

                mArkonorProcessingSkillEdit->setValue(reprocessingSkills.mArkonorProcessing);
                mBistotProcessingSkillEdit->setValue(reprocessingSkills.mBistotProcessing);
                mCrokiteProcessingSkillEdit->setValue(reprocessingSkills.mCrokiteProcessing);
                mDarkOchreProcessingSkillEdit->setValue(reprocessingSkills.mDarkOchreProcessing);
                mGneissProcessingSkillEdit->setValue(reprocessingSkills.mGneissProcessing);
                mHedbergiteProcessingSkillEdit->setValue(reprocessingSkills.mHedbergiteProcessing);
                mHemorphiteProcessingSkillEdit->setValue(reprocessingSkills.mHemorphiteProcessing);
                mIceProcessingSkillEdit->setValue(reprocessingSkills.mIceProcessing);
                mJaspetProcessingSkillEdit->setValue(reprocessingSkills.mJaspetProcessing);
                mKerniteProcessingSkillEdit->setValue(reprocessingSkills.mKerniteProcessing);
                mMercoxitProcessingSkillEdit->setValue(reprocessingSkills.mMercoxitProcessing);
                mOmberProcessingSkillEdit->setValue(reprocessingSkills.mOmberProcessing);
                mPlagioclaseProcessingSkillEdit->setValue(reprocessingSkills.mPlagioclaseProcessing);
                mPyroxeresProcessingSkillEdit->setValue(reprocessingSkills.mPyroxeresProcessing);
                mReprocessingSkillEdit->setValue(reprocessingSkills.mReprocessing);
                mReprocessingEfficiencySkillEdit->setValue(reprocessingSkills.mReprocessingEfficiency);
                mScorditeProcessingSkillEdit->setValue(reprocessingSkills.mScorditeProcessing);
                mScrapmetalProcessingSkillEdit->setValue(reprocessingSkills.mScrapmetalProcessing);
                mSpodumainProcessingSkillEdit->setValue(reprocessingSkills.mSpodumainProcessing);
                mVeldsparProcessingSkillEdit->setValue(reprocessingSkills.mVeldsparProcessing);

                mManufacturingTimeImplantBonusEdit->setValue(character->getManufacturingTimeImplantBonus());

                mIndustrySkillEdit->setValue(industrySkills.mIndustry);
                mAdvancedIndustrySkillEdit->setValue(industrySkills.mAdvancedIndustry);
                mAdvancedSmallShipConstructionSkillEdit->setValue(industrySkills.mAdvancedSmallShipConstruction);
                mAdvancedMediumShipConstructionSkillEdit->setValue(industrySkills.mAdvancedMediumShipConstruction);
                mAdvancedLargeShipConstructionSkillEdit->setValue(industrySkills.mAdvancedLargeShipConstruction);
                mAvancedIndustrialShipConstructionSkillEdit->setValue(industrySkills.mAvancedIndustrialShipConstruction);
                mAmarrStarshipEngineeringSkillEdit->setValue(industrySkills.mAmarrStarshipEngineering);
                mCaldariStarshipEngineeringSkillEdit->setValue(industrySkills.mCaldariStarshipEngineering);
                mGallenteStarshipEngineeringSkillEdit->setValue(industrySkills.mGallenteStarshipEngineering);
                mMinmatarStarshipEngineeringSkillEdit->setValue(industrySkills.mMinmatarStarshipEngineering);
                mElectromagneticPhysicsSkillEdit->setValue(industrySkills.mElectromagneticPhysics);
                mElectronicEngineeringSkillEdit->setValue(industrySkills.mElectronicEngineering);
                mGravitonPhysicsSkillEdit->setValue(industrySkills.mGravitonPhysics);
                mHighEnergyPhysicsSkillEdit->setValue(industrySkills.mHighEnergyPhysics);
                mHydromagneticPhysicsSkillEdit->setValue(industrySkills.mHydromagneticPhysics);
                mLaserPhysicsSkillEdit->setValue(industrySkills.mLaserPhysics);
                mMechanicalEngineeringSkillEdit->setValue(industrySkills.mMechanicalEngineering);
                mMolecularEngineeringSkillEdit->setValue(industrySkills.mMolecularEngineering);
                mNuclearPhysicsSkillEdit->setValue(industrySkills.mNuclearPhysics);
                mPlasmaPhysicsSkillEdit->setValue(industrySkills.mPlasmaPhysics);
                mQuantumPhysicsSkillEdit->setValue(industrySkills.mQuantumPhysics);
                mRocketScienceSkillEdit->setValue(industrySkills.mRocketScience);

                QSettings settings;
                if (settings.value(ImportSettings::importPortraitKey, ImportSettings::importPortraitDefault).toBool())
                {
                    const auto portrait = getPortraitPixmap(id);

                    if (!portrait.isNull() && portrait.devicePixelRatio() >= devicePixelRatio())
                        mPortrait->setPixmap(portrait);
                    else
                        downloadPortrait();
                }
                else
                {
                    mPortrait->setPixmap(defaultPortrait);
                }
            }
            catch (const Repository<Character>::NotFoundException &)
            {
                QMessageBox::warning(this, tr("Character error"), tr("Character not found in DB. Refresh characters."));
                handleNewCharacter(Character::invalidId);
                return;
            }
        }

        updateTimerList();

        mCorpStandingEdit->blockSignals(false);
        mFactionStandingEdit->blockSignals(false);
        mBrokersFeeBtn->blockSignals(false);
        mBuyBrokersFeeEdit->blockSignals(false);
        mSellBrokersFeeEdit->blockSignals(false);
        mTradeSkillEdit->blockSignals(false);
        mRetailSkillEdit->blockSignals(false);
        mWholesaleSkillEdit->blockSignals(false);
        mTycoonSkillEdit->blockSignals(false);
        mMarketingSkillEdit->blockSignals(false);
        mProcurementSkillEdit->blockSignals(false);
        mDaytradingSkillEdit->blockSignals(false);
        mVisibilitySkillEdit->blockSignals(false);
        mAccountingSkillEdit->blockSignals(false);
        mBrokerRelationsSkillEdit->blockSignals(false);
        mMarginTradingSkillEdit->blockSignals(false);
        mContractingSkillEdit->blockSignals(false);
        mCorporationContractingSkillEdit->blockSignals(false);
        mAlphaClone->blockSignals(false);
    }

    void CharacterWidget::downloadPortrait()
    {
        const auto id = getCharacterId();
        if (mPortraitDownloads.find(id) == std::end(mPortraitDownloads))
        {
            try
            {
                const auto savePath = devicePixelRatio() >= 2 ? getPortraitHighResPath(id) : getPortraitPath(id);
                const auto resolution = devicePixelRatio() >= 2 ? 256 : 128;
                auto download = new FileDownload{QUrl{QStringLiteral("https://image.eveonline.com/Character/%1_%2.jpg").arg(id).arg(resolution)},
                                                 savePath,
                                                 this};
                download->setProperty(downloadIdProperty, id);
                connect(download, &FileDownload::finished, this, &CharacterWidget::downloadFinished);

                mPortraitDownloads.emplace(id, download);
            }
            catch (const std::exception &e)
            {
                qWarning() << e.what();
            }
        }
    }

    void CharacterWidget::downloadFinished()
    {
        const auto id = sender()->property(downloadIdProperty).value<Character::IdType>();
        const auto it = mPortraitDownloads.find(id);
        Q_ASSERT(it != std::end(mPortraitDownloads));

        const auto portrait = getPortraitPixmap(id);
        if (!portrait.isNull() && getCharacterId() == id)
            mPortrait->setPixmap(portrait);

        mPortraitDownloads.erase(it);
    }

    void CharacterWidget::updateStanding(const QString &type, double value) const
    {
        const auto id = getCharacterId();
        Q_ASSERT(id != Character::invalidId);

        mCharacterRepository.updateStanding(id, type, value);

        emit characterDataChanged();
    }

    void CharacterWidget::updateCharacterMarketData(const CharacterData::OrderAmountSkills &orderAmountSkills)
    {
        const auto aggrData = mMarketOrderRepository.getAggregatedData(getCharacterId());
        const auto curLocale = locale();

        const auto maxBuyOrders = orderAmountSkills.mTrade * 4 +
                                  orderAmountSkills.mRetail * 8 +
                                  orderAmountSkills.mWholesale * 16 +
                                  orderAmountSkills.mTycoon * 32 + 5;

        mBuyOrderCountLabel->setText(curLocale.toString(aggrData.mBuyData.mCount));
        mSellOrderCountLabel->setText(curLocale.toString(aggrData.mSellData.mCount));
        mTotalOrderCountLabel->setText(tr("<strong>%1 of %2</strong>")
            .arg(curLocale.toString(aggrData.mBuyData.mCount + aggrData.mSellData.mCount))
            .arg(curLocale.toString(maxBuyOrders)));

        mBuyOrderVolumeLabel->setText(curLocale.toString(aggrData.mBuyData.mVolume));
        mSellOrderVolumeLabel->setText(curLocale.toString(aggrData.mSellData.mVolume));
        mTotalOrderVolumeLabel->setText(curLocale.toString(aggrData.mBuyData.mVolume + aggrData.mSellData.mVolume));

        mBuyOrderValueLabel->setText(TextUtils::currencyToString(aggrData.mBuyData.mPriceSum, curLocale));
        mSellOrderValueLabel->setText(TextUtils::currencyToString(aggrData.mSellData.mPriceSum, curLocale));
        mTotalOrderValueLabel->setText(tr("<strong>%1</strong>")
            .arg(TextUtils::currencyToString(aggrData.mBuyData.mPriceSum + aggrData.mSellData.mPriceSum, curLocale)));
    }

    QSpinBox *CharacterWidget::createSkillEdit(QSpinBox *&target, const QString &skillField)
    {
        target = new QSpinBox{this};
        target->setMaximum(5);
        target->setProperty(skillFieldProperty, skillField);
        connect(target, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &CharacterWidget::setSkillLevel);

        return target;
    }

    QPixmap CharacterWidget::getPortraitPixmap(Character::IdType id, int size)
    {
        return QIcon{getPortraitPath(id)}.pixmap(size);
    }

    QString CharacterWidget::getPortraitPath(Character::IdType id)
    {
        return getPortraitLowResPath(id);
    }

    QString CharacterWidget::getPortraitLowResPath(Character::IdType id)
    {
        return getPortraitPathTemplate(id).arg("");
    }

    QString CharacterWidget::getPortraitHighResPath(Character::IdType id)
    {
        return getPortraitPathTemplate(id).arg("@2x");
    }

    QString CharacterWidget::getPortraitPathTemplate(Character::IdType id)
    {
        return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) %
            "/portrait/" %
            QString::number(id) %
            "%1.jpg";
    }
}
