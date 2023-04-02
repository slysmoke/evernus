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

#include <unordered_map>

#include "CharacterBoundWidget.h"
#include "FileDownload.h"

class QDoubleSpinBox;
class QListWidget;
class QGroupBox;
class QCheckBox;
class QSpinBox;
class QLabel;

namespace Evernus
{
    class MarketOrderRepository;
    class CharacterRepository;
    class CacheTimerProvider;

    class CharacterWidget
        : public CharacterBoundWidget
    {
        Q_OBJECT

    public:
        CharacterWidget(const CharacterRepository &characterRepository,
                        const MarketOrderRepository &marketOrderRepository,
                        const CacheTimerProvider &cacheTimerProvider,
                        QWidget *parent = nullptr);
        virtual ~CharacterWidget() = default;

    signals:
        void importAll();
        void characterDataChanged() const;

    public slots:
        void updateData();
        void updateTimerList();
        void updateMarketData();

    private slots:
        void setCorpStanding(double value);
        void setFactionStanding(double value);
        void setBrokersFee();
        void setReprocessingImplantBonus(double value);
        void setManufacturingTimeImplantBonus(double value);
        void setAlphaClone();

        void setSkillLevel(int level);

        void downloadPortrait();
        void downloadFinished();

    private:
        static const char * const skillFieldProperty;
        static const char * const downloadIdProperty;
        static const QString defaultPortrait;

        const CharacterRepository &mCharacterRepository;
        const MarketOrderRepository &mMarketOrderRepository;
        const CacheTimerProvider &mCacheTimerProvider;

        QLabel *mPortrait = nullptr;

        QLabel *mNameLabel = nullptr;
        QLabel *mBackgroundLabel = nullptr;
        QLabel *mCorporationLabel = nullptr;
        QLabel *mISKLabel = nullptr;
        QCheckBox *mAlphaClone = nullptr;
        QLabel *mBuyOrderCountLabel = nullptr;
        QLabel *mSellOrderCountLabel = nullptr;
        QLabel *mTotalOrderCountLabel = nullptr;
        QLabel *mBuyOrderValueLabel = nullptr;
        QLabel *mSellOrderValueLabel = nullptr;
        QLabel *mTotalOrderValueLabel = nullptr;
        QLabel *mBuyOrderVolumeLabel = nullptr;
        QLabel *mSellOrderVolumeLabel = nullptr;
        QLabel *mTotalOrderVolumeLabel = nullptr;

        QDoubleSpinBox *mCorpStandingEdit = nullptr;
        QDoubleSpinBox *mFactionStandingEdit = nullptr;

        QCheckBox *mBrokersFeeBtn = nullptr;
        QDoubleSpinBox *mBuyBrokersFeeEdit = nullptr;
        QDoubleSpinBox *mSellBrokersFeeEdit = nullptr;

        QSpinBox *mTradeSkillEdit = nullptr;
        QSpinBox *mRetailSkillEdit = nullptr;
        QSpinBox *mWholesaleSkillEdit = nullptr;
        QSpinBox *mTycoonSkillEdit = nullptr;
        QSpinBox *mMarketingSkillEdit = nullptr;
        QSpinBox *mProcurementSkillEdit = nullptr;
        QSpinBox *mDaytradingSkillEdit = nullptr;
        QSpinBox *mVisibilitySkillEdit = nullptr;
        QSpinBox *mAccountingSkillEdit = nullptr;
        QSpinBox *mBrokerRelationsSkillEdit = nullptr;
        QSpinBox *mMarginTradingSkillEdit = nullptr;
        QSpinBox *mContractingSkillEdit = nullptr;
        QSpinBox *mCorporationContractingSkillEdit = nullptr;
        QDoubleSpinBox *mReprocessingImplantBonusEdit = nullptr;
        QSpinBox *mArkonorProcessingSkillEdit = nullptr;
        QSpinBox *mBistotProcessingSkillEdit = nullptr;
        QSpinBox *mCrokiteProcessingSkillEdit = nullptr;
        QSpinBox *mDarkOchreProcessingSkillEdit = nullptr;
        QSpinBox *mGneissProcessingSkillEdit = nullptr;
        QSpinBox *mHedbergiteProcessingSkillEdit = nullptr;
        QSpinBox *mHemorphiteProcessingSkillEdit = nullptr;
        QSpinBox *mIceProcessingSkillEdit = nullptr;
        QSpinBox *mJaspetProcessingSkillEdit = nullptr;
        QSpinBox *mKerniteProcessingSkillEdit = nullptr;
        QSpinBox *mMercoxitProcessingSkillEdit = nullptr;
        QSpinBox *mOmberProcessingSkillEdit = nullptr;
        QSpinBox *mPlagioclaseProcessingSkillEdit = nullptr;
        QSpinBox *mPyroxeresProcessingSkillEdit = nullptr;
        QSpinBox *mReprocessingSkillEdit = nullptr;
        QSpinBox *mReprocessingEfficiencySkillEdit = nullptr;
        QSpinBox *mScorditeProcessingSkillEdit = nullptr;
        QSpinBox *mScrapmetalProcessingSkillEdit = nullptr;
        QSpinBox *mSpodumainProcessingSkillEdit = nullptr;
        QSpinBox *mVeldsparProcessingSkillEdit = nullptr;
        QDoubleSpinBox *mManufacturingTimeImplantBonusEdit = nullptr;
        QSpinBox *mIndustrySkillEdit = nullptr;
        QSpinBox *mAdvancedIndustrySkillEdit = nullptr;
        QSpinBox *mAdvancedSmallShipConstructionSkillEdit = nullptr;
        QSpinBox *mAdvancedMediumShipConstructionSkillEdit = nullptr;
        QSpinBox *mAdvancedLargeShipConstructionSkillEdit = 0;
        QSpinBox *mAvancedIndustrialShipConstructionSkillEdit = nullptr;
        QSpinBox *mAmarrStarshipEngineeringSkillEdit = nullptr;
        QSpinBox *mCaldariStarshipEngineeringSkillEdit = nullptr;
        QSpinBox *mGallenteStarshipEngineeringSkillEdit = nullptr;
        QSpinBox *mMinmatarStarshipEngineeringSkillEdit = nullptr;
        QSpinBox *mElectromagneticPhysicsSkillEdit = nullptr;
        QSpinBox *mElectronicEngineeringSkillEdit = nullptr;
        QSpinBox *mGravitonPhysicsSkillEdit = nullptr;
        QSpinBox *mHighEnergyPhysicsSkillEdit = nullptr;
        QSpinBox *mHydromagneticPhysicsSkillEdit = nullptr;
        QSpinBox *mLaserPhysicsSkillEdit = nullptr;
        QSpinBox *mMechanicalEngineeringSkillEdit = nullptr;
        QSpinBox *mMolecularEngineeringSkillEdit = nullptr;
        QSpinBox *mNuclearPhysicsSkillEdit = nullptr;
        QSpinBox *mPlasmaPhysicsSkillEdit = nullptr;
        QSpinBox *mQuantumPhysicsSkillEdit = nullptr;
        QSpinBox *mRocketScienceSkillEdit = nullptr;

        QGroupBox *mUpdateTimersGroup = nullptr;
        QListWidget *mUpdateTimersList = nullptr;

        std::unordered_map<Character::IdType, FileDownload *> mPortraitDownloads;

        QTimer mUpdateTimer;

        virtual void handleNewCharacter(Character::IdType id) override;

        void updateStanding(const QString &type, double value) const;
        void updateCharacterMarketData(const CharacterData::OrderAmountSkills &orderAmountSkills);

        QSpinBox *createSkillEdit(QSpinBox *&target, const QString &skillField);

        static QPixmap getPortraitPixmap(Character::IdType id, int size = 128);
        // Path is the same as LowResPath because QIcon will resolve it properly to HighRes if DPR is high
        static QString getPortraitPath(Character::IdType id);
        static QString getPortraitLowResPath(Character::IdType id);
        static QString getPortraitHighResPath(Character::IdType id);
        static QString getPortraitPathTemplate(Character::IdType id);
    };
}
