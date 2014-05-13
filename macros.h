#ifndef _MACROS_H_
#define _MACROS_H_

#include <Qt>

// global OS macros for supported target operating systems
#if (defined(Q_OS_UNIX) || defined(Q_OS_LINUX)) && !defined(Q_OS_MAC)
#define QMC2_OS_UNIX
#elif defined(Q_OS_MAC)
#define QMC2_OS_MAC
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#define QMC2_OS_WIN
#else
#warning "Target OS is not supported -- QMC2 currently supports Linux/UNIX, Windows and Mac OS X!"
#endif

#if !defined(Q_OS_WIN)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <stdio.h>

// don't use SVN revision in version strings when QMC2_SVN_REV is undefined
#if !defined(QMC2_SVN_REV)
#define QMC2_SVN_REV				0
#endif

// standard sizes
#define QMC2_ONE_KILOBYTE			1024
#define QMC2_ONE_MEGABYTE			1048576
#define QMC2_ONE_GIGABYTE			1073741824
#define QMC2_ONE_TERABYTE			1099511627776
#define QMC2_1K					QMC2_ONE_KILOBYTE
#define QMC2_4K					4 * QMC2_ONE_KILOBYTE
#define QMC2_8K					8 * QMC2_ONE_KILOBYTE
#define QMC2_16K				16 * QMC2_ONE_KILOBYTE
#define QMC2_32K				32 * QMC2_ONE_KILOBYTE
#define QMC2_64K				64 * QMC2_ONE_KILOBYTE
#define QMC2_128K				128 * QMC2_ONE_KILOBYTE
#define QMC2_256K				256 * QMC2_ONE_KILOBYTE
#define QMC2_512K				512 * QMC2_ONE_KILOBYTE
#define QMC2_1M					QMC2_ONE_MEGABYTE
#define QMC2_1G					QMC2_ONE_GIGABYTE
#define QMC2_1T					QMC2_ONE_TERABYTE

// min/max of two constants
#define QMC2_MAX(a, b)				(((a) > (b)) ? (a) : (b))
#define QMC2_MIN(a, b)				(((a) < (b)) ? (a) : (b))

// determine file existance
#if !defined(QMC2_OS_WIN)
#include <unistd.h>
#define EXISTS(fn)				(access(fn, F_OK) == 0)
#endif

// this is strange, but it's the only solution I found to make a string out of a non-string constant
#define STR(s)					#s
#define XSTR(s)					STR(s)

// logger positions
#define QMC2_LOG_FRONTEND			1
#define QMC2_LOG_EMULATOR			2

// index positions of game list tabs (left)
#define QMC2_GAMELIST_INDEX			0
#define QMC2_MACHINELIST_INDEX			QMC2_GAMELIST_INDEX
#define QMC2_SEARCH_INDEX			1
#define QMC2_FAVORITES_INDEX			2
#define QMC2_PLAYED_INDEX			3
#define QMC2_FOREIGN_INDEX			4
#define QMC2_EMBED_INDEX			5

// index positions of all game/machine details (upper right)
#define QMC2_PREVIEW_INDEX			0
#define QMC2_FLYER_INDEX			1
#define QMC2_GAMEINFO_INDEX			2
#define QMC2_MACHINEINFO_INDEX			QMC2_GAMEINFO_INDEX
#define QMC2_EMUINFO_INDEX			3
#define QMC2_CONFIG_INDEX			4
#define QMC2_DEVICE_INDEX			5
#define QMC2_CABINET_INDEX			6
#define QMC2_CONTROLLER_INDEX			7
#define QMC2_MARQUEE_INDEX			8
#define QMC2_LOGO_INDEX				QMC2_MARQUEE_INDEX
#define QMC2_TITLE_INDEX			9
#define QMC2_MAWS_INDEX				10
#define QMC2_PROJECTMESS_INDEX			QMC2_MAWS_INDEX
#define QMC2_PCB_INDEX				11
#define QMC2_SOFTWARE_LIST_INDEX		12
#if defined(QMC2_YOUTUBE_ENABLED)
#define QMC2_YOUTUBE_INDEX			13
#define QMC2_SYSTEM_NOTES_INDEX			14
#else
#define QMC2_SYSTEM_NOTES_INDEX			13
#endif

// index positions of special front end tabs (lower right)
#define QMC2_FRONTENDLOG_INDEX			0
#define QMC2_EMULATORLOG_INDEX			1
#define QMC2_EMULATORCONTROL_INDEX		2
#define QMC2_AUDIOPLAYER_INDEX			3
#define QMC2_DOWNLOADS_INDEX			4

// current format version of the GLC (game list cache)
#define QMC2_GLC_VERSION			6

// current format version of the XML cache database schema
#define QMC2_XMLCACHE_VERSION			1

// current format version of the user data database schema
#define QMC2_USERDATA_VERSION			1

// number of rows added to the XML cache database in *one* transaction
#define QMC2_XMLCACHE_COMMIT			1000

// column used for the game/machine icon
#define QMC2_ICON_INDEX				2

// index positions of game/machine list view selector
#define QMC2_VIEW_DETAIL_INDEX			0
#define QMC2_VIEW_TREE_INDEX			1
#define QMC2_VIEW_CATEGORY_INDEX		2
#define QMC2_VIEW_VERSION_INDEX			3

// logical column indexes in game/machine lists
#define QMC2_GAMELIST_RESET			-1
#define QMC2_GAMELIST_COLUMN_GAME		0
#define QMC2_GAMELIST_COLUMN_TAG		1
#define QMC2_GAMELIST_COLUMN_ICON		2
#define QMC2_GAMELIST_COLUMN_YEAR		3
#define QMC2_GAMELIST_COLUMN_MANU		4
#define QMC2_GAMELIST_COLUMN_NAME		5
#define QMC2_GAMELIST_COLUMN_RTYPES		6
#define QMC2_GAMELIST_COLUMN_PLAYERS		7
#define QMC2_GAMELIST_COLUMN_DRVSTAT		8
#define QMC2_GAMELIST_COLUMN_SRCFILE		9
#define QMC2_GAMELIST_COLUMN_RANK		10
#define QMC2_GAMELIST_COLUMN_CATEGORY		11
#define QMC2_GAMELIST_COLUMN_VERSION		12
#define QMC2_MACHINELIST_RESET			QMC2_GAMELIST_RESET
#define QMC2_MACHINELIST_COLUMN_GAME		QMC2_GAMELIST_COLUMN_GAME
#define QMC2_MACHINELIST_COLUMN_TAG		QMC2_GAMELIST_COLUMN_TAG
#define QMC2_MACHINELIST_COLUMN_ICON		QMC2_GAMELIST_COLUMN_ICON
#define QMC2_MACHINELIST_COLUMN_YEAR		QMC2_GAMELIST_COLUMN_YEAR
#define QMC2_MACHINELIST_COLUMN_MANU		QMC2_GAMELIST_COLUMN_MANU
#define QMC2_MACHINELIST_COLUMN_NAME		QMC2_GAMELIST_COLUMN_NAME
#define QMC2_MACHINELIST_COLUMN_RTYPES		QMC2_GAMELIST_COLUMN_RTYPES
#define QMC2_MACHINELIST_COLUMN_PLAYERS		QMC2_GAMELIST_COLUMN_PLAYERS
#define QMC2_MACHINELIST_COLUMN_DRVSTAT		QMC2_GAMELIST_COLUMN_DRVSTAT
#define QMC2_MACHINELIST_COLUMN_SRCFILE		QMC2_GAMELIST_COLUMN_SRCFILE
#define QMC2_MACHINELIST_COLUMN_RANK		QMC2_GAMELIST_COLUMN_RANK
#define QMC2_MACHINELIST_COLUMN_CATEGORY	QMC2_GAMELIST_COLUMN_CATEGORY

