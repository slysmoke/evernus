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

#include "CitadelRepository.h"
#include "Character.h"
#include "Citadel.h"

class QPushButton;
class QCheckBox;

namespace Evernus
{
    class CitadelManagementWidget;
    class CitadelAccessCache;
    class EveDataProvider;

    class CitadelManagerDialog
        : public QDialog
    {
        Q_OBJECT

    public:
        CitadelManagerDialog(const EveDataProvider &dataProvider,
                             const CitadelRepository &citadelRepo,
                             CitadelAccessCache &citadelAccessCache,
                             Character::IdType charId,
                             QWidget *parent = nullptr);
        CitadelManagerDialog(const CitadelManagerDialog &) = default;
        CitadelManagerDialog(CitadelManagerDialog &&) = default;
        virtual ~CitadelManagerDialog() = default;

        CitadelManagerDialog &operator =(const CitadelManagerDialog &) = default;
        CitadelManagerDialog &operator =(CitadelManagerDialog &&) = default;

    signals:
        void citadelsChanged();

    public slots:
        void refreshCitadels();

    private slots:
        void applyChanges();
        void refreshAccessCache();

        void addCitadel();
        void editCitadel();
        void removeCitadel();

        void selectCitadel(Citadel::IdType id);

    private:
        const EveDataProvider &mDataProvider;
        const CitadelRepository &mCitadelRepo;
        CitadelAccessCache &mCitadelAccessCache;

        CitadelManagementWidget *mIgnoredCitadelsWidget = nullptr;
        QPushButton *mEditBtn = nullptr;
        QPushButton *mRemoveBtn = nullptr;
        QCheckBox *mClearExistingCitadelsBtn = nullptr;

        Citadel::IdType mCurrentCitadel = Citadel::invalidId;

        void showCitadelEdit(const CitadelRepository::EntityPtr &citadel);
    };
}
