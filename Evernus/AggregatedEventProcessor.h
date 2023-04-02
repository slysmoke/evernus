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

#include <cstdint>

namespace Evernus
{
    class AggregatedEventProcessor final
    {
    public:
        AggregatedEventProcessor() = default;
        AggregatedEventProcessor(const AggregatedEventProcessor &) = default;
        AggregatedEventProcessor(AggregatedEventProcessor &&) = default;
        ~AggregatedEventProcessor() = default;

        void processEvents() const;

        AggregatedEventProcessor &operator =(const AggregatedEventProcessor &) = default;
        AggregatedEventProcessor &operator =(AggregatedEventProcessor &&) = default;

    private:
        static const std::uint_fast8_t requestsPerEventsUpdate = 10;

        mutable std::uint_fast8_t mEventLoopCounter = requestsPerEventsUpdate;
    };
}