#define QMC2_GAMELIST_COLUMNS			13
#define QMC2_MACHINELIST_COLUMNS		12

// rank update delay in milliseconds (added to the general 'update delay')
#define QMC2_RANK_UPDATE_DELAY			50

// logical column indexes in the emulator control panel
#define QMC2_EMUCONTROL_COLUMN_NUMBER		0
#define QMC2_EMUCONTROL_COLUMN_ID		QMC2_EMUCONTROL_COLUMN_NUMBER
#define QMC2_EMUCONTROL_COLUMN_GAME		1
#define QMC2_EMUCONTROL_COLUMN_MACHINE		QMC2_EMUCONTROL_COLUMN_GAME
#define QMC2_EMUCONTROL_COLUMN_STATUS		2
#define QMC2_EMUCONTROL_COLUMN_LED0		3
#define QMC2_EMUCONTROL_COLUMN_LED1		4
#define QMC2_EMUCONTROL_COLUMN_LED2		5
#define QMC2_EMUCONTROL_COLUMN_PID		6
#define QMC2_EMUCONTROL_COLUMN_COMMAND		7

// logical column indexes of the MESS device configurator
#define QMC2_DEVCONFIG_COLUMN_NAME		0
#define QMC2_DEVCONFIG_COLUMN_BRIEF		1
#define QMC2_DEVCONFIG_COLUMN_TYPE		2
#define QMC2_DEVCONFIG_COLUMN_TAG		3
#define QMC2_DEVCONFIG_COLUMN_EXT		4
#define QMC2_DEVCONFIG_COLUMN_FILE		5

// delay in milliseconds before actually loading the MESS device configurator
#define QMC2_DEVCONFIG_LOAD_DELAY		50

// logical column indexes of the MESS device configurator's slot setup
#define QMC2_SLOTCONFIG_COLUMN_SLOT		0
#define QMC2_SLOTCONFIG_COLUMN_OPTION		1
#define QMC2_SLOTCONFIG_COLUMN_BIOS		2

// indexes of the device-setup tabs of the MESS device configurator
#define QMC2_DEVSETUP_TAB_DEVMAPPINGS		0
#define QMC2_DEVSETUP_TAB_SLOTCONFIG		1
#define QMC2_DEVSETUP_TAB_FILECHOOSER		2

// delay in milliseconds before updating the device map due to slot-option changes
#define QMC2_SLOTOPTION_CHANGE_DELAY		25

// file-chooser columns
#define QMC2_FILECHOOSER_COLUMN_NAME		0
#define QMC2_FILECHOOSER_COLUMN_SIZE		1
#define QMC2_FILECHOOSER_COLUMN_DATE		2

// number of rows to insert before updating the file-chooser's file-view
#define QMC2_FILECHOOSER_INSERTED_ROWS		2500

// init delay for the file-chooser's dir-view
#define QMC2_DIRCHOOSER_INIT_WAIT		250

// page indexes for the software list tool box
#define QMC2_SWLIST_KNOWN_SW_PAGE		0
#define QMC2_SWLIST_FAVORITES_PAGE		1
#define QMC2_SWLIST_SEARCH_PAGE			2

// page indexes for the 'special' stacked-widget
#define QMC2_SPECIAL_DEFAULT_PAGE		0
#define QMC2_SPECIAL_SOFTWARE_PAGE		1

// mounter-selection indexes
#define QMC2_SWLIST_MSEL_AUTO_MOUNT		0
#define QMC2_SWLIST_MSEL_SEPARATOR		1
#define QMC2_SWLIST_MSEL_DONT_MOUNT		2

// software snap positions
#define QMC2_SWSNAP_POS_ABOVE_LEFT		0
#define QMC2_SWSNAP_POS_ABOVE_CENTER		1
#define QMC2_SWSNAP_POS_ABOVE_RIGHT		2
#define QMC2_SWSNAP_POS_BELOW_LEFT		3
#define QMC2_SWSNAP_POS_BELOW_CENTER		4
#define QMC2_SWSNAP_POS_BELOW_RIGHT		5
#define QMC2_SWSNAP_POS_DISABLE_SNAPS		6

// logical column indexes for software lists
#define QMC2_SWLIST_RESET			-1
#define QMC2_SWLIST_COLUMN_TITLE		0
#define QMC2_SWLIST_COLUMN_NAME			1
#define QMC2_SWLIST_COLUMN_PUBLISHER		2
#define QMC2_SWLIST_COLUMN_YEAR			3
#define QMC2_SWLIST_COLUMN_PART			4
#define QMC2_SWLIST_COLUMN_INTERFACE		5
#define QMC2_SWLIST_COLUMN_LIST			6
#define QMC2_SWLIST_COLUMN_SUPPORTED		7
#define QMC2_SWLIST_COLUMN_DEVICECFG		8 // used only in 'favorites'

// logical column indexes for the audio effect list
#define QMC2_AUDIOEFFECT_COLUMN_NAME		0
#define QMC2_AUDIOEFFECT_COLUMN_DESC		1
#define QMC2_AUDIOEFFECT_COLUMN_ENABLE		2
#define QMC2_AUDIOEFFECT_COLUMN_SETUP		3

// logical column indexes in emulator options
#define QMC2_EMUOPT_COLUMN_OPTION		0
#define QMC2_EMUOPT_COLUMN_VALUE		1
#define QMC2_EMUOPT_COLUMN_ACTIONS		2

