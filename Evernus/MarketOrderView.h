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

#include "MarketOrderFilterProxyModel.h"
#include "EveTypeProvider.h"
#include "MarketOrder.h"
#include "EveType.h"

class QItemSelectionModel;
class QActionGroup;
class QModelIndex;
class QColor;

namespace Evernus
{
    class MarketOrdersInfoWidget;
    class LookupActionGroup;
    class ItemCostProvider;
    class MarketOrderModel;
    class EveDataProvider;
    class StyledTreeView;

    class MarketOrderView
        : public QWidget
        , public EveTypeProvider
    {
        Q_OBJECT

    public:
        MarketOrderView(const EveDataProvider &dataProvider,
                        const QString &objectName,
                        MarketOrdersInfoWidget *infoWidget,
                        const ItemCostProvider &itemCostProvider,
                        QWidget *parent = nullptr);
        virtual ~MarketOrderView() = default;

        virtual EveType::IdType getTypeId() const override;

        QItemSelectionModel *getSelectionModel() const;
        const QAbstractProxyModel &getProxyModel() const;

        void setModel(MarketOrderModel *model);

        void expandAll();

        void sortByColumn(int column, Qt::SortOrder order);

    signals:
        void closeOrderInfo();

        void statusFilterChanged(const MarketOrderFilterProxyModel::StatusFilters &filter);
        void priceStatusFilterChanged(const MarketOrderFilterProxyModel::PriceStatusFilters &filter);

        void textFilterChanged(const QString &text, bool script);

        void scriptError(const QString &message);

        void showExternalOrders(EveType::IdType id);
        void showInEve(EveType::IdType id, Character::IdType ownerId) const;

        void itemSelected();

        void notesChanged(MarketOrder::IdType id, const QString &notes);
        void stationChanged(MarketOrder::IdType orderId, uint stationId);
        void colorTagChanged(MarketOrder::IdType orderId, const QColor &color);

    public slots:
        void executeFPC();
        void executeBackwardFPC();

    private slots:
        void showPriceInfo(const QModelIndex &index);

        void selectOrder(const QItemSelection &selected);
        void removeOrders();
        void showExternalOrdersForCurrent();
        void showInEveForCurrent();

        void handleReset();

        void changeNotesForCurrent();
        void changeCustomStationForCurrent();
        void changeColorTagForCurrent(const QColor &color);

    private:
        const EveDataProvider &mDataProvider;

        StyledTreeView *mView = nullptr;
        MarketOrdersInfoWidget *mInfoWidget = nullptr;

        MarketOrderModel *mSource = nullptr;
        MarketOrderFilterProxyModel mProxy;

        QAction *mRemoveOrderAct = nullptr;
        QAction *mShowExternalOrdersAct = nullptr;
        QAction *mShowInEveAct = nullptr;
        QAction *mChangeNotesAct = nullptr;
        QAction *mChangeCustomStationAct = nullptr;
        QAction *mChangeColorTagAct = nullptr;
        QAction *mRemoveColorTagAct = nullptr;
        LookupActionGroup *mLookupGroup = nullptr;

        void showInEveFor(const QModelIndex &index) const;

        void executeFPC(int delta);
    };
}
