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

#include <QDateTime>

#include "WalletJournalEntry.h"
#include "Character.h"
#include "EveType.h"
#include "Entity.h"

namespace Evernus
{
    class WalletTransaction
        : public Entity<quint64>
    {
    public:
        enum class Type
        {
            Buy,
            Sell
        };

        using Entity::Entity;

        WalletTransaction() = default;
        WalletTransaction(const WalletTransaction &) = default;
        WalletTransaction(WalletTransaction &&) = default;
        virtual ~WalletTransaction() = default;

        Character::IdType getCharacterId() const;
        void setCharacterId(Character::IdType id);

        QDateTime getTimestamp() const;
        void setTimestamp(const QDateTime &dt);

        uint getQuantity() const noexcept;
        void setQuantity(uint value) noexcept;

        EveType::IdType getTypeId() const;
        void setTypeId(EveType::IdType id);

        double getPrice() const noexcept;
        void setPrice(double value) noexcept;

        quint64 getClientId() const noexcept;
        void setClientId(quint64 id) noexcept;

        quint64 getLocationId() const noexcept;
        void setLocationId(quint64 id) noexcept;

        Type getType() const noexcept;
        void setType(Type type) noexcept;

        WalletJournalEntry::IdType getJournalId() const;
        void setJournalId(WalletJournalEntry::IdType id);

        quint64 getCorporationId() const noexcept;
        void setCorporationId(quint64 id) noexcept;

        bool isIgnored() const noexcept;
        void setIgnored(bool flag) noexcept;

        WalletTransaction &operator =(const WalletTransaction &) = default;
        WalletTransaction &operator =(WalletTransaction &&) = default;

    private:
        Character::IdType mCharacterId = Character::invalidId;
        QDateTime mTimestamp;
        uint mQuantity = 0;
        EveType::IdType mTypeId = EveType::invalidId;
        double mPrice = 0.;
        quint64 mClientId = 0;
        quint64 mLocationId = 0;
        Type mType = Type::Buy;
        WalletJournalEntry::IdType mJournalId = WalletJournalEntry::invalidId;
        quint64 mCorporationId = 0;
        bool mIgnored = false;
    };

    bool operator <(const WalletTransaction &a, const WalletTransaction &b);
}
