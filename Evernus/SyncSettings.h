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

#if defined(EVERNUS_DROPBOX_APP_KEY) && defined(EVERNUS_DROPBOX_APP_SECRET)
#   define EVERNUS_DROPBOX_ENABLED 1
#endif

namespace Evernus
{
    namespace SyncSettings
    {
        const auto enabledOnStartupDefault = false;
        const auto enabledOnShutdownDefault = false;

        const auto enabledOnStartupKey = "sync/enabledOnStartup";
        const auto enabledOnShutdownKey = "sync/enabledOnShutdown";
        const auto dbTokenKey = "sync/db/token";
        const auto firstSyncKey = "sync/first";
    }
}
