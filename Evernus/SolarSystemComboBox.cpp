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
#include "EveDataProvider.h"

#include "SolarSystemComboBox.h"

namespace Evernus
{
    SolarSystemComboBox::SolarSystemComboBox(const EveDataProvider &dataProvider, QWidget *parent)
        : QComboBox{parent}
        , mDataProvider{dataProvider}
    {
        setEditable(true);
        setInsertPolicy(QComboBox::NoInsert);
    }

    void SolarSystemComboBox::setRegion(uint id)
    {
        clear();

        const auto systems = mDataProvider.getSolarSystemsForRegion(id);
        for (const auto &system : systems)
            addItem(system.second, system.first);
    }

    uint SolarSystemComboBox::getSelectedSolarSystem() const
    {
        return currentData().toUInt();
    }
}
