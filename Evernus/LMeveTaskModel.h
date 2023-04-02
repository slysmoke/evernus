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

#include <vector>
#include <memory>

#include <QAbstractTableModel>

#include "CharacterRepository.h"
#include "LMeveTask.h"

namespace Evernus
{
    class ItemCostProvider;
    class EveDataProvider;

    class LMeveTaskModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        typedef std::vector<std::shared_ptr<LMeveTask>> TaskList;

        LMeveTaskModel(const EveDataProvider &dataProvider,
                       const ItemCostProvider &costProvider,
                       const CharacterRepository &characterRepository,
                       QObject *parent = nullptr);
        virtual ~LMeveTaskModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        const TaskList &getTasks() const noexcept;
        void setTasks(const TaskList &data);
        void setTasks(TaskList &&data);

        void setCharacterId(Character::IdType id);

        quint64 getStationId() const noexcept;
        void setStationId(quint64 id);

    private:
        enum
        {
            typeColumn,
            activityColumn,
            runsColumn,
            runsDoneColumn,
            runsCompletedColumn,
            jobsDoneColumn,
            jobsSuccessColumn,
            jobsCompletedColumn,
            costColumn,
            priceColumn,
            profitColumn,
            marginColumn,

            numColumns
        };

        const EveDataProvider &mDataProvider;
        const ItemCostProvider &mCostProvider;
        const CharacterRepository &mCharacterRepository;

        TaskList mData;

        quint64 mStationId = 0;

        CharacterRepository::EntityPtr mCharacter;

        double getMargin(const LMeveTask &task) const;

        static double getAdjustedPrice(double price);
    };
}
