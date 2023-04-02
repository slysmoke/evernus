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

#include <QTreeView>

namespace Evernus
{
    class StyledTreeView
        : public QTreeView
    {
        Q_OBJECT

    public:
        explicit StyledTreeView(QWidget *parent = nullptr);
        explicit StyledTreeView(const QString &objectName, QWidget *parent = nullptr);
        virtual ~StyledTreeView() = default;

        virtual void setModel(QAbstractItemModel *newModel) override;

        void restoreHeaderState();

    private slots:
        void copy();
        void copyRows();
        void copyRawData();

        void setColumnsMenu(QAbstractItemModel *model = nullptr);

        void saveHeaderState();

    private:
        QMenu *mColumnsMenu = nullptr;
        bool mSaveStateEnabled = true;

        void copyRowsWithRole(int role) const;
    };
}