// logical column indexes in download manager
#define QMC2_DOWNLOAD_COLUMN_STATUS		0
#define QMC2_DOWNLOAD_COLUMN_PROGRESS		1

// logical column indexes in the additional / registered / foreign emulator list
#define QMC2_ADDTLEMUS_COLUMN_ICON		0	// 'icon' tool-button
#define QMC2_ADDTLEMUS_COLUMN_NAME		1
#define QMC2_ADDTLEMUS_COLUMN_EXEC		2
#define QMC2_ADDTLEMUS_COLUMN_WDIR		3
#define QMC2_ADDTLEMUS_COLUMN_ARGS		4
#define QMC2_ADDTLEMUS_COLUMN_CUID		5	// 'custom IDs' push-button

// logical column indexes in the 'custom ID list' for additional / registered / foreign emulators
#define QMC2_CUSTOMIDS_COLUMN_ICON		0 
#define QMC2_CUSTOMIDS_COLUMN_ID		1 
#define QMC2_CUSTOMIDS_COLUMN_DESCRIPTION	2 

// SDLMAME output notifier FIFO
#define QMC2_SDLMAME_OUTPUT_FIFO		"/tmp/sdlmame_out"
#define QMC2_SDLMESS_OUTPUT_FIFO		QMC2_SDLMAME_OUTPUT_FIFO
#define QMC2_SDLUME_OUTPUT_FIFO			QMC2_SDLMAME_OUTPUT_FIFO

// index positions of the game/machine list view stack
#define QMC2_VIEWGAMELIST_INDEX			0
#define QMC2_VIEWMACHINELIST_INDEX		QMC2_VIEWGAMELIST_INDEX
#define QMC2_VIEWHIERARCHY_INDEX		1
#define QMC2_VIEWCATEGORY_INDEX			2
#define QMC2_VIEWVERSION_INDEX			3	

// index positions of tabs in the options dialog
#define QMC2_OPTIONS_FRONTEND_INDEX		0
#define QMC2_OPTIONS_EMULATOR_INDEX		1
#define QMC2_OPTIONS_FE_GUI_INDEX		0
#define QMC2_OPTIONS_FE_FILES_INDEX		1
#define QMC2_OPTIONS_FE_GAMELIST_INDEX		2
#define QMC2_OPTIONS_FE_MACHINELIST_INDEX	QMC2_OPTIONS_FE_GAMELIST_INDEX
#define QMC2_OPTIONS_FE_SHORTCUTS_INDEX		3
#define QMC2_OPTIONS_FE_JOYSTICK_INDEX		4
#define QMC2_OPTIONS_FE_TOOLS_INDEX		5
#define QMC2_OPTIONS_EMU_CONFIG_INDEX		0
#define QMC2_OPTIONS_EMU_FILES_INDEX		1
#define QMC2_OPTIONS_EMU_ADDTLEMUS_INDEX	2

// index positions of sort criteria combobox entries
#define QMC2_SORTCRITERIA_DESCRIPTION		0
#define QMC2_SORTCRITERIA_ROMSTATE		1
#define QMC2_SORTCRITERIA_TAG			2
#define QMC2_SORTCRITERIA_YEAR			3
#define QMC2_SORTCRITERIA_MANUFACTURER		4
#define QMC2_SORTCRITERIA_GAMENAME		5
#define QMC2_SORTCRITERIA_MACHINENAME		QMC2_SORTCRITERIA_GAMENAME
#define QMC2_SORTCRITERIA_ROMTYPES		6
#define QMC2_SORTCRITERIA_PLAYERS		7
#define QMC2_SORTCRITERIA_DRVSTAT		8
#define QMC2_SORTCRITERIA_SRCFILE		9
#define QMC2_SORTCRITERIA_RANK			10
#define QMC2_SORTCRITERIA_CATEGORY		11
#define QMC2_SORTCRITERIA_VERSION		12

// search window timeout in milliseconds
#define QMC2_SEARCH_TIMEOUT			2500

// buffer size for reading zip-files
#define QMC2_ZIP_BUFFER_SIZE			QMC2_64K

// buffer size for reading regular files
#define QMC2_FILE_BUFFER_SIZE			QMC2_256K

// process timeouts in msecs
#define QMC2_PROCESS_POLL_TIME			25
#define QMC2_PROCESS_POLL_TIME_LONG		500
#define QMC2_PROCESS_POLL_RETRIES		30

// update lists every how many milliseconds (for image & sample checkers)?
#define QMC2_CHECK_UPDATE			1000
#define QMC2_CHECK_UPDATE_MEDIUM		500
#define QMC2_CHECK_UPDATE_FAST			250

// maximium length of a single filename/path in a ZIP archive
#define QMC2_MAX_PATH_LENGTH			1024

// indexes in compressed image file type selectors
#define QMC2_IMG_FILETYPE_ZIP			0
#define QMC2_IMG_FILETYPE_7Z			1

// retry loading images from 7z archives every how many milliseconds (when the dictionary is being filled up)
#define QMC2_IMG_7Z_DICT_FILL_DELAY		250

// search delay in milliseconds (so it doesn't hamper typing)
#define QMC2_SEARCH_DELAY			250

// animation timeout in milliseconds (controls animation speeds)
#define QMC2_ANIMATION_TIMEOUT			250

// button animation timeout (similar to animateClick() w/o signal emission)
#define QMC2_BUTTON_ANIMATION_TIMEOUT		50

// general polling interval for operations that need to wait until game/machine list reload has finished
#define QMC2_RELOAD_POLL_INTERVAL		250

// time to wait before trying to load a software snapshot (if any) when hovering over the entries of a software list
#define QMC2_SWSNAP_DELAY			10

// time after which the 'forced snapshot' flag is unset automatically
#define QMC2_SWSNAP_UNFORCE_DELAY		50

// gamelist sort criteria
#define QMC2_SORT_BY_DESCRIPTION		0
#define QMC2_SORT_BY_ROM_STATE			1
#define QMC2_SORT_BY_TAG			2
#define QMC2_SORT_BY_YEAR			3
#define QMC2_SORT_BY_MANUFACTURER		4
#define QMC2_SORT_BY_NAME			5
#define QMC2_SORT_BY_ROMTYPES			6
#define QMC2_SORT_BY_PLAYERS			7
#define QMC2_SORT_BY_DRVSTAT			8
#define QMC2_SORT_BY_SRCFILE			9
#define QMC2_SORT_BY_RANK			10
#define QMC2_SORT_BY_CATEGORY			11
#define QMC2_SORT_BY_VERSION			12

