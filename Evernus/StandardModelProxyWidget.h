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

#include <QWidget>

#include "Character.h"
#include "EveType.h"

class QAbstractProxyModel;
class QAbstractItemView;
class QItemSelection;
class QAction;

namespace Evernus
{
    class ModelWithTypes;

    class StandardModelProxyWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        StandardModelProxyWidget(const ModelWithTypes &dataModel,
                                 const QAbstractProxyModel &dataProxy,
                                 QWidget *parent = nullptr);
        StandardModelProxyWidget(const QAbstractProxyModel &dataProxy,
                                 QWidget *parent = nullptr);
        StandardModelProxyWidget(const StandardModelProxyWidget &) = default;
        StandardModelProxyWidget(StandardModelProxyWidget &&) = default;
        virtual ~StandardModelProxyWidget() = default;

        void installOnView(QAbstractItemView *view);
        void setCharacter(Character::IdType id) noexcept;

        StandardModelProxyWidget &operator =(const StandardModelProxyWidget &) = default;
        StandardModelProxyWidget &operator =(StandardModelProxyWidget &&) = default;

    signals:
        void showInEve(EveType::IdType id, Character::IdType ownerId);

    protected:
        void setDataModel(const ModelWithTypes &dataModel);

    private slots:
        void copyRows() const;
        void selectType(const QItemSelection &selected);
        void showInEveForCurrent();

    private:
        const ModelWithTypes *mDataModel = nullptr;
        const QAbstractProxyModel &mDataProxy;

        QAbstractItemView *mDataView = nullptr;

        QAction *mShowInEveAct = nullptr;
        QAction *mCopyRowsAct = nullptr;

        Character::IdType mCharacterId = Character::invalidId;
    };
}
