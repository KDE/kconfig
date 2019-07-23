from conans import ConanFile, CMake
import yaml
import re
import os.path


def getVersion():
    if(os.path.exists("CMakeLists.txt")):
        regx = re.compile(r"^set\(.*VERSION\s(\"|')[0-9.]+(\"|')\)")
        with open("CMakeLists.txt") as f:
            for line in f:
                if regx.match(line):
                    version = re.search("\"[0-9\.]+\"", line)
                    version = version.group().replace("\"", "")
                    return version
    return None


def getMetaField(field):
    if(os.path.exists("metainfo.yaml")):
        with open("metainfo.yaml") as f:
            metainfo = yaml.load(f.read())
        return metainfo[field]
    return None


class KConfigConan(ConanFile):
    name = getMetaField('name')
    version = getVersion()
    license = getMetaField('license')
    url = getMetaField('url')
    description = getMetaField('description')

    settings = "os", "compiler", "build_type", "arch"

    requires = (
        # CMakeLists.txt requires 5.49.0
        "extra-cmake-modules/[>=5.50.0]@kde/testing",

        "qt/[>=5.11.3]@bincrafters/stable"
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