// ROM status export sort criteria
#define QMC2_RSE_SORT_BY_DESCRIPTION		0
#define QMC2_RSE_SORT_BY_ROM_STATE		1
#define QMC2_RSE_SORT_BY_YEAR			2
#define QMC2_RSE_SORT_BY_MANUFACTURER		3
#define QMC2_RSE_SORT_BY_NAME			4
#define QMC2_RSE_SORT_BY_ROMTYPES		5

// gamelist cursor positioning mode
#define QMC2_CURSOR_POS_VISIBLE			0
#define QMC2_CURSOR_POS_TOP			1
#define QMC2_CURSOR_POS_BOTTOM			2
#define QMC2_CURSOR_POS_CENTER			3

// internal ROM state representations
#define QMC2_ROMSTATE_COUNT			5
#define QMC2_ROMSTATE_INT_C			0
#define QMC2_ROMSTATE_INT_M			1
#define QMC2_ROMSTATE_INT_I			2
#define QMC2_ROMSTATE_INT_N			3
#define QMC2_ROMSTATE_INT_U			4

// indirectly controls the ROM state filter responsiveness (number of updates)
#define QMC2_STATEFILTER_UPDATES		4

// responsiveness while reading the XML game/machine list cache
#define QMC2_XMLCACHE_RESPONSIVENESS		50

// responsiveness while reading the ROM state cache
#define QMC2_ROMCACHE_RESPONSIVENESS		500

// responsiveness while pre-caching icons
#define QMC2_ICONCACHE_RESPONSIVENESS		250

// responsiveness while loading the YouTube video info map
#define QMC2_YOUTUBE_VIDEO_INFO_RSP		50

// delay in milliseconds before an automatic ROM check gets triggered
#define QMC2_AUTOROMCHECK_DELAY			250

// delay in milliseconds between consecutive ROM checks of tagged sets
#define QMC2_TAGGEDROMCHECK_DELAY		100

// interval in milliseconds between activity checks
#define QMC2_ACTIVITY_CHECK_INTERVAL		500

// responsiveness when processing the set of "remaining games/machines" (sets remaining after an interrupted full audit / sets not mentioned during a full audit)
#define QMC2_REMAINING_SETS_CHECK_RSP		100

// emulator option types
#define QMC2_EMUOPT_TYPE_UNKNOWN		0
#define QMC2_EMUOPT_TYPE_BOOL			1
#define QMC2_EMUOPT_TYPE_INT			2
#define QMC2_EMUOPT_TYPE_FLOAT			3
#define QMC2_EMUOPT_TYPE_FLOAT1			QMC2_EMUOPT_TYPE_FLOAT
#define QMC2_EMUOPT_TYPE_STRING			4
#define QMC2_EMUOPT_TYPE_FILE			5
#define QMC2_EMUOPT_TYPE_DIRECTORY		6
#define QMC2_EMUOPT_TYPE_COMBO			7
#define QMC2_EMUOPT_TYPE_FLOAT2			8
#define QMC2_EMUOPT_TYPE_FLOAT3			9

// supported image formats
#define QMC2_IMAGE_FORMAT_INDEX_PNG		0
#define QMC2_IMAGE_FORMAT_INDEX_BMP		1
#define QMC2_IMAGE_FORMAT_INDEX_GIF		2
#define QMC2_IMAGE_FORMAT_INDEX_JPG		3
#define QMC2_IMAGE_FORMAT_INDEX_PBM		4
#define QMC2_IMAGE_FORMAT_INDEX_PGM		5
#define QMC2_IMAGE_FORMAT_INDEX_PPM		6
#define QMC2_IMAGE_FORMAT_INDEX_TIFF		7
#define QMC2_IMAGE_FORMAT_INDEX_XBM		8
#define QMC2_IMAGE_FORMAT_INDEX_XPM		9
#define QMC2_IMAGE_FORMAT_INDEX_SVG		10
#define QMC2_IMAGE_FORMAT_INDEX_TGA		11

#define QMC2_IMAGE_FORMAT_COUNT			12

// image format setup column indexes
#define QMC2_IMGFMT_SETUP_COLUMN_NAME		0
#define QMC2_IMGFMT_SETUP_COLUMN_DESC		1
#define QMC2_IMGFMT_SETUP_COLUMN_EXT		2
#define QMC2_IMGFMT_SETUP_COLUMN_ACT		3

// default decimals for float values of emulator options
#define QMC2_EMUOPT_DFLT_DECIMALS		6

// MiniWebBrowser: hide status bar after how many milliseconds (if no longer required)
#define QMC2_BROWSER_STATUS_TIMEOUT		250

// MiniWebBrowser: in-memory icon cache size
#define QMC2_BROWSER_ICONCACHE_SIZE		QMC2_ONE_MEGABYTE

// item downloader: number of retries on "operation canceled" errors
#define QMC2_DOWNLOAD_OPCANCEL_RETRY		3

// item downloader: base number of milliseconds to wait before automatic retries
// (the real wait time will be between 5 and 10 times QMC2_DOWNLOAD_RETRY_DELAY)
#define QMC2_DOWNLOAD_RETRY_DELAY		1000

// how many milliseconds between download connection checks
#define QMC2_DOWNLOAD_CHECK_TIMEOUT		10000

// how many milliseconds before automatically cleaning up finished downloads
#define QMC2_DOWNLOAD_CLEANUP_DELAY		250

// MAWS homepage URL (default)
#define QMC2_MAWS_HOMEPAGE_URL			"http://maws.mameworld.info/"

// MAWS ROM set base URL (default)
#define QMC2_MAWS_BASE_URL			"http://maws.mameworld.info/maws/romset/%1"

// MAWS default URL
#define QMC2_MAWS_DEFAULT_URL			"http://www.progettoemma.net/gioco.php?game=%1&lang=en"

// ProjectMESS driver look-up URL
#define QMC2_PROJECTMESS_PATTERN_URL		"http://www.progettoemma.net/mess/system.php?machine=%1"

// MAWS image links base URL (default)
#define QMC2_MAWS_IMGLINKS_BASE_URL		"http://maws.mameworld.info/maws/"

// MAWS / ProjectMESS web cache size (in-memory, 8 MB fixed for now)
#define QMC2_MAWS_CACHE_SIZE			8 * QMC2_ONE_MEGABYTE
#define QMC2_PROJECTMESS_CACHE_SIZE		QMC2_MAWS_CACHE_SIZE

