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

#include "Character.h"

namespace Evernus
{
    class CorpMarketOrderValueSnapshot
        : public Entity<uint>
    {
    public:
        using Entity::Entity;

        CorpMarketOrderValueSnapshot() = default;
        CorpMarketOrderValueSnapshot(const CorpMarketOrderValueSnapshot &) = default;
        CorpMarketOrderValueSnapshot(CorpMarketOrderValueSnapshot &&) = default;
        virtual ~CorpMarketOrderValueSnapshot() = default;

        QDateTime getTimestamp() const;
        void setTimestamp(const QDateTime &dt);

        quint64 getCorporationId() const noexcept;
        void setCorporationId(quint64 id) noexcept;

        double getBuyValue() const noexcept;
        void setBuyValue(double value) noexcept;

        double getSellValue() const noexcept;
        void setSellValue(double value) noexcept;

        CorpMarketOrderValueSnapshot &operator =(const CorpMarketOrderValueSnapshot &) = default;
        CorpMarketOrderValueSnapshot &operator =(CorpMarketOrderValueSnapshot &&) = default;

    private:
        QDateTime mTimestamp;
        quint64 mCorporationId = 0;
        double mBuyValue = 0., mSellValue = 0.;
    };
}
