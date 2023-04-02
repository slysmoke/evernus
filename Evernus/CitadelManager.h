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

#include <functional>
#include <vector>

#include <QString>

#include "StructureHuntInterface.h"

namespace Evernus
{
    class Citadel;

    class CitadelManager final
    {
    public:
        using Callback = std::function<void (std::vector<Citadel> &&citadels, const QString &error)>;

        CitadelManager() = default;
        CitadelManager(const CitadelManager &) = default;
        CitadelManager(CitadelManager &&) = default;
        ~CitadelManager() = default;

        void fetchCitadels(Callback callback) const;

        CitadelManager &operator =(const CitadelManager &) = default;
        CitadelManager &operator =(CitadelManager &&) = default;

    private:
        StructureHuntInterface mInterface;
    };
}
