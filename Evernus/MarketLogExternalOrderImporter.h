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

#include <memory>
#include <list>

#include "MarketLogExternalOrderImporterThread.h"
#include "ExternalOrderImporter.h"

namespace Evernus
{
    class MarketLogExternalOrderImporter
        : public ExternalOrderImporter
    {
        Q_OBJECT

    public:
        using ExternalOrderList = MarketLogExternalOrderImporterThread::ExternalOrderList;

        using ExternalOrderImporter::ExternalOrderImporter;
        virtual ~MarketLogExternalOrderImporter();

        virtual void fetchExternalOrders(Character::IdType id, const TypeLocationPairs &target) const override;

    private slots:
        void threadFinished(const ExternalOrderList &orders);
        void threadError(const QString &info);

    private:
        mutable std::list<std::unique_ptr<MarketLogExternalOrderImporterThread>> mScanningThreads;

        void deleteThread(MarketLogExternalOrderImporterThread *thread);
    };
}
