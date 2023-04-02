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

#include <mutex>

#include <QTextStream>
#include <QFile>

namespace Evernus
{
    class ChainableFileLogger final
    {
    public:
        ChainableFileLogger(const ChainableFileLogger &) = delete;
        ChainableFileLogger(ChainableFileLogger &&) = delete;

        ChainableFileLogger &operator =(const ChainableFileLogger &) = delete;
        ChainableFileLogger &operator =(ChainableFileLogger &&) = delete;

        static void initialize(std::size_t maxLogSize, uint maxLogFiles);

    private:
        static ChainableFileLogger *instance;

        static const uint logCheckCount = 100;
        static const QString fileNameBase;

        std::size_t mMaxLogSize = 10 * 1024 * 1024;
        uint mMaxLogFiles = 3;

        QFile mLogFile;
        QTextStream mStream{&mLogFile};
        QtMessageHandler mPrevHandler = nullptr;

        uint mCurrentLogCheckCount = 0;
        uint mRotationCount = 0;    // keep current log chain from being removed

        std::mutex mStreamMutex;

        ChainableFileLogger(std::size_t maxLogSize, uint maxLogFiles);
        ~ChainableFileLogger();

        void writeMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);
        void rotateLogs();

        bool openLog();

        static void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);
        static QString getLogDir();
    };
}
