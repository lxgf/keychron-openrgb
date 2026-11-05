/*---------------------------------------------------------*\
| ViaTranslation.h                                          |
|                                                           |
|   Follows OpenRGB's interface language. OpenRGB installs  |
|   one translator whose "Settings / English - US" entry is |
|   the language's own name, so asking for that string      |
|   tells which language is active, whatever the settings   |
|   file says.                                              |
|                                                           |
|   Mode names are not translated: OpenRGB matches profile  |
|   modes and "Direct" by name.                             |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <QString>

/*---------------------------------------------------------*\
| Translation of an English UI string for the active        |
| OpenRGB language, or the string itself                    |
\*---------------------------------------------------------*/
QString via_tr(const char* text);
