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
#include <algorithm>

#include <boost/range/adaptor/reversed.hpp>

#include <QtDebug>

#include <QStandardPaths>
#include <QCollator>
#include <QDir>

#include "ChainableFileLogger.h"

namespace Evernus
{
    ChainableFileLogger *ChainableFileLogger::instance = nullptr;

    const QString ChainableFileLogger::fileNameBase = "main.log";

    ChainableFileLogger::ChainableFileLogger(std::size_t maxLogSize, uint maxLogFiles)
        : mMaxLogSize{maxLogSize}
        , mMaxLogFiles{maxLogFiles}
        , mLogFile{getLogDir() + fileNameBase}
    {
        QDir{}.mkpath(getLogDir());

        if (!openLog())
            qWarning() << "Error opening main.log file at:" << getLogDir();
        else
            mPrevHandler = qInstallMessageHandler(&ChainableFileLogger::handleMessage);
    }

    ChainableFileLogger::~ChainableFileLogger()
    {
        qInstallMessageHandler(mPrevHandler);
    }

    void ChainableFileLogger::initialize(std::size_t maxLogSize, uint maxLogFiles)
    {
        Q_ASSERT(instance == nullptr);

        static ChainableFileLogger instance{maxLogSize, maxLogFiles};
        ChainableFileLogger::instance = &instance;
    }

    void ChainableFileLogger::writeMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        if (mPrevHandler != nullptr)
            mPrevHandler(type, context, msg);

        const auto log = qFormatLogMessage(type, context, msg);
        if (Q_LIKELY(!log.isEmpty()))
        {
            std::lock_guard<std::mutex> lock{mStreamMutex};

            if (Q_UNLIKELY(mCurrentLogCheckCount == 0))
            {
                if (Q_UNLIKELY(static_cast<std::size_t>(mLogFile.size()) > mMaxLogSize))
                    rotateLogs();

                mCurrentLogCheckCount = logCheckCount;
            }
            else
            {
                --mCurrentLogCheckCount;
            }

            mStream << log << '\n';
            mStream.flush();
        }
    }

    void ChainableFileLogger::rotateLogs()
    {
        mLogFile.close();

        QDir dir{getLogDir()};

        auto logFiles = dir.entryList({ fileNameBase + ".*" }, QDir::Files);

        QCollator collator;
        collator.setNumericMode(true);

        std::sort(std::begin(logFiles), std::end(logFiles), collator);

        for (const auto &file : logFiles | boost::adaptors::reversed)
        {
            auto ok = false;

            auto number = file.mid(file.lastIndexOf('.') + 1).toUInt(&ok);
            if (Q_UNLIKELY(!ok))
                continue;

            ++number;

            if (number >= mMaxLogFiles && number >= mRotationCount)
                dir.remove(file);
            else
                dir.rename(file, QStringLiteral("%1.%2").arg(fileNameBase).arg(number));
        }

        dir.rename(fileNameBase, fileNameBase + ".0");

        ++mRotationCount;

        if (!openLog())
        {
            qWarning() << "Error re-opening main.log file at:" << getLogDir();
            qInstallMessageHandler(mPrevHandler);
        }
    }

    bool ChainableFileLogger::openLog()
    {
        return mLogFile.open(QIODevice::Append | QIODevice::Text);
    }

    void ChainableFileLogger::handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        Q_ASSERT(instance != nullptr);
        instance->writeMessage(type, context, msg);
    }

    QString ChainableFileLogger::getLogDir()
    {
        return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/log/");
    }
}
