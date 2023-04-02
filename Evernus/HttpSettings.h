/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Http Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Http Public License for more details.
 *
 *  You should have received a copy of the GNU Http Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <QtGlobal>

namespace Evernus
{
    namespace HttpSettings
    {
        const auto enabledDefault = false;
        const auto portDefault = 4633;

        const auto cryptKey = Q_UINT64_C(0xaab439c6740ee721);

        const auto enabledKey = "http/enabled";
        const auto portKey = "http/port";
        const auto userKey = "http/user";
        const auto passwordKey = "http/password";
    }
}
