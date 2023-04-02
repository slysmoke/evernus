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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>
#include <QSpinBox>
#include <QLabel>

#include "WalletSettings.h"

#include "WalletPreferencesWidget.h"

namespace Evernus
{
    WalletPreferencesWidget::WalletPreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};

        auto journalGroup = new QGroupBox{tr("Journal"), this};
        mainLayout->addWidget(journalGroup);

        auto journalLayout = new QVBoxLayout{journalGroup};

        mDeleteOldJournalBtn = new QCheckBox{tr("Delete old entries"), this};
        journalLayout->addWidget(mDeleteOldJournalBtn);
        connect(mDeleteOldJournalBtn, &QCheckBox::stateChanged, this, &WalletPreferencesWidget::deleteOldJournalToggled);

        auto journalDaysLayout = new QHBoxLayout{};
        journalLayout->addLayout(journalDaysLayout);

        journalDaysLayout->addWidget(new QLabel{tr("Delete older than:"), this});

        mOldJournalDaysEdit = new QSpinBox{this};
        journalDaysLayout->addWidget(mOldJournalDaysEdit);
        mOldJournalDaysEdit->setMinimum(2);
        mOldJournalDaysEdit->setSuffix(tr(" days"));
        mOldJournalDaysEdit->setValue(settings.value(WalletSettings::oldJournalDaysKey, WalletSettings::oldJournalDaysDefault).toInt());

        journalDaysLayout->addStretch();

        const auto deleteJournal
            = settings.value(WalletSettings::deleteOldJournalKey, WalletSettings::deleteOldJournalDefault).toBool();
        mDeleteOldJournalBtn->setChecked(deleteJournal);
        mOldJournalDaysEdit->setEnabled(deleteJournal);

        auto transactionsGroup = new QGroupBox{tr("Transactions"), this};
        mainLayout->addWidget(transactionsGroup);

        auto transactionsLayout = new QVBoxLayout{transactionsGroup};

        mDeleteOldTransactionsBtn = new QCheckBox{tr("Delete old entries"), this};
        transactionsLayout->addWidget(mDeleteOldTransactionsBtn);
        connect(mDeleteOldTransactionsBtn, &QCheckBox::stateChanged, this, &WalletPreferencesWidget::deleteOldTransactionsToggled);

        auto transactionsDaysLayout = new QHBoxLayout{};
        transactionsLayout->addLayout(transactionsDaysLayout);

        transactionsDaysLayout->addWidget(new QLabel{tr("Delete older than:"), this});

        mOldTransactionsDaysEdit = new QSpinBox{this};
        transactionsDaysLayout->addWidget(mOldTransactionsDaysEdit);
        mOldTransactionsDaysEdit->setMinimum(2);
        mOldTransactionsDaysEdit->setSuffix(tr(" days"));
        mOldTransactionsDaysEdit->setValue(settings.value(WalletSettings::oldTransactionsDaysKey, WalletSettings::oldTransactionsDaysDefault).toInt());

        transactionsDaysLayout->addStretch();

        const auto deleteTransactions
            = settings.value(WalletSettings::deleteOldTransactionsKey, WalletSettings::deleteOldTransactionsDefault).toBool();
        mDeleteOldTransactionsBtn->setChecked(deleteTransactions);
        mOldTransactionsDaysEdit->setEnabled(deleteTransactions);

        mainLayout->addStretch();
    }

    void WalletPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(WalletSettings::deleteOldJournalKey, mDeleteOldJournalBtn->isChecked());
        settings.setValue(WalletSettings::oldJournalDaysKey, mOldJournalDaysEdit->value());
        settings.setValue(WalletSettings::deleteOldTransactionsKey, mDeleteOldTransactionsBtn->isChecked());
        settings.setValue(WalletSettings::oldTransactionsDaysKey, mOldTransactionsDaysEdit->value());
    }

    void WalletPreferencesWidget::deleteOldJournalToggled(int state)
    {
        mOldJournalDaysEdit->setEnabled(state == Qt::Checked);
    }

    void WalletPreferencesWidget::deleteOldTransactionsToggled(int state)
    {
        mOldTransactionsDaysEdit->setEnabled(state == Qt::Checked);
    }
}
