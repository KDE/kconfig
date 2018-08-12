from conans import ConanFile, CMake


class KConfigConan(ConanFile):
    name = "kconfig"
    version = "5.50.0"
    license = "GPLv2"
    url = "https://api.kde.org/frameworks/kconfig/html/index.html"
    description = "Persistent platform-independent application settings."

    settings = "os", "compiler", "build_type", "arch"

    requires = (
        "extra-cmake-modules/5.50.0@kde/testing", # CMakeLists.txt requires 5.49.0

        "Qt/5.11.1@bincrafters/stable"
        # "qt-gui/5.8.0@qt/testing",
        # "qt-xml/5.8.0@qt/testing",
    )

    generators = "cmake"
    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
     }

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def package_info(self):
        self.cpp_info.resdirs = ["share"]
