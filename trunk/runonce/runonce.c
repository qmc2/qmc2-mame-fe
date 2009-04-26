/* runonce.c --- GUI program-launching wrapper
 * Copyright © 2001 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * Created: 29-Jul-2001.
 *
 * Modified for use with the QMC2 project by R. Reucher: 23-Jan-2009.
 *
 * The idea here is that clicking on an icon in the panel should only ever
 * launch a single copy of the program: clicking again should raise the
 * existing window instead of launching a new copy.
 *
 * The trick is, the program might not have mapped a window yet.
 * In that case, multi-clicks should be ignored.  So we have to keep
 * track of the pid of the program.
 *
 * Usage:
 *            gcc -Wall -o runonce runonce.c -L/usr/X11R6/lib -lXmu -lX11
 *            runonce XTerm xterm -fg black -bg white &
 *
 */

#include <stdlib.h>
#include <unistd.h>		/* for getpid() and execvp() */
#include <stdio.h>
#include <string.h>
#include <sys/file.h>		/* for flock() */
#include <sys/time.h>
#include <time.h>

#include <sys/errno.h>
#include <sys/types.h>
#include <signal.h>		/* for kill() */

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>		/* for XGetClassHint() */
#include <X11/Xmu/WinUtil.h>	/* for XmuClientWindow() */

char *progname;
int verbose_p;


static int
ignore_errors_handler (Display *dpy, XErrorEvent *error)
{
  return 0;
}


static int
check_for_window (const char *window_name, const char *window_title)
{
  Display *dpy;
  Window window = 0;
  int window_found = 0;
  char *d = getenv ("DISPLAY");
  if (!d || !*d) {
      fprintf (stderr, "%s: DISPLAY is unset.\n", progname);
      exit (1);
  }

  if (verbose_p > 1)
    fprintf (stderr, "%s: looking for window \"%s\", title = \"%s\"\n", progname, window_name, window_title);

  dpy = XOpenDisplay (d);
  if (!dpy) {
      fprintf (stderr, "%s: unable to open display \"%s\"\n", progname, d);
      exit (1);
  }

  {
    XErrorHandler old_handler = 0;
    Window root = RootWindowOfScreen (DefaultScreenOfDisplay (dpy));
    Window root2, parent, *kids;
    unsigned int nkids;
    int i;

    if (! XQueryTree (dpy, root, &root2, &parent, &kids, &nkids)) {
        fprintf (stderr, "%s: unable to list windows?\n", progname);
        exit (1);
    }

    if (!kids) nkids = 0;

    old_handler = XSetErrorHandler (ignore_errors_handler);
    XSync (dpy, False);

    for (i = 0; i < nkids; i++) {
        XClassHint hint;
        char *name = 0;
        char *class = 0;
        char *title = 0;

        Window w = XmuClientWindow (dpy, kids[i]);

        if (!w) continue;

        memset (&hint, 0, sizeof(hint));
        XGetClassHint (dpy, w, &hint);
        name = hint.res_name;
        class = hint.res_class;
        XFetchName (dpy, w, &title);

        if (verbose_p)
          fprintf (stderr, "%s: checking 0x%08lX: %s/%s: \"%s\"\n",
                   progname, (unsigned long) w,
                   (class ? class : "null"),
                   (name  ? name  : "null"),
                   (title ? title : "null"));

        if ( (name && !strcmp(window_name, name)) && (title && !strcmp(window_title, title)) ) {
          if (verbose_p)
            fprintf (stderr, "%s: matched 0x%08lX: %s/%s: \"%s\"\n",
                     progname, (unsigned long) w,
                     (class ? class : "null"),
                     (name  ? name  : "null"),
                     (title ? title : "null"));

          window = w;
        }

        if (name)  XFree (name);
        if (class) XFree (class);
        if (title) XFree (title);
        name = 0;

        if (window) {
          if (verbose_p)
            fprintf (stderr, "%s: raising window 0x%08lX (%s)\n",
                     progname, (unsigned long) window, window_name);

          XMapRaised (dpy, window);
          XSync (dpy, False);
          window_found = 1;
        }

        window = 0;
      }

    XSync (dpy, False);
    XSetErrorHandler (old_handler);
    XSync (dpy, False);
  }

  XCloseDisplay (dpy);
  return window_found;
}


