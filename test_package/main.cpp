#include <iostream>
#include <string>

#include <QString>

#include <KConfig>
#include <KConfigGroup>

static const QString TESTGROUP="General";
static const QString TESTENTRY="TestEntry";
static const QString TESTVALUE="TestValue";

int main() {

    QString configPath = QString(BUILD_DIRECTORY) + "/test_config";

    KConfig config(configPath, KConfig::SimpleConfig);
    KConfigGroup generalGroup( &config, TESTGROUP);

    generalGroup.writeEntry(TESTENTRY, TESTVALUE);
    generalGroup.config()->sync();

    QString readValue = generalGroup.readEntry(TESTENTRY, QString());

    if (readValue == TESTVALUE) {
        std::cout << "Test OK" << std::endl;
        return 0;
    }
    return 1;
}
