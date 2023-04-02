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

#include <optional>

#include <QString>

namespace Evernus
{
    struct CharacterData
    {
        typedef double ISKType;

        struct OrderAmountSkills
        {
            int mTrade = 0;
            int mRetail = 0;
            int mWholesale = 0;
            int mTycoon = 0;
        };

        struct TradeRangeSkills
        {
            int mMarketing = 0;
            int mProcurement = 0;
            int mDaytrading = 0;
            int mVisibility = 0;
        };

        struct FeeSkills
        {
            int mAccounting = 0;
            int mBrokerRelations = 0;
            int mMarginTrading = 0;
        };

        struct ContractSkills
        {
            int mContracting = 0;
            int mCorporationContracting = 0;
        };

        struct ReprocessingSkills
        {
            int mArkonorProcessing = 0;
            int mBistotProcessing = 0;
            int mCrokiteProcessing = 0;
            int mDarkOchreProcessing = 0;
            int mGneissProcessing = 0;
            int mHedbergiteProcessing = 0;
            int mHemorphiteProcessing = 0;
            int mIceProcessing = 0;
            int mJaspetProcessing = 0;
            int mKerniteProcessing = 0;
            int mMercoxitProcessing = 0;
            int mOmberProcessing = 0;
            int mPlagioclaseProcessing = 0;
            int mPyroxeresProcessing = 0;
            int mReprocessing = 0;
            int mReprocessingEfficiency = 0;
            int mScorditeProcessing = 0;
            int mScrapmetalProcessing = 0;
            int mSpodumainProcessing = 0;
            int mVeldsparProcessing = 0;
        };

        struct IndustrySkills
        {
            int mIndustry = 0;
            int mAdvancedIndustry = 0;
            int mAdvancedSmallShipConstruction = 0;
            int mAdvancedMediumShipConstruction = 0;
            int mAdvancedLargeShipConstruction = 0;
            int mAvancedIndustrialShipConstruction = 0;
            int mAmarrStarshipEngineering = 0;
            int mCaldariStarshipEngineering = 0;
            int mGallenteStarshipEngineering = 0;
            int mMinmatarStarshipEngineering = 0;
            int mElectromagneticPhysics = 0;
            int mElectronicEngineering = 0;
            int mGravitonPhysics = 0;
            int mHighEnergyPhysics = 0;
            int mHydromagneticPhysics = 0;
            int mLaserPhysics = 0;
            int mMechanicalEngineering = 0;
            int mMolecularEngineering = 0;
            int mNuclearPhysics = 0;
            int mPlasmaPhysics = 0;
            int mQuantumPhysics = 0;
            int mRocketScience = 0;
        };

        QString mName;
        QString mCorporationName;
        quint64 mCorporationId = 0;
        QString mRace;
        QString mBloodline;
        QString mAncestry;
        QString mGender;
        ISKType mISK = 0;
        float mCorpStanding = 0.f;
        float mFactionStanding = 0.f;
        std::optional<double> mBuyBrokersFee;
        std::optional<double> mSellBrokersFee;
        float mReprocessingImplantBonus = 0.f;
        float mManufacturingTimeImplantBonus = 0.f;
        bool mAlphaClone = false;

        OrderAmountSkills mOrderAmountSkills;
        TradeRangeSkills mTradeRangeSkills;
        FeeSkills mFeeSkills;
        ContractSkills mContractSkills;
        ReprocessingSkills mReprocessingSkills;
        IndustrySkills mIndustrySkills;
    };
}
