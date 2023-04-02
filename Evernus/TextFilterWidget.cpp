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
#include <QAbstractItemView>
#include <QCompleter>
#include <QLineEdit>
#include <QMenu>

#include "FilterTextRepository.h"

#include "TextFilterWidget.h"

namespace Evernus
{
    TextFilterWidget::TextFilterWidget(const FilterTextRepository &filterRepo, QWidget *parent)
        : QComboBox{parent}
        , mFilterRepo{filterRepo}
    {
        addItems(mFilterRepo.fetchRecentlyUsed());
        setEditable(true);
        setInsertPolicy(QComboBox::InsertAtTop);
        setCurrentText(QString{});

        completer()->setCaseSensitivity(Qt::CaseSensitive);

        auto edit = lineEdit();
        edit->setPlaceholderText(getDefaultPlaceholderText());
        connect(edit, &QLineEdit::returnPressed, this, &TextFilterWidget::applyCurrentFilter);

        auto itemView = view();
        itemView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(itemView, &QAbstractItemView::customContextMenuRequested, this, &TextFilterWidget::showItemContextMenu);

        connect(&mFilterRepo, &FilterTextRepository::filtersChanged, this, &TextFilterWidget::refreshHistory);
    }

    QString TextFilterWidget::getDefaultPlaceholderText()
    {
        return tr("type in wildcard and press Enter");
    }

    void TextFilterWidget::applyCurrentFilter()
    {
        const auto text = currentText();
        if (!text.isEmpty())
        {
            mFilterRepo.store(text);

            const auto index = findText(text, Qt::MatchExactly);
            if (index != -1)
            {
                removeItem(index);
                insertItem(0, text);
                setCurrentText(text);
            }
        }

        emit filterEntered(text);
    }

    void TextFilterWidget::refreshHistory()
    {
        const auto text = currentText();

        clear();
        addItems(mFilterRepo.fetchRecentlyUsed());
        setCurrentText(text);
    }

    void TextFilterWidget::showItemContextMenu(const QPoint &pos)
    {
        const auto itemView = view();
        const auto index = itemView->indexAt(pos);

        if (!index.isValid())
            return;

        QMenu menu;

        auto action = menu.addAction(QIcon{":/images/cross.png"}, tr("Remove"));
        connect(action, &QAction::triggered, this, [index, this] {
            mFilterRepo.remove(itemText(index.row()));
            showPopup();
        });

        menu.exec(itemView->mapToGlobal(pos));
    }
}
