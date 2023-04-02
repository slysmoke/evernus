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
#include <QIcon>

#include "IconUtils.h"

namespace Evernus
{
    namespace IconUtils
    {
        QIcon getIconForMetaGroup(const QString &metaName)
        {
            if (metaName == "Tech II")
                return QIcon{":/images/meta_tech2.png"};
            else if (metaName == "Tech III")
                return QIcon{":/images/meta_tech3.png"};
            else if (metaName == "Faction")
                return QIcon{":/images/meta_faction.png"};
            else if (metaName == "Storyline")
                return QIcon{":/images/meta_storyline.png"};
            else if (metaName == "Officer")
                return QIcon{":/images/meta_officer.png"};
            else if (metaName == "Deadspace")
                return QIcon{":/images/meta_deadspace.png"};

            return QIcon{};
        }
    }
}
