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

#include "ContractFilterProxyModel.h"
#include "AssignedContractModel.h"
#include "CharacterBoundWidget.h"
#include "IssuedContractModel.h"

class QPushButton;

namespace Evernus
{
    class FilterTextRepository;
    class CharacterRepository;
    class CacheTimerProvider;
    class ContractProvider;
    class EveDataProvider;

    class ContractWidget
        : public CharacterBoundWidget
    {
        Q_OBJECT

    public:
        ContractWidget(const CacheTimerProvider &cacheTimerProvider,
                       const EveDataProvider &dataProvider,
                       const ContractProvider &contractProvider,
                       const FilterTextRepository &filterRepo,
                       const CharacterRepository &characterRepo,
                       bool corp,
                       QWidget *parent = nullptr);
        virtual ~ContractWidget() = default;

    public slots:
        void updateData();

    private slots:
        void setStatusFilter(const ContractFilterProxyModel::StatusFilters &filter);

    private:
        IssuedContractModel mIssuedModel;
        AssignedContractModel mAssignedModel;

        QPushButton *mStatusFilterBtn = nullptr;

        virtual void handleNewCharacter(Character::IdType id) override;

        static QString getStatusFilterButtonText(const ContractFilterProxyModel::StatusFilters &filter);
    };
}
