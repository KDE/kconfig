#!/bin/bash -eu
#
# SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>
# SPDX-License-Identifier: LGPL-2.0-or-later

apt-get update && \
    apt-get install -y ninja-build

pip install cmake

git clone --depth 1 --branch=dev git://code.qt.io/qt/qtbase.git
git clone --depth 1 -b master https://invent.kde.org/frameworks/extra-cmake-modules.git
