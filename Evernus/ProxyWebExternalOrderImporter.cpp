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
#include <QSettings>

#include "ProxyWebExternalOrderImporter.h"
#include "SSOUtils.h"

namespace Evernus
{
    ProxyWebExternalOrderImporter::ProxyWebExternalOrderImporter(const EveDataProvider &dataProvider,
                                                                 ESIInterfaceManager &interfaceManager,
                                                                 QObject *parent)
        : ExternalOrderImporter{parent}
        , mDataProvider{dataProvider}
        , mESIIndividualImporter{
            std::make_unique<ESIIndividualExternalOrderImporter>(mDataProvider, interfaceManager, parent)
        }
        , mESIWholeImporter{
            std::make_unique<ESIWholeExternalOrderImporter>(mDataProvider, interfaceManager, parent)
        }
    {
        setCurrentImporter();

        connectImporter(*mESIIndividualImporter);
        connectImporter(*mESIWholeImporter);
    }

    void ProxyWebExternalOrderImporter::fetchExternalOrders(Character::IdType id, const TypeLocationPairs &target) const
    {
        if (mCurrentOrderImportType == ImportSettings::MarketOrderImportType::Auto)
        {
            if (SSOUtils::useWholeMarketImport(target, mDataProvider))
                mESIWholeImporter->fetchExternalOrders(id, target);
            else
                mESIIndividualImporter->fetchExternalOrders(id, target);
        }
        else if (mCurrentOrderImportType == ImportSettings::MarketOrderImportType::Individual)
        {
            mESIIndividualImporter->fetchExternalOrders(id, target);
        }
        else
        {
            mESIWholeImporter->fetchExternalOrders(id, target);
        }
    }

    void ProxyWebExternalOrderImporter::handleNewPreferences()
    {
        setCurrentImporter();
    }

    template<class T>
    void ProxyWebExternalOrderImporter::connectImporter(T &importer)
    {
        connect(&importer, &T::externalOrdersChanged,
                this, &ProxyWebExternalOrderImporter::externalOrdersChanged);
        connect(&importer, &T::statusChanged,
                this, &ProxyWebExternalOrderImporter::statusChanged);
    }

    void ProxyWebExternalOrderImporter::setCurrentImporter()
    {
        QSettings settings;

        mCurrentOrderImportType
            = static_cast<ImportSettings::MarketOrderImportType>(
                settings.value(ImportSettings::marketOrderImportTypeKey, static_cast<int>(ImportSettings::marketOrderImportTypeDefault)).toInt());
    }
}
