#ifndef KEYSEQUENCES_H
#define KEYSEQUENCES_H

// common key-sequences
#define QMC2_ARCADE_ADD_COMMON_KEYSEQUENCES(stringList)         (stringList) << "Enter" \
                                                                             << "Return" \
                                                                             << "F11" \
                                                                             << "Alt+Enter" \
                                                                             << "Alt+Return"

// common key-sequence descriptions
#define QMC2_ARCADE_ADD_COMMON_DESCRIPTIONS(stringList)         (stringList) << QObject::tr("Start emulation") \
                                                                             << QObject::tr("Start emulation") \
                                                                             << QObject::tr("Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("Toggle full-screen / windowed mode")
// toxicwaste-specific key-sequences
#define QMC2_ARCADE_ADD_TOXIXCWASTE_KEYSEQUENCES(stringList)    (stringList) << "Up" \
                                                                             << "Down" \
                                                                             << "Left" \
                                                                             << "Right" \
                                                                             << "PgUp" \
                                                                             << "PgDown" \
                                                                             << "Home" \
                                                                             << "End" \
                                                                             << "Ctrl+F" \
                                                                             << "Ctrl+M" \
                                                                             << "Ctrl+O" \
                                                                             << "Ctrl+P" \
                                                                             << "Esc" \
                                                                             << "Ctrl+X" \
                                                                             << "Ctrl+Backspace"
// toxicwaste-specific key-sequence dscriptions
#define QMC2_ARCADE_ADD_TOXIXCWASTE_DESCRIPTIONS(stringList)    (stringList) << QObject::tr("Cursor up") \
                                                                             << QObject::tr("Cursor down") \
                                                                             << QObject::tr("Cursor left") \
                                                                             << QObject::tr("Cursor right") \
                                                                             << QObject::tr("Page up") \
                                                                             << QObject::tr("Page down") \
                                                                             << QObject::tr("Start of list") \
                                                                             << QObject::tr("End of list") \
                                                                             << QObject::tr("Focus search box") \
                                                                             << QObject::tr("Toggle menu-bar") \
                                                                             << QObject::tr("Toggle preferences") \
                                                                             << QObject::tr("Start emulation") \
                                                                             << QObject::tr("Exit") \
                                                                             << QObject::tr("Exit") \
                                                                             << QObject::tr("Flip cabinet / game-card")
// darkone-specific key-sequences
#define QMC2_ARCADE_ADD_DARKONE_KEYSEQUENCES(stringList)        (stringList) << "Ctrl+Shift+Up" \
                                                                             << "Ctrl+Shift+Down" \
                                                                             << "Ctrl+Up" \
                                                                             << "Ctrl+Down" \
                                                                             << "Space" \
                                                                             << "Up" \
                                                                             << "PgUp" \
                                                                             << "Home" \
                                                                             << "Down" \
                                                                             << "PgDown" \
                                                                             << "End" \
                                                                             << "Left" \
                                                                             << "Right" \
                                                                             << "Tab" \
                                                                             << "Ctrl+Right" \
                                                                             << "Shift+Tab" \
                                                                             << "Ctrl+Left" \
                                                                             << "Plus" \
                                                                             << "Minus" \
                                                                             << "Ctrl+P" \
                                                                             << "Ctrl+O" \
                                                                             << "Ctrl+L" \
                                                                             << "Ctrl+T" \
                                                                             << "Ctrl+S" \
                                                                             << "Alt+F" \
                                                                             << "Esc" \
                                                                             << "Ctrl+Q"
// darkone-specific key-sequence descriptions
#define QMC2_ARCADE_ADD_DARKONE_DESCRIPTIONS(stringList)       (stringList)  << QObject::tr("Zoom in / List top") \
                                                                             << QObject::tr("Zoom out / List bottom") \
                                                                             << QObject::tr("Previous widget / List page up") \
                                                                             << QObject::tr("Next widget / List page down") \
                                                                             << QObject::tr("Select") \
                                                                             << QObject::tr("List up") \
                                                                             << QObject::tr("List page up") \
                                                                             << QObject::tr("List top") \
                                                                             << QObject::tr("List down") \
                                                                             << QObject::tr("List page down") \
                                                                             << QObject::tr("List bottom") \
                                                                             << QObject::tr("Hide list") \
                                                                             << QObject::tr("Show list") \
                                                                             << QObject::tr("Next widget") \
                                                                             << QObject::tr("Next widget") \
                                                                             << QObject::tr("Previous widget") \
                                                                             << QObject::tr("Previous widget") \
                                                                             << QObject::tr("Zoom in") \
                                                                             << QObject::tr("Zoom out") \
                                                                             << QObject::tr("Start emulation") \
                                                                             << QObject::tr("Toggle options") \
                                                                             << QObject::tr("Toggle list") \
                                                                             << QObject::tr("Toggle toolbar") \
                                                                             << QObject::tr("Search") \
                                                                             << QObject::tr("Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("Abort game launch") \
                                                                             << QObject::tr("Exit")

#endif // KEYSEQUENCES_H
