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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QModelIndex>
#include <QIcon>

#include "CheckableTreeView.h"

#include "CitadelManagementWidget.h"

namespace Evernus
{
    CitadelManagementWidget::CitadelManagementWidget(const EveDataProvider &dataProvider,
                                                     const CitadelAccessCache &citadelAccessCache,
                                                     Character::IdType charId,
                                                     QWidget *parent)
        : QWidget{parent}
        , mModel{dataProvider, citadelAccessCache, charId}
    {
        const auto mainLayout = new QVBoxLayout{this};

        const auto buttonLayout = new QHBoxLayout{};
        mainLayout->addLayout(buttonLayout);
        buttonLayout->addStretch();

        const auto mainView = new CheckableTreeView{this};
        mainLayout->addWidget(mainView);
        mainView->setModel(&mModel);
        mainView->setHeaderHidden(true);
        connect(mainView->selectionModel(), &QItemSelectionModel::currentChanged, this, &CitadelManagementWidget::toggleActions);

        auto btn = new QPushButton{this};
        buttonLayout->addWidget(btn);
        btn->setIcon(QIcon{QStringLiteral(":/images/arrow_out.png")});
        btn->setToolTip(tr("Expand all"));
        connect(btn, &QPushButton::clicked, mainView, &QTreeView::expandAll);

        btn = new QPushButton{this};
        buttonLayout->addWidget(btn);
        btn->setIcon(QIcon{QStringLiteral(":/images/arrow_in.png")});
        btn->setToolTip(tr("Collapse all"));
        connect(btn, &QPushButton::clicked, mainView, &QTreeView::collapseAll);
    }

    CitadelManagementWidget::CitadelList CitadelManagementWidget::getSelectedCitadels() const
    {
        return mModel.getSelectedCitadels();
    }

    void CitadelManagementWidget::refresh()
    {
        mModel.refresh();
    }

    void CitadelManagementWidget::toggleActions(const QModelIndex &selection)
    {
        emit citadelSelected(mModel.getCitadel(selection));
    }
}
