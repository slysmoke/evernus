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

#include <algorithm>

#include <QString>

namespace Evernus
{
    namespace NetworkSettings
    {
        const auto useProxyDefault = false;
        const auto defaultAPIProvider = "https://api.eveonline.com";
        const auto maxReplyTimeDefault = 1800u;
        const auto ignoreSslErrorsDefault = false;
        const auto maxRetriesDefault = 3u;
        const auto logESIRepliesDefault = false;
        const auto useHTTP2Default = true;

        const auto cryptKey = Q_UINT64_C(0x468c4a0e33a6fe01);

        const auto useProxyKey = QStringLiteral("network/useProxy");
        const auto proxyTypeKey = QStringLiteral("network/proxy/type");
        const auto proxyHostKey = QStringLiteral("network/proxy/host");
        const auto proxyPortKey = QStringLiteral("network/proxy/port");
        const auto proxyUserKey = QStringLiteral("network/proxy/user");
        const auto proxyPasswordKey = QStringLiteral("network/proxy/password");
        const auto maxReplyTimeKey = QStringLiteral("network/maxReplyTime");
        const auto ignoreSslErrorsKey = QStringLiteral("network/security/ignoreSslErrors");
        const auto maxRetriesKey = QStringLiteral("network/maxRetries");
        const auto logESIRepliesKey = QStringLiteral("network/logESIReplies");
        const auto useHTTP2Key = QStringLiteral("network/useHTTP2");
    }
}
