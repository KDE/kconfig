/*
Copyright (c) 2003,2004 Waldo Bastian <bastian@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "test4.h"
#include <QGuiApplication>
#include <QStandardPaths>
#include <QFile>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    {
        KConfig initialConfig(QLatin1String("test4rc"));
        KConfigGroup group = initialConfig.group(QLatin1String("Foo"));
        group.writeEntry(QLatin1String("foo bar"), QStringLiteral("Value"));
    }
    Test4 *t = Test4::self();
    bool ok = QFile::exists(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/test4rc");
    if (!ok) {
        qWarning() << "config file was not created!";
    }
    if (t->fooBar() != QStringLiteral("Value")) {
        qWarning() << "wrong value for foo bar:" << t->fooBar();
    }
    delete t;
    return ok ? 0 : 1;
}
