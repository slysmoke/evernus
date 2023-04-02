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
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QSettings>
#include <QLabel>
#include <QIcon>

#include <QtDebug>

#include "CitadelManagementWidget.h"
#include "CitadelAccessCache.h"
#include "CitadelEditDialog.h"
#include "ImportSettings.h"

#include "CitadelManagerDialog.h"

namespace Evernus
{
    CitadelManagerDialog::CitadelManagerDialog(const EveDataProvider &dataProvider,
                                               const CitadelRepository &citadelRepo,
                                               CitadelAccessCache &citadelAccessCache,
                                               Character::IdType charId,
                                               QWidget *parent)
        : QDialog{parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint}
        , mDataProvider{dataProvider}
        , mCitadelRepo{citadelRepo}
        , mCitadelAccessCache{citadelAccessCache}
    {
        const auto mainLayout = new QVBoxLayout{this};

        const auto ignoredBox = new QGroupBox{tr("Ignored citadels"), this};
        mainLayout->addWidget(ignoredBox);

        const auto ignoredBoxLayout = new QVBoxLayout{ignoredBox};

        ignoredBoxLayout->addWidget(new QLabel{tr("Ignored citadels will not have their data imported."), this});

        mIgnoredCitadelsWidget = new CitadelManagementWidget{mDataProvider, mCitadelAccessCache, charId, this};
        ignoredBoxLayout->addWidget(mIgnoredCitadelsWidget);
        connect(mIgnoredCitadelsWidget, &CitadelManagementWidget::citadelSelected, this, &CitadelManagerDialog::selectCitadel);

        ignoredBoxLayout->addWidget(new QLabel{tr("<s>Citadel Name</s> - citadel unavailable for current character"), this});

        QSettings settings;

        mClearExistingCitadelsBtn = new QCheckBox{tr("Clear existing citadels on import"), this};
        mainLayout->addWidget(mClearExistingCitadelsBtn);
        mClearExistingCitadelsBtn->setChecked(settings.value(ImportSettings::clearExistingCitadelsKey, ImportSettings::clearExistingCitadelsDefault).toBool());

        const auto btnLayout = new QHBoxLayout{};
        mainLayout->addLayout(btnLayout);

        const auto refreshAccessCacheBtn = new QPushButton{QIcon{QStringLiteral(":/images/arrow_refresh.png")}, tr("Refresh access cache"), this};
        btnLayout->addWidget(refreshAccessCacheBtn);
        refreshAccessCacheBtn->setToolTip(tr("Clear citadel access cache to check if your characters have access to various citadels."));
        connect(refreshAccessCacheBtn, &QPushButton::clicked, this, &CitadelManagerDialog::refreshAccessCache);

        const auto addCitadelBtn = new QPushButton{QIcon{QStringLiteral(":/images/add.png")}, tr("Add citadel..."), this};
        btnLayout->addWidget(addCitadelBtn);
        connect(addCitadelBtn, &QPushButton::clicked, this, &CitadelManagerDialog::addCitadel);

        mEditBtn = new QPushButton{QIcon{QStringLiteral(":/images/pencil.png")}, tr("Edit selected..."), this};
        btnLayout->addWidget(mEditBtn);
        mEditBtn->setEnabled(false);
        connect(mEditBtn, &QPushButton::clicked, this, &CitadelManagerDialog::editCitadel);

        mRemoveBtn = new QPushButton{QIcon{QStringLiteral(":/images/delete.png")}, tr("Remove selected"), this};
        btnLayout->addWidget(mRemoveBtn);
        mRemoveBtn->setEnabled(false);
        connect(mRemoveBtn, &QPushButton::clicked, this, &CitadelManagerDialog::removeCitadel);

        const auto buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
        mainLayout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &CitadelManagerDialog::applyChanges);
        connect(buttons, &QDialogButtonBox::rejected, this, &CitadelManagerDialog::reject);

        setWindowTitle(tr("Citadel manager"));
    }

    void CitadelManagerDialog::refreshCitadels()
    {
        mIgnoredCitadelsWidget->refresh();
    }

    void CitadelManagerDialog::applyChanges()
    {
        QSettings settings;
        settings.setValue(ImportSettings::clearExistingCitadelsKey, mClearExistingCitadelsBtn->isChecked());

        mCitadelRepo.setIgnored(mIgnoredCitadelsWidget->getSelectedCitadels());
        accept();
    }

    void CitadelManagerDialog::refreshAccessCache()
    {
        mCitadelAccessCache.clear();
        mIgnoredCitadelsWidget->refresh();
    }

    void CitadelManagerDialog::addCitadel()
    {
        showCitadelEdit({});
    }

    void CitadelManagerDialog::editCitadel()
    {
        try
        {
            showCitadelEdit(mCitadelRepo.find(mCurrentCitadel));
        }
        catch (const CitadelRepository::NotFoundException &)
        {
            qWarning() << "Trying to edit inexistent citadel:" << mCurrentCitadel;
        }
    }

    void CitadelManagerDialog::removeCitadel()
    {
        mCitadelRepo.remove(mCurrentCitadel);
        emit citadelsChanged();

        mIgnoredCitadelsWidget->refresh();
    }

    void CitadelManagerDialog::selectCitadel(Citadel::IdType id)
    {
        mCurrentCitadel = id;

        const auto enabled = mCurrentCitadel != Citadel::invalidId;
        mEditBtn->setEnabled(enabled);
        mRemoveBtn->setEnabled(enabled);
    }

    void CitadelManagerDialog::showCitadelEdit(const CitadelRepository::EntityPtr &citadel)
    {
        CitadelEditDialog editDlg{mDataProvider, (citadel) ? (*citadel) : (Citadel{}), this};
        if (editDlg.exec() == QDialog::Accepted)
        {
            mCitadelRepo.store(editDlg.getCitadel());

            emit citadelsChanged();

            mIgnoredCitadelsWidget->refresh();
        }
    }
}
