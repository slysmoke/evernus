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

#include <exception>

#include <QException>

namespace Evernus
{
    class StandardExceptionQtWrapperException final
        : public QException
    {
    public:
        StandardExceptionQtWrapperException(std::exception_ptr e)
            : QException{}
            , mException{e}
        {
        }

        StandardExceptionQtWrapperException(const StandardExceptionQtWrapperException &) = default;
        StandardExceptionQtWrapperException(StandardExceptionQtWrapperException &&) = default;
        virtual ~StandardExceptionQtWrapperException() = default;

        virtual QException *clone() const override
        {
            return new StandardExceptionQtWrapperException{*this};
        }

        virtual void raise() const override
        {
            throw *this;
        }

        [[noreturn]] void rethrow() const
        {
            std::rethrow_exception(mException);
        }

        StandardExceptionQtWrapperException &operator =(const StandardExceptionQtWrapperException &) = default;
        StandardExceptionQtWrapperException &operator =(StandardExceptionQtWrapperException &&) = default;

    private:
        std::exception_ptr mException;
    };
}
