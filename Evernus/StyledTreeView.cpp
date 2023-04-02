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
#include <QApplication>
#include <QHeaderView>
#include <QClipboard>
#include <QSettings>
#include <QAction>
#include <QMenu>

#include "StyledTreeViewItemDelegate.h"
#include "UISettings.h"

#include "StyledTreeView.h"

namespace Evernus
{
    StyledTreeView::StyledTreeView(QWidget *parent)
        : QTreeView(parent)
    {
        setSortingEnabled(true);
        setItemDelegate(new StyledTreeViewItemDelegate{this});
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setContextMenuPolicy(Qt::ActionsContextMenu);

        header()->resizeSections(QHeaderView::Stretch);

        auto action = new QAction{QIcon{":/images/page_copy.png"}, tr("&Copy"), this};
        action->setShortcuts(QKeySequence::Copy);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        connect(action, &QAction::triggered, this, &StyledTreeView::copy);
        addAction(action);

        action = new QAction{tr("Copy &rows"), this};
        connect(action, &QAction::triggered, this, &StyledTreeView::copyRows);
        addAction(action);

        action = new QAction{tr("Copy raw &data"), this};
        connect(action, &QAction::triggered, this, &StyledTreeView::copyRawData);
        addAction(action);

        mColumnsMenu = new QMenu{this};

        action = new QAction{tr("Show/hide columns"), this};
        action->setMenu(mColumnsMenu);
        addAction(action);

        connect(header(), &QHeaderView::sectionMoved, this, &StyledTreeView::saveHeaderState, Qt::DirectConnection);
        connect(header(), &QHeaderView::sectionResized, this, &StyledTreeView::saveHeaderState, Qt::DirectConnection);
    }

    StyledTreeView::StyledTreeView(const QString &objectName, QWidget *parent)
        : StyledTreeView{parent}
    {
        setObjectName(objectName);
    }

    void StyledTreeView::setModel(QAbstractItemModel *newModel)
    {
        auto prevModel = model();
        if (prevModel != nullptr)
            disconnect(prevModel, SIGNAL(modelReset()), this, SLOT(setColumnsMenu()));

        mSaveStateEnabled = false;
        QTreeView::setModel(newModel);
        mSaveStateEnabled = true;

        if (newModel != nullptr)
        {
            connect(newModel, &QAbstractItemModel::modelReset, this, [=] {
                setColumnsMenu();
            });

            restoreHeaderState();
            setColumnsMenu(newModel);
        }
    }

    void StyledTreeView::restoreHeaderState()
    {
        const auto name = objectName();
        if (!name.isEmpty())
        {
            QSettings settings;

            const auto state = settings.value(UISettings::headerStateKey.arg(name)).toByteArray();
            if (!state.isEmpty())
            {
                header()->restoreState(state);
            }
            else
            {
                mSaveStateEnabled = false;
                header()->resizeSections(QHeaderView::ResizeToContents);
                mSaveStateEnabled = true;
            }
        }
    }

    void StyledTreeView::copy()
    {
        const auto curModel = model();
        if (curModel != nullptr)
        {
            const auto data = curModel->data(currentIndex(), Qt::DisplayRole);
            if (static_cast<QMetaType::Type>(data.type()) == QMetaType::QString)
                QApplication::clipboard()->setText(data.toString());
        }
    }

    void StyledTreeView::copyRows()
    {
        copyRowsWithRole(Qt::DisplayRole);
    }

    void StyledTreeView::copyRawData()
    {
        copyRowsWithRole(Qt::UserRole);
    }

    void StyledTreeView::setColumnsMenu(QAbstractItemModel *model)
    {
        if (model == nullptr)
            model = static_cast<QAbstractItemModel *>(sender());

        mColumnsMenu->clear();

        const auto columns = model->columnCount();
        for (auto i = 0; i < columns; ++i)
        {
            const auto name = model->headerData(i, Qt::Horizontal);
            auto action = mColumnsMenu->addAction(name.toString());
            action->setCheckable(true);
            action->setChecked(!isColumnHidden(i));
            connect(action, &QAction::triggered, this, [i, this](bool checked) {
                setColumnHidden(i, !checked);
            });
        }
    }

    void StyledTreeView::saveHeaderState()
    {
        if (!mSaveStateEnabled)
            return;

        const auto name = objectName();
        if (!name.isEmpty())
        {
            // don't save state when the model is empty - hack for not saving state when e.g. initial geometry changes
            const auto curModel = model();
            if (curModel == nullptr || curModel->rowCount() == 0)
                return;

            QSettings settings;
            settings.setValue(UISettings::headerStateKey.arg(name), header()->saveState());
        }
    }

    void StyledTreeView::copyRowsWithRole(int role) const
    {
        const auto curModel = model();
        if (curModel != nullptr)
        {
            const auto indexes = selectionModel()->selectedIndexes();
            if (indexes.isEmpty())
                return;

            QSettings settings;
            const auto delim
                = settings.value(UISettings::columnDelimiterKey, UISettings::columnDelimiterDefault).value<char>();

            QString result;

            auto prevRow = indexes.first().row();
            for (const auto &index : indexes)
            {
                if (prevRow != index.row())
                {
                    prevRow = index.row();
                    result.append('\n');
                }

                const auto data = curModel->data(index, role);
                if (static_cast<QMetaType::Type>(data.type()) == QMetaType::QVariantList)
                    result.append(data.toList().first().toString());
                else
                    result.append(data.toString());

                result.append(delim);
            }

            QApplication::clipboard()->setText(result);
        }
    }
}
