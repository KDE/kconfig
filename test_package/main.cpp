#include <iostream>
#include <string>

#include <QString>

#include <KConfig>

int main() {

    QString configPath = QString(BUILD_DIRECTORY) + "/test_config";

    KConfig config(configPath, KConfig::NoGlobals);

    std::cout << "Test OK" << std::endl;
    return 0;
}