// delay creation of MAWS quick download menu for how many milliseconds
// (use same delay to start auto downloads)
#define QMC2_MAWS_QDL_DELAY			250

// ProjectMESS base URL (default)
#define QMC2_PROJECT_MESS_BASE_URL		"http://www.progettoemma.net/mess/gioco.php?game=%1&list=%2"

// ProjectMESS cache size (in-memory, 8 MB fixed for now)
#define QMC2_PROJECT_MESS_CACHE_SIZE		8 * QMC2_ONE_MEGABYTE

// delay forced background widget updates for how many milliseconds
#define QMC2_MAWS_BWU_DELAY			25

// maximum age of an MAWS disk-cache entry in seconds (24h for now)
#define QMC2_MAWS_MAX_CACHE_AGE			86400

// maximum number of pages held in QtWebKit's page cache
#define QMC2_BROWSER_CACHE_PAGES		100

// type conversions
#define QMC2_TO_UINT32(a)			((uchar)*((a) + 0) * (quint32)16777216ULL + \
						(uchar)*((a) + 1) * (quint32)65536ULL + \
						(uchar)*((a) + 2) * (quint32)256ULL + \
						(uchar)*((a) + 3))
#define QMC2_TO_UINT64(a)			((uchar)*((a) + 0) * (quint64)72057594037927936ULL + \
						(uchar)*((a) + 1) * (quint64)281474976710656ULL + \
						(uchar)*((a) + 2) * (quint64)1099511627776ULL + \
						(uchar)*((a) + 3) * (quint64)4294967296ULL + \
						(uchar)*((a) + 4) * (quint64)16777216ULL + \
						(uchar)*((a) + 5) * (quint64)65536ULL + \
						(uchar)*((a) + 6) * (quint64)256ULL + \
						(uchar)*((a) + 7))

// static cast to QSettings
#define QMC2_QSETTINGS_CAST(settings)		static_cast<QSettings*>((settings))

// additional pre-compile checks
#define QMC2_USE_PHONON_API			(QMC2_PHONON == 1)

// number of milliseconds to wait before automatically resuming audio/video playback
#define QMC2_AUDIOPLAYER_RESUME_DELAY		2000
#define QMC2_VIDEOPLAYER_RESUME_DELAY		2000

// audio player seek offset (in milliseconds)
#define QMC2_AUDIOPLAYER_SEEK_OFFSET		1000

// audio player fader timeout 
#if defined(QMC2_FADER_SPEED)
#define QMC2_AUDIOPLAYER_FADER_TIMEOUT		QMC2_FADER_SPEED
#else
#define QMC2_AUDIOPLAYER_FADER_TIMEOUT 		100
#endif

// audio player fader functions
#define QMC2_AUDIOPLAYER_FADER_PAUSE		0
#define QMC2_AUDIOPLAYER_FADER_PLAY		1

// QMC2 variant launcher specific stuff
#define QMC2_COMMAND_RUNONCE			"runonce"
#define QMC2_VARIANT_SDLMAME_NAME		"qmc2-sdlmame"
#define QMC2_VARIANT_SDLMAME_BUNDLE_ID		"net.arcadehits.qmc2." QMC2_VARIANT_SDLMAME_NAME
#define QMC2_VARIANT_SDLMAME_TITLE		MainWindow::tr("M.A.M.E. Catalog / Launcher II") + QString(" [Qt %1]").arg(qVersion())
#define QMC2_VARIANT_SDLMESS_NAME		"qmc2-sdlmess"
#define QMC2_VARIANT_SDLMESS_BUNDLE_ID		"net.arcadehits.qmc2." QMC2_VARIANT_SDLMESS_NAME
#define QMC2_VARIANT_SDLMESS_TITLE		MainWindow::tr("M.E.S.S. Catalog / Launcher II") + QString(" [Qt %1]").arg(qVersion())
#define QMC2_VARIANT_SDLUME_NAME		"qmc2-sdlume"
#define QMC2_VARIANT_SDLUME_BUNDLE_ID		"net.arcadehits.qmc2." QMC2_VARIANT_SDLUME_NAME
#define QMC2_VARIANT_SDLUME_TITLE		MainWindow::tr("U.M.E. Catalog / Launcher II") + QString(" [Qt %1]").arg(qVersion())
#define QMC2_VARIANT_MAME_NAME			"qmc2-mame.exe"
#define QMC2_VARIANT_MAME_BUNDLE_ID		"net.arcadehits.qmc2." QMC2_VARIANT_MAME_NAME
#define QMC2_VARIANT_MAME_TITLE			MainWindow::tr("M.A.M.E. Catalog / Launcher II") + QString(" [Qt %1]").arg(qVersion())
#define QMC2_VARIANT_MESS_NAME			"qmc2-mess.exe"
#define QMC2_VARIANT_MESS_BUNDLE_ID		"net.arcadehits.qmc2." QMC2_VARIANT_MESS_NAME
#define QMC2_VARIANT_MESS_TITLE			MainWindow::tr("M.E.S.S. Catalog / Launcher II") + QString(" [Qt %1]").arg(qVersion())
#define QMC2_VARIANT_UME_NAME			"qmc2-ume.exe"
#define QMC2_VARIANT_UME_BUNDLE_ID		"net.arcadehits.qmc2." QMC2_VARIANT_UME_NAME
#define QMC2_VARIANT_UME_TITLE			MainWindow::tr("U.M.E. Catalog / Launcher II") + QString(" [Qt %1]").arg(qVersion())

