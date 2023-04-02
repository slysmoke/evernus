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
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>

#include "DumpUploader.h"

namespace Evernus
{
    DumpUploader::DumpUploader(const QString &dumpPath, QObject *parent)
        : QObject{parent}
        , mDumpDir{dumpPath}
        , mDumpFiles{mDumpDir.entryList({ QStringLiteral("*.dmp") }, QDir::Files | QDir::Readable)}
    {
    }

    void DumpUploader::run() const
    {
        if (Q_LIKELY(mDumpFiles.isEmpty()))
            return;

        const auto ret = QMessageBox::question(nullptr,
                                               tr("Evernus"),
                                               tr("The application crashed in a previous run. Create bug report and upload crash data?"),
                                               QMessageBox::Yes | QMessageBox::No,
                                               QMessageBox::Yes);
        if (ret == QMessageBox::Yes)
            uploadDumps();
        else if (QMessageBox::question(nullptr, tr("Evernus"), tr("Delete crash reports?")) == QMessageBox::Yes)
            removeDumps();
    }

    void DumpUploader::uploadDumps() const
    {
        QDesktopServices::openUrl(QUrl{QStringLiteral("file:///%1").arg(mDumpDir.path()), QUrl::TolerantMode});
        QDesktopServices::openUrl(QUrl{QStringLiteral("https://bitbucket.org/krojew/evernus/issues/new")});
    }

    void DumpUploader::removeDumps() const
    {
        for (const auto &file : mDumpFiles)
            mDumpDir.remove(file);
    }
}