static void
PERROR (const char *fmt, const char *arg)
{
  char buf[500];
  char buf2[500];
  sprintf (buf2, fmt, arg);
  sprintf (buf, "%.100s: %.200s", progname, buf2);
  perror (buf);
}


static const char *
pidfile (void)
{
  char *pf = 0;
  if (!pf)
    {
      char *name = ".runonce.pids";
      char *tmp = getenv ("TMPDIR");
      if (!tmp) tmp = "/tmp";
      pf = (char *) malloc (strlen(tmp) + strlen(name) + 3);
      strcpy (pf, tmp);
      if (pf[strlen(pf)-1] != '/')
        strcat (pf, "/");
      strcat (pf, name);
    }
  return pf;
}


typedef struct pid_pair pid_pair;
struct pid_pair {
  time_t t;
  pid_t p;
  char *name;
  pid_pair *next;
};


static pid_pair *pairs = 0;


static int
check_for_pid (const char *window_name)
{
  pid_t matched = (pid_t) 0;

  time_t now = time((time_t *) 0);

  int timeout = 60 * 5;  /* it shouldn't take longer than 5 minutes for
                            an app to open a window, no matter how badly
                            the machine is thrashing... */

  const char *f = pidfile();
  FILE *in = fopen (f, "a+");
  char buf[1024];
  int line = 1;
  pid_pair *tail = pairs;

  if (verbose_p > 1)
    fprintf (stderr, "%s: reading %s\n", progname, f);

  if (!in)
    {
      PERROR ("couldn't open", f);
      return 0;
    }

  if (flock (fileno (in), LOCK_EX))
    {
      fclose (in);
      PERROR ("unable to lock %s", f);
      return 0;
    }

  if (fseek (in, 0, SEEK_SET))
    {
      PERROR ("unable to rewind %s", f);
      goto FAIL;
    }

  while (fgets (buf, sizeof(buf)-1, in))
    {
      time_t t;
      pid_t p;
      if (2 == sscanf (buf, "%lu %lu",
                       (unsigned long *) &t,
                       (unsigned long *) &p))
        {
          if (t < now - timeout)
            {
              /* This line was added long in the past.  Lose it. */
              if (verbose_p > 1)
                fprintf (stderr,
                         "%s: discarding line %d: expired: %s",
                         progname, line, buf);
            }
          else if (kill (p, 0) < 0 &&
                   errno == ESRCH)
            {
              /* This line was added recently, but the process is dead. */
              if (verbose_p > 1)
                fprintf (stderr,
                         "%s: discarding line %d: dead: %s",
                         progname, line, buf);
            }
          else
            {
              /* This line is still a going concern. */
              pid_pair *pair = (pid_pair *) calloc (1, sizeof(pid_pair));
              pair->t = t;
              pair->p = p;
              pair->next = 0;

              pair->name = buf;

              /* skip to first space */
              while (*pair->name && *pair->name != ' ') pair->name++;
              if (*pair->name) pair->name++;
              /* skip to second space */
              while (pair->name && *pair->name != ' ') pair->name++;
              if (*pair->name) pair->name++;
              pair->name = strdup (pair->name);

              if (pair->name[strlen(pair->name)-1] == '\n')
                pair->name[strlen(pair->name)-1] = 0;

              if (tail)
                tail->next = pair;
              tail = pair;
              if (!pairs)
                pairs = pair;

              if (verbose_p > 1)
                fprintf (stderr, "%s: keeping line %d: %s",
                         progname, line, buf);

              if (!matched && !strcmp (pair->name, window_name))
                {
                  if (verbose_p > 1)
                    fprintf (stderr, "%s: matched line %d: %s",
                             progname, line, buf);
                  matched = pair->p;
                }
            }
        }
      else
        {
          fprintf (stderr, "%s: line %d unparsable\n", progname, line);
        }
      line++;
    }

  /* Now that we've parsed the file and pruned bogus entries, add ourselves. */
  if (!matched)
    {
      pid_pair *pair = (pid_pair *) calloc (1, sizeof(pid_pair));
      pair->t = now;
      pair->p = getpid();
      pair->name = strdup (window_name);
      pair->next = 0;

      if (tail)
        tail->next = pair;
      tail = pair;
      if (!pairs)
        pairs = pair;

      if (verbose_p > 1)
        fprintf (stderr, "%s: added line %d: %lu %lu %s\n",
                 progname, line,
                 (unsigned long) tail->t, (unsigned long) tail->p,
                 tail->name);
    }

  /* Now rewrite the file.
   */
  if (fseek (in, 0, SEEK_SET))
    {
      PERROR ("unable to rewind %s", f);
      goto FAIL;
    }

  if (ftruncate (fileno (in), 0))
    {
      PERROR ("unable to truncate %s", f);
      goto FAIL;
    }

  for (tail = pairs; tail; tail = tail->next)
    fprintf (in, "%lu %lu %s\n",
             (unsigned long) tail->t,
             (unsigned long) tail->p,
             tail->name);

 FAIL:
  if (flock (fileno (in), LOCK_UN))
    PERROR ("unable to unlock %s", f);
  fclose (in);

  if (verbose_p > 1)
    fprintf (stderr, "%s: wrote %s\n", progname, f);

  /* Yeah, I know, I never free `pairs'.  It's small, and life is short. */

  if (verbose_p && matched)
    fprintf (stderr, "%s: pid %lu is still alive (for window \"%s\")\n",
             progname, (unsigned long) matched, window_name);

  return (matched ? 1 : 0);
}


