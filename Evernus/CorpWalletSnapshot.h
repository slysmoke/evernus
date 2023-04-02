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

#include "Entity.h"

namespace Evernus
{
    class CorpWalletSnapshot
        : public Entity<uint>
    {
    public:
        using Entity::Entity;

        CorpWalletSnapshot() = default;
        CorpWalletSnapshot(const IdType &id, double balance);
        CorpWalletSnapshot(const CorpWalletSnapshot &) = default;
        CorpWalletSnapshot(CorpWalletSnapshot &&) = default;
        virtual ~CorpWalletSnapshot() = default;

        QDateTime getTimestamp() const;
        void setTimestamp(const QDateTime &dt);

        quint64 getCorporationId() const noexcept;
        void setCorporationId(quint64 id) noexcept;

        double getBalance() const noexcept;
        void setBalance(double balance) noexcept;

        CorpWalletSnapshot &operator =(const CorpWalletSnapshot &) = default;
        CorpWalletSnapshot &operator =(CorpWalletSnapshot &&) = default;

    private:
        QDateTime mTimestamp;
        quint64 mCorporationId = 0;
        double mBalance = 0.;
    };
}
