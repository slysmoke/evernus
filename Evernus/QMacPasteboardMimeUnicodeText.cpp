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
#include "QMacPasteboardMimeUnicodeText.h"

#ifdef Q_OS_OSX

#include <QTextCodec>

QString QMacPasteboardMimeUnicodeText::convertorName()
{
    return QLatin1String("UnicodeTextUtf8Default");
}

QString QMacPasteboardMimeUnicodeText::flavorFor(const QString &mime)
{
    if (mime == QLatin1String("text/plain"))
        return QLatin1String("public.utf8-plain-text");
    int i = mime.indexOf(QLatin1String("charset="));
    if (i >= 0) {
        QString cs(mime.mid(i+8).toLower());
        i = cs.indexOf(QLatin1Char(';'));
        if (i>=0)
            cs = cs.left(i);
        if (cs == QLatin1String("system"))
            return QLatin1String("public.utf8-plain-text");
        else if (cs == QLatin1String("iso-10646-ucs-2")
                 || cs == QLatin1String("utf16"))
            return QLatin1String("public.utf16-plain-text");
    }
    return QString();
}

QString QMacPasteboardMimeUnicodeText::mimeFor(QString flav)
{
    if (flav == QLatin1String("public.utf8-plain-text"))
        return QLatin1String("text/plain");
    if (flav == QLatin1String("public.utf16-plain-text"))
        return QLatin1String("text/plain;charset=utf16");
    return QString();
}

bool QMacPasteboardMimeUnicodeText::canConvert(const QString &mime, QString flav)
{
    return (mime.startsWith(QLatin1String("text/plain"))
            && (flav == QLatin1String("public.utf8-plain-text") || (flav == QLatin1String("public.utf16-plain-text"))));
}

QVariant QMacPasteboardMimeUnicodeText::convertToMime(const QString &mimetype, QList<QByteArray> data, QString flavor)
{
    if (data.count() > 1)
        qWarning("QMacPasteboardMimeUnicodeText: Cannot handle multiple member data");
    const QByteArray &firstData = data.first();
    // I can only handle two types (system and unicode) so deal with them that way
    QVariant ret;
    if (flavor == QLatin1String("public.utf8-plain-text")) {
        ret = QString::fromUtf8(firstData);
    } else if (flavor == QLatin1String("public.utf16-plain-text")) {
        ret = QTextCodec::codecForName("UTF-16")->toUnicode(firstData);
    } else {
        qWarning("QMime::convertToMime: unhandled mimetype: %s", qPrintable(mimetype));
    }
    return ret;
}

QList<QByteArray> QMacPasteboardMimeUnicodeText::convertFromMime(const QString &, QVariant data, QString flavor)
{
    QList<QByteArray> ret;
    QString string = data.toString();
    if (flavor == QLatin1String("public.utf8-plain-text"))
        ret.append(string.toUtf8());
    else if (flavor == QLatin1String("public.utf16-plain-text"))
        ret.append(QTextCodec::codecForName("UTF-16")->fromUnicode(string));
    return ret;
}

#endif
