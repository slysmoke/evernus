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
#include <QLineEdit>

#include "FilterTextRepository.h"

#include "TextFilterWidget.h"

namespace Evernus
{
    TextFilterWidget::TextFilterWidget(const FilterTextRepository &filterRepo, QWidget *parent)
        : QComboBox(parent)
        , mFilterRepo(filterRepo)
    {
        addItems(mFilterRepo.fetchRecentlyUsed());
        setEditable(true);
        setInsertPolicy(QComboBox::InsertAtTop);
        setCurrentText(QString{});

        auto edit = lineEdit();
        edit->setPlaceholderText(tr("type in wildcard and press Enter"));
        edit->setClearButtonEnabled(true);
        connect(edit, &QLineEdit::returnPressed, this, &TextFilterWidget::applyCurrentFilter);

        connect(&mFilterRepo, &FilterTextRepository::filtersChanged, this, &TextFilterWidget::refreshHistory);
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
}
