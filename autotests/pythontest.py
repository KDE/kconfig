#!/usr/bin/env python
#-*- coding: utf-8 -*-

from __future__ import print_function

import sys

sys.path.append(sys.argv[1])

from PyQt5 import QtCore
from PyQt5 import QtGui
from PyQt5 import QtWidgets

from PyKF5 import KConfigCore
from PyKF5 import KConfigGui

def main():
    app = QtWidgets.QApplication(sys.argv)

    kcg = KConfigCore.KConfigGroup();

    assert(not kcg.isValid())

    sc = KConfigGui.KStandardShortcut.open()
    assert(sc == [QtGui.QKeySequence("CTRL+O")])

if __name__ == '__main__':
    sys.exit(main())
