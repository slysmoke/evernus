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
#include <QItemSelectionModel>
#include <QAbstractProxyModel>
#include <QAbstractItemView>
#include <QAction>

#include "ModelWithTypes.h"
#include "ModelUtils.h"

#include "StandardModelProxyWidget.h"

namespace Evernus
{
    StandardModelProxyWidget::StandardModelProxyWidget(const ModelWithTypes &dataModel,
                                                       const QAbstractProxyModel &dataProxy,
                                                       QWidget *parent)
        : QWidget{parent}
        , mDataModel{&dataModel}
        , mDataProxy{dataProxy}
    {
    }

    StandardModelProxyWidget::StandardModelProxyWidget(const QAbstractProxyModel &dataProxy,
                                                       QWidget *parent)
        : QWidget{parent}
        , mDataProxy{dataProxy}
    {
    }

    void StandardModelProxyWidget::installOnView(QAbstractItemView *view)
    {
        Q_ASSERT(view != nullptr);
        Q_ASSERT(mDataView == nullptr);

        mDataView = view;
        connect(mDataView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &StandardModelProxyWidget::selectType);

        mShowInEveAct = new QAction{tr("Show in EVE"), this};
        mShowInEveAct->setEnabled(false);
        mDataView->addAction(mShowInEveAct);
        connect(mShowInEveAct, &QAction::triggered, this, &StandardModelProxyWidget::showInEveForCurrent);

        mCopyRowsAct = new QAction{tr("&Copy"), this};
        mCopyRowsAct->setEnabled(false);
        mCopyRowsAct->setShortcut(QKeySequence::Copy);
        mDataView->addAction(mCopyRowsAct);
        connect(mCopyRowsAct, &QAction::triggered, this, &StandardModelProxyWidget::copyRows);
    }

    void StandardModelProxyWidget::setCharacter(Character::IdType id) noexcept
    {
        mCharacterId = id;
    }

    void StandardModelProxyWidget::setDataModel(const ModelWithTypes &dataModel)
    {
        mDataModel = &dataModel;
    }

    void StandardModelProxyWidget::copyRows() const
    {
        ModelUtils::copyRowsToClipboard(mDataView->selectionModel()->selectedIndexes(), mDataProxy);
    }

    void StandardModelProxyWidget::selectType(const QItemSelection &selected)
    {
        const auto enabled = !selected.isEmpty();
        mShowInEveAct->setEnabled(enabled);
        mCopyRowsAct->setEnabled(enabled);
    }

    void StandardModelProxyWidget::showInEveForCurrent()
    {
        if (mCharacterId == Character::invalidId)
            return;

        Q_ASSERT(mDataModel != nullptr);

        const auto id = mDataModel->getTypeId(mDataProxy.mapToSource(mDataView->currentIndex()));
        if (id != EveType::invalidId)
            emit showInEve(id, mCharacterId);
    }
}
