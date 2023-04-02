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
#include <QLocale>

#include "TextUtils.h"

#include "Character.h"

namespace Evernus
{
    QString Character::getName() const &
    {
        return mData.mName;
    }

    QString &&Character::getName() && noexcept
    {
        return std::move(mData.mName);
    }

    void Character::setName(const QString &name)
    {
        mData.mName = name;
    }

    void Character::setName(QString &&name)
    {
        mData.mName = std::move(name);
    }

    QString Character::getCorporationName() const &
    {
        return mData.mCorporationName;
    }

    QString &&Character::getCorporationName() && noexcept
    {
        return std::move(mData.mCorporationName);
    }

    void Character::setCorporationName(const QString &name)
    {
        mData.mCorporationName = name;
    }

    void Character::setCorporationName(QString &&name)
    {
        mData.mCorporationName = std::move(name);
    }

    quint64 Character::getCorporationId() const noexcept
    {
        return mData.mCorporationId;
    }

    void Character::setCorporationId(quint64 id) noexcept
    {
        mData.mCorporationId = id;
    }

    QString Character::getRace() const &
    {
        return mData.mRace;
    }

    QString &&Character::getRace() && noexcept
    {
        return std::move(mData.mRace);
    }

    void Character::setRace(const QString &race)
    {
        mData.mRace = race;
    }

    void Character::setRace(QString &&race)
    {
        mData.mRace = std::move(race);
    }

    QString Character::getBloodline() const &
    {
        return mData.mBloodline;
    }

    QString &&Character::getBloodline() && noexcept
    {
        return std::move(mData.mBloodline);
    }

    void Character::setBloodline(const QString &bloodline)
    {
        mData.mBloodline = bloodline;
    }

    void Character::setBloodline(QString &&bloodline)
    {
        mData.mBloodline = std::move(bloodline);
    }

    QString Character::getAncestry() const &
    {
        return mData.mAncestry;
    }

    QString &&Character::getAncestry() && noexcept
    {
        return std::move(mData.mAncestry);
    }

    void Character::setAncestry(const QString &ancestry)
    {
        mData.mAncestry = ancestry;
    }

    void Character::setAncestry(QString &&ancestry)
    {
        mData.mAncestry = std::move(ancestry);
    }

    QString Character::getGender() const &
    {
        return mData.mGender;
    }

    QString &&Character::getGender() && noexcept
    {
        return std::move(mData.mGender);
    }

    void Character::setGender(const QString &gender)
    {
        mData.mGender = gender;
    }

    void Character::setGender(QString &&gender)
    {
        mData.mGender = std::move(gender);
    }

    CharacterData::ISKType Character::getISK() const noexcept
    {
        return mData.mISK;
    }

    QString Character::getISKPresentation() const
    {
        QLocale locale;
        return TextUtils::currencyToString(getISK(), locale);
    }

    void Character::setISK(CharacterData::ISKType isk) noexcept
    {
        mData.mISK = isk;
    }

    float Character::getCorpStanding() const noexcept
    {
        return mData.mCorpStanding;
    }

    void Character::setCorpStanding(float standing) noexcept
    {
        mData.mCorpStanding = standing;
    }

    float Character::getFactionStanding() const noexcept
    {
        return mData.mFactionStanding;
    }

    void Character::setFactionStanding(float standing) noexcept
    {
        mData.mFactionStanding = standing;
    }

    std::optional<double> Character::getBuyBrokersFee() const
    {
        return mData.mBuyBrokersFee;
    }

    void Character::setBuyBrokersFee(std::optional<double> fee)
    {
        mData.mBuyBrokersFee = std::move(fee);
    }

    std::optional<double> Character::getSellBrokersFee() const
    {
        return mData.mSellBrokersFee;
    }

    void Character::setSellBrokersFee(std::optional<double> fee)
    {
        mData.mSellBrokersFee = std::move(fee);
    }

    float Character::getReprocessingImplantBonus() const noexcept
    {
        return mData.mReprocessingImplantBonus;
    }

