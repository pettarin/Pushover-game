/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the COPYRIGHT file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#include "tools.h"

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <lmcons.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <dirent.h>

#include <stdexcept>

#if defined(__AROS__)
#include <proto/locale.h>
#include <proto/dos.h>
#endif

uint64_t getTime(void)
{
#ifdef WIN32
  FILETIME currentTime;
  GetSystemTimeAsFileTime(&currentTime);
  return currentTime.dwLowDateTime;
#else
  return time(0);
#endif
}


void srandFromTime(void) {

  srand(getTime());
}

std::string getHome(void) {

#ifdef WIN32

  static char userHome[MAX_PATH+1];

  HKEY key;
  DWORD size = MAX_PATH;

  if (RegOpenKeyEx(HKEY_CURRENT_USER,
                   "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                   0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
    userHome[0] = '\0';

  else if (RegQueryValueEx(key, "Personal", 0, 0, (LPBYTE)userHome, &size ) != ERROR_SUCCESS)
    userHome[0] = '\0';

  else
    RegCloseKey(key);

  size = strlen(userHome);
  userHome[size] = '\0';

  std::string home = std::string(userHome) + "\\pushover\\";

#elif defined (__AROS__)

  static char binaryPath[1024] = {0};
  if (!*binaryPath)
    NameFromLock(GetProgramDir(), binaryPath, sizeof(binaryPath));
  std::string home = std::string(binaryPath);

#else

  std::string home = std::string(getenv("HOME"))+"/.pushover/";

#endif

  DIR * dir = ::opendir(home.c_str());

  if (dir)
  {
    closedir(dir);
  }
  else
  {
    // create it
#ifdef WIN32
    if (::mkdir(home.c_str()) != 0)
#else
    if (::mkdir(home.c_str(), S_IRWXU) != 0)
#endif
      throw std::runtime_error("Can't create home directory: " + home);
  }

#if defined (__AROS__)
  home += '/';
#endif

  return home;
}

std::vector<std::string> directoryEntries(const std::string & path) {
  DIR * dir = ::opendir(path.c_str());
  if (dir == NULL)
    throw std::runtime_error("Can't open directory: " + path);
  std::vector<std::string> entries;
  for (struct dirent * i = ::readdir(dir); i != NULL; i = ::readdir(dir))
    entries.push_back(i->d_name);
  if (::closedir(dir) != 0)
    throw std::runtime_error("Can't close directory: " + path);
  return entries;
}


void amigaLocale(void)
{

#if defined (__AROS__)
  struct LocaleConv
  {
    char *langstr;
    char *gmostr;
  };

  struct LocaleConv LocaleConvTab[] =
  {
    {"català", "ca"}, // Catalan
    {"czech", "cz"}, // French
    {"dansk", "da"}, // Danish
    {"deutsch", "de"}, // German
    {"español", "es"}, // Spanish
    {"esperanto", "eo"}, // Esperanto
    {"français", "fr"}, // French
    {"hrvatski", "hr"}, // Croatian
    {"ellinikí", "el"}, // Greek
    {"íslenska", "is"}, // Icelandic
    {"italiano", "it"}, // Italian
    {"magyar", "hu"}, // Hungarian <3
    {"malti", "mt"}, // Maltese
    {"nederlands", "nl"}, // Dutch
    {"norsk", "no"}, // Norwegian Nynorsk
    {"polski", "pl"}, // Polish
    {"português" , "pt"}, // Portuguese
    {"russian", "ru"}, // Russian
    {"shqipja", "sq"}, // Albanian
    {"svenska", "sv"}, // Swedish
    {"suomi", "fi"}, // Finnish
    {"thai", "th"}, // Thai
    {"türkçe", "tr"}, // Turkish
    {0, 0}
  };

  struct Locale *defaultLocale;
  char *locale = NULL;

  if ((defaultLocale = OpenLocale(NULL)))
  {
    int i = 0;
    while (!locale && (i < 10) && defaultLocale->loc_PrefLanguages[i])
    {
      struct LocaleConv *lcptr = LocaleConvTab;

      while (lcptr->langstr)
      {
        if (!StrnCmp(defaultLocale, lcptr->langstr, defaultLocale->loc_PrefLanguages[i], -1, SC_ASCII))
          break;

        lcptr++;
      }

      locale = lcptr->gmostr;

      i++;
    }
    CloseLocale(defaultLocale);
  }

  if (locale)
  {
    SetVar("LC_MESSAGES", locale, strlen(locale) + 1, GVF_LOCAL_ONLY);
  }
#endif
}

