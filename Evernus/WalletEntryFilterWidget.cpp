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
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDate>

#include "TextFilterWidget.h"
#include "DateRangeWidget.h"

#include "WalletEntryFilterWidget.h"

namespace Evernus
{
    WalletEntryFilterWidget::WalletEntryFilterWidget(const QStringList &typeFilters, const FilterTextRepository &filterRepo, QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QHBoxLayout{this};

        mainLayout->addWidget(new QLabel(tr("Show:"), this));

        mTypeCombo = new QComboBox{this};
        mainLayout->addWidget(mTypeCombo);

        for (auto i = 0; i < typeFilters.count(); ++i)
            mTypeCombo->addItem(typeFilters[i], i);

        connect(mTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &WalletEntryFilterWidget::changeEntryType);

        mRangeEdit = new DateRangeWidget{this};
        mainLayout->addWidget(mRangeEdit);
        connect(mRangeEdit, &DateRangeWidget::rangeChanged, this, &WalletEntryFilterWidget::applyRange);

        mFilterEdit = new TextFilterWidget{filterRepo, this};
        mainLayout->addWidget(mFilterEdit, 1);
        connect(mFilterEdit, &TextFilterWidget::filterEntered, this, &WalletEntryFilterWidget::applyWildcard);
    }

    void WalletEntryFilterWidget::setFilter(const QDate &from, const QDate &to, const QString &filter, int type)
    {
        mTypeCombo->blockSignals(true);

        mTypeCombo->setCurrentIndex(0);
        mRangeEdit->setRange(from, to);
        mFilterEdit->setCurrentText(filter);

        mTypeCombo->blockSignals(false);

        emit filterChanged(from, to, mFilterEdit->currentText(), mCurrentType);
    }

    void WalletEntryFilterWidget::changeEntryType()
    {
        mCurrentType = mTypeCombo->currentData().toInt();
        emit filterChanged(mRangeEdit->getFrom(), mRangeEdit->getTo(), mFilterEdit->currentText(), mCurrentType);
    }

    void WalletEntryFilterWidget::applyWildcard(const QString &text)
    {
        emit filterChanged(mRangeEdit->getFrom(), mRangeEdit->getTo(), text, mCurrentType);
    }

    void WalletEntryFilterWidget::applyRange(const QDate &from, const QDate &to)
    {
        emit filterChanged(from, to, mFilterEdit->currentText(), mCurrentType);
    }
}
