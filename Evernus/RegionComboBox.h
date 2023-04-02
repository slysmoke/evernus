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

#include <unordered_set>

#include <QComboBox>

namespace Evernus
{
    class EveDataProvider;

    class RegionComboBox
        : public QComboBox
    {
        Q_OBJECT

    public:
        using RegionList = std::unordered_set<uint>;

        RegionComboBox(const EveDataProvider &dataProvider,
                       const QString &settingsKey,
                       QWidget *parent = nullptr);
        explicit RegionComboBox(const EveDataProvider &dataProvider,
                                QWidget *parent = nullptr);
        RegionComboBox(const RegionComboBox &) = default;
        RegionComboBox(RegionComboBox &&) = default;
        virtual ~RegionComboBox() = default;

        RegionList getSelectedRegionList() const;
        void setSelectedRegionList(const RegionList &regions);

        RegionComboBox &operator =(const RegionComboBox &) = default;
        RegionComboBox &operator =(RegionComboBox &&) = default;

    private slots:
        void setRegionText();

    private:
        static const auto allRegionsIndex = 0;

        void addData(const EveDataProvider &dataProvider, const RegionList &saved);
    };
}
