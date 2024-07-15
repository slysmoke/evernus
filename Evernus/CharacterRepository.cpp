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
#include <QSqlQuery>

#include <boost/throw_exception.hpp>

#include "CharacterRepository.h"

namespace Evernus
{
    QString CharacterRepository::getTableName() const
    {
        return QStringLiteral("characters");
    }

    QString CharacterRepository::getIdColumn() const
    {
        return QStringLiteral("id");
    }

    CharacterRepository::EntityPtr CharacterRepository::populate(const QSqlRecord &record) const
    {
        auto character = std::make_shared<Character>(record.value(QStringLiteral("id")).value<Character::IdType>());
        character->setName(record.value(QStringLiteral("name")).toString());
        character->setCorporationName(record.value(QStringLiteral("corporation_name")).toString());
        character->setCorporationId(record.value(QStringLiteral("corporation_id")).toULongLong());
        character->setRace(record.value(QStringLiteral("race")).toString());
        character->setBloodline(record.value(QStringLiteral("bloodline")).toString());
        
        character->setGender(record.value(QStringLiteral("gender")).toString());
        character->setISK(record.value(QStringLiteral("isk")).value<CharacterData::ISKType>());
        character->setCorpStanding(record.value(QStringLiteral("corp_standing")).toFloat());
        character->setFactionStanding(record.value(QStringLiteral("faction_standing")).toFloat());
        character->setEnabled(record.value(QStringLiteral("enabled")).toBool());
        character->setReprocessingImplantBonus(record.value(QStringLiteral("reprocessing_implant_bonus")).toFloat());
        character->setManufacturingTimeImplantBonus(record.value(QStringLiteral("manufacturing_time_implant_bonus")).toFloat());
        character->setAlphaClone(record.value(QStringLiteral("alpha_clone")).toBool());

        if (!record.value(QStringLiteral("brokers_fee")).isNull())
            character->setBuyBrokersFee(record.value(QStringLiteral("brokers_fee")).toDouble());
        if (!record.value(QStringLiteral("sell_brokers_fee")).isNull())
            character->setSellBrokersFee(record.value(QStringLiteral("sell_brokers_fee")).toDouble());

        CharacterData::OrderAmountSkills orderAmountSkills;
        orderAmountSkills.mTrade = record.value(QStringLiteral("trade_skill")).toInt();
        orderAmountSkills.mRetail = record.value(QStringLiteral("retail_skill")).toInt();
        orderAmountSkills.mWholesale = record.value(QStringLiteral("wholesale_skill")).toInt();
        orderAmountSkills.mTycoon = record.value(QStringLiteral("tycoon_skill")).toInt();

        CharacterData::TradeRangeSkills tradeRangeSkills;
        tradeRangeSkills.mMarketing = record.value(QStringLiteral("marketing_skill")).toInt();
        tradeRangeSkills.mProcurement = record.value(QStringLiteral("procurement_skill")).toInt();
        tradeRangeSkills.mDaytrading = record.value(QStringLiteral("daytrading_skill")).toInt();
        tradeRangeSkills.mVisibility = record.value(QStringLiteral("visibility_skill")).toInt();

        CharacterData::FeeSkills feeSkills;
        feeSkills.mAccounting = record.value(QStringLiteral("accounting_skill")).toInt();
        feeSkills.mBrokerRelations = record.value(QStringLiteral("broker_relations_skill")).toInt();
        feeSkills.mMarginTrading = record.value(QStringLiteral("margin_trading_skill")).toInt();

        CharacterData::ContractSkills contractSkills;
        contractSkills.mContracting = record.value(QStringLiteral("contracting_skill")).toInt();
        contractSkills.mCorporationContracting = record.value(QStringLiteral("corporation_contracting_skill")).toInt();

        CharacterData::ReprocessingSkills reprocessingSkills;
        reprocessingSkills.mArkonorProcessing = record.value(QStringLiteral("arkonor_processing")).toInt();
        reprocessingSkills.mBistotProcessing = record.value(QStringLiteral("bistot_processing")).toInt();
        reprocessingSkills.mCrokiteProcessing = record.value(QStringLiteral("crokite_processing")).toInt();
        reprocessingSkills.mDarkOchreProcessing = record.value(QStringLiteral("dark_ochre_processing")).toInt();
        reprocessingSkills.mGneissProcessing = record.value(QStringLiteral("gneiss_processing")).toInt();
        reprocessingSkills.mHedbergiteProcessing = record.value(QStringLiteral("hedbergite_processing")).toInt();
        reprocessingSkills.mHemorphiteProcessing = record.value(QStringLiteral("hemorphite_processing")).toInt();
        reprocessingSkills.mIceProcessing = record.value(QStringLiteral("ice_processing")).toInt();
        reprocessingSkills.mJaspetProcessing = record.value(QStringLiteral("jaspet_processing")).toInt();
        reprocessingSkills.mKerniteProcessing = record.value(QStringLiteral("kernite_processing")).toInt();
        reprocessingSkills.mMercoxitProcessing = record.value(QStringLiteral("mercoxit_processing")).toInt();
        reprocessingSkills.mOmberProcessing = record.value(QStringLiteral("omber_processing")).toInt();
        reprocessingSkills.mPlagioclaseProcessing = record.value(QStringLiteral("plagioclase_processing")).toInt();
        reprocessingSkills.mPyroxeresProcessing = record.value(QStringLiteral("pyroxeres_processing")).toInt();
        reprocessingSkills.mReprocessing = record.value(QStringLiteral("reprocessing")).toInt();
        reprocessingSkills.mReprocessingEfficiency = record.value(QStringLiteral("reprocessing_efficiency")).toInt();
        reprocessingSkills.mScorditeProcessing = record.value(QStringLiteral("scordite_processing")).toInt();
        reprocessingSkills.mScrapmetalProcessing = record.value(QStringLiteral("scrapmetal_processing")).toInt();
        reprocessingSkills.mSpodumainProcessing = record.value(QStringLiteral("spodumain_processing")).toInt();
        reprocessingSkills.mVeldsparProcessing = record.value(QStringLiteral("veldspar_processing")).toInt();

        CharacterData::IndustrySkills industrySkills;
        industrySkills.mIndustry = record.value(QStringLiteral("industry")).toInt();
        industrySkills.mAdvancedIndustry = record.value(QStringLiteral("advanced_industry")).toInt();
        industrySkills.mAdvancedSmallShipConstruction = record.value(QStringLiteral("advanced_small_ship_construction")).toInt();
        industrySkills.mAdvancedMediumShipConstruction = record.value(QStringLiteral("advanced_medium_ship_construction")).toInt();
        industrySkills.mAdvancedLargeShipConstruction = record.value(QStringLiteral("advanced_large_ship_construction")).toInt();
        industrySkills.mAvancedIndustrialShipConstruction = record.value(QStringLiteral("avanced_industrial_ship_construction")).toInt();
        industrySkills.mAmarrStarshipEngineering = record.value(QStringLiteral("amarr_starship_engineering")).toInt();
        industrySkills.mCaldariStarshipEngineering = record.value(QStringLiteral("caldari_starship_engineering")).toInt();
        industrySkills.mGallenteStarshipEngineering = record.value(QStringLiteral("gallente_starship_engineering")).toInt();
        industrySkills.mMinmatarStarshipEngineering = record.value(QStringLiteral("minmatar_starship_engineering")).toInt();
        industrySkills.mElectromagneticPhysics = record.value(QStringLiteral("electromagnetic_physics")).toInt();
        industrySkills.mElectronicEngineering = record.value(QStringLiteral("electronic_engineering")).toInt();
        industrySkills.mGravitonPhysics = record.value(QStringLiteral("graviton_physics")).toInt();
        industrySkills.mHighEnergyPhysics = record.value(QStringLiteral("high_energy_physics")).toInt();
        industrySkills.mHydromagneticPhysics = record.value(QStringLiteral("hydromagnetic_physics")).toInt();
        industrySkills.mLaserPhysics = record.value(QStringLiteral("laser_physics")).toInt();
        industrySkills.mMechanicalEngineering = record.value(QStringLiteral("mechanical_engineering")).toInt();
        industrySkills.mMolecularEngineering = record.value(QStringLiteral("molecular_engineering")).toInt();
        industrySkills.mNuclearPhysics = record.value(QStringLiteral("nuclear_physics")).toInt();
        industrySkills.mPlasmaPhysics = record.value(QStringLiteral("plasma_physics")).toInt();
        industrySkills.mQuantumPhysics = record.value(QStringLiteral("quantum_physics")).toInt();
        industrySkills.mRocketScience = record.value(QStringLiteral("rocket_science")).toInt();

        character->setOrderAmountSkills(std::move(orderAmountSkills));
        character->setTradeRangeSkills(std::move(tradeRangeSkills));
        character->setFeeSkills(std::move(feeSkills));
        character->setContractSkills(std::move(contractSkills));
        character->setReprocessingSkills(std::move(reprocessingSkills));
        character->setIndustrySkills(std::move(industrySkills));
        character->setNew(false);

        return character;
    }

