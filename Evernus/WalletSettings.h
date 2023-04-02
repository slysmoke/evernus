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
    namespace WalletSettings
    {
        const auto deleteOldJournalDefault = true;
        const auto deleteOldTransactionsDefault = true;
        const auto oldJournalDaysDefault = 60;
        const auto oldTransactionsDaysDefault = 60;

        const auto deleteOldJournalKey = "wallet/journal/deleteOld";
        const auto oldJournalDaysKey = "wallet/journal/oldDays";

        const auto deleteOldTransactionsKey = "wallet/transactions/deleteOld";
        const auto oldTransactionsDaysKey = "wallet/transactions/oldDays";
    }
}
