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

#include "LocationBookmark.h"

class QComboBox;

namespace Evernus
{
    class LocationBookmarkRepository;
    class EveDataProvider;

    class LocationBookmarkSelectDialog
        : public QDialog
    {
        Q_OBJECT

    public:
        LocationBookmarkSelectDialog(const EveDataProvider &dataProvider,
                                     const LocationBookmarkRepository &locationBookmarkRepo,
                                     QWidget *parent = nullptr);
        virtual ~LocationBookmarkSelectDialog() = default;

        LocationBookmark::IdType getSelectedBookmark() const;

    private:
        QComboBox *mBookmarkCombo = nullptr;
    };
}