// separation for QMC2 variants
#define QMC2_ARCADE_PREFIX_MAME			QString("Arcade/MAME/")
#define QMC2_ARCADE_PREFIX_MESS			QString("Arcade/MESS/")
#define QMC2_ARCADE_PREFIX_UME			QString("Arcade/UME/")
#if defined(QMC2_SDLMAME)
#define QMC2_FRONTEND_PREFIX_MAME		QString("Frontend/qmc2-sdlmame/")
#define QMC2_FRONTEND_PREFIX_MESS		QString("Frontend/qmc2-sdlmess/")
#define QMC2_FRONTEND_PREFIX_UME		QString("Frontend/qmc2-sdlume/")
#define QMC2_FRONTEND_PREFIX			QMC2_FRONTEND_PREFIX_MAME
#define QMC2_EMULATOR_PREFIX_MAME		QString("MAME/")
#define QMC2_EMULATOR_PREFIX_MESS		QString("MESS/")
#define QMC2_EMULATOR_PREFIX_UME		QString("UME/")
#define QMC2_EMULATOR_PREFIX			QMC2_EMULATOR_PREFIX_MAME
#define QMC2_ARCADE_PREFIX			QMC2_ARCADE_PREFIX_MAME
#define QMC2_VARIANT_NAME			QString(QMC2_VARIANT_SDLMAME_NAME)
#define QMC2_EMU_NAME				QString("MAME")
#define QMC2_EMU_NAME_VARIANT			QString("SDLMAME")
#define QMC2_EMUTYPE_MAME
#elif defined(QMC2_SDLMESS)
#define QMC2_FRONTEND_PREFIX_MAME		QString("Frontend/qmc2-sdlmame/")
#define QMC2_FRONTEND_PREFIX_MESS		QString("Frontend/qmc2-sdlmess/")
#define QMC2_FRONTEND_PREFIX_UME		QString("Frontend/qmc2-sdlume/")
#define QMC2_FRONTEND_PREFIX			QMC2_FRONTEND_PREFIX_MESS
#define QMC2_EMULATOR_PREFIX_MAME		QString("MAME/")
#define QMC2_EMULATOR_PREFIX_MESS		QString("MESS/")
#define QMC2_EMULATOR_PREFIX_UME		QString("UME/")
#define QMC2_EMULATOR_PREFIX			QMC2_EMULATOR_PREFIX_MESS
#define QMC2_ARCADE_PREFIX			QMC2_ARCADE_PREFIX_MESS
#define QMC2_VARIANT_NAME			QString(QMC2_VARIANT_SDLMESS_NAME)
#define QMC2_EMU_NAME				QString("MESS")
#define QMC2_EMU_NAME_VARIANT			QString("SDLMESS")
#define QMC2_EMUTYPE_MESS
#elif defined(QMC2_SDLUME)
#define QMC2_FRONTEND_PREFIX_MAME		QString("Frontend/qmc2-sdlmame/")
#define QMC2_FRONTEND_PREFIX_MESS		QString("Frontend/qmc2-sdlmess/")
#define QMC2_FRONTEND_PREFIX_UME		QString("Frontend/qmc2-sdlume/")
#define QMC2_FRONTEND_PREFIX			QMC2_FRONTEND_PREFIX_UME
#define QMC2_EMULATOR_PREFIX_MAME		QString("MAME/")
#define QMC2_EMULATOR_PREFIX_MESS		QString("MESS/")
#define QMC2_EMULATOR_PREFIX_UME		QString("UME/")
#define QMC2_EMULATOR_PREFIX			QMC2_EMULATOR_PREFIX_UME
#define QMC2_ARCADE_PREFIX			QMC2_ARCADE_PREFIX_UME
#define QMC2_VARIANT_NAME			QString(QMC2_VARIANT_SDLUME_NAME)
#define QMC2_EMU_NAME				QString("UME")
#define QMC2_EMU_NAME_VARIANT			QString("SDLUME")
#define QMC2_EMUTYPE_UME
#elif defined(QMC2_MAME)
#define QMC2_FRONTEND_PREFIX_MAME		QString("Frontend/qmc2-mame/")
#define QMC2_FRONTEND_PREFIX_MESS		QString("Frontend/qmc2-mess/")
#define QMC2_FRONTEND_PREFIX_UME		QString("Frontend/qmc2-ume/")
#define QMC2_FRONTEND_PREFIX			QMC2_FRONTEND_PREFIX_MAME
#define QMC2_EMULATOR_PREFIX_MAME		QString("MAME/")
#define QMC2_EMULATOR_PREFIX_MESS		QString("MESS/")
#define QMC2_EMULATOR_PREFIX_UME		QString("UME/")
#define QMC2_EMULATOR_PREFIX			QMC2_EMULATOR_PREFIX_MAME
#define QMC2_ARCADE_PREFIX			QMC2_ARCADE_PREFIX_MAME
#define QMC2_VARIANT_NAME			QString(QMC2_VARIANT_MAME_NAME)
#define QMC2_EMU_NAME				QString("MAME")
#define QMC2_EMU_NAME_VARIANT			QString("MAME")
#define QMC2_EMUTYPE_MAME
#elif defined(QMC2_MESS)
#define QMC2_FRONTEND_PREFIX_MAME		QString("Frontend/qmc2-mame/")
#define QMC2_FRONTEND_PREFIX_MESS		QString("Frontend/qmc2-mess/")
#define QMC2_FRONTEND_PREFIX_UME		QString("Frontend/qmc2-ume/")
#define QMC2_FRONTEND_PREFIX			QMC2_FRONTEND_PREFIX_MESS
#define QMC2_EMULATOR_PREFIX_MAME		QString("MAME/")
#define QMC2_EMULATOR_PREFIX_MESS		QString("MESS/")
#define QMC2_EMULATOR_PREFIX_UME		QString("UME/")
#define QMC2_EMULATOR_PREFIX			QMC2_EMULATOR_PREFIX_MESS
#define QMC2_ARCADE_PREFIX			QMC2_ARCADE_PREFIX_MESS
#define QMC2_VARIANT_NAME			QString(QMC2_VARIANT_MESS_NAME)
#define QMC2_EMU_NAME				QString("MESS")
#define QMC2_EMU_NAME_VARIANT			QString("MESS")
#define QMC2_EMUTYPE_MESS
#elif defined(QMC2_UME)
#define QMC2_FRONTEND_PREFIX_MAME		QString("Frontend/qmc2-mame/")
#define QMC2_FRONTEND_PREFIX_MESS		QString("Frontend/qmc2-mess/")
#define QMC2_FRONTEND_PREFIX_UME		QString("Frontend/qmc2-ume/")
#define QMC2_FRONTEND_PREFIX			QMC2_FRONTEND_PREFIX_UME
#define QMC2_EMULATOR_PREFIX_MAME		QString("MAME/")
#define QMC2_EMULATOR_PREFIX_MESS		QString("MESS/")
#define QMC2_EMULATOR_PREFIX_UME		QString("UME/")
#define QMC2_EMULATOR_PREFIX			QMC2_EMULATOR_PREFIX_UME
#define QMC2_ARCADE_PREFIX			QMC2_ARCADE_PREFIX_UME
#define QMC2_VARIANT_NAME			QString(QMC2_VARIANT_UME_NAME)
#define QMC2_EMU_NAME				QString("UME")
#define QMC2_EMU_NAME_VARIANT			QString("UME")
#define QMC2_EMUTYPE_UME
#endif

