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

#include <QActionGroup>

namespace Evernus
{
    class EveTypeProvider;

    class LookupActionGroup
        : public QActionGroup
    {
        Q_OBJECT

    public:
        LookupActionGroup(const EveTypeProvider &typeProvider, QObject *parent);
        LookupActionGroup(const LookupActionGroup &) = default;
        LookupActionGroup(LookupActionGroup &&) = default;
        virtual ~LookupActionGroup() = default;

        LookupActionGroup &operator =(const LookupActionGroup &) = default;
        LookupActionGroup &operator =(LookupActionGroup &&) = default;

    private slots:
        void lookupOnEveMarketdata();
        void lookupOnEveMarketer();

    private:
        const EveTypeProvider &mTypeProvider;

        void lookupOnWeb(const QString &baseUrl) const;
    };
}
