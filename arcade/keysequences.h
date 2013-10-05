#ifndef KEYSEQUENCES_H
#define KEYSEQUENCES_H

#define QMC2_ARCADE_ADD_COMMON_KEYSEQUENCES(stringList)         (stringList) << "Up" << "Down" << "Left" << "Right" << "PgUp" << "PgDown" << "Pos1" << "End" << "Esc" << "Enter" << "Return"
#define QMC2_ARCADE_ADD_COMMON_DESCRIPTIONS(stringList)         (stringList) << tr("Cursor up") << tr("Cursor down") << tr("Cursor left") << tr("Cursor right") << tr("Page up") << tr("Page down") << tr("Start of list") << tr("End of list") << tr("Exit") << tr("Start emulation") << tr("Start emulation");
#define QMC2_ARCADE_ADD_TOXIXCWASTE_KEYSEQUENCES(stringList)    (stringList) << "F11" << "Alt+Enter" << "Alt+Return" << "Ctrl+F" << "Ctrl+M" << "Ctrl+O" << "Ctrl+P" << "Ctrl+X"
#define QMC2_ARCADE_ADD_TOXIXCWASTE_DESCRIPTIONS(stringList)    (stringList) << tr("Toggle full-screen / windowed mode") << tr("Toggle full-screen / windowed mode") << tr("Toggle full-screen / windowed mode") << tr("Focus search box") << tr("Toggle menu-bar") << tr("Toggle preferences") << tr("Start emulation") << tr("Exit");
#define QMC2_ARCADE_ADD_DARKONE_KEYSEQUENCES(stringList)        ; // FIXME: add darkone-specific key-sequences
#define QMC2_ARCADE_ADD_DARKONE_DESCRIPTIONS(stringList)        ; // FIXME: add darkone-specific key-sequence descriptions

#endif // KEYSEQUENCES_H
