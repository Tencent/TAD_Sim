/* ===================================================
 *  file:       crgPortability.c
 * ---------------------------------------------------
 *  purpose:	collections of methods which may be
 *              subject to portability issues
 * ---------------------------------------------------
 *  first edit:	26.05.2009 by M. Dupuis @ VIRES GmbH
 *  last mod.:  16.03.2010 by M. Dupuis @ VIRES GmbH
 * ===================================================
    Copyright 2013 VIRES Simulationstechnologie GmbH

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */
/* ====== INCLUSIONS ====== */
#include <stdarg.h>
#include <stdio.h>
#include "crgBaseLibPrivate.h"

/*
 * try to stay compatible with older MSM compilers
 */
#if defined(_MSC_VER) && (_MSC_VER < 1500)
#  define vsnprintf _vsnprintf
#endif

/* ====== LOCAL VARIABLES ====== */
static int mMsgLevel = dCrgMsgLevelNotice;
static int mMaxWarnMsgs = -1;
static int mMaxLogMsgs = -1;

static void* (*mCallocCallback)(size_t nmemb, size_t size) = NULL;
static void* (*mReallocCallback)(void* ptr, size_t size) = NULL;
static void (*mFreeCallback)(void* ptr) = NULL;
static int (*mMsgCallback)(int level, char* message) = NULL;

void crgMsgPrint(int level, const char* format, ...) {
  va_list ap;
  int ret;

  if (mMsgLevel < level) return;

  /* --- is re-direction activated? --- */
  if (mMsgCallback) {
    char buffer[1024]; /* limit message text to 1024 characters */

    buffer[0] = '\0';

    va_start(ap, format);
    ret = vsnprintf(buffer, 1023, format, ap);
    /* NOTE: on some compilers, vsnprintf() may be called _vsnprintf() */
    va_end(ap);

    if (ret <= 0)
      fprintf(stderr, "crgMsgPrint: Cannot create message.\n");
    else
      mMsgCallback(level, buffer);

    return;
  }

  /** @todo: this is just a temporary solution and should be completed until 1.0 */
  if (!mMaxWarnMsgs) return;

  if (mMaxWarnMsgs > 0) mMaxWarnMsgs--;

  fprintf(stderr, "%7s: ", crgMsgGetLevelName(level));

  va_start(ap, format);
  ret = vfprintf(stderr, format, ap);
  va_end(ap);

  if (ret <= 0) fprintf(stderr, "crgMsgPrint: Cannot create message.\n");
}

void crgPortSetMsgLevel(int level) {
  if (level >= dCrgMsgLevelNone && level <= dCrgMsgLevelDebug) mMsgLevel = level;
}

void* crgCalloc(size_t nmemb, size_t size) {
  if (mCallocCallback) return mCallocCallback(nmemb, size);
  return calloc(nmemb, size);
}

void crgCallocSetCallback(void* (*func)(size_t nmemb, size_t size)) { mCallocCallback = func; }

void* crgRealloc(void* ptr, size_t size) {
  if (mReallocCallback) return mReallocCallback(ptr, size);
  return realloc(ptr, size);
}

void crgReallocSetCallback(void* (*func)(void* ptr, size_t size)) { mReallocCallback = func; }

void crgFree(void* ptr) {
  if (mFreeCallback) {
    mFreeCallback(ptr);
    return;
  }
  free(ptr);
}

void crgFreeSetCallback(void (*func)(void* ptr)) { mFreeCallback = func; }

void crgPortSetMaxWarnMsgs(int maxNo) { mMaxWarnMsgs = maxNo; }

void crgPortSetMaxLogMsgs(int maxNo) { mMaxLogMsgs = maxNo; }

int crgPortMsgIsPrintable(int level) { return mMaxWarnMsgs != 0; }

void crgMsgSetCallback(int (*func)(int level, char* message)) { mMsgCallback = func; }
