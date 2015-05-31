#ifndef KEYSEQUENCES_H
#define KEYSEQUENCES_H

// common key-sequences
#define QMC2_ARCADE_ADD_COMMON_KEYSEQUENCES(stringList)         (stringList) << "Ctrl+P" \
                                                                             << "F11" \
                                                                             << "Alt+Enter" \
                                                                             << "Alt+Return" \
                                                                             << "Alt+F" \
                                                                             << "Ctrl+O"

// common key-sequence descriptions
#define QMC2_ARCADE_ADD_COMMON_DESCRIPTIONS(stringList)         (stringList) << QObject::tr("[global] Start emulation") \
                                                                             << QObject::tr("[global] Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("[global] Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("[global] Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("[global] Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("[global] Toggle preferences")
// ToxicWaste-specific key-sequences
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
                                                                             << "Enter" \
                                                                             << "Return" \
                                                                             << "Esc" \
                                                                             << "Ctrl+X" \
                                                                             << "Ctrl+Backspace"
// ToxicWaste-specific key-sequence descriptions
#define QMC2_ARCADE_ADD_TOXIXCWASTE_DESCRIPTIONS(stringList)    (stringList) << QObject::tr("[global] Cursor up") \
                                                                             << QObject::tr("[global] Cursor down") \
                                                                             << QObject::tr("[global] Cursor left") \
                                                                             << QObject::tr("[global] Cursor right") \
                                                                             << QObject::tr("[global] Page up") \
                                                                             << QObject::tr("[global] Page down") \
                                                                             << QObject::tr("[global] Start of list") \
                                                                             << QObject::tr("[global] End of list") \
                                                                             << QObject::tr("[global] Focus search box") \
                                                                             << QObject::tr("[global] Toggle menu-bar") \
                                                                             << QObject::tr("[global] Start emulation") \
                                                                             << QObject::tr("[global] Start emulation") \
                                                                             << QObject::tr("[global] Exit") \
                                                                             << QObject::tr("[global] Exit") \
                                                                             << QObject::tr("[global] Flip cabinet / machine-card")
// darkone-specific key-sequences
#define QMC2_ARCADE_ADD_DARKONE_KEYSEQUENCES(stringList)        (stringList) << "Ctrl+Up" \
                                                                             << "Ctrl+Down" \
                                                                             << "Left" \
                                                                             << "Right" \
                                                                             << "Up" \
                                                                             << "Down" \
                                                                             << "Enter" \
                                                                             << "Esc" \
                                                                             << "Ctrl+Shift+Up" \
                                                                             << "Ctrl+Shift+Down" \
                                                                             << "PgUp" \
                                                                             << "PgDown" \
                                                                             << "Home" \
                                                                             << "End" \
                                                                             << "1" \
                                                                             << "2" \
                                                                             << "Tab" \
                                                                             << "Ctrl+Right" \
                                                                             << "Backtab" \
                                                                             << "Ctrl+Left" \
                                                                             << "Plus" \
                                                                             << "Minus" \
                                                                             << "Ctrl+S" \
                                                                             << "Ctrl+L" \
                                                                             << "Ctrl+T" \
                                                                             << "Ctrl+Q"
// darkone-specific key-sequence descriptions
#define QMC2_ARCADE_ADD_DARKONE_DESCRIPTIONS(stringList)       (stringList)  << QObject::tr("[context] Previous component / List page up / Info page up") \
                                                                             << QObject::tr("[context] Next component / List page down / Info page down") \
                                                                             << QObject::tr("[context] Hide list / Previous item / Slide left / Cycle backwards") \
                                                                             << QObject::tr("[context] Show list / Next item / Slide right / Cycle forwards") \
                                                                             << QObject::tr("[context] Show toolbar / List up / Info up / Previous widget") \
                                                                             << QObject::tr("[context] Hide toolbar / List down / Info down / Next widget") \
                                                                             << QObject::tr("[context] Select / Set / Toggle details / Start emulation") \
                                                                             << QObject::tr("[context] Abort machine launch / Hide preferences") \
                                                                             << QObject::tr("[context] Zoom in / List top") \
                                                                             << QObject::tr("[context] Zoom out / List bottom") \
                                                                             << QObject::tr("[context] List page up / Flick page up") \
                                                                             << QObject::tr("[context] List page down / Flick page down") \
                                                                             << QObject::tr("[context] List top") \
                                                                             << QObject::tr("[context] List bottom") \
                                                                             << QObject::tr("[context] Set primary display data item") \
                                                                             << QObject::tr("[context] Set secondary display data item") \
                                                                             << QObject::tr("[global] Next component") \
                                                                             << QObject::tr("[global] Next component") \
                                                                             << QObject::tr("[global] Previous component") \
                                                                             << QObject::tr("[global] Previous component") \
                                                                             << QObject::tr("[global] Zoom in") \
                                                                             << QObject::tr("[global] Zoom out") \
                                                                             << QObject::tr("[global] Search") \
                                                                             << QObject::tr("[global] Toggle list") \
                                                                             << QObject::tr("[global] Toggle toolbar") \
                                                                             << QObject::tr("[global] Exit")

#endif // KEYSEQUENCES_H