// Mac OS X uses "~/Library/Application Support/app" rather than "~/.app"
#define QMC2_SYSCONF_PATH			(QString(XSTR(SYSCONFDIR)).replace(QChar(':'), QLatin1String(" ")) + "/qmc2")
#if defined(QMC2_OS_MAC)
#define QMC2_DOT_PATH				(QDir::homePath() + "/Library/Application Support/qmc2")
#define QMC2_DEFAULT_DATA_PATH			(QDir::homePath() + "/Library/Application Support/qmc2")
#else
#define QMC2_DOT_PATH				(QDir::homePath() + "/.qmc2")
#define QMC2_DEFAULT_DATA_PATH			QString("data")
#endif

// this allows for changing the configuration path dynamically by adding '-config_path <config_path>' on the command line
#define QMC2_DYNAMIC_DOT_PATH			(qApp->arguments().indexOf("-config_path") >= 0 && qApp->arguments().indexOf("-config_path") + 1 <= qApp->arguments().count() ? qApp->arguments()[qApp->arguments().indexOf("-config_path") + 1] : (qApp->arguments().indexOf("-qmc2_config_path") >= 0 && qApp->arguments().indexOf("-qmc2_config_path") + 1 <= qApp->arguments().count() ? qApp->arguments()[qApp->arguments().indexOf("-qmc2_config_path") + 1] : QMC2_DOT_PATH))

// -cc: clear (all emulator) caches 
#define QMC2_CLI_OPT_CLEAR_ALL_CACHES		(qApp->arguments().indexOf("-cc") >= 0)

// -help/-?/-h: show usage help and exit
#define QMC2_CLI_OPT_HELP			(qApp->arguments().indexOf(QRegExp("(-help|-\\?|-h)")) >= 0)

// determine if memory infomation can be made available at all
#if defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE) && defined(_SC_AVPHYS_PAGES)
#define QMC2_MEMORY_INFO_ENABLED
#define QMC2_MEMORY_INFO_TYPE_POSIX
#define QMC2_MEMORY_UPDATE_TIME			500
#endif

// X11 and Windows only: embedder specific delays (in ms)
#define QMC2_EMBED_DELAY			250
#define QMC2_EMBED_MAXIMIZE_DELAY		100
#define QMC2_EMBED_FOCUS_DELAY			100
#define QMC2_EMBED_PAUSERESUME_DELAY		250
#define QMC2_EMBED_RELEASE_DELAY		25

// X11 and Windows only: embedder snapshot item default size (100% zoom)
#define QMC2_EMBED_SNAPSHOT_DEFAULT_ITEM_SIZE	50.0

// maximum number of retries to find an emulator window via xlib (we'll wait for up to three seconds for the emulator to get ready)
#define QMC2_MAX_XWININFO_RETRIES		60
#define QMC2_XWININFO_DELAY			50

// same for the Windows world
#define QMC2_MAX_WININFO_RETRIES		60
#define QMC2_WININFO_DELAY			50

// responsiveness while loading an additional information source
#define QMC2_INFOSOURCE_RESPONSIVENESS		10000

// software-info responsiveness
#define QMC2_SWINFO_RESPONSIVENESS		25000

// maximum delay in ms before finally killing an external tool (if required to)
#define QMC2_TOOL_KILL_WAIT			2000

// supported DB drivers
#define QMC2_DB_DRIVER_MYSQL			0
#define QMC2_DB_DRIVER_SQLITE			1

// DB server default ports
#define QMC2_DB_DFLT_PORT_MYSQL			3306

// DB connection check button reset delay
#define QMC2_DB_RESET_CCB_DELAY			1000

// MAME/MESS exit codes (from MAME source -- src/emu/mame.h)
#define	QMC2_MAME_ERROR_NONE			0	// no error
#define	QMC2_MAME_ERROR_FAILED_VALIDITY		1	// failed validity checks
#define	QMC2_MAME_ERROR_MISSING_FILES		2	// missing files
#define	QMC2_MAME_ERROR_FATALERROR		3	// some other fatal error
#define	QMC2_MAME_ERROR_DEVICE			4	// device initialization error (MESS-specific)
#define	QMC2_MAME_ERROR_NO_SUCH_GAME		5	// game was specified but doesn't exist
#define	QMC2_MAME_ERROR_INVALID_CONFIG		6	// some sort of error in configuration
#define	QMC2_MAME_ERROR_IDENT_NONROMS		7	// identified all non-ROM files
#define	QMC2_MAME_ERROR_IDENT_PARTIAL		8	// identified some files but not all
#define	QMC2_MAME_ERROR_IDENT_NONE		9	// identified no files
#define	QMC2_MAME_ERROR_UNKNOWN			-1	// unknown exit code

// exchangable (de)compression routines
#define QMC2_COMPRESS(data)			qCompress((data))
#define QMC2_UNCOMPRESS(data)			(data).isEmpty() ? QByteArray() : qUncompress(data)

// timeout (in ms) for locking the log-mutex
#define QMC2_LOG_MUTEX_LOCK_TIMEOUT		100

// X11 only: time (in ms) between KeyPress and KeyRelease events when simulating keys sent to an emulator
#define QMC2_XKEYEVENT_TRANSITION_TIME		50

// page indexes for the software detail widget
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
#define QMC2_SWINFO_SNAPSHOT_PAGE		0
#define QMC2_SWINFO_PROJECTMESS_PAGE		1
#define QMC2_SWINFO_NOTES_PAGE			2
#define QMC2_SWINFO_INFO_PAGE			3
#elif defined(QMC2_EMUTYPE_MAME)
#define QMC2_SWINFO_SNAPSHOT_PAGE		0
#define QMC2_SWINFO_NOTES_PAGE			1
#define QMC2_SWINFO_INFO_PAGE			2
#endif

// size-/count-thresholds for displaying alternate label when loading or checking software-lists
#define QMC2_SWLIST_SIZE_THRESHOLD		512 * QMC2_ONE_KILOBYTE
#define QMC2_SWLIST_COUNT_THRESHOLD		1000

// when loading software-lists, be 'responsive' on every n'th start of an XML element
#define QMC2_SWLIST_LOAD_RESPONSE		1000
#define QMC2_SWLIST_LOAD_RESPONSE_LONG		2500
#define QMC2_SWLIST_LOAD_ANIM_DELAY		5000

// responsiveness in sofware-state checks
#define QMC2_SWLIST_CHECK_RESPONSE		100
#define QMC2_SWLIST_CHECK_RESPONSE_SHORT	25

// when analyzing sets that were not mentioned during full audit, be responsive every n'th line in the XML that's being read
#define QMC2_AUDIT_NMS_RESPONSE			1000

// responsiveness upon reading the slot-info cache
#define QMC2_SLOTINFO_READ_RESPONSE		500

