#include <QDomElement>
#include <QSettings>

#include "ImportSettings.h"

namespace Evernus
{
    namespace APIDomParser
    {
        template<>
        Character parse<Character>(const QDomElement &node)
        {
            auto balance = node.firstChildElement("balance").text();
            balance.remove('.');

            Character result;
            result.setId(node.firstChildElement("characterID").text().toULongLong());
            result.setName(node.firstChildElement("name").text());
            result.setCorporationName(node.firstChildElement("corporationName").text());
            result.setRace(node.firstChildElement("race").text());
            result.setBloodline(node.firstChildElement("bloodLine").text());
            result.setAncestry(node.firstChildElement("ancestry").text());
            result.setGender(node.firstChildElement("gender").text());
            result.setISK(balance.toLongLong());

            QSettings settings;
            if (settings.value(ImportSettings::importSkillsKey, true).toBool())
            {
                const auto rowsets = node.elementsByTagName("rowset");

                QDomElement skillsElement;
                for (auto i = 0; i < rowsets.count(); ++i)
                {
                    const auto element = rowsets.at(i).toElement();
                    if (element.attribute("name") == "skills")
                    {
                        skillsElement = element;
                        break;
                    }
                }

                CharacterData::OrderAmountSkills orderAmountSkills;
                CharacterData::TradeRangeSkills tradeRangeSkills;
                CharacterData::FeeSkills feeSkills;
                CharacterData::ContractSkills contractSkills;

                const auto skills = node.elementsByTagName("row");
                for (auto i = 0; i < skills.count(); ++i)
                {
                    const auto element = skills.at(i).toElement();
                    switch (element.attribute("typeID").toInt()) {
                    case 3443:
                        orderAmountSkills.mTrade = element.attribute("level").toInt();
                        break;
                    case 3444:
                        orderAmountSkills.mRetail = element.attribute("level").toInt();
                        break;
                    case 16596:
                        orderAmountSkills.mWholesale = element.attribute("level").toInt();
                        break;
                    case 18580:
                        orderAmountSkills.mTycoon = element.attribute("level").toInt();
                        break;
                    case 16598:
                        tradeRangeSkills.mMarketing = element.attribute("level").toInt();
                        break;
                    case 16594:
                        tradeRangeSkills.mProcurement = element.attribute("level").toInt();
                        break;
                    case 16595:
                        tradeRangeSkills.mDaytrading = element.attribute("level").toInt();
                        break;
                    case 3447:
                        tradeRangeSkills.mVisibility = element.attribute("level").toInt();
                        break;
                    case 16622:
                        feeSkills.mAccounting = element.attribute("level").toInt();
                        break;
                    case 3446:
                        feeSkills.mBrokerRelations = element.attribute("level").toInt();
                        break;
                    case 16597:
                        feeSkills.mMarginTrading = element.attribute("level").toInt();
                        break;
                    case 25235:
                        contractSkills.mContracting = element.attribute("level").toInt();
                        break;
                    case 25233:
                        contractSkills.mCorporationContracting = element.attribute("level").toInt();
                    }
                }

                result.setOrderAmountSkills(std::move(orderAmountSkills));
                result.setTradeRangeSkills(std::move(tradeRangeSkills));
                result.setFeeSkills(std::move(feeSkills));
                result.setContractSkills(std::move(contractSkills));
            }

            return result;
        }
    }
}
