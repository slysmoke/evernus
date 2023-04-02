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

#include "ContractFilterProxyModel.h"

class QLabel;

namespace Evernus
{
    class StyledTreeView;
    class ContractModel;

    class ContractView
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit ContractView(const QString &objectName, QWidget *parent = nullptr);
        virtual ~ContractView() = default;

        void setModel(ContractModel *model);

    public slots:
        void setFilterWildcard(const QString &pattern);
        void setStatusFilter(const ContractFilterProxyModel::StatusFilters &filter);

    private slots:
        void updateInfo();

    private:
        StyledTreeView *mView = nullptr;
        QLabel *mTotalContractsLabel = nullptr;
        QLabel *mTotalPriceLabel = nullptr;
        QLabel *mTotalRewardLabel = nullptr;
        QLabel *mTotalCollateralLabel = nullptr;
        QLabel *mTotalVolumeLabel = nullptr;

        ContractFilterProxyModel mProxy;
        ContractModel *mModel = nullptr;
    };
}
