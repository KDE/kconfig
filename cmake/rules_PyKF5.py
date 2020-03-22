#
# SPDX-FileCopyrightText: 2016 Shaheed Haque <srhaque@theiet.org>
# SPDX-FileCopyrightText: 2016 Stephen Kelly <steveire@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause


import os, sys

import rules_engine
sys.path.append(os.path.dirname(os.path.dirname(rules_engine.__file__)))
import Qt5Ruleset

from copy import deepcopy

from clang.cindex import AccessSpecifier, CursorKind

def set_skeleton_item_base(container, sip, matcher):
    if not sip["base_specifiers"] or sip["base_specifiers"][-1].endswith(">"):
        sip["base_specifiers"] = ["KConfigSkeletonItem"]

def set_skeleton_item_base_gui(container, sip, matcher):
    sip["base_specifiers"] = ["KConfigSkeletonItem"]

def mark_and_discard_QSharedData(container, sip, matcher):
    rules_engine.container_mark_abstract(container, sip, matcher)
    rules_engine.discard_QSharedData_base(container, sip, matcher)

def discard_base(container, sip, matcher):
    sip["base_specifiers"] = []

def local_container_rules():
    return [
        [".*", "KConfigBase", ".*", ".*", ".*", rules_engine.container_mark_abstract],

        [".*KCoreConfigSkeleton.*", ".*ItemString", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemUrl", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemProperty", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemBool", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemInt", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemLongLong", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemUInt", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemULongLong", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemDouble", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemRect", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemPoint", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemSize", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemDateTime", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemStringList", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemUrlList", ".*", ".*", ".*", set_skeleton_item_base],
        [".*KCoreConfigSkeleton.*", ".*ItemIntList", ".*", ".*", ".*", set_skeleton_item_base],

        ["KConfigSkeleton", "ItemColor", ".*", ".*", ".*", set_skeleton_item_base_gui],
        ["KConfigSkeleton", "ItemFont", ".*", ".*", ".*", set_skeleton_item_base_gui],

        [".*", "KSharedConfig", ".*", ".*", ".*", rules_engine.discard_QSharedData_base],

        [".*", "KConfigCompilerSignallingItem", ".*", ".*", ".*", rules_engine.container_discard],

        [".*", "KEntryMap", ".*", ".*", ".*", discard_base],
    ]

def local_function_rules():
    return [
        ["KConfigBase", "group", ".*", ".*", ".*const char.*", rules_engine.function_discard],
        ["KConfigBase", "group", ".*", ".*", ".*QByteArray.*", rules_engine.function_discard],
        ["KConfigBase", "group", ".*", "const KConfigGroup", ".*", rules_engine.function_discard],

        ["KConfigBase", "groupImpl", ".*", "const KConfigGroup", ".*", rules_engine.function_discard],
        ["KConfig", "groupImpl", ".*", "const KConfigGroup", ".*", rules_engine.function_discard],

        ["KSharedConfig", "openConfig", ".*", ".*", ".*", rules_engine.function_discard],
        ["KSharedConfig", "openStateConfig", ".*", ".*", ".*", rules_engine.function_discard],

        ["KConfigGroup", "KConfigGroup", ".*", ".*", ".*KConfigBase.*", rules_engine.function_discard],
        ["KConfigGroup", "config", ".*", "const KConfig.*", ".*", rules_engine.function_discard],

        ["KConfigWatcher", "config", ".*", "KSharedConfig::Ptr", ".*", rules_engine.function_discard],

        ["KDesktopFile", ".*", ".*", "const KConfigGroup", ".*", rules_engine.function_discard],

        ["KConfigGroup", ".*", ".*", "KConfigGroup", ".*", rules_engine.function_discard],

        ["KCoreConfigSkeleton", "config", ".*", "const KConfig.*", ".*", rules_engine.function_discard],
        ["KCoreConfigSkeleton", "sharedConfig", ".*", ".*", ".*", rules_engine.function_discard],

        ["KEntryMap", "getEntryOption", ".*", ".*", ".*", rules_engine.function_discard],
        ["KEntryMap", "setEntryOption", ".*", ".*", ".*", rules_engine.function_discard],
        ["KEntryMap", "findEntry", ".*", ".*", ".*", rules_engine.function_discard],
        ["KEntryMap", "findExactEntry", ".*", ".*", ".*", rules_engine.function_discard],
    ]

def local_typedef_rules():
    return [
        ["KConfigSkeletonItem", "DictIterator", rules_engine.typedef_discard],
        [".*", "KEntryMapIterator", rules_engine.typedef_discard],
        [".*", "KEntryMapConstIterator", rules_engine.typedef_discard],
    ]


def _kcoreconfigskeleton_item_xxx(function, sip, entry):
    sip["code"] = """
        %MethodCode
            sipCpp = new sipKCoreConfigSkeleton_Item{} (*a0, *a1, a2, a3);
        %End
        """.replace("{}", entry["ctx"])
    sip["parameters"][2] = sip["parameters"][2].replace("&", "")


def _kcoreconfigskeleton_item_enum(function, sip, entry):
    sip["code"] = """
        %MethodCode
            sipCpp = new sipKCoreConfigSkeleton_ItemEnum (*a0, *a1, a2, *a3, a4);
        %End
        """.replace("{}", entry["ctx"])
    sip["parameters"][2] = sip["parameters"][2].replace("&", "")


