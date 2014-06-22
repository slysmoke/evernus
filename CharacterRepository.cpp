#include <QSqlQuery>

#include "KeyRepository.h"

#include "CharacterRepository.h"

namespace Evernus
{
    QString CharacterRepository::getTableName() const
    {
        return "characters";
    }

    QString CharacterRepository::getIdColumn() const
    {
        return "id";
    }

    Character CharacterRepository::populate(const QSqlRecord &record) const
    {
        Character character;
        character.setKeyId(record.value("key_id").value<Key::IdType>());
        character.setName(record.value("name").toString());
        character.setCorporationName(record.value("corporation_name").toString());
        character.setRace(record.value("race").toString());
        character.setBloodline(record.value("bloodline").toString());
        character.setAncestry(record.value("ancestry").toString());
        character.setGender(record.value("gender").toString());
        character.setSkillPoints(record.value("skill_points").toUInt());
        character.setISK(Character::ISKType{record.value("isk").toString().toLatin1().data()});
        character.setCorpStanding(record.value("corp_standing").toFloat());
        character.setFactionStanding(record.value("faction_standing").toFloat());
        character.setEnabled(record.value("enabled").toBool());

        Character::OrderAmountSkills orderAmountSkills;
        orderAmountSkills.mTrade = record.value("trade_skill").toInt();
        orderAmountSkills.mRetail = record.value("retail_skill").toInt();
        orderAmountSkills.mWholesale = record.value("wholesale_skill").toInt();
        orderAmountSkills.mTycoon = record.value("tycoon_skill").toInt();

        Character::TradeRangeSkills tradeRangeSkills;
        tradeRangeSkills.mMarketing = record.value("marketing_skill").toInt();
        tradeRangeSkills.mProcurement = record.value("procurement_skill").toInt();
        tradeRangeSkills.mDaytrading = record.value("daytrading_skill").toInt();
        tradeRangeSkills.mVisibility = record.value("visibility_skill").toInt();

        Character::FeeSkills feeSkills;
        feeSkills.mAccounting = record.value("accounting_skill").toInt();
        feeSkills.mBrokerRelations = record.value("broker_relations_skill").toInt();
        feeSkills.mMarginTrading = record.value("margin_trading_skill").toInt();

        Character::ContractSkills contractSkills;
        contractSkills.mContracting = record.value("contracting_skill").toInt();
        contractSkills.mCorporationContracting = record.value("corporation_contracting_skill").toInt();

        character.setOrderAmountSkills(std::move(orderAmountSkills));
        character.setTradeRangeSkills(std::move(tradeRangeSkills));
        character.setFeeSkills(std::move(feeSkills));
        character.setContractSkills(std::move(contractSkills));
        character.setNew(false);

        return character;
    }

    void CharacterRepository::create(const KeyRepository &keyRepository) const
    {
        exec(QString{R"(CREATE TABLE IF NOT EXISTS %1 (
            id BIGINT PRIMARY KEY,
            key_id INTEGER NULL REFERENCES %2(id) ON UPDATE SET NULL ON DELETE SET NULL,
            name TEXT NOT NULL,
            corporation_name TEXT NOT NULL,
            race TEXT NOT NULL,
            bloodline TEXT NOT NULL,
            ancestry TEXT NOT NULL,
            gender TEXT NOT NULL,
            skill_points INTEGER NOT NULL,
            isk NUMERIC NOT NULL,
            corp_standing FLOAT NOT NULL,
            faction_standing FLOAT NOT NULL,
            trade_skill TINYINT NOT NULL,
            retail_skill TINYINT NOT NULL,
            wholesale_skill TINYINT NOT NULL,
            tycoon_skill TINYINT NOT NULL,
            marketing_skill TINYINT NOT NULL,
            procurement_skill TINYINT NOT NULL,
            daytrading_skill TINYINT NOT NULL,
            visibility_skill TINYINT NOT NULL,
            accounting_skill TINYINT NOT NULL,
            broker_relations_skill TINYINT NOT NULL,
            margin_trading_skill TINYINT NOT NULL,
            contracting_skill TINYINT NOT NULL,
            corporation_contracting_skill TINYINT NOT NULL,
            enabled TINYINT NOT NULL
        ))"}.arg(getTableName()).arg(keyRepository.getTableName()));

        exec(QString{"CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(key_id)"}.arg(getTableName()).arg(keyRepository.getTableName()));
    }

    QStringList CharacterRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "key_id"
            << "name"
            << "corporation_name"
            << "race"
            << "bloodline"
            << "ancestry"
            << "gender"
            << "skill_points"
            << "isk"
            << "corp_standing"
            << "faction_standing"
            << "trade_skill"
            << "retail_skill"
            << "wholesale_skill"
            << "tycoon_skill"
            << "marketing_skill"
            << "procurement_skill"
            << "daytrading_skill"
            << "visibility_skill"
            << "accounting_skill"
            << "broker_relations_skill"
            << "margin_trading_skill"
            << "contracting_skill"
            << "corporation_contracting_skill"
            << "enabled";
    }

    void CharacterRepository::bindValues(const Character &entity, QSqlQuery &query) const
    {
        const auto keyId = entity.getKeyId();

        const auto orderAmountSkills = entity.getOrderAmountSkills();
        const auto tradeRangeSkills = entity.getTradeRangeSkills();
        const auto feeSkills = entity.getFeeSkills();
        const auto contractSkills = entity.getContractSkills();

        query.bindValue(":id", entity.getId());
        query.bindValue(":key_id", (keyId) ? (*keyId) : (QVariant{QVariant::UInt}));
        query.bindValue(":name", entity.getName());
        query.bindValue(":corporation_name", entity.getCorporationName());
        query.bindValue(":race", entity.getRace());
        query.bindValue(":bloodline", entity.getBloodline());
        query.bindValue(":ancestry", entity.getAncestry());
        query.bindValue(":gender", entity.getGender());
        query.bindValue(":skill_points", entity.getSkillPoints());
        query.bindValue(":isk", QString::fromStdString(entity.getISK().str(20, 0)));
        query.bindValue(":corp_standing", entity.getCorpStanding());
        query.bindValue(":faction_standing", entity.getFactionStanding());
        query.bindValue(":trade_skill", orderAmountSkills.mTrade);
        query.bindValue(":retail_skill", orderAmountSkills.mRetail);
        query.bindValue(":wholesale_skill", orderAmountSkills.mWholesale);
        query.bindValue(":tycoon_skill", orderAmountSkills.mTycoon);
        query.bindValue(":marketing_skill", tradeRangeSkills.mMarketing);
        query.bindValue(":procurement_skill", tradeRangeSkills.mProcurement);
        query.bindValue(":daytrading_skill", tradeRangeSkills.mDaytrading);
        query.bindValue(":visibility_skill", tradeRangeSkills.mVisibility);
        query.bindValue(":accounting_skill", feeSkills.mAccounting);
        query.bindValue(":broker_relations_skill", feeSkills.mBrokerRelations);
        query.bindValue(":margin_trading_skill", feeSkills.mMarginTrading);
        query.bindValue(":contracting_skill", contractSkills.mContracting);
        query.bindValue(":corporation_contracting_skill", contractSkills.mCorporationContracting);
        query.bindValue(":enabled", entity.isEnabled());
    }
}
