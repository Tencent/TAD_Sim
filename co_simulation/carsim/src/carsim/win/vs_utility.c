/* This library contains simple utility C functions that are used in
   VehicleSim-related programs.

   Copyright 1996 - 2007. Mechanical Simulation Corporation.

   Revison Log
   ===========
   Sep 04, 07. M. Sayers. Removed unused extern.
   May 01, 07. M. Sayers. Put checks into calls to free.
   Mar 12, 07. M. Sayers. Released CarSim 7.0.
   Oct 05, 06. M. Sayers. Created, taking code from other library files.
   */

#include <stdio.h>  // Standard C libraries...
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef _MSC_VER
  #include <windows.h> // used for Windows clock
#endif
#include <time.h> // used for date function

#include "vs_deftypes.h" // VS types and definitions
#include "vs_utility.h"  // VS utility functions

/* ----------------------------------------------------------------------------
   Compare real and int.
---------------------------------------------------------------------------- */
int vs_compare_real_int(vs_real x, int i)
{
  return vs_nint(x) == i;
}

/* ----------------------------------------------------------------------------
   Do nothing (used to force inclusion of a variable in VehicleSim.)
---------------------------------------------------------------------------- */
void vs_do_nothing (vs_real var) {;}

/* ----------------------------------------------------------------------------
   Round a floating point number toward zero.
---------------------------------------------------------------------------- */
vs_real vs_fix(vs_real x)
{
  if (x < 0.0) return ceil(x);
  else         return floor(x);
}


/* ----------------------------------------------------------------------------
   Return x_then if TEST > 0, return x_else otherwise.
---------------------------------------------------------------------------- */
vs_real vs_if_gt0_then(vs_real test, vs_real x_then, vs_real x_else)
{
  if (test > 0.0) return x_then;
  else return x_else;
}

/* ----------------------------------------------------------------------------
   Return x_then if TEST is not NULL, return x_else otherwise.
---------------------------------------------------------------------------- */
vs_real vs_if_true_then(vs_real test, vs_real x_then, vs_real x_else)
{
  if (test) return x_then;
  else return x_else;
}

/* ----------------------------------------------------------------------------
   Simple replacement for FORTRAN max and min functions (limited to 2 args,
   vs_real).
---------------------------------------------------------------------------- */
vs_real vs_max(vs_real a, vs_real b)
{
  if (a > b) return a;
  else       return b;
}

vs_real vs_min(vs_real a, vs_real b)
{
  if (a < b) return a;
  else       return b;
}


/* ----------------------------------------------------------------------------
   This is a C implementation of a standard FORTRAN function.
---------------------------------------------------------------------------- */
int vs_nint(vs_real a)
{
  if      (a > 0.0) return (int)(a + 0.5);
  else if (a < 0.0) return (int)(a - 0.5);
  else              return 0;
}

/* ----------------------------------------------------------------------------
   C implementation of a standard FORTRAN function.
---------------------------------------------------------------------------- */
vs_real vs_sign(vs_real a, vs_real b)
{
  if (b < 0.0) return -fabs(a);
  else         return  fabs(a);
}


/* ----------------------------------------------------------------------------
   Number of seconds since reference time.
---------------------------------------------------------------------------- */
vs_real vs_seconds_elapsed(void)
{
#ifdef _MSC_VER
  SYSTEMTIME time;
  GetSystemTime (&time);
  return (vs_real)time.wDay*24*3600.0 + time.wHour*3600.0 + time.wMinute*60.0
                  + time.wSecond + time.wMilliseconds*0.001;
#else
  time_t now;
  struct tm *date;
  long hours = 3600, mins = 60;

  now = time(NULL);
  date = localtime(&now);
  return (vs_real)(date->tm_hour*hours + date->tm_min*mins + date->tm_sec);
#endif
}


/* ----------------------------------------------------------------------------
  A smooth step function that uses a cosine taper in the transition region.

 --> x      vs_real  Independent variable for fstep_as function.
 --> x0     vs_real  Value of ind. var. at which step transition begins.
 --> h0     vs_real  Value of fstep for x < x0.
 --> x1     vs_real  Value of ind. var. at which step transition ends.
 --> h1     vs_real  Value of fstep for x > x0.
---------------------------------------------------------------------------- */
vs_real vs_step_smooth (vs_real x, vs_real x0, vs_real h0, vs_real x1, vs_real h1)
{
  if (x <= x0)      return h0;
  else if (x >= x1) return h1;
  else              return h0 + 0.5*(h1-h0)*(1.0 - cos(PI*(x-x0)/(x1-x0)));
}


/* ----------------------------------------------------------------------------
   Use this function to allocate memory for structure strings and set the value
 --------------------------------------------------------------------------- */
void vs_string_duplicate (char **target, char *source)
{
  if (source == NULL) *target = NULL;
  else
    {
    *target = (char *)malloc(sizeof(char)*(strlen(source) + 1));
    strcpy (*target, source);
    }
}


/* ----------------------------------------------------------------------------
   string find and replace. Return NULL (and leave *string alone) if size
   is too small (size = length of string + 1 for NULL delimiter)
---------------------------------------------------------------------------- */
char *vs_string_find_replace(char *string, char *find, char *replace,
                             unsigned int size)
{
  unsigned int lenf = strlen(find), lenr = strlen(replace), expand = (lenr > lenf),
      count = 0;
  char *p, *replaced, *next;

  // see if string is large enough to handle changes
  if (expand)
    {
    p = strstr(string, find);
    while (p)
      {
      p += lenf;
      count++;
      p = strstr(p, find);
      }
    if (strlen(string) + count*(lenr -lenf) > size -1) return NULL;
    }

  // go for it.
  replaced = (char *)malloc(size*sizeof(char));
  replaced[0] = 0;

  p = string;
  next = strstr(p, find);
  while (next)
    {
    strncat (replaced, p, next - p);
    strcat (replaced, replace);
    p = next + lenf;
    next = strstr(p, find);
    }
  if (*p) strcat(replaced, p);
  strcpy(string, replaced);
  free (replaced);
  return string;
}


/* ----------------------------------------------------------------------------
   Capitalize string.
---------------------------------------------------------------------------- */
char *vs_string_to_upper(char *s)
{
  char   *p = s;
  while (*p)
    {
    *p = toupper(*p);
    *p++;
    }
  return s;
}


/* ----------------------------------------------------------------------------
   Remove trailing spaces from a string.
---------------------------------------------------------------------------- */
char *vs_string_trim_trailing_space (char *str)
{
  int  i;

  if (str == NULL) return NULL;

  for (i = strlen(str) - 1; i && isspace(str[i]); i--)
    ;
  str[i+1] = '\0';
  return str;
}

/* ----------------------------------------------------------------------------
   Get the time and date.
---------------------------------------------------------------------------- */
void vs_time_and_date(char *t_date)
  {
    time_t now;
    struct tm *date;
    int    hour, min, day, year, mon;
    char   month[5];
    char   *months = "JanFebMarAprMayJunJulAugSepOctNovDec";

    now = time(NULL);
    date = localtime( &now );

    hour = date->tm_hour;
    min = date->tm_min;
    day = date->tm_mday;
    year = date->tm_year + 1900;
    mon = date->tm_mon;
    if (mon <= 11 && mon >=0)
      {
      month[0] = months[(mon*3)];
      month[1] = months[(mon*3+1)];
      month[2] = months[(mon*3+2)];
      month[3] = '.';
      month[4] = '\0';
      }
    sprintf (t_date, "%02d:%02d on %s %02d, %4d", hour, min, month, day, year);
}
