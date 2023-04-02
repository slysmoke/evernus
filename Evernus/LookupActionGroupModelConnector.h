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

#include <QObject>

#include "EveTypeProvider.h"

class QSortFilterProxyModel;
class QAbstractItemView;
class QItemSelection;

namespace Evernus
{
    class LookupActionGroup;
    class ModelWithTypes;

    class LookupActionGroupModelConnector
        : public QObject
        , public EveTypeProvider
    {
        Q_OBJECT

    public:
        LookupActionGroupModelConnector(ModelWithTypes &model,
                                        const QSortFilterProxyModel &proxy,
                                        QAbstractItemView &view,
                                        QObject *parent = nullptr);
        LookupActionGroupModelConnector(const LookupActionGroupModelConnector &) = default;
        LookupActionGroupModelConnector(LookupActionGroupModelConnector &&) = default;
        virtual ~LookupActionGroupModelConnector() = default;

        virtual EveType::IdType getTypeId() const override;

        LookupActionGroupModelConnector &operator =(const LookupActionGroupModelConnector &) = default;
        LookupActionGroupModelConnector &operator =(LookupActionGroupModelConnector &&) = default;

    private slots:
        void selectType(const QItemSelection &selected);

    private:
        const ModelWithTypes &mModel;
        const QSortFilterProxyModel &mProxy;
        const QAbstractItemView &mView;

        LookupActionGroup *mLookupGroup = nullptr;
    };
}
