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
#include <QtGlobal>

#ifdef Q_OS_OSX

#include <QMacPasteboardMime>

class QMacPasteboardMimeUnicodeText : public QMacPasteboardMime {
public:
    QMacPasteboardMimeUnicodeText() : QMacPasteboardMime(MIME_ALL) { }
    QString convertorName() override;

    QString flavorFor(const QString &mime) override;
    QString mimeFor(QString flav) override;
    bool canConvert(const QString &mime, QString flav) override;
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav) override;
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav) override;
};

#endif
