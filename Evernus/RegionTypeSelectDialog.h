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
#pragma once

#include <QDialog>

#include "TypeLocationPairs.h"

class QListWidget;

namespace Evernus
{
    class RegionTypePresetRepository;
    class TradeableTypesTreeView;
    class MarketGroupRepository;
    class EveTypeRepository;
    class EveDataProvider;

    class RegionTypeSelectDialog
        : public QDialog
    {
        Q_OBJECT

    public:
        static const QString settingsTypesKey;

        RegionTypeSelectDialog(const EveDataProvider &dataProvider,
                               const EveTypeRepository &typeRepo,
                               const MarketGroupRepository &groupRepo,
                               const RegionTypePresetRepository &regionTypePresetRepo,
                               QWidget *parent = nullptr);
        virtual ~RegionTypeSelectDialog() = default;

    signals:
        void selected(const TypeLocationPairs &pairs);

    public slots:
        virtual void accept() override;

    private slots:
        void savePreset();
        void loadPreset();

    private:
        static const QString settingsRegionsKey;

        const RegionTypePresetRepository &mRegionTypePresetRepo;

        QListWidget *mRegionList = nullptr;
        TradeableTypesTreeView *mTypeView = nullptr;

        QString mLastLoadedPreset;

        void setRegionCheckStates(Qt::CheckState state);
    };
}
