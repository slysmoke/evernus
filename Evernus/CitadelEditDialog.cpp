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
#include <limits>

#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>

#include "SingleRegionComboBox.h"
#include "SolarSystemComboBox.h"
#include "EveDataProvider.h"
#include "EveType.h"

#include "CitadelEditDialog.h"

namespace Evernus
{
    CitadelEditDialog::CitadelEditDialog(const EveDataProvider &dataProvider, QWidget *parent)
        : CitadelEditDialog{dataProvider, {}, parent}
    {
    }

    CitadelEditDialog::CitadelEditDialog(const EveDataProvider &dataProvider, const Citadel &citadel, QWidget *parent)
        : QDialog{parent}
        , mCitadel{citadel}
    {
        const auto mainLayout = new QFormLayout{this};

        mIdEdit = new QLineEdit{QString::number(mCitadel.getId()), this};
        mainLayout->addRow(tr("Id:"), mIdEdit);
        mIdEdit->setValidator(new QDoubleValidator{0., static_cast<double>(std::numeric_limits<Citadel::IdType>::max()), 0, this});

        mNameEdit = new QLineEdit{mCitadel.getName(), this};
        mainLayout->addRow(tr("Name:"), mNameEdit);

        mTypeCombo = new QComboBox{this};
        mainLayout->addRow(tr("Type:"), mTypeCombo);

        const auto citadels = dataProvider.getCitadelTypeIds();
        for (const auto citadel : citadels)
            mTypeCombo->addItem(dataProvider.getTypeName(citadel), citadel);

        mTypeCombo->model()->sort(0);

        if (!mCitadel.isNew())
            mTypeCombo->setCurrentIndex(mTypeCombo->findData(mCitadel.getTypeId()));

        mRegionCombo = new SingleRegionComboBox{dataProvider, this};
        mainLayout->addRow(tr("Region:"), mRegionCombo);

        mSolarSystemCombo = new SolarSystemComboBox{dataProvider, this};
        mainLayout->addRow(tr("Solar system:"), mSolarSystemCombo);
        connect(mRegionCombo, QOverload<int>::of(&SingleRegionComboBox::currentIndexChanged), this, &CitadelEditDialog::filterSolarSystems);

        filterSolarSystems();

        if (!mCitadel.isNew())
            mRegionCombo->setCurrentIndex(mRegionCombo->findData(mCitadel.getRegionId()));

        const auto advancedGroup = new QGroupBox{tr("Advanced"), this};
        mainLayout->addRow(advancedGroup);
        advancedGroup->setCheckable(true);
        advancedGroup->setChecked(false);

        const auto advancedGroupLayout = new QFormLayout{advancedGroup};

        mXEdit = new QLineEdit{QString::number(mCitadel.getX()), this};
        advancedGroupLayout->addRow(tr("X:"), mXEdit);
        mXEdit->setValidator(new QDoubleValidator{this});

        mYEdit = new QLineEdit{QString::number(mCitadel.getY()), this};
        advancedGroupLayout->addRow(tr("Y:"), mYEdit);
        mYEdit->setValidator(new QDoubleValidator{this});

        mZEdit = new QLineEdit{QString::number(mCitadel.getZ()), this};
        advancedGroupLayout->addRow(tr("Z:"), mZEdit);
        mZEdit->setValidator(new QDoubleValidator{this});

        mIgnoredBtn = new QCheckBox{tr("Ignored"), this};
        advancedGroupLayout->addRow(mIgnoredBtn);
        mIgnoredBtn->setChecked(mCitadel.isIgnored());

        mPublicBtn = new QCheckBox{tr("Public"), this};
        advancedGroupLayout->addRow(mPublicBtn);
        mPublicBtn->setChecked(mCitadel.isPublic());

        const auto buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
        mainLayout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &CitadelEditDialog::addCitadel);
        connect(buttons, &QDialogButtonBox::rejected, this, &CitadelEditDialog::reject);

        setWindowTitle(tr("Citadel"));
    }

    const Citadel &CitadelEditDialog::getCitadel() const noexcept
    {
        return mCitadel;
    }

    Citadel &CitadelEditDialog::getCitadel() noexcept
    {
        return mCitadel;
    }

    void CitadelEditDialog::addCitadel()
    {
        mCitadel.setId(mIdEdit->text().toULongLong());
        mCitadel.setName(mNameEdit->text());

        if (mCitadel.getId() == Citadel::invalidId || mCitadel.getName().isEmpty())
        {
            QMessageBox::information(this, tr("Missing data"), tr("Please endter citadel id and name."));
            return;
        }

        mCitadel.setTypeId(mTypeCombo->currentData().value<EveType::IdType>());
        mCitadel.setRegionId(mRegionCombo->getSelectedRegion());
        mCitadel.setSolarSystemId(mSolarSystemCombo->getSelectedSolarSystem());
        mCitadel.setX(mXEdit->text().toDouble());
        mCitadel.setY(mYEdit->text().toDouble());
        mCitadel.setZ(mZEdit->text().toDouble());
        mCitadel.setIgnored(mIgnoredBtn->isChecked());
        mCitadel.setPublic(mPublicBtn->isChecked());
        mCitadel.setLastSeen(QDateTime::currentDateTimeUtc());

        if (!mCitadel.getFirstSeen().isValid())
            mCitadel.setFirstSeen(mCitadel.getLastSeen());

        accept();
    }

    void CitadelEditDialog::filterSolarSystems()
    {
        mSolarSystemCombo->setRegion(mRegionCombo->getSelectedRegion());
    }
}
