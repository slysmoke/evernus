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

#include "AggregatedEventProcessor.h"
#include "ExternalOrderImporter.h"
#include "ProgressiveCounter.h"
#include "ExternalOrder.h"

namespace Evernus
{
    class CallbackExternalOrderImporter
        : public ExternalOrderImporter
    {
        Q_OBJECT

    public:
        using ExternalOrderImporter::ExternalOrderImporter;
        virtual ~CallbackExternalOrderImporter() = default;

    protected:
        mutable ProgressiveCounter mCounter;
        mutable bool mPreparingRequests = false;

        mutable std::vector<ExternalOrder> mResult;
        mutable QStringList mAggregatedErrors;

        void processResult(std::vector<ExternalOrder> &&orders, const QString &errorText) const;
        void processEvents() const;

    private:
        AggregatedEventProcessor mEventProcessor;

        virtual void filterOrders(std::vector<ExternalOrder> &orders) const;
    };
}
