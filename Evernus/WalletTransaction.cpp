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
#include "WalletTransaction.h"

namespace Evernus
{
    Character::IdType WalletTransaction::getCharacterId() const
    {
        return mCharacterId;
    }

    void WalletTransaction::setCharacterId(Character::IdType id)
    {
        mCharacterId = id;
    }

    QDateTime WalletTransaction::getTimestamp() const
    {
        return mTimestamp;
    }

    void WalletTransaction::setTimestamp(const QDateTime &dt)
    {
        mTimestamp = dt;
    }

    uint WalletTransaction::getQuantity() const noexcept
    {
        return mQuantity;
    }

    void WalletTransaction::setQuantity(uint value) noexcept
    {
        mQuantity = value;
    }

    EveType::IdType WalletTransaction::getTypeId() const
    {
        return mTypeId;
    }

    void WalletTransaction::setTypeId(EveType::IdType id)
    {
        mTypeId = id;
    }

    double WalletTransaction::getPrice() const noexcept
    {
        return mPrice;
    }

    void WalletTransaction::setPrice(double value) noexcept
    {
        mPrice = value;
    }

    quint64 WalletTransaction::getClientId() const noexcept
    {
        return mClientId;
    }

    void WalletTransaction::setClientId(quint64 id) noexcept
    {
        mClientId = id;
    }

    quint64 WalletTransaction::getLocationId() const noexcept
    {
        return mLocationId;
    }

    void WalletTransaction::setLocationId(quint64 id) noexcept
    {
        mLocationId = id;
    }

    WalletTransaction::Type WalletTransaction::getType() const noexcept
    {
        return mType;
    }

    void WalletTransaction::setType(Type type) noexcept
    {
        mType = type;
    }

    WalletJournalEntry::IdType WalletTransaction::getJournalId() const
    {
        return mJournalId;
    }

    void WalletTransaction::setJournalId(WalletJournalEntry::IdType id)
    {
        mJournalId = id;
    }

    quint64 WalletTransaction::getCorporationId() const noexcept
    {
        return mCorporationId;
    }

    void WalletTransaction::setCorporationId(quint64 id) noexcept
    {
        mCorporationId = id;
    }

    bool WalletTransaction::isIgnored() const noexcept
    {
        return mIgnored;
    }

    void WalletTransaction::setIgnored(bool flag) noexcept
    {
        mIgnored = flag;
    }

    bool operator <(const WalletTransaction &a, const WalletTransaction &b)
    {
        return a.getId() < b.getId();
    }
}
