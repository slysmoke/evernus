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

#include <unordered_map>
#include <vector>

#include <QThread>

#include "ExternalOrder.h"

namespace Evernus
{
    class ExternalOrder;

    class MarketLogExternalOrderImporterThread
        : public QThread
    {
        Q_OBJECT

    public:
        typedef std::vector<ExternalOrder> ExternalOrderList;

        using QThread::QThread;
        virtual ~MarketLogExternalOrderImporterThread() = default;

    signals:
        void finished(const ExternalOrderList &orders);
        void error(const QString &info);

    protected:
        virtual void run() override;

    private:
        typedef std::unordered_map<EveType::IdType, QDateTime> LogTimeMap;

        static void getExternalOrder(const QString &logPath, ExternalOrderList &prices, bool deleteLog, LogTimeMap &timeMap);
    };
}

Q_DECLARE_METATYPE(Evernus::MarketLogExternalOrderImporterThread::ExternalOrderList);
