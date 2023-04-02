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
#include <QRadioButton>
#include <QVBoxLayout>

#include "StationView.h"

#include "StationSelectDialog.h"

namespace Evernus
{
    StationSelectDialog::StationSelectDialog(const EveDataProvider &dataProvider, bool allowNone, QWidget *parent)
        : QDialog(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

        if (allowNone)
        {
            mAnyStationBtn = new QRadioButton{tr("No specific station"), this};
            mainLayout->addWidget(mAnyStationBtn);
            mAnyStationBtn->setChecked(true);

            mCustomStationBtn = new QRadioButton{tr("Custom station"), this};
            mainLayout->addWidget(mCustomStationBtn);
        }

        mStationView = new StationView{dataProvider, this};
        mainLayout->addWidget(mStationView);

        if (allowNone)
        {
            mStationView->setEnabled(false);
            connect(mAnyStationBtn, &QRadioButton::toggled, mStationView, &StationView::setDisabled);
        }

        auto buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel};
        mainLayout->addWidget(buttonBox);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &StationSelectDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &StationSelectDialog::reject);

        setWindowTitle(tr("Select station"));
        setMinimumWidth(300);
    }

    QVariantList StationSelectDialog::getSelectedPath() const
    {
        return (mAnyStationBtn != nullptr && mAnyStationBtn->isChecked()) ? (QVariantList{}) : (mStationView->getSelectedPath());
    }

    void StationSelectDialog::selectPath(const QVariantList &path)
    {
        if (path.isEmpty())
        {
            if (mAnyStationBtn != nullptr)
                mAnyStationBtn->setChecked(true);
        }
        else
        {
            mStationView->selectPath(path);

            if (mCustomStationBtn != nullptr)
                mCustomStationBtn->setChecked(true);
        }
    }

    quint64 StationSelectDialog::getStationId() const
    {
        return (mAnyStationBtn != nullptr && mAnyStationBtn->isChecked()) ? (0) : (mStationView->getStationId());
    }
}
