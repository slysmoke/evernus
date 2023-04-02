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

namespace Evernus
{
    class Character;

    namespace PriceUtils
    {
        struct Taxes
        {
            double mBuyBrokerFee;
            double mSellBrokerFee;
            double mSalesTax;
        };

        Taxes calculateTaxes(const Character &character);

        double getBuyPrice(double buyPrice, const Taxes &taxes, bool limitOrder = true);
        double getSellPrice(double sellPrice, const Taxes &taxes, bool limitOrder = true);
        double getMargin(double cost, double price, const Taxes &taxes, bool limitOrder = true);
		double round_to_digits(double value, int digits);
		double getPriceStep(double price);

        double getPriceDelta();
    }
}
