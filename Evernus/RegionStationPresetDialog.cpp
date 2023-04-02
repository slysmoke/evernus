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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QtDebug>
#include <QIcon>

#include "RegionStationPresetRepository.h"
#include "RegionStationPreset.h"
#include "StationSelectButton.h"
#include "RegionComboBox.h"

#include "RegionStationPresetDialog.h"

namespace Evernus
{
    RegionStationPresetDialog::RegionStationPresetDialog(const RegionStationPresetRepository &regionStationPresetRepository,
                                                         const EveDataProvider &dataProvider,
                                                         QWidget *parent)
        : QDialog{parent}
        , mRegionStationPresetRepository{regionStationPresetRepository}
    {
        const auto mainLayout = new QVBoxLayout{this};

        const auto presetGroup = new QGroupBox{tr("Preset"), this};
        mainLayout->addWidget(presetGroup);

        const auto presetGroupLayout = new QHBoxLayout{presetGroup};

        presetGroupLayout->addWidget(new QLabel{tr("Name:"), this});

        mPresetNameEdit = new QComboBox{this};
        presetGroupLayout->addWidget(mPresetNameEdit, 1);
        mPresetNameEdit->setEditable(true);
        mPresetNameEdit->setInsertPolicy(QComboBox::NoInsert);
        mPresetNameEdit->addItems(mRegionStationPresetRepository.getAllNames());
        mPresetNameEdit->setToolTip(tr("Select a present and press Load to load it, or type a name and press Save to save a new one or replace an existing one."));

        const auto loadBtn = new QPushButton{QIcon{QStringLiteral(":/images/arrow_refresh.png")}, tr("Load"), this};
        presetGroupLayout->addWidget(loadBtn);
        connect(loadBtn, &QPushButton::clicked, this, &RegionStationPresetDialog::load);

        const auto saveBtn = new QPushButton{QIcon{QStringLiteral(":/images/disk.png")}, tr("Save"), this};
        presetGroupLayout->addWidget(saveBtn);
        connect(saveBtn, &QPushButton::clicked, this, &RegionStationPresetDialog::save);

        const auto removeBtn = new QPushButton{QIcon{QStringLiteral(":/images/delete.png")}, tr("Remove"), this};
        presetGroupLayout->addWidget(removeBtn);
        connect(removeBtn, &QPushButton::clicked, this, &RegionStationPresetDialog::remove);

        const auto enableButtons = [=](const auto &text) {
            const auto enabled = !text.isEmpty();

            loadBtn->setEnabled(enabled);
            saveBtn->setEnabled(enabled);
            removeBtn->setEnabled(enabled);
        };

        enableButtons(mPresetNameEdit->currentText());

        connect(mPresetNameEdit, &QComboBox::currentTextChanged, this, enableButtons);

        const auto srcGroup = new QGroupBox{tr("Source regions/station"), this};
        mainLayout->addWidget(srcGroup);

        const auto srcGroupLayout = new QHBoxLayout{srcGroup};

        mSrcRegions = new RegionComboBox{dataProvider, this};
        srcGroupLayout->addWidget(mSrcRegions);

        mSrcStationBtn = new StationSelectButton{dataProvider, this};
        srcGroupLayout->addWidget(mSrcStationBtn);

        const auto dstGroup = new QGroupBox{tr("Destination regions/station"), this};
        mainLayout->addWidget(dstGroup);

        const auto dstGroupLayout = new QHBoxLayout{dstGroup};

        mDstRegions = new RegionComboBox{dataProvider, this};
        dstGroupLayout->addWidget(mDstRegions);

        mDstStationBtn = new StationSelectButton{dataProvider, this};
        dstGroupLayout->addWidget(mDstStationBtn);

        const auto btnBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
        mainLayout->addWidget(btnBox);
        connect(btnBox, &QDialogButtonBox::accepted, this, &RegionStationPresetDialog::accept);
        connect(btnBox, &QDialogButtonBox::rejected, this, &RegionStationPresetDialog::reject);

        setWindowTitle(tr("Region/station presets"));
    }

    RegionStationPresetDialog::RegionList RegionStationPresetDialog::getSrcRegions() const
    {
        return mSrcRegions->getSelectedRegionList();
    }

    RegionStationPresetDialog::RegionList RegionStationPresetDialog::getDstRegions() const
    {
        return mDstRegions->getSelectedRegionList();
    }

    quint64 RegionStationPresetDialog::getSrcStationId() const
    {
        return mSrcStationBtn->getSelectedStationId();
    }

    quint64 RegionStationPresetDialog::getDstStationId() const
    {
        return mDstStationBtn->getSelectedStationId();
    }

    void RegionStationPresetDialog::load()
    {
        try
        {
            const auto preset = mRegionStationPresetRepository.find(mPresetNameEdit->currentText());

            mSrcRegions->setSelectedRegionList(preset->getSrcRegions());
            mDstRegions->setSelectedRegionList(preset->getDstRegions());
            mSrcStationBtn->setSelectedStationId(preset->getSrcStationId().value_or(0));
            mDstStationBtn->setSelectedStationId(preset->getDstStationId().value_or(0));
        }
        catch (const RegionStationPresetRepository::NotFoundException &)
        {
            qWarning() << "Trying to load non-existing preset:" << mPresetNameEdit->currentText();
        }
    }

    void RegionStationPresetDialog::save()
    {
        const auto name = mPresetNameEdit->currentText();

        RegionStationPreset preset{name};
        preset.setSrcRegions(mSrcRegions->getSelectedRegionList());
        preset.setDstRegions(mDstRegions->getSelectedRegionList());

        auto stationId = mSrcStationBtn->getSelectedStationId();
        if (stationId != 0)
            preset.setSrcStationId(stationId);

        stationId = mDstStationBtn->getSelectedStationId();
        if (stationId != 0)
            preset.setDstStationId(stationId);

        mRegionStationPresetRepository.store(preset);

        mPresetNameEdit->addItem(name);
    }

    void RegionStationPresetDialog::remove()
    {
        mRegionStationPresetRepository.remove(mPresetNameEdit->currentText());
        mPresetNameEdit->removeItem(mPresetNameEdit->currentIndex());
    }
}
