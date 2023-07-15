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
#include "EveTypeProvider.h"
#include "TypeLookupUtils.h"

#include "LookupActionGroup.h"

namespace Evernus
{
    LookupActionGroup::LookupActionGroup(const EveTypeProvider &typeProvider, QObject *parent)
        : QActionGroup{parent}
        , mTypeProvider{typeProvider}
    {
        auto action = addAction(tr("Lookup item on evetycoon.com"));
        connect(action, &QAction::triggered, this, &LookupActionGroup::lookupOnEveMarketdata);

        action = addAction(tr("Lookup item on adam4eve.eu"));
        connect(action, &QAction::triggered, this, &LookupActionGroup::lookupOnAdam4Eve);

        action = addAction(tr("Lookup item on evemarketer.com"));
        connect(action, &QAction::triggered, this, &LookupActionGroup::lookupOnEveMarketer);
    }

    void LookupActionGroup::lookupOnEveMarketdata()
    {
        lookupOnWeb(TypeLookupUtils::eveMarketdataUrl);
    }

    void LookupActionGroup::lookupOnEveMarketer()
    {
        lookupOnWeb(TypeLookupUtils::eveMarketerUrl);
    }

    void LookupActionGroup::lookupOnAdam4Eve()
    {
        lookupOnWeb(TypeLookupUtils::adam4eveUrl);
    }

    void LookupActionGroup::lookupOnWeb(const QString &baseUrl) const
    {
        TypeLookupUtils::lookupType(mTypeProvider.getTypeId(), baseUrl);
    }
}
