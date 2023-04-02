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

#include "EveType.h"

class QTableWidget;

namespace Evernus
{
    class CustomFPCDialog final
        : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomFPCDialog(QWidget *parent = nullptr);
        CustomFPCDialog(const CustomFPCDialog &) = default;
        CustomFPCDialog(CustomFPCDialog &&) = default;
        virtual ~CustomFPCDialog() = default;

        CustomFPCDialog &operator =(const CustomFPCDialog &) = default;
        CustomFPCDialog &operator =(CustomFPCDialog &&) = default;

    signals:
        void showInEve(EveType::IdType id) const;

    public slots:
        void executeFPC();
        void executeBackwardFPC();

    private slots:
        void pasteData();

    private:
        QTableWidget *mDataView = nullptr;

        void copyData(int row) const;
    };
}
