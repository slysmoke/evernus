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
#include <QtDebug>

#include "CallbackExternalOrderImporter.h"

namespace Evernus
{
    void CallbackExternalOrderImporter::processResult(std::vector<ExternalOrder> &&orders, const QString &errorText) const
    {
        if (mCounter.advanceAndCheckBatch())
            emit statusChanged(tr("EVE import: waiting for %1 server replies").arg(mCounter.getCount()));

        qDebug() << "Got reply," << mCounter.getCount() << "remaining.";

        if (Q_UNLIKELY(!errorText.isEmpty()))
        {
            mAggregatedErrors << errorText;

            if (Q_UNLIKELY(mCounter.isEmpty()))
            {
                emit externalOrdersChanged(mAggregatedErrors.join("\n"), mResult);

                mAggregatedErrors.clear();
                mResult.clear();
            }

            return;
        }

        filterOrders(orders);

        mResult.reserve(mResult.size() + orders.size());
        mResult.insert(std::end(mResult),
                       std::make_move_iterator(std::begin(orders)),
                       std::make_move_iterator(std::end(orders)));

        if (mCounter.isEmpty() && !mPreparingRequests)
        {
            emit externalOrdersChanged(mAggregatedErrors.join("\n"), mResult);

            mAggregatedErrors.clear();
            mResult.clear();
        }
    }

    void CallbackExternalOrderImporter::processEvents() const
    {
        mEventProcessor.processEvents();
    }

    void CallbackExternalOrderImporter::filterOrders(std::vector<ExternalOrder> &orders) const
    {
        Q_UNUSED(orders);
    }
}