    void Character::setReprocessingImplantBonus(float value) noexcept
    {
        mData.mReprocessingImplantBonus = value;
    }

    float Character::getManufacturingTimeImplantBonus() const noexcept
    {
        return mData.mManufacturingTimeImplantBonus;
    }

    void Character::setManufacturingTimeImplantBonus(float value) noexcept
    {
        mData.mManufacturingTimeImplantBonus = value;
    }

    bool Character::isAlphaClone() const noexcept
    {
        return mData.mAlphaClone;
    }

    void Character::setAlphaClone(bool flag) noexcept
    {
        mData.mAlphaClone = flag;
    }

    CharacterData::OrderAmountSkills Character::getOrderAmountSkills() const noexcept
    {
        return mData.mOrderAmountSkills;
    }

    void Character::setOrderAmountSkills(const CharacterData::OrderAmountSkills &skills) noexcept
    {
        mData.mOrderAmountSkills = skills;
    }

    void Character::setOrderAmountSkills(CharacterData::OrderAmountSkills &&skills) noexcept
    {
        mData.mOrderAmountSkills = std::move(skills);
    }

    CharacterData::TradeRangeSkills Character::getTradeRangeSkills() const noexcept
    {
        return mData.mTradeRangeSkills;
    }

    void Character::setTradeRangeSkills(const CharacterData::TradeRangeSkills &skills) noexcept
    {
        mData.mTradeRangeSkills = skills;
    }

    void Character::setTradeRangeSkills(CharacterData::TradeRangeSkills &&skills) noexcept
    {
        mData.mTradeRangeSkills = std::move(skills);
    }

    CharacterData::FeeSkills Character::getFeeSkills() const noexcept
    {
        return mData.mFeeSkills;
    }

    void Character::setFeeSkills(const CharacterData::FeeSkills &skills) noexcept
    {
        mData.mFeeSkills = skills;
    }

    void Character::setFeeSkills(CharacterData::FeeSkills &&skills) noexcept
    {
        mData.mFeeSkills = std::move(skills);
    }

    CharacterData::ContractSkills Character::getContractSkills() const noexcept
    {
        return mData.mContractSkills;
    }

    void Character::setContractSkills(const CharacterData::ContractSkills &skills) noexcept
    {
        mData.mContractSkills = skills;
    }

    void Character::setContractSkills(CharacterData::ContractSkills &&skills) noexcept
    {
        mData.mContractSkills = std::move(skills);
    }

    CharacterData::ReprocessingSkills Character::getReprocessingSkills() const noexcept
    {
        return mData.mReprocessingSkills;
    }

    void Character::setReprocessingSkills(const CharacterData::ReprocessingSkills &skills) noexcept
    {
        mData.mReprocessingSkills = skills;
    }

    void Character::setReprocessingSkills(CharacterData::ReprocessingSkills &&skills) noexcept
    {
        mData.mReprocessingSkills = std::move(skills);
    }

    CharacterData::IndustrySkills Character::getIndustrySkills() const noexcept
    {
        return mData.mIndustrySkills;
    }

    void Character::setIndustrySkills(const CharacterData::IndustrySkills &skills) noexcept
    {
        mData.mIndustrySkills = skills;
    }

    void Character::setIndustrySkills(CharacterData::IndustrySkills &&skills) noexcept
    {
        mData.mIndustrySkills = std::move(skills);
    }

    CharacterData Character::getCharacterData() const &
    {
        return mData;
    }

    CharacterData &&Character::getCharacterData() && noexcept
    {
        return std::move(mData);
    }

    void Character::setCharacterData(const CharacterData &data)
    {
        mData = data;
    }

    void Character::setCharacterData(CharacterData &&data)
    {
        mData = std::move(data);
    }

    bool Character::isEnabled() const noexcept
    {
        return mEnabled;
    }

    void Character::setEnabled(bool flag) noexcept
    {
        mEnabled = flag;
    }
}
