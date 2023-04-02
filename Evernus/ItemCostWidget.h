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

#include <QSortFilterProxyModel>
#include <QWidget>

#include "ItemCostModel.h"
#include "Character.h"

class QSortFilterProxyModel;
class QItemSelection;
class QPushButton;
class QLineEdit;

namespace Evernus
{
    class ItemCostProvider;
    class EveDataProvider;
    class StyledTreeView;
    class ItemCost;
    class EveType;

    class ItemCostWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        ItemCostWidget(const ItemCostProvider &costProvider,
                       const EveDataProvider &eveDataProvider,
                       QWidget *parent = nullptr);
        virtual ~ItemCostWidget() = default;

    public slots:
        void setCharacter(Character::IdType id);

        void updateData();

    private slots:
        void addCost();
        void editCost();
        void deleteCost();
        void deleteAllCost();

        void selectCost(const QItemSelection &selected, const QItemSelection &deselected);

        void applyWildcard();

        void importCsv();
        void exportCsv();

    private:
        const ItemCostProvider &mCostProvider;
        const EveDataProvider &mEveDataProvider;

        QPushButton *mAddBtn = nullptr;
        QPushButton *mEditBtn = nullptr;
        QPushButton *mRemoveBtn = nullptr;

        QLineEdit *mFilterEdit = nullptr;

        StyledTreeView *mView = nullptr;

        ItemCostModel mModel;
        QSortFilterProxyModel mProxy;

        Character::IdType mCharacterId = Character::invalidId;

        QModelIndexList mSelectedCosts;

        bool mBlockUpdate = false;

        void showCostEditDialog(ItemCost &cost);

        static QChar getCsvSeparator();
    };
}
