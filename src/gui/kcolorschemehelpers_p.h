/*
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORSCHEME_P_H
#define KCOLORSCHEME_P_H

#include "kconfiggui_export.h"

#include <KSharedConfig>

#include <QCoreApplication>
#include <QPalette>

KSharedConfigPtr kColorSchemeDefaultConfig();

class KCONFIGGUI_EXPORT StateEffects
{
public:
    explicit StateEffects(QPalette::ColorGroup state, const KSharedConfigPtr &);
    ~StateEffects()
    {
    }

    QBrush brush(const QBrush &background) const;
    QBrush brush(const QBrush &foreground, const QBrush &background) const;

private:
    enum EffectTypes {
        Intensity,
        Color,
        Contrast,
        NEffectTypes,
    };

    enum IntensityEffects {
        IntensityNoEffect,
        IntensityShade,
        IntensityDarken,
        IntensityLighten,
        NIntensityEffects,
    };

    enum ColorEffects {
        ColorNoEffect,
        ColorDesaturate,
        ColorFade,
        ColorTint,
        NColorEffects,
    };

    enum ContrastEffects {
        ContrastNoEffect,
        ContrastFade,
        ContrastTint,
        NContrastEffects,
    };

    int _effects[NEffectTypes];
    double _amount[NEffectTypes];
    QColor _color;
};

#endif
