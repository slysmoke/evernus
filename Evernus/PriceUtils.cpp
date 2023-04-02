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
#include <random>
#include <cmath>
#include <string> 
#include <cstdlib>
#include <iostream>


#include <QSettings>

#include "PriceSettings.h"
#include "Character.h"

#include "PriceUtils.h"

namespace Evernus
{
    namespace PriceUtils
    {
        Taxes calculateTaxes(const Character &character)
        {
            const auto feeSkills = character.getFeeSkills();
            const auto salesTax = (0.08 - feeSkills.mAccounting * 0.0088);

            auto calcBrokersFee = [&](auto customBrokersFee) {
                return (customBrokersFee) ? (*customBrokersFee) : (0.03 - (feeSkills.mBrokerRelations * 0.003 +
                    0.0003 * character.getFactionStanding() + 0.0002 * character.getCorpStanding()));
            };

            return Taxes{calcBrokersFee(character.getBuyBrokersFee()), calcBrokersFee(character.getSellBrokersFee()), salesTax};
        }

		double round_to_digits(double value, int digits)
		{
			if (value == 0.0) // otherwise it will return 'nan' due to the log10() of zero
				return 0.0;

			double factor = pow(10.0, digits - ceil(log10(fabs(value))));
			return round(value * factor) / factor;
		}



		double getPriceStep(double price) {
			double order = pow(10, floor(log(price) / 2.302585092994046 + 0.000000001));
			return order * .001;
		}

        double getBuyPrice(double buyPrice, const Taxes &taxes, bool limitOrder)
        {
            return (limitOrder) ? (buyPrice + buyPrice * taxes.mBuyBrokerFee) : (buyPrice);
        }

        double getSellPrice(double sellPrice, const Taxes &taxes, bool limitOrder)
        {
            const auto taxedRevenue = sellPrice * (1. - taxes.mSalesTax);
            return (limitOrder) ? (taxedRevenue - sellPrice * taxes.mSellBrokerFee) : (taxedRevenue);
        }

        double getMargin(double cost, double price, const Taxes &taxes, bool limitOrder)
        {
            const auto realCost = getBuyPrice(cost, taxes, limitOrder);
            const auto realPrice = getSellPrice(price, taxes, limitOrder);
            return 100. * (realPrice - realCost) / realPrice;
        }

        double getPriceDelta()
        {
            QSettings settings;

            const auto priceDeltaRandom
                = settings.value(PriceSettings::priceDeltaRandomKey, PriceSettings::priceDeltaRandomDefault).toDouble();
            auto priceDelta = settings.value(PriceSettings::priceDeltaKey, PriceSettings::priceDeltaDefault).toDouble();

            if (!qFuzzyIsNull(priceDeltaRandom))
            {
                std::random_device rd;
                std::mt19937 gen{rd()};
                std::uniform_real_distribution<> dis(0., priceDeltaRandom);

                priceDelta += dis(gen);
            }

            return priceDelta;
        }
    }
}
