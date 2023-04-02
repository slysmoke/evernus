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

#include <QStringList>
#include <QObject>
#include <QDir>

namespace Evernus
{
    class DumpUploader final
        : public QObject
    {
        Q_OBJECT

    public:
        explicit DumpUploader(const QString &dumpPath, QObject *parent = nullptr);
        virtual ~DumpUploader() = default;

        void run() const;

    private:
        mutable QDir mDumpDir;
        QStringList mDumpFiles;

        void uploadDumps() const;
        void removeDumps() const;
    };
}
