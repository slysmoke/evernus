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

#include <functional>

#include <QEvent>

namespace Evernus
{
    class CallbackEvent final
        : public QEvent
    {
    public:
        using Callback = std::function<void ()>;

        explicit CallbackEvent(Callback callback);
        CallbackEvent(const CallbackEvent &) = default;
        CallbackEvent(CallbackEvent &&) = default;
        virtual ~CallbackEvent() = default;

        void execute();

        CallbackEvent &operator =(const CallbackEvent &) = default;
        CallbackEvent &operator =(CallbackEvent &&) = default;

        static Type customType() noexcept;

    private:
        static const int mType;

        Callback mCallback;
    };
}
