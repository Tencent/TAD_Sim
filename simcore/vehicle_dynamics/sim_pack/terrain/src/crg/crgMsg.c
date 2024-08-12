/* ===================================================
 *  file:       crgMsg.c
 * ---------------------------------------------------
 *  purpose:	message handling routines for the
 *              OpenCRG project
 * ---------------------------------------------------
 *  first edit:	31.10.2008 by M. Dupuis @ VIRES GmbH
 *  last mod.:  02.11.2009 by M. Dupuis @ VIRES GmbH
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
#include <stdio.h>
#include "crgBaseLibPrivate.h"

/* ====== LOCAL VARIABLES ====== */
static int mLevel = dCrgMsgLevelNotice;

/* ====== LOCAL METHODS ====== */

/* ====== IMPLEMENTATION ====== */

void crgMsgSetLevel(int level) {
  if (level >= dCrgMsgLevelNone && level <= dCrgMsgLevelDebug) mLevel = level;

  /* --- update the setting in the portability libraries --- */
  crgPortSetMsgLevel(mLevel);
}

int crgMsgGetLevel(void) { return mLevel; }

const char* crgMsgGetLevelName(int level) {
  if (level == dCrgMsgLevelNone) return "NONE";

  if (level == dCrgMsgLevelFatal) return "FATAL";

  if (level == dCrgMsgLevelWarn) return "WARNING";

  if (level == dCrgMsgLevelNotice) return "NOTICE";

  if (level == dCrgMsgLevelInfo) return "INFO";

  if (level == dCrgMsgLevelDebug) return "DEBUG";

  return "unknown level";
}

void crgMsgSetMaxWarnMsgs(int maxNo) { crgPortSetMaxWarnMsgs(maxNo); }

void crgMsgSetMaxLogMsgs(int maxNo) { crgPortSetMaxLogMsgs(maxNo); }

int crgMsgIsPrintable(int level) {
  if (mLevel < level) return 0;

  return crgPortMsgIsPrintable(level);
}
