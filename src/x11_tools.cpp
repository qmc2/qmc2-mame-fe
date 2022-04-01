#include "x11_tools.h"

#if defined(QMC2_OS_UNIX)

#include <QX11Info>
#include <QRegExp>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

Window x11SearchTree(Window window, QString titlePattern, QString classHintPattern)
{
	Window root_win, parent_win, *child_list, win_return = 0;
	unsigned int num_children;
	if ( XQueryTree(QX11Info::display(), window, &root_win, &parent_win, &child_list, &num_children) ) {
		char *win_name;
		QRegExp rxTitle(titlePattern, Qt::CaseSensitive, QRegExp::RegExp2);
		QRegExp rxClassHint(classHintPattern, Qt::CaseSensitive, QRegExp::RegExp2);
		for (int i = num_children - 1; i >= 0 && !win_return; i--) {
			if ( XFetchName(QX11Info::display(), child_list[i], &win_name) ) {
				if ( win_name ) {
					if ( rxTitle.indexIn(QString(win_name)) >= 0 ) {
						XClassHint class_hint;
						if ( XGetClassHint(QX11Info::display(), child_list[i], &class_hint)) {
							if ( class_hint.res_name ) {
								if ( rxClassHint.indexIn(QString(class_hint.res_name)) >= 0 )
									win_return = child_list[i];
								XFree(class_hint.res_name);
							}
							if ( class_hint.res_class ) {
								if ( !win_return && rxClassHint.indexIn(QString(class_hint.res_class)) >= 0 )
									win_return = child_list[i];
								XFree(class_hint.res_class);
							}
						}
					}
					XFree(win_name);
				}
			}
			if ( !win_return )
				win_return = x11SearchTree(child_list[i], titlePattern, classHintPattern);
		}
		XFree((char *)child_list);
	}
	return win_return;
}

WId x11FindWindowId(QString titlePattern, QString classHintPattern)
{
	return (WId)x11SearchTree(QX11Info::appRootWindow(), titlePattern, classHintPattern);
}

#endif
