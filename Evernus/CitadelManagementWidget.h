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

#include <QWidget>

#include "CitadelManagementModel.h"
#include "Character.h"
#include "Citadel.h"

class QModelIndex;

namespace Evernus
{
    class CitadelAccessCache;
    class EveDataProvider;

    class CitadelManagementWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        using CitadelList = CitadelManagementModel::CitadelList;

        CitadelManagementWidget(const EveDataProvider &dataProvider,
                                const CitadelAccessCache &citadelAccessCache,
                                Character::IdType charId,
                                QWidget *parent = nullptr);
        CitadelManagementWidget(const CitadelManagementWidget &) = default;
        CitadelManagementWidget(CitadelManagementWidget &&) = default;
        virtual ~CitadelManagementWidget() = default;

        CitadelList getSelectedCitadels() const;

        CitadelManagementWidget &operator =(const CitadelManagementWidget &) = default;
        CitadelManagementWidget &operator =(CitadelManagementWidget &&) = default;

    signals:
        void citadelSelected(Citadel::IdType id);

    public slots:
        void refresh();

    private slots:
        void toggleActions(const QModelIndex &selection);

    private:
        CitadelManagementModel mModel;
    };
}
