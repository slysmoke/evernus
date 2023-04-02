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

#include "Entity.h"

#include "CharacterData.h"

namespace Evernus
{
    class Character
        : public Entity<quint64>
    {
    public:
        using Entity::Entity;

        Character() = default;
        Character(const Character &) = default;
        Character(Character &&) = default;
        virtual ~Character() = default;

        QString getName() const &;
        QString &&getName() && noexcept;
        void setName(const QString &name);
        void setName(QString &&name);

        QString getCorporationName() const &;
        QString &&getCorporationName() && noexcept;
        void setCorporationName(const QString &name);
        void setCorporationName(QString &&name);

        quint64 getCorporationId() const noexcept;
        void setCorporationId(quint64 id) noexcept;

        QString getRace() const &;
        QString &&getRace() && noexcept;
        void setRace(const QString &race);
        void setRace(QString &&race);

        QString getBloodline() const &;
        QString &&getBloodline() && noexcept;
        void setBloodline(const QString &bloodline);
        void setBloodline(QString &&bloodline);

        QString getAncestry() const &;
        QString &&getAncestry() && noexcept;
        void setAncestry(const QString &ancestry);
        void setAncestry(QString &&ancestry);

        QString getGender() const &;
        QString &&getGender() && noexcept;
        void setGender(const QString &gender);
        void setGender(QString &&gender);

        CharacterData::ISKType getISK() const noexcept;
        QString getISKPresentation() const;
        void setISK(CharacterData::ISKType isk) noexcept;

        float getCorpStanding() const noexcept;
        void setCorpStanding(float standing) noexcept;

        float getFactionStanding() const noexcept;
        void setFactionStanding(float standing) noexcept;

        std::optional<double> getBuyBrokersFee() const;
        void setBuyBrokersFee(std::optional<double> fee);

        std::optional<double> getSellBrokersFee() const;
        void setSellBrokersFee(std::optional<double> fee);

        float getReprocessingImplantBonus() const noexcept;
        void setReprocessingImplantBonus(float value) noexcept;

        float getManufacturingTimeImplantBonus() const noexcept;
        void setManufacturingTimeImplantBonus(float value) noexcept;

        bool isAlphaClone() const noexcept;
        void setAlphaClone(bool flag) noexcept;

        CharacterData::OrderAmountSkills getOrderAmountSkills() const noexcept;
        void setOrderAmountSkills(const CharacterData::OrderAmountSkills &skills) noexcept;
        void setOrderAmountSkills(CharacterData::OrderAmountSkills &&skills) noexcept;

        CharacterData::TradeRangeSkills getTradeRangeSkills() const noexcept;
        void setTradeRangeSkills(const CharacterData::TradeRangeSkills &skills) noexcept;
        void setTradeRangeSkills(CharacterData::TradeRangeSkills &&skills) noexcept;

        CharacterData::FeeSkills getFeeSkills() const noexcept;
        void setFeeSkills(const CharacterData::FeeSkills &skills) noexcept;
        void setFeeSkills(CharacterData::FeeSkills &&skills) noexcept;

        CharacterData::ContractSkills getContractSkills() const noexcept;
        void setContractSkills(const CharacterData::ContractSkills &skills) noexcept;
        void setContractSkills(CharacterData::ContractSkills &&skills) noexcept;

        CharacterData::ReprocessingSkills getReprocessingSkills() const noexcept;
        void setReprocessingSkills(const CharacterData::ReprocessingSkills &skills) noexcept;
        void setReprocessingSkills(CharacterData::ReprocessingSkills &&skills) noexcept;

        CharacterData::IndustrySkills getIndustrySkills() const noexcept;
        void setIndustrySkills(const CharacterData::IndustrySkills &skills) noexcept;
        void setIndustrySkills(CharacterData::IndustrySkills &&skills) noexcept;

        CharacterData getCharacterData() const &;
        CharacterData &&getCharacterData() && noexcept;
        void setCharacterData(const CharacterData &data);
        void setCharacterData(CharacterData &&data);

        bool isEnabled() const noexcept;
        void setEnabled(bool flag) noexcept;

        Character &operator =(const Character &) = default;
        Character &operator =(Character &&) = default;

    private:
        CharacterData mData;
        bool mEnabled = true;
    };
}
