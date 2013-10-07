#ifndef KEYSEQUENCES_H
#define KEYSEQUENCES_H

#define QMC2_ARCADE_ADD_COMMON_KEYSEQUENCES(stringList)         (stringList) << "Up" \
                                                                             << "Down" \
                                                                             << "Left" \
                                                                             << "Right" \
                                                                             << "PgUp" \
                                                                             << "PgDown" \
                                                                             << "Home" \
                                                                             << "End" \
                                                                             << "Esc" \
                                                                             << "Enter" \
                                                                             << "Return"
#define QMC2_ARCADE_ADD_COMMON_DESCRIPTIONS(stringList)         (stringList) << QObject::tr("Cursor up") \
                                                                             << QObject::tr("Cursor down") \
                                                                             << QObject::tr("Cursor left") \
                                                                             << QObject::tr("Cursor right") \
                                                                             << QObject::tr("Page up") \
                                                                             << QObject::tr("Page down") \
                                                                             << QObject::tr("Start of list") \
                                                                             << QObject::tr("End of list") \
                                                                             << QObject::tr("Exit") \
                                                                             << QObject::tr("Start emulation") \
                                                                             << QObject::tr("Start emulation")
#define QMC2_ARCADE_ADD_TOXIXCWASTE_KEYSEQUENCES(stringList)    (stringList) << "F11" \
                                                                             << "Alt+Enter" \
                                                                             << "Alt+Return" \
                                                                             << "Ctrl+F" \
                                                                             << "Ctrl+M" \
                                                                             << "Ctrl+O" \
                                                                             << "Ctrl+P" \
                                                                             << "Ctrl+X" \
                                                                             << "Ctrl+Backspace"
#define QMC2_ARCADE_ADD_TOXIXCWASTE_DESCRIPTIONS(stringList)    (stringList) << QObject::tr("Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("Focus search box") \
                                                                             << QObject::tr("Toggle menu-bar") \
                                                                             << QObject::tr("Toggle preferences") \
                                                                             << QObject::tr("Start emulation") \
                                                                             << QObject::tr("Exit") \
                                                                             << QObject::tr("Flip cabinet / game-card")
#define QMC2_ARCADE_ADD_DARKONE_KEYSEQUENCES(stringList)        ; // FIXME: add darkone-specific key-sequences
#define QMC2_ARCADE_ADD_DARKONE_DESCRIPTIONS(stringList)        ; // FIXME: add darkone-specific key-sequence descriptions

#endif // KEYSEQUENCES_H