def _kcoreconfigskeleton_add_item_xxx(function, sip, entry):
    sip["code"] = """
        %MethodCode
            sipRes = new PyItem{} (sipCpp->currentGroup(), a3->isNull() ? *a0 : *a3, a1, a2);
            sipCpp->addItem(sipRes, *a0);
        %End
        """.format(entry["ctx"])

def _kcoreconfigskeleton_item_add_py_subclass(filename, sip, entry):
    result = """
%ModuleHeaderCode
#include <kcoreconfigskeleton.h>
"""
    for ctx in ({"Type": "Bool",      "cpptype": "bool",    "defaultValue": 1},
                {"Type": "Int",       "cpptype": "qint32",  "defaultValue": 1},
                {"Type": "UInt",      "cpptype": "quint32", "defaultValue": 1},
                {"Type": "LongLong",  "cpptype": "qint64",  "defaultValue": 1},
                {"Type": "ULongLong", "cpptype": "quint64", "defaultValue": 1},
                {"Type": "Double",    "cpptype": "double",  "defaultValue": 1},
        ):
        result += """
class PyItem{Type} : public KCoreConfigSkeleton::Item{Type}
{{
public:
    PyItem{Type} (const QString &group, const QString &key, {cpptype}& val, {cpptype} defaultValue = {defaultValue}) :
        KCoreConfigSkeleton::Item{Type} (group, key, this->value, defaultValue),
        value(val)
    {{
    }}

private:
    {cpptype} value;
}};
""".format(**ctx)

    result += """
class PyItemEnum : public KCoreConfigSkeleton::ItemEnum
{
public:
    PyItemEnum (const QString& group, const QString& key, int& val, const QList<KCoreConfigSkeleton::ItemEnum::Choice>& choices, int defaultValue = 0) :
        KCoreConfigSkeleton::ItemEnum(group, key, this->value, choices, defaultValue),
        value(val)
    {
    };

private:
    int value;
};
%End\n
"""

    sip["code"] = result


class RuleSet(Qt5Ruleset.RuleSet):
    def __init__(self):
        Qt5Ruleset.RuleSet.__init__(self)
        self._fn_db = rules_engine.FunctionRuleDb(lambda: local_function_rules() + Qt5Ruleset.function_rules())
        self._container_db = rules_engine.ContainerRuleDb(lambda: local_container_rules() + Qt5Ruleset.container_rules())
        self._typedef_db = rules_engine.TypedefRuleDb(lambda: local_typedef_rules() + Qt5Ruleset.typedef_rules())
        self._methodcode = rules_engine.MethodCodeDb({
            "KCoreConfigSkeleton::ItemBool":
            {
                "ItemBool":
                {
                    "code": _kcoreconfigskeleton_item_xxx,
                    "ctx": "Bool",
                },
            },
            "KCoreConfigSkeleton::ItemInt":
            {
                "ItemInt":
                {
                    "code": _kcoreconfigskeleton_item_xxx,
                    "ctx": "Int",
                },
            },
            "KCoreConfigSkeleton::ItemLongLong":
            {
                "ItemLongLong":
                {
                    "code": _kcoreconfigskeleton_item_xxx,
                    "ctx": "LongLong",
                },
            },
            "KCoreConfigSkeleton::ItemEnum":
            {
                "ItemEnum":
                {
                    "code": _kcoreconfigskeleton_item_enum,
                    "ctx": "Enum",
                },
            },
            "KCoreConfigSkeleton::ItemUInt":
            {
                "ItemUInt":
                {
                    "code": _kcoreconfigskeleton_item_xxx,
                    "ctx": "UInt",
                },
            },
            "KCoreConfigSkeleton::ItemULongLong":
            {
                "ItemULongLong":
                {
                    "code": _kcoreconfigskeleton_item_xxx,
                    "ctx": "ULongLong",
                },
            },
            "KCoreConfigSkeleton::ItemDouble":
            {
                "ItemDouble":
                {
                    "code": _kcoreconfigskeleton_item_xxx,
                    "ctx": "Double",
                },
            },
            "KCoreConfigSkeleton":
            {
                "addItemBool":
                {
                    "code": _kcoreconfigskeleton_add_item_xxx,
                    "ctx": "Bool",
                },
                "addItemInt":
                {
                    "code": _kcoreconfigskeleton_add_item_xxx,
                    "ctx": "Int",
                },
                "addItemUInt":
                {
                    "code": _kcoreconfigskeleton_add_item_xxx,
                    "ctx": "UInt",
                },
                "addItemLongLong":
                {
                    "code": _kcoreconfigskeleton_add_item_xxx,
                    "ctx": "LongLong",
                },
                "addItemInt64":
                {
                    "code": _kcoreconfigskeleton_add_item_xxx,
                    "ctx": "LongLong",
                },
                "addItemULongLong":
                {
                    "code": _kcoreconfigskeleton_add_item_xxx,
                    "ctx": "ULongLong",
                },
                "addItemUInt64":
                {
                    "code": _kcoreconfigskeleton_add_item_xxx,
                    "ctx": "ULongLong",
                },
                "addItemDouble":
                {
                    "code": _kcoreconfigskeleton_add_item_xxx,
                    "ctx": "Double",
                },
            },
        })

        self._modulecode = rules_engine.ModuleCodeDb({
            "kcoreconfigskeleton.h":
            {
                "code": _kcoreconfigskeleton_item_add_py_subclass,
            },
            })