    void CharacterRepository::create() const
    {
        exec(getCreateQuery());
    }

    void CharacterRepository::updateSkill(Character::IdType id, const QString &skill, int level) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET %2 = ? WHERE %3 = ?")
            .arg(getTableName())
            .arg(skill)
            .arg(getIdColumn()));
        query.bindValue(0, level);
        query.bindValue(1, id);

        DatabaseUtils::execQuery(query);
    }

    void CharacterRepository::updateStanding(Character::IdType id, const QString &type, double value) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET %2 = ? WHERE %3 = ?")
            .arg(getTableName())
            .arg(type)
            .arg(getIdColumn()));
        query.bindValue(0, value);
        query.bindValue(1, id);

        DatabaseUtils::execQuery(query);
    }

    void CharacterRepository::updateBrokersFee(Character::IdType id, const std::optional<double> &buy, const std::optional<double> &sell) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET brokers_fee = ?, sell_brokers_fee = ? WHERE %2 = ?")
            .arg(getTableName())
            .arg(getIdColumn()));
        query.bindValue(0, (buy) ? (*buy) : (QVariant{QVariant::Double}));
        query.bindValue(1, (sell) ? (*sell) : (QVariant{QVariant::Double}));
        query.bindValue(2, id);

        DatabaseUtils::execQuery(query);
    }

    void CharacterRepository::updateReprocessingImplantBonus(Character::IdType id, double value) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET reprocessing_implant_bonus = ? WHERE %2 = ?")
            .arg(getTableName())
            .arg(getIdColumn()));
        query.bindValue(0, value);
        query.bindValue(1, id);

        DatabaseUtils::execQuery(query);
    }

    void CharacterRepository::updateManufacturingTimeImplantBonus(Character::IdType id, double value) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET manufacturing_time_implant_bonus = ? WHERE %2 = ?")
            .arg(getTableName())
            .arg(getIdColumn()));
        query.bindValue(0, value);
        query.bindValue(1, id);

        DatabaseUtils::execQuery(query);
    }

    void CharacterRepository::updateAlphaClone(Character::IdType id, bool flag) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET alpha_clone = ? WHERE %2 = ?")
            .arg(getTableName())
            .arg(getIdColumn()));
        query.bindValue(0, flag);
        query.bindValue(1, id);

        DatabaseUtils::execQuery(query);
    }

    bool CharacterRepository::hasCharacters() const
    {
        auto query = exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(getTableName()));
        query.next();

        return query.value(0).toUInt() != 0;
    }

    std::unordered_set<Character::IdType> CharacterRepository::fetchAllIds() const
    {
        std::unordered_set<Character::IdType> result;

        auto query = exec(QStringLiteral("SELECT id FROM %1").arg(getTableName()));
        while (query.next())
            result.emplace(query.value(0).value<Character::IdType>());

        return result;
    }

    QString CharacterRepository::getNameColumn() const
    {
        return QStringLiteral("name");
    }

    quint64 CharacterRepository::getCorporationId(Character::IdType id) const
    {
        auto query = prepare(QStringLiteral("SELECT corporation_id FROM %1 WHERE %2 = ?").arg(getTableName()).arg(getIdColumn()));
        query.bindValue(0, id);

        DatabaseUtils::execQuery(query);

        if (!query.next())
            BOOST_THROW_EXCEPTION(NotFoundException{});

        return query.value(0).toULongLong();
    }

    QString CharacterRepository::getName(Character::IdType id) const
    {
        auto query = prepare(QStringLiteral("SELECT name FROM %1 WHERE %2 = ?").arg(getTableName()).arg(getIdColumn()));
        query.bindValue(0, id);

        DatabaseUtils::execQuery(query);

        if (!query.next())
            BOOST_THROW_EXCEPTION(NotFoundException{});

        return query.value(0).toString();
    }

    QSqlQuery CharacterRepository::getEnabledQuery() const
    {
        return exec(QStringLiteral("SELECT %2, name FROM %1 WHERE enabled != 0 ORDER BY name")
            .arg(getTableName())
            .arg(getIdColumn()));
    }

    QString CharacterRepository::getCreateQuery() const
    {
        return QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "id BIGINT PRIMARY KEY,"
            "name TEXT NOT NULL,"
            "corporation_name TEXT NOT NULL,"
            "corporation_id BIGINT NOT NULL,"
            "race TEXT,"
            "bloodline TEXT,"
            
            "gender TEXT,"
            "isk DOUBLE NOT NULL,"
            "corp_standing FLOAT NOT NULL,"
            "faction_standing FLOAT NOT NULL,"
            "trade_skill TINYINT NOT NULL,"
            "retail_skill TINYINT NOT NULL,"
            "wholesale_skill TINYINT NOT NULL,"
            "tycoon_skill TINYINT NOT NULL,"
            "marketing_skill TINYINT NOT NULL,"
            "procurement_skill TINYINT NOT NULL,"
            "daytrading_skill TINYINT NOT NULL,"
            "visibility_skill TINYINT NOT NULL,"
            "accounting_skill TINYINT NOT NULL,"
            "broker_relations_skill TINYINT NOT NULL,"
            "margin_trading_skill TINYINT NOT NULL,"
            "contracting_skill TINYINT NOT NULL,"
            "corporation_contracting_skill TINYINT NOT NULL,"
            "enabled TINYINT NOT NULL,"
            "brokers_fee FLOAT NULL,"
            "sell_brokers_fee FLOAT NULL,"
            "reprocessing_implant_bonus FLOAT NOT NULL,"
            "arkonor_processing TINYINT NOT NULL,"
            "bistot_processing TINYINT NOT NULL,"
            "crokite_processing TINYINT NOT NULL,"
            "dark_ochre_processing TINYINT NOT NULL,"
            "gneiss_processing TINYINT NOT NULL,"
            "hedbergite_processing TINYINT NOT NULL,"
            "hemorphite_processing TINYINT NOT NULL,"
            "ice_processing TINYINT NOT NULL,"
            "jaspet_processing TINYINT NOT NULL,"
            "kernite_processing TINYINT NOT NULL,"
            "mercoxit_processing TINYINT NOT NULL,"
            "omber_processing TINYINT NOT NULL,"
            "plagioclase_processing TINYINT NOT NULL,"
            "pyroxeres_processing TINYINT NOT NULL,"
            "reprocessing TINYINT NOT NULL,"
            "reprocessing_efficiency TINYINT NOT NULL,"
            "scordite_processing TINYINT NOT NULL,"
            "scrapmetal_processing TINYINT NOT NULL,"
            "spodumain_processing TINYINT NOT NULL,"
            "veldspar_processing TINYINT NOT NULL,"
            "manufacturing_time_implant_bonus FLOAT NOT NULL,"
            "industry TINYINT NOT NULL,"
            "advanced_industry TINYINT NOT NULL,"
            "advanced_small_ship_construction TINYINT NOT NULL,"
            "advanced_medium_ship_construction TINYINT NOT NULL,"
            "advanced_large_ship_construction TINYINT NOT NULL,"
            "avanced_industrial_ship_construction TINYINT NOT NULL,"
            "amarr_starship_engineering TINYINT NOT NULL,"
            "caldari_starship_engineering TINYINT NOT NULL,"
            "gallente_starship_engineering TINYINT NOT NULL,"
            "minmatar_starship_engineering TINYINT NOT NULL,"
            "electromagnetic_physics TINYINT NOT NULL,"
            "electronic_engineering TINYINT NOT NULL,"
            "graviton_physics TINYINT NOT NULL,"
            "high_energy_physics TINYINT NOT NULL,"
            "hydromagnetic_physics TINYINT NOT NULL,"
            "laser_physics TINYINT NOT NULL,"
            "mechanical_engineering TINYINT NOT NULL,"
            "molecular_engineering TINYINT NOT NULL,"
            "nuclear_physics TINYINT NOT NULL,"
            "plasma_physics TINYINT NOT NULL,"
            "quantum_physics TINYINT NOT NULL,"
            "rocket_science TINYINT NOT NULL,"
            "alpha_clone TINYINT NOT NULL"
        ")").arg(getTableName());
    }

    QStringList CharacterRepository::getColumns() const
    {
        return {
            QStringLiteral("id"),
            QStringLiteral("name"),
            QStringLiteral("corporation_name"),
            QStringLiteral("corporation_id"),
            QStringLiteral("race"),
            QStringLiteral("bloodline"),
        
            QStringLiteral("gender"),
            QStringLiteral("isk"),
            QStringLiteral("corp_standing"),
            QStringLiteral("faction_standing"),
            QStringLiteral("trade_skill"),
            QStringLiteral("retail_skill"),
            QStringLiteral("wholesale_skill"),
            QStringLiteral("tycoon_skill"),
            QStringLiteral("marketing_skill"),
            QStringLiteral("procurement_skill"),
            QStringLiteral("daytrading_skill"),
            QStringLiteral("visibility_skill"),
            QStringLiteral("accounting_skill"),
            QStringLiteral("broker_relations_skill"),
            QStringLiteral("margin_trading_skill"),
            QStringLiteral("contracting_skill"),
            QStringLiteral("corporation_contracting_skill"),
            QStringLiteral("enabled"),
            QStringLiteral("brokers_fee"),
            QStringLiteral("sell_brokers_fee"),
            QStringLiteral("reprocessing_implant_bonus"),
            QStringLiteral("arkonor_processing"),
            QStringLiteral("bistot_processing"),
            QStringLiteral("crokite_processing"),
            QStringLiteral("dark_ochre_processing"),
            QStringLiteral("gneiss_processing"),
            QStringLiteral("hedbergite_processing"),
            QStringLiteral("hemorphite_processing"),
            QStringLiteral("ice_processing"),
            QStringLiteral("jaspet_processing"),
            QStringLiteral("kernite_processing"),
            QStringLiteral("mercoxit_processing"),
            QStringLiteral("omber_processing"),
            QStringLiteral("plagioclase_processing"),
            QStringLiteral("pyroxeres_processing"),
            QStringLiteral("reprocessing"),
            QStringLiteral("reprocessing_efficiency"),
            QStringLiteral("scordite_processing"),
            QStringLiteral("scrapmetal_processing"),
            QStringLiteral("spodumain_processing"),
            QStringLiteral("veldspar_processing"),
            QStringLiteral("manufacturing_time_implant_bonus"),
            QStringLiteral("industry"),
            QStringLiteral("advanced_industry"),
            QStringLiteral("advanced_small_ship_construction"),
            QStringLiteral("advanced_medium_ship_construction"),
            QStringLiteral("advanced_large_ship_construction"),
            QStringLiteral("avanced_industrial_ship_construction"),
            QStringLiteral("amarr_starship_engineering"),
            QStringLiteral("caldari_starship_engineering"),
            QStringLiteral("gallente_starship_engineering"),
            QStringLiteral("minmatar_starship_engineering"),
            QStringLiteral("electromagnetic_physics"),
            QStringLiteral("electronic_engineering"),
            QStringLiteral("graviton_physics"),
            QStringLiteral("high_energy_physics"),
            QStringLiteral("hydromagnetic_physics"),
            QStringLiteral("laser_physics"),
            QStringLiteral("mechanical_engineering"),
            QStringLiteral("molecular_engineering"),
            QStringLiteral("nuclear_physics"),
            QStringLiteral("plasma_physics"),
            QStringLiteral("quantum_physics"),
            QStringLiteral("rocket_science"),
            QStringLiteral("alpha_clone"),
        };
    }

    void CharacterRepository::bindValues(const Character &entity, QSqlQuery &query) const
    {
        const auto orderAmountSkills = entity.getOrderAmountSkills();
        const auto tradeRangeSkills = entity.getTradeRangeSkills();
        const auto feeSkills = entity.getFeeSkills();
        const auto contractSkills = entity.getContractSkills();
        const auto reprocessingSkills = entity.getReprocessingSkills();
        const auto industrySkills = entity.getIndustrySkills();

        if (entity.getId() != Character::invalidId)
            query.bindValue(QStringLiteral(":id"), entity.getId());

        query.bindValue(QStringLiteral(":name"), entity.getName());
        query.bindValue(QStringLiteral(":corporation_name"), entity.getCorporationName());
        query.bindValue(QStringLiteral(":corporation_id"), entity.getCorporationId());
        query.bindValue(QStringLiteral(":race"), entity.getRace());
        query.bindValue(QStringLiteral(":bloodline"), entity.getBloodline());
       
        query.bindValue(QStringLiteral(":gender"), entity.getGender());
        query.bindValue(QStringLiteral(":isk"), entity.getISK());
        query.bindValue(QStringLiteral(":corp_standing"), entity.getCorpStanding());
        query.bindValue(QStringLiteral(":faction_standing"), entity.getFactionStanding());
        query.bindValue(QStringLiteral(":trade_skill"), orderAmountSkills.mTrade);
        query.bindValue(QStringLiteral(":retail_skill"), orderAmountSkills.mRetail);
        query.bindValue(QStringLiteral(":wholesale_skill"), orderAmountSkills.mWholesale);
        query.bindValue(QStringLiteral(":tycoon_skill"), orderAmountSkills.mTycoon);
        query.bindValue(QStringLiteral(":marketing_skill"), tradeRangeSkills.mMarketing);
        query.bindValue(QStringLiteral(":procurement_skill"), tradeRangeSkills.mProcurement);
        query.bindValue(QStringLiteral(":daytrading_skill"), tradeRangeSkills.mDaytrading);
        query.bindValue(QStringLiteral(":visibility_skill"), tradeRangeSkills.mVisibility);
        query.bindValue(QStringLiteral(":accounting_skill"), feeSkills.mAccounting);
        query.bindValue(QStringLiteral(":broker_relations_skill"), feeSkills.mBrokerRelations);
        query.bindValue(QStringLiteral(":margin_trading_skill"), feeSkills.mMarginTrading);
        query.bindValue(QStringLiteral(":contracting_skill"), contractSkills.mContracting);
        query.bindValue(QStringLiteral(":corporation_contracting_skill"), contractSkills.mCorporationContracting);
        query.bindValue(QStringLiteral(":enabled"), entity.isEnabled());
        query.bindValue(QStringLiteral(":brokers_fee"), (entity.getBuyBrokersFee()) ? (*entity.getBuyBrokersFee()) : (QVariant{QVariant::Double}));
        query.bindValue(QStringLiteral(":sell_brokers_fee"), (entity.getBuyBrokersFee()) ? (*entity.getSellBrokersFee()) : (QVariant{QVariant::Double}));
        query.bindValue(QStringLiteral(":reprocessing_implant_bonus"), entity.getReprocessingImplantBonus());
        query.bindValue(QStringLiteral(":arkonor_processing"), reprocessingSkills.mArkonorProcessing);
        query.bindValue(QStringLiteral(":bistot_processing"), reprocessingSkills.mBistotProcessing);
        query.bindValue(QStringLiteral(":crokite_processing"), reprocessingSkills.mCrokiteProcessing);
        query.bindValue(QStringLiteral(":dark_ochre_processing"), reprocessingSkills.mDarkOchreProcessing);
        query.bindValue(QStringLiteral(":gneiss_processing"), reprocessingSkills.mGneissProcessing);
        query.bindValue(QStringLiteral(":hedbergite_processing"), reprocessingSkills.mHedbergiteProcessing);
        query.bindValue(QStringLiteral(":hemorphite_processing"), reprocessingSkills.mHemorphiteProcessing);
        query.bindValue(QStringLiteral(":ice_processing"), reprocessingSkills.mIceProcessing);
        query.bindValue(QStringLiteral(":jaspet_processing"), reprocessingSkills.mJaspetProcessing);
        query.bindValue(QStringLiteral(":kernite_processing"), reprocessingSkills.mKerniteProcessing);
        query.bindValue(QStringLiteral(":mercoxit_processing"), reprocessingSkills.mMercoxitProcessing);
        query.bindValue(QStringLiteral(":omber_processing"), reprocessingSkills.mOmberProcessing);
        query.bindValue(QStringLiteral(":plagioclase_processing"), reprocessingSkills.mPlagioclaseProcessing);
        query.bindValue(QStringLiteral(":pyroxeres_processing"), reprocessingSkills.mPyroxeresProcessing);
        query.bindValue(QStringLiteral(":reprocessing"), reprocessingSkills.mReprocessing);
        query.bindValue(QStringLiteral(":reprocessing_efficiency"), reprocessingSkills.mReprocessingEfficiency);
        query.bindValue(QStringLiteral(":scordite_processing"), reprocessingSkills.mScorditeProcessing);
        query.bindValue(QStringLiteral(":scrapmetal_processing"), reprocessingSkills.mScrapmetalProcessing);
        query.bindValue(QStringLiteral(":spodumain_processing"), reprocessingSkills.mSpodumainProcessing);
        query.bindValue(QStringLiteral(":veldspar_processing"), reprocessingSkills.mVeldsparProcessing);
        query.bindValue(QStringLiteral(":manufacturing_time_implant_bonus"), entity.getManufacturingTimeImplantBonus());
        query.bindValue(QStringLiteral(":industry"), industrySkills.mIndustry);
        query.bindValue(QStringLiteral(":advanced_industry"), industrySkills.mAdvancedIndustry);
        query.bindValue(QStringLiteral(":advanced_small_ship_construction"), industrySkills.mAdvancedSmallShipConstruction);
        query.bindValue(QStringLiteral(":advanced_medium_ship_construction"), industrySkills.mAdvancedMediumShipConstruction);
        query.bindValue(QStringLiteral(":advanced_large_ship_construction"), industrySkills.mAdvancedLargeShipConstruction);
        query.bindValue(QStringLiteral(":avanced_industrial_ship_construction"), industrySkills.mAvancedIndustrialShipConstruction);
        query.bindValue(QStringLiteral(":amarr_starship_engineering"), industrySkills.mAmarrStarshipEngineering);
        query.bindValue(QStringLiteral(":caldari_starship_engineering"), industrySkills.mCaldariStarshipEngineering);
        query.bindValue(QStringLiteral(":gallente_starship_engineering"), industrySkills.mGallenteStarshipEngineering);
        query.bindValue(QStringLiteral(":minmatar_starship_engineering"), industrySkills.mMinmatarStarshipEngineering);
        query.bindValue(QStringLiteral(":electromagnetic_physics"), industrySkills.mElectromagneticPhysics);
        query.bindValue(QStringLiteral(":electronic_engineering"), industrySkills.mElectronicEngineering);
        query.bindValue(QStringLiteral(":graviton_physics"), industrySkills.mGravitonPhysics);
        query.bindValue(QStringLiteral(":high_energy_physics"), industrySkills.mHighEnergyPhysics);
        query.bindValue(QStringLiteral(":hydromagnetic_physics"), industrySkills.mHydromagneticPhysics);
        query.bindValue(QStringLiteral(":laser_physics"), industrySkills.mLaserPhysics);
        query.bindValue(QStringLiteral(":mechanical_engineering"), industrySkills.mMechanicalEngineering);
        query.bindValue(QStringLiteral(":molecular_engineering"), industrySkills.mMolecularEngineering);
        query.bindValue(QStringLiteral(":nuclear_physics"), industrySkills.mNuclearPhysics);
        query.bindValue(QStringLiteral(":plasma_physics"), industrySkills.mPlasmaPhysics);
        query.bindValue(QStringLiteral(":quantum_physics"), industrySkills.mQuantumPhysics);
        query.bindValue(QStringLiteral(":rocket_science"), industrySkills.mRocketScience);
        query.bindValue(QStringLiteral(":alpha_clone"), entity.isAlphaClone());
    }

    void CharacterRepository::bindPositionalValues(const Character &entity, QSqlQuery &query) const
    {
        const auto orderAmountSkills = entity.getOrderAmountSkills();
        const auto tradeRangeSkills = entity.getTradeRangeSkills();
        const auto feeSkills = entity.getFeeSkills();
        const auto contractSkills = entity.getContractSkills();
        const auto reprocessingSkills = entity.getReprocessingSkills();
        const auto industrySkills = entity.getIndustrySkills();

        if (entity.getId() != Character::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getName());
        query.addBindValue(entity.getCorporationName());
        query.addBindValue(entity.getCorporationId());
        query.addBindValue(entity.getRace());
        query.addBindValue(entity.getBloodline());
        
        query.addBindValue(entity.getGender());
        query.addBindValue(entity.getISK());
        query.addBindValue(entity.getCorpStanding());
        query.addBindValue(entity.getFactionStanding());
        query.addBindValue(orderAmountSkills.mTrade);
        query.addBindValue(orderAmountSkills.mRetail);
        query.addBindValue(orderAmountSkills.mWholesale);
        query.addBindValue(orderAmountSkills.mTycoon);
        query.addBindValue(tradeRangeSkills.mMarketing);
        query.addBindValue(tradeRangeSkills.mProcurement);
        query.addBindValue(tradeRangeSkills.mDaytrading);
        query.addBindValue(tradeRangeSkills.mVisibility);
        query.addBindValue(feeSkills.mAccounting);
        query.addBindValue(feeSkills.mBrokerRelations);
        query.addBindValue(feeSkills.mMarginTrading);
        query.addBindValue(contractSkills.mContracting);
        query.addBindValue(contractSkills.mCorporationContracting);
        query.addBindValue(entity.isEnabled());
        query.addBindValue((entity.getBuyBrokersFee()) ? (*entity.getBuyBrokersFee()) : (QVariant{QVariant::Double}));
        query.addBindValue((entity.getSellBrokersFee()) ? (*entity.getSellBrokersFee()) : (QVariant{QVariant::Double}));
        query.addBindValue(entity.getReprocessingImplantBonus());
        query.addBindValue(reprocessingSkills.mArkonorProcessing);
        query.addBindValue(reprocessingSkills.mBistotProcessing);
        query.addBindValue(reprocessingSkills.mCrokiteProcessing);
        query.addBindValue(reprocessingSkills.mDarkOchreProcessing);
        query.addBindValue(reprocessingSkills.mGneissProcessing);
        query.addBindValue(reprocessingSkills.mHedbergiteProcessing);
        query.addBindValue(reprocessingSkills.mHemorphiteProcessing);
        query.addBindValue(reprocessingSkills.mIceProcessing);
        query.addBindValue(reprocessingSkills.mJaspetProcessing);
        query.addBindValue(reprocessingSkills.mKerniteProcessing);
        query.addBindValue(reprocessingSkills.mMercoxitProcessing);
        query.addBindValue(reprocessingSkills.mOmberProcessing);
        query.addBindValue(reprocessingSkills.mPlagioclaseProcessing);
        query.addBindValue(reprocessingSkills.mPyroxeresProcessing);
        query.addBindValue(reprocessingSkills.mReprocessing);
        query.addBindValue(reprocessingSkills.mReprocessingEfficiency);
        query.addBindValue(reprocessingSkills.mScorditeProcessing);
        query.addBindValue(reprocessingSkills.mScrapmetalProcessing);
        query.addBindValue(reprocessingSkills.mSpodumainProcessing);
        query.addBindValue(reprocessingSkills.mVeldsparProcessing);
        query.addBindValue(entity.getManufacturingTimeImplantBonus());
        query.addBindValue(industrySkills.mIndustry);
        query.addBindValue(industrySkills.mAdvancedIndustry);
        query.addBindValue(industrySkills.mAdvancedSmallShipConstruction);
        query.addBindValue(industrySkills.mAdvancedMediumShipConstruction);
        query.addBindValue(industrySkills.mAdvancedLargeShipConstruction);
        query.addBindValue(industrySkills.mAvancedIndustrialShipConstruction);
        query.addBindValue(industrySkills.mAmarrStarshipEngineering);
        query.addBindValue(industrySkills.mCaldariStarshipEngineering);
        query.addBindValue(industrySkills.mGallenteStarshipEngineering);
        query.addBindValue(industrySkills.mMinmatarStarshipEngineering);
        query.addBindValue(industrySkills.mElectromagneticPhysics);
        query.addBindValue(industrySkills.mElectronicEngineering);
        query.addBindValue(industrySkills.mGravitonPhysics);
        query.addBindValue(industrySkills.mHighEnergyPhysics);
        query.addBindValue(industrySkills.mHydromagneticPhysics);
        query.addBindValue(industrySkills.mLaserPhysics);
        query.addBindValue(industrySkills.mMechanicalEngineering);
        query.addBindValue(industrySkills.mMolecularEngineering);
        query.addBindValue(industrySkills.mNuclearPhysics);
        query.addBindValue(industrySkills.mPlasmaPhysics);
        query.addBindValue(industrySkills.mQuantumPhysics);
        query.addBindValue(industrySkills.mRocketScience);
        query.addBindValue(entity.isAlphaClone());
    }
}