static void
run_program (char **argv)
{
  if (verbose_p)
    {
      int i = 0;
      fprintf (stderr, "%s: pid %lu: executing:",
               progname, (unsigned long) getpid());
      while (argv[i])
        fprintf (stderr, " %s", argv[i++]);
      fprintf (stderr, "\n");
    }

  execvp (argv[0], argv);
  PERROR ("%s", argv[0]);
}



static void
usage (void)
{
  fprintf (stderr, "usage: %s [--verbose] window-name window-title program args ...\n",
           progname);
  exit (1);
}


int
main (int argc, char **argv)
{
  char *window_name;
  char *window_title;
  char *s;

  progname = argv[0];
  s = strrchr (progname, '/');
  if (s) progname = s+1;
  if (strlen(progname) > 100) progname[100] = 0;

  argc--; argv++;   /* pop this program off the front. */

  if (argc <= 0) usage();

  while (argc > 0 &&
         (!strcmp (argv[0], "-v") ||
          !strcmp (argv[0], "-vv") ||
          !strcmp (argv[0], "-vvv") ||
          !strcmp (argv[0], "-verbose") ||
          !strcmp (argv[0], "--verbose")))
    {
      if (!strcmp (argv[0], "-vv")) verbose_p += 2;
      else if (!strcmp (argv[0], "-vvv")) verbose_p += 3;
      else verbose_p++;

      argc--; argv++;
    }

  if (argc <= 0) usage();
  if (argv[0][0] == '-') usage();

  window_name = argv[0];
  argc--; argv++;

  if (argc <= 0) usage();
  if (argv[0][0] == '-') usage();

  window_title = argv[0];
  argc--; argv++;

  if (argc <= 0) usage();
  if (argv[0][0] == '-') usage();  /* assume no programs being with dash... */

  /* If the window exists, raise it, and exit.
   */
  if (check_for_window (window_name, window_title))
    exit (0);

  /* If we've recently launched a copy of this program, and the pid is still
     alive, it must still be starting up (since it hasn't mapped a window
     yet.)  So, just exit and assume that window will pop up soon.
   */
  if (check_for_pid (window_name))
    exit (0);

  /* Otherwise there is no evidence that this program is already running:
     launch it, and remember the pid.  This does not return.
   */
  run_program (argv);

  exit (1);
}
