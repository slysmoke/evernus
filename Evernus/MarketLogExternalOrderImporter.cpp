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
#include "ExternalOrder.h"

#include "MarketLogExternalOrderImporter.h"

namespace Evernus
{
    MarketLogExternalOrderImporter::~MarketLogExternalOrderImporter()
    {
        try
        {
            for (auto &thread : mScanningThreads)
            {
                thread->requestInterruption();
                thread->wait(10 * 1000);
                if (thread->isRunning())
                    thread->terminate();
            }
        }
        catch (...)
        {
            std::terminate();
        }
    }

    void MarketLogExternalOrderImporter::fetchExternalOrders(Character::IdType id, const TypeLocationPairs &target) const
    {
        Q_UNUSED(id);
        Q_UNUSED(target);

        auto thread = std::make_unique<MarketLogExternalOrderImporterThread>();
        connect(thread.get(), &MarketLogExternalOrderImporterThread::finished, this, &MarketLogExternalOrderImporter::threadFinished);
        connect(thread.get(), &MarketLogExternalOrderImporterThread::error, this, &MarketLogExternalOrderImporter::threadError);
        thread->start();

        mScanningThreads.emplace_back(std::move(thread));
    }

    void MarketLogExternalOrderImporter::threadFinished(const ExternalOrderList &orders)
    {
        deleteThread(static_cast<MarketLogExternalOrderImporterThread *>(sender()));
        emit externalOrdersChanged({}, orders);
    }

    void MarketLogExternalOrderImporter::threadError(const QString &info)
    {
        deleteThread(static_cast<MarketLogExternalOrderImporterThread *>(sender()));
        emit externalOrdersChanged(info, {});
    }

    void MarketLogExternalOrderImporter::deleteThread(MarketLogExternalOrderImporterThread *thread)
    {
        thread->deleteLater();

        const auto it = std::find_if(std::begin(mScanningThreads), std::end(mScanningThreads), [thread](const auto &entry) {
            return entry.get() == thread;
        });

        Q_ASSERT(it != std::end(mScanningThreads));
        it->release();
        mScanningThreads.erase(it);
    }
}