// default launch-modes
#define QMC2_LAUNCH_MODE_INDEPENDENT		0
#define QMC2_LAUNCH_MODE_EMBEDDED		1

// number of milliseconds the splash screen is shown after QMainWindow::init()
#define QMC2_SPLASH_DURATION			2000

// settings-related organization name/domain
#define QMC2_ORGANIZATION_NAME			QString("qmc2")
#define QMC2_ORGANIZATION_DOMAIN		QString("qmc2.arcadehits.net")

// time to wait before retrying operations when 'critical sections' are active
#define QMC2_CRITSECT_POLLING_TIME		10

// numeric image types
#define QMC2_IMGTYPE_NONE			-1
#define QMC2_IMGTYPE_PREVIEW			0
#define QMC2_IMGTYPE_FLYER			1
#define QMC2_IMGTYPE_CABINET			2
#define QMC2_IMGTYPE_CONTROLLER			3
#define QMC2_IMGTYPE_MARQUEE			4
#define QMC2_IMGTYPE_LOGO			QMC2_IMGTYPE_MARQUEE
#define QMC2_IMGTYPE_TITLE			5
#define QMC2_IMGTYPE_PCB			6
#define QMC2_IMGTYPE_SWSNAP			7

// image checker image types
#define QMC2_IMGCHK_INDEX_NONE			-1
#if defined(QMC2_EMUTYPE_MESS)
#define QMC2_IMGCHK_INDEX_PREVIEW		0
#define QMC2_IMGCHK_INDEX_FLYER			1
#define QMC2_IMGCHK_INDEX_CABINET		2
#define QMC2_IMGCHK_INDEX_CONTROLLER		3
#define QMC2_IMGCHK_INDEX_MARQUEE		4
#define QMC2_IMGCHK_INDEX_LOGO			QMC2_IMGCHK_INDEX_MARQUEE
#define QMC2_IMGCHK_INDEX_PCB			5
#define QMC2_IMGCHK_INDEX_SEPARATOR		6
#define QMC2_IMGCHK_INDEX_ICON			7
#else
#define QMC2_IMGCHK_INDEX_PREVIEW		0
#define QMC2_IMGCHK_INDEX_FLYER			1
#define QMC2_IMGCHK_INDEX_CABINET		2
#define QMC2_IMGCHK_INDEX_CONTROLLER		3
#define QMC2_IMGCHK_INDEX_MARQUEE		4
#define QMC2_IMGCHK_INDEX_TITLE			5
#define QMC2_IMGCHK_INDEX_PCB			6
#define QMC2_IMGCHK_INDEX_SEPARATOR		7
#define QMC2_IMGCHK_INDEX_ICON			8
#endif

// demo / arcade mode: driver maturity states
#define QMC2_DEMO_MODE_DRV_STATUS_GOOD		0
#define QMC2_DEMO_MODE_DRV_STATUS_IMPERFECT	1
#define QMC2_DEMO_MODE_DRV_STATUS_PRELIMINARY	2
#define QMC2_ARCADE_DRV_STATUS_GOOD		QMC2_DEMO_MODE_DRV_STATUS_GOOD
#define QMC2_ARCADE_DRV_STATUS_IMPERFECT	QMC2_DEMO_MODE_DRV_STATUS_IMPERFECT
#define QMC2_ARCADE_DRV_STATUS_PRELIMINARY	QMC2_DEMO_MODE_DRV_STATUS_PRELIMINARY

// number of images per work-unit
#define QMC2_IMGCHK_WORKUNIT_SIZE		50

// image checker work-unit-mutex-lock-timeout
#define QMC2_IMGCHK_WU_MUTEX_LOCK_TIMEOUT	10

// alignment constants (i.e. for icon alignment in the IconLineEdit)
#define QMC2_ALIGN_LEFT				0
#define QMC2_ALIGN_MIDDLE			1
#define QMC2_ALIGN_RIGHT			2

// delay between full screen switches (avoids problems when the menu-bar is hidden)
#define QMC2_FULLSCREEN_SWITCH_DELAY		250

// indexed QMC2 Arcade themes
#define QMC2_ARCADE_THEME_COUNT			2
#define QMC2_ARCADE_THEME_TOXICWASTE		0
#define QMC2_ARCADE_THEME_DARKONE		1

// arcade mode setup: column indexes for key-sequence and joystick-function maps
#define QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION	0
#define QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM	1
#define QMC2_ARCADE_JOYMAP_COLUMN_FUNCTION	QMC2_ARCADE_KEYMAP_COLUMN_FUNCTION
#define QMC2_ARCADE_JOYMAP_COLUMN_CUSTOM	QMC2_ARCADE_KEYMAP_COLUMN_CUSTOM

// update search result list every how many (searched) entries
#if defined(QMC2_EMUTYPE_MESS)
#define QMC2_SEARCH_RESULT_UPDATE		2500
#else
#define QMC2_SEARCH_RESULT_UPDATE		5000
#endif

// macros to determine ZIP / 7z compressed icon file type to use
#define QMC2_ICON_FILETYPE_ZIP			(qmc2UseIconFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFileType").toInt() == QMC2_IMG_FILETYPE_ZIP)
#define QMC2_ICON_FILETYPE_7Z			(qmc2UseIconFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFileType").toInt() == QMC2_IMG_FILETYPE_7Z)

// debugging macros
#define QMC2_PRINT_TXT(t)			printf("%s\n", #t)
#define QMC2_PRINT_STR(s)			printf("%s = %s\n", #s, s.toLocal8Bit().constData())
#define QMC2_PRINT_CSTR(s)			printf("%s = %s\n", #s, s)
#define QMC2_PRINT_PTR(p)			printf("%s = %p\n", #p, p)
#define QMC2_PRINT_INT(i)			printf("%s = %ld\n", #i, i)
#define QMC2_PRINT_HEX(x)			printf("%s = %x\n", #x, x)
#define QMC2_PRINT_BOOL(b)			printf("%s = %s\n", #b, b ? "true" : "false")
#define QMC2_PRINT_STRLST(l)			for (int i = 0; i < l.count(); i++) printf("%s[%ld] = %s\n", #l, i, l[i].toLocal8Bit().constData())

// Google dev-key
#define QMC2_GOOGLE_DEV_KEY			"AI39si7D4n3EI-m_1eeFsfbmD8fG7Sqb9gpkgjyliWrSVs4M4hUPZsLnE_Z7XQUMSPK4GuV9A3wbzKkcSBxOTniTe-d8Y1dOlw"

#endif
