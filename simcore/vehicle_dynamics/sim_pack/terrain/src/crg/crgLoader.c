/* ===================================================
 *  file:       crgLoader.c
 * ---------------------------------------------------
 *  purpose:	loads a CRG file and pre-processes the
 *              data
 * ---------------------------------------------------
 *  based on routines by Dr. Jochen Rauh, Daimler AG
 * ---------------------------------------------------
 *  first edit:	31.10.2008 by M. Dupuis @ VIRES GmbH
 *  last mod.:  31.10.2014 by M. Dupuis @ VIRES GmbH
 * ===================================================
    Copyright 2014 VIRES Simulationstechnologie GmbH

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
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "crgBaseLibPrivate.h"

/* ====== DEFINITIONS ====== */
#define dCrgLoaderMaxTagLen 128
#define dCrgLoaderBufferLen 1024

#define dOpcodeNone 0
#define dOpcodeRefLineStartU 1
#define dOpcodeRefLineStartX 2
#define dOpcodeRefLineStartY 3
#define dOpcodeRefLineStartPhi 4
#define dOpcodeRefLineEndU 5
#define dOpcodeRefLineEndX 6
#define dOpcodeRefLineEndY 7
#define dOpcodeRefLineEndPhi 8
#define dOpcodeRefLineIncrement 9
#define dOpcodeLongSectionVRight 10
#define dOpcodeLongSectionVLeft 11
#define dOpcodeLongSectionVIncrement 12
#define dOpcodeRefLineStartBank 13
#define dOpcodeRefLineEndBank 14
#define dOpcodeRefLineStartSlope 15
#define dOpcodeRefLineEndSlope 16
#define dOpcodeRefLineStartZ 17
#define dOpcodeRefLineEndZ 18
#define dOpcodeIncludeItem 19
#define dOpcodeIncludeDone 20

#define dFileSectionNone 0
#define dFileSectionRoadCrg 1
#define dFileSectionComment 2
#define dFileSectionDataDef 3
#define dFileSectionDataContent 4
#define dFileSectionOptions 5
#define dFileSectionModifiers 6
#define dFileSectionFile 7

#define dDataFormatUndefined 0x0000
#define dDataFormatCompact 0x0001
#define dDataFormatLong 0x0002
#define dDataFormatPrecisionSingle 0x0004
#define dDataFormatPrecisionDouble 0x0008
#define dDataFormatASCII 0x0010
#define dDataFormatBinary 0x0020

#ifdef _WIN64
#  define stat _stat64
#elif _WIN32
// Nothing to do, default stat uses 32 bit on Windows
#elif __linux__
// Nothing to do, Linux automatically switches between 32-bit and 64-bit version of stat depending on the architecture
#endif

/* ====== TYPE DEFINITIONS ====== */
typedef struct {
  char tag[dCrgLoaderMaxTagLen];
  int (*func)(CrgDataStruct*, const char*, int);
  int opcode;
} CrgReaderCallbackStruct;

/* ====== LOCAL METHODS ====== */
/**
 * initialize a data structure
 * @param crgData    pointer to the CRG data set which is to be initialized
 */
static void initData(CrgDataStruct* crgData);

/**
 * clear temporary data
 * @param crgData    pointer to the CRG data set which is to be altered
 */
static void clearTmpData(CrgDataStruct* crgData);

/**
 * clear data contained in a single channel
 * @param chan pointer to the channel
 */
static void clearChannel(CrgChannelBaseStruct* chan);

/**
 * clear all channels of a data set
 * @param crgData    pointer to the CRG data set which is to be altered
 */
static void clearChannels(CrgDataStruct* crgData);

/**
 * terminate the reader in a nice fashion (clean up the waste)
 * @param crgData    pointer to the CRG data set which is to be altered
 * @param retCode    the code which shall be returned by the reader
 * @return the return code given as argument above
 */
static int terminateReader(CrgDataStruct* crgData, int retCode);

/**
 * scans an ASCII buffer for a keyword and returns the pointer to the
 * method that should be called according to the keyword
 * @param buffer     the ASCII buffer which is to be evaluated
 * @param cbs        a structure holding callback methods vs. keywords
 * @param opcode     the opcode resulting from the found callback
 * @return the callback method or NULL if none was found
 */
static int (*scanTagsForCallback(const char* buffer, CrgReaderCallbackStruct* cbs, int* opcode))();

/**
 * check whether the first tag contained in the buffer is and "end-of-section" tag
 * @param buffer     the ASCII buffer which is to be evaluated
 * @return 1 if buffer content is an "end-of-section" tag, otherwise 0
 */
static int tagIsEndOfSection(const char* buffer);

/**
 * check whether the buffer content is a comment
 * @param buffer     the ASCII buffer which is to be evaluated
 * @return 1 if buffer content is a comment, otherwise 0
 */
static int isComment(const char* buffer);

/**
 * callback method
 * decode a numeric double parameter definition within the header
 * @param crgData    pointer to the CRG data set which is to be altered
 * @param buffer     the ASCII buffer which is to be evaluated
 * @param opcode     the opcode according to which to decode the value
 * @return 1 if successful, otherwise 0
 */
static int decodeHdrDouble(CrgDataStruct* crgData, const char* buffer, int opcode);

/**
 * callback method
 * decode an option or modifier definition within the header
 * @param crgData    pointer to the CRG data set which is to be altered
 * @param buffer     the ASCII buffer which is to be evaluated
 * @param opcode     the opcode according to which to decode the value
 * @return 1 if successful, otherwise 0
 */
static int decodeHdrOpMod(CrgDataStruct* crgData, const char* buffer, int opcode);

/**
 * callback method
 * set the type of the data file section that is being evaluated
 * @param crgData    pointer to the CRG data set which is to be altered
 * @param buffer     ignored
 * @param newSection the new section type which is to be set
 * @return 1 if successful, otherwise 0
 */
static int setSection(CrgDataStruct* crgData, const char* buffer, int newSection);

/**
 * callback method
 * decode an independent parameter (not yet implemented)
 * @param crgData    pointer to the CRG data set which is to be altered
 * @param buffer the buffer containing the parameter definition
 * @param code   optional operation code (ignored)
 * @return 1 if successful, otherwise 0
 */
static int decodeIndependent(CrgDataStruct* crgData, const char* buffer, int code);

/**
 * callback method
 * decode a parameter definition
 * @param crgData    pointer to the CRG data set which is to be altered
 * @param buffer the buffer containing the parameter definition
 * @param code   optional operation code (ignored)
 * @return 1 if successful, otherwise 0
 */
static int decodeDefined(CrgDataStruct* crgData, const char* buffer, int code);

/**
 * decode a data format indicator
 * @param crgData    pointer to the CRG data set which is to be altered
 * @param buffer the original buffer data (string)
 * @param code   the code which is to be taken care of while decoding the buffer
 * @return 1 if successful, otherwise 0
 */
static int decodeDataFormat(CrgDataStruct* crgData, const char* buffer, int code);

/**
 * find first occurence  of a token in the given character string and return
 * the pointer to the character AFTER the end of the token
 * leading spaces will be omitted
 * @param haystack   the haystack which will be searched
 * @param token      the token to search for
 * @return pointer to the character after the token or NULL if token was not found
 */
static const char* findToken(const char* haystack, const char* token);

/**
 * check the consistency of the header information
 * @param crgData    pointer to the CRG data set which is to be altered
 * @return 1 if successful, otherwise 0
 */
static int checkHeaderConsistency(CrgDataStruct* crgData);

/**
 * prepare data if v data is defined by position
 * @param crgData    pointer to the CRG data set which is to be altered
 * @return 1 if successful, otherwise 0
 */
static int prepareFromPosDef(CrgDataStruct* crgData);

/**
 * prepare data if v data is defined by index
 * @param crgData    pointer to the CRG data set which is to be altered
 * @return 1 if successful, otherwise 0
 */
static int prepareFromIndexDef(CrgDataStruct* crgData);

/**
 * calculate the size of a single data record
 * @param crgData    pointer to the CRG data set which is to be altered
 */
static void calcRecordSize(CrgDataStruct* crgData);

/**
 * retrieve an ASCII line from a buffer (i.e. get all bytes until the
 * terminating '\n' or end of buffer)
 * @param  dstBuffer  pointer to the destination where to copy the resulting ASCII data
 * @param  dstSize    maximum number of bytes which may be stored in the destination
 * @param  srcBuffer  the source from which to copy the data
 * @param  srcSize    maximum number of bytes which may be retrieved from the source
 * @return number of bytes successfully retrieved (0 = EOF)
 */
static size_t getLineFromData(char* dstBuffer, int dstSize, char* srcBuffer, size_t srcSize);

/**
 * parse the file header (ASCII part)
 * @param  crgData     pointer to the CRG data set which is to be altered
 * @param  dataPtr     pointer to the file data (will be altered)
 * @param  nBytesLeft  number of bytes left for interpretation (will be altered)
 * @return 1 upon success, otherwise 0
 */
static int parseFileHeader(CrgDataStruct* crgData, char** dataPtr, size_t* nBytesLeft);

/**
 * get pointer to the next data record from the input file
 * @param  recordSize  size of a single record
 * @param  dataFormat  formatting of a single record
 * @param  dataPtr     pointer to the file data
 * @param  nBytesLeft  number of bytes left for interpretation
 * @return pointer to the data record
 */
static char* getNextRecord(size_t recordSize, int dataFormat, char* dataPtr, size_t nBytesLeft);

/**
 * decode a single data record
 * @param  crgData     pointer to the CRG data set which is to be altered
 * @param  dataPtr     pointer to the record data
 * @param  length      number of bytes in the record
 * @return 1 upon success, otherwise 0
 */
static int decodeRecord(CrgDataStruct* crgData, char* dataPtr, size_t length);

/**
 * get the next data record and decode it
 * @param  crgData     pointer to the CRG data set which is to be altered
 * @param  dataPtr     pointer to the current record data (will be altered)
 * @param  length      number of bytes left for decoding (will be altered)
 * @return 1 upon success, otherwise 0
 */
static int decodeNextRecord(CrgDataStruct* crgData, char** dataPtr, size_t* nBytesLeft);

/**
 * parse the data section of the file for information about the center line
 * @param  crgData     pointer to the CRG data set which is to be altered
 * @param  dataPtr     pointer to the file data
 * @param  nBytesLeft  number of bytes left for interpretation
 * @return 1 upon success, otherwise 0
 */
static int parseCenterLine(CrgDataStruct* crgData, char* dataPtr, size_t nBytesLeft);

/**
 * allocate space for the actual channel data
 * @return 1 upon success, otherwise 0
 */
static int allocateChannels(CrgDataStruct* crgData);

/**
 * read the actual CRG data
 * @param  crgData     pointer to the CRG data set which is to be altered
 */
static void readData(CrgDataStruct* crgData);

/**
 * calculate the CRG reference line
 * @param  crgData     pointer to the CRG data set which is to be altered
 */
static void calcRefLine(CrgDataStruct* crgData);

/**
 * calculate the CRG reference line's elevation profile
 * @param  crgData     pointer to the CRG data set which is to be altered
 */
static void calcRefLineZ(CrgDataStruct* crgData);

/**
 * normalize the CRG reference line
 * @param  crgData     pointer to the CRG data set which is to be altered
 */
static void normalizeRefLine(CrgDataStruct* crgData);

/**
 * normalize the CRG z data
 * @param  crgData     pointer to the CRG data set which is to be altered
 */
static void normalizeZ(CrgDataStruct* crgData);

/**
 * smoothen the reference line if curvature data is given
 * @param  crgData     pointer to the CRG data set which is to be altered
 */
static void smoothenRefLine(CrgDataStruct* crgData);

/**
 * read a double value from the data set and perform any necessary endian conversion
 * @param dataPtr pointer where to start reading the number from
 * @param tgt     pointer to the location where to store the value
 * @return        1 if successful, 0 if error occured, -1 if value is NaN
 */
static int readDouble(char* dataPtr, double* tgt);

/**
 * read a float value from the data set and perform any necessary endian conversion
 * @param dataPtr pointer where to start reading the number from
 * @param tgt     pointer to the location where to store the value
 * @return        1 if successful, 0 if error occured, -1 if value is NaN
 */
static int readFloat(char* dataPtr, float* tgt);

/**
 * check whether machine is little endian
 * @return 1 if machine is little endian, otherwise 0
 */
static int isLittleEndian(void);

/**
 * check whether the first string begins with the characters of the second, ignoring case;
 * this method was introduced due to incompatibility of strncasecmp with
 * standard c compilers
 * @param str1	string whose begin should is tested for occurrence of str2
 * @param str2   string to be searched for at the begin of str1
 * @return 1 if strings match, otherwise 0
 */
static int crgStrBeginsWithStrNoCase(const char* str1, const char* str2);

/**
 * initialize the loader before reading the first file
 */
static void crgLoaderInit(void);

/**
 * decode a reference to an include file
 * @param crgData    pointer to the CRG data set which is to be altered
 * @param buffer the original buffer data (string)
 * @param code   the code which is to be taken care of while decoding the buffer
 * @return 1 if successful, otherwise 0
 */
static int decodeIncludeFile(CrgDataStruct* crgData, const char* buffer, int code);

/**
 * add data from a given file to existing data
 * @param filename   full filename of the CRG input file including path
 * @param crgData    pointer to the CRG data set which is to be allocated or altered
 * @return 1 if successful, otherwise 0 or error code
 */
static int crgLoaderAddFile(const char* filename, CrgDataStruct** crgData);

/* ====== LOCAL VARIABLES ====== */

static CrgReaderCallbackStruct sLoaderCallbacksCommon[] = {{"$ROAD_CRG_MODS", setSection, dFileSectionModifiers},
                                                           {"$ROAD_CRG_OPTS", setSection, dFileSectionOptions},
                                                           {"$ROAD_CRG_FILE", setSection, dFileSectionFile},
                                                           {"$ROAD_CRG", setSection, dFileSectionRoadCrg},
                                                           {"$CT", setSection, dFileSectionComment},
                                                           {"$KD_DEFINITION", setSection, dFileSectionDataDef},
                                                           {"$$$$", setSection, dFileSectionDataContent},
                                                           {"$", setSection, dFileSectionNone},
                                                           {"", NULL, -1}};

static CrgReaderCallbackStruct sLoaderCallbacksRoad[] = {
    {"reference_line_start_u", decodeHdrDouble, dOpcodeRefLineStartU},
    {"reference_line_start_x", decodeHdrDouble, dOpcodeRefLineStartX},
    {"reference_line_start_y", decodeHdrDouble, dOpcodeRefLineStartY},
    {"reference_line_start_z", decodeHdrDouble, dOpcodeRefLineStartZ},
    {"reference_line_start_phi", decodeHdrDouble, dOpcodeRefLineStartPhi},
    {"reference_line_start_b", decodeHdrDouble, dOpcodeRefLineStartBank},
    {"reference_line_start_s", decodeHdrDouble, dOpcodeRefLineStartSlope},
    {"reference_line_end_u", decodeHdrDouble, dOpcodeRefLineEndU},
    {"reference_line_end_x", decodeHdrDouble, dOpcodeRefLineEndX},
    {"reference_line_end_y", decodeHdrDouble, dOpcodeRefLineEndY},
    {"reference_line_end_z", decodeHdrDouble, dOpcodeRefLineEndZ},
    {"reference_line_end_phi", decodeHdrDouble, dOpcodeRefLineEndPhi},
    {"reference_line_end_b", decodeHdrDouble, dOpcodeRefLineEndBank},
    {"reference_line_end_s", decodeHdrDouble, dOpcodeRefLineEndSlope},
    {"reference_line_increment", decodeHdrDouble, dOpcodeRefLineIncrement},
    {"long_section_v_right", decodeHdrDouble, dOpcodeLongSectionVRight},
    {"long_section_v_left", decodeHdrDouble, dOpcodeLongSectionVLeft},
    {"long_section_v_increment", decodeHdrDouble, dOpcodeLongSectionVIncrement},
    {"reference_line_offset_x", decodeHdrDouble, dOpcodeNone},
    {"reference_line_offset_y", decodeHdrDouble, dOpcodeNone},
    {"reference_line_offset_z", decodeHdrDouble, dOpcodeNone},
    {"reference_line_offset_phi", decodeHdrDouble, dOpcodeNone},
    {"$", setSection, dFileSectionNone},
    {"", NULL, -1}};

static CrgReaderCallbackStruct sLoaderCallbacksOpts[] = {
    {"border_mode_u", decodeHdrOpMod, dCrgCpOptionBorderModeU},
    {"border_offset_u", decodeHdrOpMod, dCrgCpOptionBorderOffsetU},
    {"border_mode_v", decodeHdrOpMod, dCrgCpOptionBorderModeV},
    {"border_offset_v", decodeHdrOpMod, dCrgCpOptionBorderOffsetV},
    {"border_smooth_ubeg", decodeHdrOpMod, dCrgCpOptionSmoothUBegin},
    {"border_smooth_uend", decodeHdrOpMod, dCrgCpOptionSmoothUEnd},
    {"refline_continuation", decodeHdrOpMod, dCrgCpOptionRefLineContinue},
    {"refline_search_far", decodeHdrOpMod, dCrgCpOptionRefLineFar},
    {"refline_search_close", decodeHdrOpMod, dCrgCpOptionRefLineClose},
    {"refline_search_u", decodeHdrOpMod, dCrgCpOptionRefLineSearchU},
    {"refline_search_ufrac", decodeHdrOpMod, dCrgCpOptionRefLineSearchUFrac},
    {"warn_msgs", decodeHdrOpMod, dCrgCpOptionWarnMsgs},
    {"warn_curv_local", decodeHdrOpMod, dOpcodeNone},
    {"warn_curv_global", decodeHdrOpMod, dOpcodeNone},
    {"log_msgs", decodeHdrOpMod, dOpcodeNone},
    {"log_eval", decodeHdrOpMod, dOpcodeNone},
    {"log_eval_freq", decodeHdrOpMod, dOpcodeNone},
    {"log_hist", decodeHdrOpMod, dOpcodeNone},
    {"log_hist_freq", decodeHdrOpMod, dOpcodeNone},
    {"log_stat", decodeHdrOpMod, dOpcodeNone},
    {"log_stat_freq", decodeHdrOpMod, dOpcodeNone},
    {"check_eps", decodeHdrOpMod, dCrgCpOptionCheckEps},
    {"check_inc", decodeHdrOpMod, dCrgCpOptionCheckInc},
    {"check_tol", decodeHdrOpMod, dCrgCpOptionCheckTol},
    {"$", setSection, dFileSectionNone},
    {"", NULL, -1}};

static CrgReaderCallbackStruct sLoaderCallbacksMods[] = {
    {"scale_z_grid", decodeHdrOpMod, dCrgModScaleZ},
    {"scale_slope", decodeHdrOpMod, dCrgModScaleSlope},
    {"scale_banking", decodeHdrOpMod, dCrgModScaleBank},
    {"scale_length", decodeHdrOpMod, dCrgModScaleLength},
    {"scale_width", decodeHdrOpMod, dCrgModScaleWidth},
    {"scale_curvature", decodeHdrOpMod, dCrgModScaleCurvature},
    {"grid_nan_mode", decodeHdrOpMod, dCrgModGridNaNMode},
    {"grid_nan_offset", decodeHdrOpMod, dCrgModGridNaNOffset},
    {"refline_rotcenter_x", decodeHdrOpMod, dCrgModRefLineRotCenterX},
    {"refline_rotcenter_y", decodeHdrOpMod, dCrgModRefLineRotCenterY},
    {"refline_offset_phi", decodeHdrOpMod, dCrgModRefLineOffsetPhi},
    {"refline_offset_x", decodeHdrOpMod, dCrgModRefLineOffsetX},
    {"refline_offset_y", decodeHdrOpMod, dCrgModRefLineOffsetY},
    {"refline_offset_z", decodeHdrOpMod, dCrgModRefLineOffsetZ},
    {"refpoint_u_fraction", decodeHdrOpMod, dCrgModRefPointUFrac},
    {"refpoint_u_offset", decodeHdrOpMod, dCrgModRefPointUOffset},
    {"refpoint_u", decodeHdrOpMod, dCrgModRefPointU},
    {"refpoint_v_fraction", decodeHdrOpMod, dCrgModRefPointVFrac},
    {"refpoint_v_offset", decodeHdrOpMod, dCrgModRefPointVOffset},
    {"refpoint_v", decodeHdrOpMod, dCrgModRefPointV},
    {"refpoint_x", decodeHdrOpMod, dCrgModRefPointX},
    {"refpoint_y", decodeHdrOpMod, dCrgModRefPointY},
    {"refpoint_z", decodeHdrOpMod, dCrgModRefPointZ},
    {"refpoint_phi", decodeHdrOpMod, dCrgModRefPointPhi},
    {"$", setSection, dFileSectionNone},
    {"", NULL, -1}};

static CrgReaderCallbackStruct sLoaderCallbacksDataDef[] = {{"U:", decodeIndependent, dOpcodeNone},
                                                            {"D:", decodeDefined, dOpcodeNone},
                                                            {"#:", decodeDataFormat, dOpcodeNone},
                                                            {"$", setSection, dFileSectionNone},
                                                            {"", NULL, -1}};

static CrgReaderCallbackStruct sLoaderCallbacksFile[] = {{"$", decodeIncludeFile, dOpcodeIncludeDone}, {"", NULL, -1}};

/* ====== GLOBAL VARIABLES ====== */
int mCrgBigEndian = 0; /* internal data format is little endian per default */

/* ====== LOCAL VARIABLES ====== */
static int mFileLevel = 0; /* level at which current file is being read (for include files) */
static int mOptLevel = -1; /* level at which current options have been defined              */
static int mModLevel = -1; /* level at which current modifiers have been defined            */

/* ====== IMPLEMENTATION ====== */
static void initData(CrgDataStruct* crgData) {
  if (!crgData) return;

  /* --- fallback definitions --- */
  crgData->channelU.info.inc = 0.01;
  crgData->channelV.info.inc = 0.01;

  crgData->admin.dataFormat = dDataFormatUndefined;
}

static void clearTmpData(CrgDataStruct* crgData) {
  if (!crgData) return;

  if (crgData->admin.recordBuffer) free(crgData->admin.recordBuffer);

  crgData->admin.recordBuffer = NULL;
}

static void clearChannel(CrgChannelBaseStruct* chan) {
  if (!chan) return;

  if (chan->data) crgFree(chan->data);

  memset(chan, 0, sizeof(CrgChannelStruct));
}

static void clearChannels(CrgDataStruct* crgData) {
  size_t i;

  if (!crgData) return;

  clearChannel((CrgChannelBaseStruct*)&(crgData->channelU));
  clearChannel((CrgChannelBaseStruct*)&(crgData->channelX));
  clearChannel((CrgChannelBaseStruct*)&(crgData->channelY));
  clearChannel((CrgChannelBaseStruct*)&(crgData->channelPhi));
  clearChannel((CrgChannelBaseStruct*)&(crgData->channelSlope));
  clearChannel((CrgChannelBaseStruct*)&(crgData->channelBank));

  /* clear the actual data channels */
  for (i = 0; i < crgData->channelV.info.size; i++) clearChannel((CrgChannelBaseStruct*)&(crgData->channelZ[i]));

  clearChannel((CrgChannelBaseStruct*)&(crgData->channelV));
}

static int terminateReader(CrgDataStruct* crgData, int retCode) {
  if (!crgData) return retCode;

  clearTmpData(crgData);

  if (crgData->admin.fileBuffer) crgFree(crgData->admin.fileBuffer);

  crgData->admin.fileBuffer = NULL;

  /* --- if return code is fail code, then delete all data, otherwise keep the channels --- */
  if (retCode) return retCode;

  clearChannels(crgData);

  return retCode;
}

static int (*scanTagsForCallback(const char* buffer, CrgReaderCallbackStruct* cbs, int* opcode))() {
  const char* checkPtr = buffer;

  /* --- reset the resulting opcode --- */
  *opcode = dOpcodeNone;

  /* --- go to the first non-space character --- */
  while (*checkPtr == ' ') checkPtr++;

  while (cbs) {
    /* function defined? */
    if (!cbs->func) break;

    if (crgStrBeginsWithStrNoCase(checkPtr, cbs->tag)) {
      *opcode = cbs->opcode;
      return (int (*)())(cbs->func);
    }
    ++cbs;
  }

  /* --- no corresponding callback found --- */
  return NULL;
}

static int tagIsEndOfSection(const char* buffer) {
  const char* checkPtr = buffer;

  /* --- go to the first non-space character --- */
  /* disabled on April 12, 2009: since it may interfere with reference to environment variables! */
  /*
  while ( *checkPtr == ' ' )
       checkPtr++;
  */

  return !strncmp(checkPtr, "$", strlen("$"));
}

static int isComment(const char* buffer) {
  const char* checkPtr = buffer;

  /* --- ignore empty lines --- */
  if (strlen(buffer) < 1) return 1;

  /* --- decide upon the first non-space character --- */
  while (*checkPtr != '\0') {
    if (*checkPtr != ' ') return *checkPtr == '*';
    checkPtr++;
  }

  return 0;
}

static int decodeHdrDouble(CrgDataStruct* crgData, const char* buffer, int opcode) {
  /* --- get the assignment character --- */
  double value;
  char* bufPtr = (char*)strchr(buffer, '=');

  if (!bufPtr) return 0;

  value = atof(++bufPtr);

  switch (opcode) {
    case dOpcodeRefLineStartU:
      crgData->channelU.info.first = value;
      break;
    case dOpcodeRefLineStartX:
      crgData->channelX.info.first = value;
      break;
    case dOpcodeRefLineStartY:
      crgData->channelY.info.first = value;
      break;
    case dOpcodeRefLineStartPhi:
      crgData->channelPhi.info.first = value;
      break;
    case dOpcodeRefLineEndU:
      crgData->channelU.info.last = value;
      break;
    case dOpcodeRefLineEndX:
      crgData->channelX.info.last = value;
      crgData->admin.defMask |= dCrgDataDefXEnd;
      break;
    case dOpcodeRefLineEndY:
      crgData->channelY.info.last = value;
      crgData->admin.defMask |= dCrgDataDefYEnd;
      break;
    case dOpcodeRefLineEndPhi:
      crgData->channelPhi.info.last = value;
      break;
    case dOpcodeRefLineIncrement:
      crgData->channelU.info.inc = value;
      break;
    case dOpcodeLongSectionVRight:
      crgData->channelV.info.first = value;
      break;
    case dOpcodeLongSectionVLeft:
      crgData->channelV.info.last = value;
      break;
    case dOpcodeLongSectionVIncrement:
      crgData->channelV.info.inc = value;
      break;
    case dOpcodeRefLineStartBank:
      crgData->channelBank.info.first = value;
      crgData->util.hasBank = crgData->util.hasBank || (value != 0.0);
      break;
    case dOpcodeRefLineEndBank:
      crgData->channelBank.info.last = value;
      crgData->admin.defMask |= dCrgDataDefBankEnd;
      crgData->util.hasBank = crgData->util.hasBank || (value != 0.0);
      break;
    case dOpcodeRefLineStartSlope:
      crgData->channelSlope.info.first = value;
      break;
    case dOpcodeRefLineEndSlope:
      crgData->channelSlope.info.last = value;
      crgData->admin.defMask |= dCrgDataDefSlopeEnd;
      break;
    case dOpcodeRefLineStartZ:
      crgData->channelRefZ.info.first = value;
      crgData->admin.defMask |= dCrgDataDefZStart;
      break;
    case dOpcodeRefLineEndZ:
      crgData->channelRefZ.info.last = value;
      crgData->admin.defMask |= dCrgDataDefZEnd;
      break;
    case dOpcodeNone:
      break;

    default:
      crgMsgPrint(dCrgMsgLevelWarn, "decodeHdrDouble: unhandled opcode <%d>\n", opcode);
      return 0;
      break;
  }

  /* --- ok, this seems to have been successful --- */
  return 1;
}

static int decodeHdrOpMod(CrgDataStruct* crgData, const char* buffer, int opcode) {
  /* --- get the assignment character --- */
  char* bufPtr = (char*)strchr(buffer, '=');

  /* --- is decoding of options and modifiers allowed at current level? --- */
  int optionEnabled = (mFileLevel == 0) || (mOptLevel == mFileLevel);
  int modifierEnabled = (mFileLevel == 0) || (mModLevel == mFileLevel);

  crgMsgPrint(dCrgMsgLevelDebug, "decodeHdrOpMod: extracting option/modifier <%s> from <%s>\n",
              crgOptionGetName(opcode), buffer);

  if (!bufPtr) return 0;

  switch (opcode) {
    case dCrgCpOptionBorderModeU:
    case dCrgCpOptionBorderModeV:
      if (!optionEnabled) break;
      {
        int iValue = atoi(++bufPtr);
        switch (iValue) {
          case 0:
            crgOptionSetInt(&(crgData->options), opcode, dCrgBorderModeNone);
            break;
          case 1:
            crgOptionSetInt(&(crgData->options), opcode, dCrgBorderModeExZero);
            break;
          case 2:
            crgOptionSetInt(&(crgData->options), opcode, dCrgBorderModeExKeep);
            break;
          case 3:
            crgOptionSetInt(&(crgData->options), opcode, dCrgBorderModeRepeat);
            break;
          case 4:
            crgOptionSetInt(&(crgData->options), opcode, dCrgBorderModeReflect);
            break;
          default:
            crgMsgPrint(dCrgMsgLevelWarn, "decodeHdrOpMod: unhandled value for option <%s>\n",
                        crgOptionGetName(opcode));
            return 0;
        }
      }
      break;

    case dCrgCpOptionRefLineContinue:
      if (!optionEnabled) break;
      {
        int iValue = atoi(++bufPtr);
        switch (iValue) {
          case 0:
            crgOptionSetInt(&(crgData->options), opcode, dCrgRefLineExtrapolate);
            break;
          case 1:
            crgOptionSetInt(&(crgData->options), opcode, dCrgRefLineCloseTrack);
            break;
          default:
            crgMsgPrint(dCrgMsgLevelWarn, "decodeHdrOpMod: unhandled value for option <%s>\n",
                        crgOptionGetName(opcode));
            return 0;
        }
      }
      break;

    case dCrgCpOptionBorderOffsetU:
    case dCrgCpOptionBorderOffsetV:
    case dCrgCpOptionSmoothUBegin:
    case dCrgCpOptionSmoothUEnd:
    case dCrgCpOptionRefLineFar:
    case dCrgCpOptionRefLineClose:
    case dCrgCpOptionRefLineSearchU:
    case dCrgCpOptionRefLineSearchUFrac:
    case dCrgCpOptionCheckEps:
    case dCrgCpOptionCheckInc:
    case dCrgCpOptionCheckTol:
      if (!optionEnabled) break;
      {
        double dValue = atof(++bufPtr);
        crgOptionSetDouble(&(crgData->options), opcode, dValue);
      }
      break;

    case dCrgModScaleZ:
    case dCrgModScaleSlope:
    case dCrgModScaleBank:
    case dCrgModScaleLength:
    case dCrgModScaleWidth:
    case dCrgModScaleCurvature:
    case dCrgModGridNaNOffset:
    case dCrgModRefLineRotCenterX:
    case dCrgModRefLineRotCenterY:
    case dCrgModRefLineOffsetPhi:
    case dCrgModRefLineOffsetX:
    case dCrgModRefLineOffsetY:
    case dCrgModRefLineOffsetZ:
    case dCrgModRefPointUFrac:
    case dCrgModRefPointUOffset:
    case dCrgModRefPointU:
    case dCrgModRefPointVFrac:
    case dCrgModRefPointVOffset:
    case dCrgModRefPointV:
    case dCrgModRefPointX:
    case dCrgModRefPointY:
    case dCrgModRefPointZ:
    case dCrgModRefPointPhi:
      if (!modifierEnabled) break;
      {
        double dValue = atof(++bufPtr);
        crgOptionSetDouble(&(crgData->modifiers), opcode, dValue);
      }
      break;

    case dCrgModGridNaNMode:
      if (!modifierEnabled) break;
      {
        int iValue = atoi(++bufPtr);
        switch (iValue) {
          case 0:
            crgOptionSetInt(&(crgData->modifiers), opcode, dCrgGridNaNKeep);
            break;
          case 1:
            crgOptionSetInt(&(crgData->modifiers), opcode, dCrgGridNaNSetZero);
            break;
          case 2:
            crgOptionSetInt(&(crgData->modifiers), opcode, dCrgGridNaNKeepLast);
            break;
          default:
            crgMsgPrint(dCrgMsgLevelWarn, "decodeHdrOpMod: unhandled value for modifier <%s>\n",
                        crgOptionGetName(opcode));
            return 0;
        }
      }
      break;

    case dCrgCpOptionWarnMsgs: {
      int iValue = atoi(++bufPtr);

      crgMsgSetMaxWarnMsgs(iValue);
    } break;

    case dOpcodeNone:
      break;

    default:
      crgMsgPrint(dCrgMsgLevelWarn, "decodeHdrOpMod: unhandled opcode <%d>\n", opcode);
      return 0;
      break;
  }

  /* --- ok, this seems to have been successful --- */
  return 1;
}

static int setSection(CrgDataStruct* crgData, const char* buffer, int newSection) {
  /* --- changing from none or to none? --- */
  if ((crgData->admin.sectionType == dFileSectionNone) || (newSection == dFileSectionNone)) {
    crgData->admin.sectionType = newSection;

    /* --- depending on the new section type, some things may need to be initialized --- */
    switch (newSection) {
      case dFileSectionOptions:
        /* remove previously defined options and replace with default options */
        crgMsgPrint(dCrgMsgLevelDebug, "setSection: restoring default options\n");

        /* options may always be defined by the top level file; they may be defined by lower
           level files only if no options have yet been defined by top level file            */
        if (mFileLevel == 0 || mOptLevel < 0) {
          crgOptionSetDefaultOptions(&(crgData->options));
          mOptLevel = mFileLevel;
        }
        break;

      case dFileSectionModifiers:
        /* remove previously defined modifiers and replase with default modifiers */
        crgMsgPrint(dCrgMsgLevelDebug, "setSection: restoring default modifiers\n");
        /* modifiers may always be defined by the top level file; they may be defined by lower
           level files only if no modifiers have yet been defined by top level file            */
        if (mFileLevel == 0 || mModLevel < 0) {
          /* new policy: clear all options upon first occurence of option block and don't define any default options */
          /* crgOptionSetDefaultModifiers( &( crgData->modifiers ) ); */
          crgOptionRemoveAll(&(crgData->modifiers));
          mModLevel = mFileLevel;
        }
        break;

      default:
        break;
    }
    return 1;
  }
  return 0;
}

static int decodeIndependent(CrgDataStruct* crgData, const char* buffer, int code) { return 1; }

static int decodeDefined(CrgDataStruct* crgData, const char* buffer, int code) {
  const char* bufPtr = buffer;
  const char* tmpPtr = NULL;
  size_t i;
  double chanPos;
  int insertData = 0;

  if (!(bufPtr = findToken(buffer, "D:"))) return 0;

  /* --- is a v position defined? --- */
  if ((tmpPtr = findToken(bufPtr, "long section"))) {
    bufPtr = tmpPtr;

    if ((tmpPtr = findToken(bufPtr, "at v "))) {
      crgData->admin.defMask |= dCrgDataDefVPos;

      if (!(tmpPtr = findToken(tmpPtr, "="))) {
        crgMsgPrint(dCrgMsgLevelFatal, "decodeDefined: illegal long section position\n");
        return 0;
      }
      bufPtr = tmpPtr;
    } else
      crgData->admin.defMask |= dCrgDataDefVIndex;

    /* --- what's the position of the channel? --- */
    chanPos = atof(bufPtr);

    /* --- now for the correct unit --- */
    if (!(bufPtr = findToken(bufPtr, ","))) {
      crgMsgPrint(dCrgMsgLevelFatal, "decodeDefined: could not identify long section unit\n");
      return 0;
    }

    if (*bufPtr != 'm') {
      crgMsgPrint(dCrgMsgLevelFatal, "decodeDefined: wrong section unit\n");
      return 0;
    }

    /* --- (re-)allocate the channel data --- */
    crgData->channelZ = (CrgChannelFStruct*)crgRealloc(crgData->channelZ,
                                                       (crgData->channelV.info.size + 1) * sizeof(CrgChannelFStruct));
    memset(&(crgData->channelZ[crgData->channelV.info.size]), 0, sizeof(CrgChannelStruct));

    /* --- mark channel as single precision --- */
    crgData->channelZ[crgData->channelV.info.size].info.singlePrec = 1;

    crgData->channelV.data =
        (double*)crgRealloc(crgData->channelV.data, (crgData->channelV.info.size + 1) * sizeof(double));
    crgData->channelV.data[crgData->channelV.info.size] = 0.0;

    /* --- insert the channel at the correct position of v and z data --- */
    for (i = crgData->channelV.info.size + 1; i > 0; --i) {
      /* --- make this code a bit more robust for optimization --- */
      if (!(i - 1))
        insertData = 1;
      else
        insertData = (chanPos > crgData->channelV.data[i - 2]);

      if (insertData) {
        crgData->channelZ[i - 1].info.defined = 1;
        crgData->channelZ[i - 1].info.valid = 1;
        crgData->channelZ[i - 1].info.index = crgData->noChannels;
        crgData->channelZ[i - 1].info.size = crgData->channelU.info.size;

        crgData->channelV.data[i - 1] = chanPos;
        break;
      } else {
        memcpy(&(crgData->channelZ[i - 1]), &(crgData->channelZ[i - 2]), sizeof(CrgChannelFStruct));
        crgData->channelV.data[i - 1] = crgData->channelV.data[i - 2];
      }
    }

    /* --- ok, the channel may be registered --- */
    crgData->channelV.info.size++;
  } else if ((bufPtr = findToken(bufPtr, "reference line"))) {
    CrgChannelStruct* tgtChannel = NULL;
    const char* unitStr = "m";

    if ((tmpPtr = findToken(bufPtr, "x"))) {
      crgMsgPrint(dCrgMsgLevelDebug, "decodeDefined: found x channel\n");
      tgtChannel = &(crgData->channelX);
    } else if ((tmpPtr = findToken(bufPtr, "y"))) {
      crgMsgPrint(dCrgMsgLevelDebug, "decodeDefined: found y channel\n");
      tgtChannel = &(crgData->channelY);
    } else if ((tmpPtr = findToken(bufPtr, "u"))) {
      crgMsgPrint(dCrgMsgLevelDebug, "decodeDefined: found u channel\n");
      tgtChannel = &(crgData->channelU);
    } else if ((tmpPtr = findToken(bufPtr, "phi"))) {
      crgMsgPrint(dCrgMsgLevelDebug, "decodeDefined: found phi channel\n");
      tgtChannel = &(crgData->channelPhi);
      unitStr = "rad";
    } else if ((tmpPtr = findToken(bufPtr, "banking"))) {
      crgMsgPrint(dCrgMsgLevelDebug, "decodeDefined: found bank channel\n");
      tgtChannel = &(crgData->channelBank);
      unitStr = "m/m";
    } else if ((tmpPtr = findToken(bufPtr, "slope"))) {
      crgMsgPrint(dCrgMsgLevelDebug, "decodeDefined: found slope channel, index = %d\n", crgData->noChannels);
      tgtChannel = &(crgData->channelSlope);
      unitStr = "m/m";
    } else {
      crgMsgPrint(dCrgMsgLevelFatal, "decodeDefined: unknown definition\n");
      return 0;
    }

    if (!(tmpPtr = findToken(tmpPtr, ","))) {
      crgMsgPrint(dCrgMsgLevelFatal, "decodeDefined: missing unit information\n");
      return 0;
    }

    if (!findToken(tmpPtr, unitStr)) {
      crgMsgPrint(dCrgMsgLevelFatal, "decodeDefined: wrong unit information\n");
      return 0;
    }

    /* --- all right, we have a defined and, thus, valid channel for the reference line --- */
    tgtChannel->info.defined = 1;
    tgtChannel->info.valid = 1;
    tgtChannel->info.index = crgData->noChannels;
  }

  /* --- ok, we seem to have another valid channel definition --- */
  crgData->noChannels++;

  crgMsgPrint(dCrgMsgLevelDebug, "decodeDefined: buffer = <%s>, noChannels = %d\n", buffer, crgData->noChannels);

  return 1;
}

static int decodeDataFormat(CrgDataStruct* crgData, const char* buffer, int code) {
  const char* bufPtr = buffer;
  char dataFormat[] = "0000";

  if (!(bufPtr = findToken(buffer, "#:"))) return 0;

  /* data format are the first 4 characters after token */
  strncpy(dataFormat, bufPtr, 4);

  crgData->admin.dataFormat |= strchr(dataFormat, 'L') ? dDataFormatLong : dDataFormatCompact;
  crgData->admin.dataFormat |= strchr(dataFormat, 'D') ? dDataFormatPrecisionDouble : dDataFormatPrecisionSingle;
  crgData->admin.dataFormat |= strchr(dataFormat, 'F') ? dDataFormatASCII : dDataFormatBinary;

  return 1;
}

static const char* findToken(const char* haystack, const char* token) {
  const char* bufPtr = haystack;

  /*
  crgMsgPrint( dCrgMsgLevelDebug, "findToken: searching <%s> for <%s>\n", haystack, token );
  */

  /* --- get rid of leading spaces --- */
  while (*bufPtr == ' ') bufPtr++;

  /* parse until end of string or until comment character */
  while (*bufPtr != '\0' && *bufPtr != '!') {
    if (crgStrBeginsWithStrNoCase(bufPtr, token)) return bufPtr + strlen(token);

    ++bufPtr;
  }

  return NULL;
}

static int checkHeaderConsistency(CrgDataStruct* crgData) {
  if (!crgData) return 0;

  /* --- check for x and y definitions of reference line --- */
  if (crgData->channelX.info.defined != crgData->channelY.info.defined) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgLoader: reference line x, y must be both (un)defined\n");
    return 0;
  }

  if (crgData->channelV.info.size < 2) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgLoader: no or insufficient long section data available\n");
    return 0;
  }

  if ((crgData->admin.defMask & dCrgDataDefVPos) && (crgData->admin.defMask & dCrgDataDefVIndex)) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgLoader: inconsistent long section definition\n");
    return 0;
  }

  return 1;
}

static int prepareFromPosDef(CrgDataStruct* crgData) {
  double dvMin = 0.0;
  double dvMax = 0.0;
  size_t i;

  if (!crgData) return 0;

  /* --- look for minimum and maximum spacing of positions --- */
  for (i = 1; i < crgData->channelV.info.size; i++) {
    double dv = crgData->channelV.data[i] - crgData->channelV.data[i - 1];

    if ((i == 1) || (dv < dvMin)) dvMin = dv;

    if ((i == 1) || (dv > dvMax)) dvMax = dv;
  }

  /* --- minimum spacing must be larger than 1.e-6m --- */
  if (dvMin < 1.e-6) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgLoader: long section definition spacing too small\n");
    return 0;
  }

  /* --- v spacing is considered constant if relative spacing tolerance is less than 1.e-3 --- */
  if (((dvMax - dvMin) / dvMin) < 1.e-3) {
    /* --- get minimum and increment --- */
    crgData->channelV.info.first = crgData->channelV.data[0];
    crgData->channelV.info.inc = (crgData->channelV.data[crgData->channelV.info.size - 1] - crgData->channelV.data[0]) /
                                 (crgData->channelV.info.size - 1);
  } else {
    /* --- store v values with variable spacing --- */
    /* @todo: this may be obsolete! */
  }

  return 1;
}

static int prepareFromIndexDef(CrgDataStruct* crgData) {
  size_t i;

  if (!crgData) return 0;

  /* --- check number consistency --- */
  for (i = 0; i < crgData->channelV.info.size; i++)
    if (crgData->channelV.data[i] != 1.0 * (i + 1)) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgLoader: inconsistent long section numbers\n");
      return 0;
    }

  /* --- convert numbers to positions --- */
  /* crgData->channelV.info.first = - 0.5 * ( crgData->channelV.info.size - 1 ) * crgData->channelV.info.inc; */

  for (i = 0; i < crgData->channelV.info.size; i++)
    crgData->channelV.data[i] = crgData->channelV.info.first + i * crgData->channelV.info.inc;

  return 1;
}

static void calcRecordSize(CrgDataStruct* crgData) {
  size_t recordSize = 0;

  if (!crgData) return;

  if (crgData->admin.dataFormat & dDataFormatASCII)
    recordSize = (crgData->admin.dataFormat & dDataFormatPrecisionSingle) ? 10 : 20;
  else
    recordSize = (crgData->admin.dataFormat & dDataFormatPrecisionSingle) ? 4 : 8;

  recordSize *= crgData->noChannels;

  if (crgData->admin.dataFormat & dDataFormatLong)
    recordSize = (recordSize % 80) ? ((recordSize / 80 + 1) * 80) : recordSize;

  /* --- allocate space for v channel administration --- */
  clearTmpData(crgData);

  if (recordSize) crgData->admin.recordBuffer = (double*)crgCalloc(crgData->noChannels, sizeof(double));

  crgData->admin.recordSize = recordSize;

  crgMsgPrint(dCrgMsgLevelDebug, "calcRecordSize: expecting %lu channels, recordSize = %lu\n", crgData->noChannels,
              recordSize);
}

static size_t getLineFromData(char* dstBuffer, int dstSize, char* srcBuffer, size_t srcSize) {
  char* tgtPtr = strchr(srcBuffer, '\n');
  char* testPtr = strchr(srcBuffer, '\r');

  size_t xferSize;

  tgtPtr = (testPtr && (testPtr < tgtPtr)) ? testPtr : tgtPtr;

  if (tgtPtr) {
    while ((*tgtPtr == '\n') || (*tgtPtr == '\r')) tgtPtr++;
    xferSize = tgtPtr - srcBuffer;
  } else
    xferSize = dstSize - 1;

  /* --- cannot copy more data than is available or may be stored --- */
  if (xferSize > srcSize) xferSize = srcSize;

  if (xferSize >= (size_t)dstSize) xferSize = dstSize - 1;

  memset(dstBuffer, 0, dstSize);
  memcpy(dstBuffer, srcBuffer, xferSize);

  return xferSize;
}

static int parseFileHeader(CrgDataStruct* crgData, char** dataPtr, size_t* nBytesLeft) {
  char buffer[dCrgLoaderBufferLen];
  char* srcPtr = *dataPtr;
  size_t srcBytesLeft = *nBytesLeft;
  int (*func)(CrgDataStruct*, const char*, int);
  int opcode;
  size_t bytesRead;
  int lineOfFile = 0;
  CrgReaderCallbackStruct* cbs = sLoaderCallbacksCommon;

  /* --- initialize the section indicator --- */
  crgData->admin.sectionType = dFileSectionNone;

  while (srcBytesLeft) {
    bytesRead = getLineFromData(buffer, sizeof(buffer), srcPtr, srcBytesLeft);

    ++lineOfFile;

    srcPtr += bytesRead;
    srcBytesLeft -= bytesRead;

    /* --- first: check which callback methods are to be used --- */
    switch (crgData->admin.sectionType) {
      case dFileSectionRoadCrg:
        cbs = sLoaderCallbacksRoad;
        break;

      case dFileSectionDataDef:
        cbs = sLoaderCallbacksDataDef;
        break;

      case dFileSectionOptions:
        cbs = sLoaderCallbacksOpts;
        break;

      case dFileSectionModifiers:
        cbs = sLoaderCallbacksMods;
        break;

      case dFileSectionFile:
        cbs = sLoaderCallbacksFile;
        break;

      default:
        cbs = sLoaderCallbacksCommon;
        break;
    }

    /* --- now: parse with the appropriate methods --- */
    switch (crgData->admin.sectionType) {
      /* --- when in comment section, ignore all inputs until end of section --- */
      case dFileSectionComment:
        if (tagIsEndOfSection(buffer))
          setSection(crgData, NULL, dFileSectionNone);
        else
          crgMsgPrint(dCrgMsgLevelDebug, "parseFileHeader: comment: %s.\n", buffer);
        break;

      case dFileSectionFile:
        if (isComment(buffer)) break;

        if (tagIsEndOfSection(buffer)) {
          if (!decodeIncludeFile(crgData, buffer, dOpcodeIncludeDone)) return 0;

          setSection(crgData, NULL, dFileSectionNone);
        } else
          decodeIncludeFile(crgData, buffer, dOpcodeIncludeItem);
        break;

      default:
        if (isComment(buffer)) break;

        if ((func = (int (*)(CrgDataStruct*, const char*, int))(scanTagsForCallback(buffer, cbs, &opcode)))) {
          if (!func(crgData, buffer, opcode))
            crgMsgPrint(dCrgMsgLevelWarn, "parseFileHeader: Error parsing line %d.\n", lineOfFile);

          /* --- did we make it to the data section? --- */
          if (crgData->admin.sectionType == dFileSectionDataContent) {
            calcRecordSize(crgData);

            /* --- the number of bytes to be read may differ from the remaining file size due to alignment issues --- */
            /* --- therefore, calculate the maximum size which is to be read                                      --- */
            if (crgData->admin.dataFormat & dDataFormatBinary) {
              *nBytesLeft =
                  crgData->admin.recordSize *
                  ((size_t)((crgData->channelU.info.last - crgData->channelU.info.first) / crgData->channelU.info.inc +
                            0.5) +
                   1);

              if (*nBytesLeft > srcBytesLeft)
                crgMsgPrint(dCrgMsgLevelWarn,
                            "parseFileHeader: data section seems too small! Expecting %ld bytes but only have %ld "
                            "bytes. Reading available data only.\n",
                            *nBytesLeft, srcBytesLeft);
            } else
              *nBytesLeft = srcBytesLeft;

            crgMsgPrint(dCrgMsgLevelInfo, "parseFileHeader: (max.) nBytesLeft = %ld, srcBytesLeft = %ld\n", *nBytesLeft,
                        srcBytesLeft);
            /* --- reading the data section is business of another method --- */
            *dataPtr = srcPtr;
            return 1;
          }
        } else if (crgData->admin.sectionType != dFileSectionComment)
          crgMsgPrint(dCrgMsgLevelWarn, "parseFileHeader: ignoring line %d: <%s>.\n", lineOfFile, buffer);
        break;
    }
  }

  /* --- return with 0 if no data section was identified in the file --- */
  return 0;
}

static char* getNextRecord(size_t recordSize, int dataFormat, char* dataPtr, size_t nBytesLeft) {
  size_t i;

  if (!nBytesLeft) return NULL;

  /* --- is the data format binary? --- */
  if (dataFormat & dDataFormatBinary) {
    if (nBytesLeft >= recordSize) return dataPtr + recordSize;

    return NULL;
  }

  /* --- remove leading line breaks etc. --- */
  while (dataPtr && nBytesLeft) {
    if (*dataPtr == '\n' || *dataPtr == '\r') {
      dataPtr++;
      nBytesLeft--;
    } else
      break;
  }

  /* --- we handle ASCII data --- */
  if (dataFormat & dDataFormatLong) {
    size_t noLines = recordSize / 80;

    for (i = 0; i < noLines && dataPtr && nBytesLeft; i++) {
      char* oldDataPtr = dataPtr;
      char* termPtr = (nBytesLeft > (size_t)(2 * recordSize)) ? (dataPtr + 2 * recordSize) : NULL;
      char termChar = 0;
      char* testPtr = NULL;

      /* --- search for line termination in data set; temporarily terminate data set in order to increase search speed
       * --- */
      /* --- otherwise - in long data sets - strchr() will be very slow --- */
      if (termPtr) {
        termChar = *termPtr;
        *termPtr = '\0';
      }

      dataPtr = strchr(oldDataPtr, '\n');
      testPtr = strchr(oldDataPtr, '\r');

      if (testPtr) dataPtr = (testPtr < dataPtr) ? dataPtr : testPtr;

      if (dataPtr) {
        dataPtr++;
        nBytesLeft -= dataPtr - oldDataPtr;
      }

      /* --- restore terminated string --- */
      if (termPtr) *termPtr = termChar;
    }
    return dataPtr;
  }

  if (nBytesLeft >= recordSize) return dataPtr + recordSize;

  return NULL;
}

static int decodeRecord(CrgDataStruct* crgData, char* dataPtr, size_t nBytes) {
  size_t i;
  double value;
  float fValue;
  size_t nBytesLeft = nBytes;
  char tmpStr[32];
  size_t length;

  if (crgData->admin.dataFormat & dDataFormatASCII) {
    if (crgData->admin.dataFormat & dDataFormatPrecisionDouble)
      length = 20;
    else
      length = 10;
  } else if (crgData->admin.dataFormat & dDataFormatPrecisionDouble)
    length = 8;
  else
    length = 4;

  for (i = 0; i < crgData->noChannels; i++) {
    if (nBytesLeft < length) return 0;

    if (crgData->admin.dataFormat & dDataFormatASCII) {
      /* --- get rid of leading '\n' and ' ' characters --- */
      while ((*dataPtr == '\n' || *dataPtr == '\r') && nBytesLeft) {
        nBytesLeft--;
        dataPtr++;

        if (nBytesLeft < length) return 0;
      }

      strncpy(tmpStr, dataPtr, length);
      tmpStr[length] = '\0';

      /* check for missing values or NaNs */
      if (strspn(tmpStr, "0123456789+-.eEdD ") != strlen(tmpStr)) {
        if (!strcmp(tmpStr, "**unused**"))
          value = 0.0;
        else
          crgSetNan(&value);
      } else
        value = atof(tmpStr);
    } else if (crgData->admin.dataFormat & dDataFormatPrecisionDouble) {
      if (readDouble(dataPtr, &value) < 0) crgSetNan(&value);
    } else {
      if (readFloat(dataPtr, &fValue) < 0)
        crgSetNan(&value);
      else
        value = fValue;
    }

    memcpy(&(crgData->admin.recordBuffer[i]), &value, sizeof(value));

    dataPtr += length;
    nBytesLeft -= length;
  }
  return 1;
}

static int decodeNextRecord(CrgDataStruct* crgData, char** dataPtr, size_t* nBytesLeft) {
  char* recPtr = *dataPtr; /* pointer to begin of record */
  size_t nBytesRead = 0;

  if (!(recPtr = getNextRecord(crgData->admin.recordSize, crgData->admin.dataFormat, *dataPtr, *nBytesLeft))) return 0;

  nBytesRead = recPtr - *dataPtr;

  /* --- decode the record --- */
  if (!decodeRecord(crgData, *dataPtr, nBytesRead)) {
    crgMsgPrint(dCrgMsgLevelDebug, "decodeNextRecord: error parsing data record.\n");
    return 0;
  }

  /* --- go to the next frame --- */
  *dataPtr = recPtr;
  *nBytesLeft -= nBytesRead;

  /* this cannot happen */
  /*
  if ( *nBytesLeft < 0 )
  {
      crgMsgPrint( dCrgMsgLevelFatal, "decodeNextRecord: data access error.\n" );
      return 0;
  }
  */
  return 1;
}

static int parseCenterLine(CrgDataStruct* crgData, char* dataPtr, size_t nBytesLeft) {
  char* recPtr = dataPtr; /* pointer to begin of record */
  size_t srcBytesLeft = nBytesLeft;
  size_t nRec = 0;
  double uLast = 0.0;
  double duMin = 0.0;
  double duMax = 0.0;
  double du;
  double xLast = 0.0;
  double yLast = 0.0;
  double dx;
  double dy;
  double ds;
  double dsMin = 0.0;
  double dsMax = 0.0;
  double sLast = 0.0;

  /* --- parse through all records --- */
  while (decodeNextRecord(crgData, &recPtr, &srcBytesLeft)) {
    nRec++;

    if (crgData->channelU.info.defined) {
      if (nRec == 1)
        crgData->channelU.info.first = crgData->admin.recordBuffer[crgData->channelU.info.index];
      else {
        du = crgData->admin.recordBuffer[crgData->channelU.info.index] - uLast;
        if (nRec == 2 || du < duMin) duMin = du;
        if (nRec == 2 || du > duMax) duMax = du;
      }
      uLast = crgData->admin.recordBuffer[crgData->channelU.info.index];
    }

    if (crgData->channelX.info.defined) {
      if (nRec > 1) {
        dx = crgData->admin.recordBuffer[crgData->channelX.info.index] - xLast;
        dy = crgData->admin.recordBuffer[crgData->channelY.info.index] - yLast;
        ds = sqrt(dx * dx + dy * dy);

        if (nRec == 2 || ds < dsMin) dsMin = ds;
        if (nRec == 2 || ds > dsMax) dsMax = ds;

        sLast += ds;
      }
      xLast = crgData->admin.recordBuffer[crgData->channelX.info.index];
      yLast = crgData->admin.recordBuffer[crgData->channelY.info.index];
    }

    /*
    if ( nBytesLeft )
        crgMsgPrint( dCrgMsgLevelNotice, "MMM: parseCenterLine: read %.3f %% of data\n", ( 100.0f * ( nBytesLeft -
    srcBytesLeft ) ) / nBytesLeft );
    */
  }

  /* --- check (x, y) channel consistency --- */
  if (crgData->channelX.info.defined) {
    /* --- remember the channel size --- */
    crgData->channelX.info.size = nRec;
    crgData->channelY.info.size = nRec;

    /* --- minimum s spacing must be larger than 1.e-6m --- */
    if (dsMin < 1.0e-6) {
      crgMsgPrint(dCrgMsgLevelFatal, "parseCenterLine: reference line s spacing too small.\n");
      return 0;
    }

    /* --- relative spacing tolerance must be smaller than 3.e-2 --- */
    if ((dsMax - dsMin) / dsMin > 3.e-2) {
      crgMsgPrint(dCrgMsgLevelFatal, "parseCenterLine: non-constant reference line s spacing.\n");
      return 0;
    }

    /* --- set u spacing --- */
    crgData->channelU.info.inc = sLast / (crgData->channelX.info.size - 1);
    crgData->channelU.info.last = sLast;
    crgData->channelU.info.size = crgData->channelX.info.size;
  }
  /* --- check u channel consistency --- */
  else if (crgData->channelU.info.defined) {
    /* --- remember the channel size --- */
    crgData->channelU.info.size = nRec;

    /* --- minimum u spacing must be larger than 1.e-6m --- */
    if (duMin < 1.0e-6) {
      crgMsgPrint(dCrgMsgLevelFatal, "parseCenterLine: reference line u spacing too small.\n");
      return 0;
    }

    /* --- relative spacing tolerance must be smaller than 3.e-2 --- */
    if ((duMax - duMin) / duMin > 3.e-2) {
      crgMsgPrint(dCrgMsgLevelFatal, "parseCenterLine: non-constant reference line u spacing.\n");
      return 0;
    }

    /* --- calculate u increment --- */
    crgData->channelU.info.last = uLast;
    crgData->channelU.info.inc = (crgData->channelU.info.last - crgData->channelU.info.first) / duMax;
  } else {
    /* --- no explicit u or x/y data given, so calculate from increment data --- */
    crgData->channelU.info.size = nRec;
    crgData->channelU.info.last =
        crgData->channelU.info.first + crgData->channelU.info.inc * (crgData->channelU.info.size - 1);
  }

  /* --- if phi is defined then resulting data must be stored in x/y fields --- */
  if (crgData->channelPhi.info.defined) crgData->channelPhi.info.size = nRec;

  /* --- remember the channel size --- */
  crgData->channelX.info.size = nRec;
  crgData->channelY.info.size = nRec;

  return 1;
}

static int allocateChannels(CrgDataStruct* crgData) {
  size_t i;

  if (!crgData) return 0;

  crgMsgPrint(dCrgMsgLevelDebug, "allocateChannels: crgData->channelU.info.size = %ld\n", crgData->channelU.info.size);

  /* --- the z channels --- */
  for (i = 0; i < crgData->channelV.info.size; i++) {
    /* copy size information */
    crgData->channelZ[i].info.size = crgData->channelU.info.size;

    if (!(crgData->channelZ[i].data = (float*)crgCalloc(crgData->channelZ[i].info.size, sizeof(float)))) return 0;
  }

  /* --- print some debug information --- */
  for (i = 0; i < crgData->channelV.info.size; i++)
    crgMsgPrint(dCrgMsgLevelDebug, "allocateChannels: channelZ[%ld].info.index = %ld, channelZ[%ld].info.size = %ld\n",
                i, crgData->channelZ[i].info.index, i, crgData->channelZ[i].info.size);

  /* --- and the other ones --- */
  if (crgData->channelX.info.size) {
    if (!(crgData->channelX.data = (double*)crgCalloc(crgData->channelX.info.size, sizeof(double)))) return 0;

    if (!(crgData->channelY.data = (double*)crgCalloc(crgData->channelY.info.size, sizeof(double)))) return 0;
  }

  if (crgData->channelPhi.info.valid) {
    if (!(crgData->channelPhi.data = (double*)crgCalloc(crgData->channelPhi.info.size, sizeof(double)))) return 0;
  }

  if (crgData->channelBank.info.valid) {
    /* copy size information */
    crgData->channelBank.info.size = crgData->channelU.info.size;
    crgData->util.hasBank = 1;

    if (!(crgData->channelBank.data = (double*)crgCalloc(crgData->channelBank.info.size, sizeof(double)))) return 0;
  }

  if (crgData->channelSlope.info.valid) {
    /* copy size information */
    crgData->channelSlope.info.size = crgData->channelU.info.size;

    if (!(crgData->channelSlope.data = (double*)crgCalloc(crgData->channelSlope.info.size, sizeof(double)))) return 0;
  }

  /* reference line z needs to be allocated if either an explicit slope channel is given or if
   *  slope at start of reference line is not zero
   */
  if (crgData->channelSlope.info.valid || (crgData->channelSlope.info.first != 0.0)) {
    crgData->channelRefZ.info.size = crgData->channelU.info.size;

    if (!(crgData->channelRefZ.data = (double*)crgCalloc(crgData->channelRefZ.info.size, sizeof(double)))) return 0;
  }

  return 1;
}

static void readData(CrgDataStruct* crgData) {
  char* recPtr = crgData->admin.dataSection; /* pointer to begin of record */
  size_t srcBytesLeft = crgData->admin.dataSize;
  size_t i;
  size_t nRec = 0;

  /* --- parse through all records --- */
  while (decodeNextRecord(crgData, &recPtr, &srcBytesLeft)) {
    /* crgMsgPrint( dCrgMsgLevelNotice, "readData: channelZ at cross section no. %ld\n", nRec ); */
    for (i = 0; i < crgData->channelV.info.size; i++) {
      if (crgIsNan(&(crgData->admin.recordBuffer[crgData->channelZ[i].info.index])))
        crgSetNanf(&(crgData->channelZ[i].data[nRec]));
      else
        crgData->channelZ[i].data[nRec] = (float)crgData->admin.recordBuffer[crgData->channelZ[i].info.index];
    }
    /* crgMsgPrint( dCrgMsgLevelNotice, "readData: channelZ at cross section no. %ld finished.\n", nRec ); */

    if (crgData->channelX.info.defined) {
      crgData->channelX.data[nRec] = crgData->admin.recordBuffer[crgData->channelX.info.index];
      crgData->channelY.data[nRec] = crgData->admin.recordBuffer[crgData->channelY.info.index];
    }

    if (crgData->channelPhi.info.defined) {
      if (!nRec)
        crgData->channelPhi.data[nRec] = crgData->channelPhi.info.first;
      else
        crgData->channelPhi.data[nRec] = crgData->admin.recordBuffer[crgData->channelPhi.info.index];
      crgMsgPrint(dCrgMsgLevelDebug, "readData: channelPhi.data[%ld] = %.3f\n", nRec, crgData->channelPhi.data[nRec]);
    }

    if (crgData->channelBank.info.defined)
      crgData->channelBank.data[nRec] = crgData->admin.recordBuffer[crgData->channelBank.info.index];

    if (crgData->channelSlope.info.defined)
      crgData->channelSlope.data[nRec] = crgData->admin.recordBuffer[crgData->channelSlope.info.index];

    nRec++;

    /*
    if ( crgData->admin.dataSize )
        crgMsgPrint( dCrgMsgLevelNotice, "MMM: readData: read %.2f %% of data\n", ( 100.0f * ( crgData->admin.dataSize -
    srcBytesLeft ) ) / crgData->admin.dataSize );
    */
  }

  /* --- ok, file data copy is no longer needed, get rid of it --- */
  if (crgData->admin.fileBuffer) free(crgData->admin.fileBuffer);

  crgData->admin.fileBuffer = NULL;
}

void crgLoaderHandleNaNs(CrgDataStruct* crgData, int mode, double offset) {
  int totalNaN = 0;
  size_t minIndexLR = crgData->channelV.info.size;
  size_t maxIndexRL = 0;
  size_t i;
  size_t v;
  int offsetApplied = 0;

  /* --- at least, NaNs need to be counted, so don't exit --- */
  /* --- even if mode is dCrgGridNaNKeep                  --- */

  for (i = 0; i < crgData->channelU.info.size; i++) {
    size_t nan = 0;
    size_t indexLR = 0;
    size_t indexRL = crgData->channelV.info.size - 1;

    /* --- right to left --- */
    for (v = 1; v < crgData->channelV.info.size; v++) {
      if (crgIsNanf(&(crgData->channelZ[v].data[i]))) {
        nan++;

        switch (mode) {
          case dCrgGridNaNSetZero:
            crgData->channelZ[v].data[i] = (float)offset;
            break;

          case dCrgGridNaNKeepLast:
            /* --- copy data from right neighbor --- */
            memcpy(&(crgData->channelZ[v].data[i]), &(crgData->channelZ[v - 1].data[i]),
                   sizeof(crgData->channelZ[v].data[0]));
            if (!crgIsNanf(&(crgData->channelZ[v].data[i])) && !offsetApplied) {
              crgData->channelZ[v].data[i] += (float)offset;
              offsetApplied = 1;
            }
            break;

          default:
            break;
        }
      } else
        indexLR = v;
    }

    /* --- left to right --- */
    offsetApplied = 0;
    for (v = crgData->channelV.info.size - 1; v > 0; v--) {
      if (crgIsNanf(&(crgData->channelZ[v - 1].data[i])) && !crgIsNanf(&(crgData->channelZ[v].data[i]))) {
        nan++;

        switch (mode) {
          case dCrgGridNaNSetZero:
            crgData->channelZ[v - 1].data[i] = (float)offset;
            break;

          case dCrgGridNaNKeepLast:
            /* --- copy data from right neighbor --- */
            memcpy(&(crgData->channelZ[v - 1].data[i]), &(crgData->channelZ[v].data[i]),
                   sizeof(crgData->channelZ[v - 1].data[0]));

            if (!crgIsNanf(&(crgData->channelZ[v - 1].data[i])) && !offsetApplied) {
              crgData->channelZ[v - 1].data[i] += (float)offset;
              offsetApplied = 1;
            }
            break;

          default:
            break;
        }
      } else
        indexRL = v - 1;
    }

    if (nan > 0) {
      crgMsgPrint(dCrgMsgLevelInfo,
                  "crgLoaderHandleNaNs: cross section %ld: NaNs total: %4ld, left: %4ld, right %4ld\n", i, nan,
                  crgData->channelV.info.size - indexLR, indexRL);
    }

    totalNaN += nan;

    if (indexRL > maxIndexRL) maxIndexRL = indexRL;

    if (indexLR < minIndexLR) minIndexLR = indexLR;
  }

  if (!totalNaN) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgLoaderHandleNaNs: no NaNs found.\n");
    return;
  }

  crgMsgPrint(dCrgMsgLevelNotice, "crgLoaderHandleNaNs: Summary of NaN handling information:\n");
  crgMsgPrint(dCrgMsgLevelNotice, "                     NaNs in crg data replaced by constant extrapolation.\n");
  crgMsgPrint(dCrgMsgLevelNotice, "                     total NaNs in data [-]:        %ld\n", totalNaN);
  crgMsgPrint(dCrgMsgLevelNotice, "                     max. NaN count from left [-]:  %ld\n",
              crgData->channelV.info.size - minIndexLR);
  crgMsgPrint(dCrgMsgLevelNotice, "                     max. NaN count from right [-]: %ld\n", maxIndexRL);
}

static void calcRefLine(CrgDataStruct* crgData) {
  double x = crgData->channelX.info.first;
  double y = crgData->channelY.info.first;
  double dx;
  double dy;
  size_t i = 0u;

  /* --- calculate x/y only if phi is given --- */
  if (!crgData->channelPhi.info.defined) {
    /* --- we have a straight line --- */
    for (i = 0; i < crgData->channelU.info.size; i++) {
      crgData->channelX.data[i] = x + i * crgData->channelU.info.inc * cos(crgData->channelPhi.info.first);
      crgData->channelY.data[i] = y + i * crgData->channelU.info.inc * sin(crgData->channelPhi.info.first);
      crgData->channelX.info.valid = 1;
      crgData->channelY.info.valid = 1;
    }

    /* --- check for data consistency --- */
    if ((crgData->admin.defMask & dCrgDataDefXEnd) && (crgData->admin.defMask & dCrgDataDefYEnd)) {
      dx = crgData->channelX.info.last - crgData->channelX.data[crgData->channelX.info.size - 1];
      dy = crgData->channelY.info.last - crgData->channelY.data[crgData->channelY.info.size - 1];

      if (fabs(dx) > 5.e-4 || fabs(dy) > 5.e-4) {
        crgMsgPrint(dCrgMsgLevelWarn, "calcRefLine: Reference line definition error at end:\n");
        crgMsgPrint(dCrgMsgLevelWarn, "             x[m] = %10.4f\n", dx);
        crgMsgPrint(dCrgMsgLevelWarn, "             y[m] = %10.4f\n", dy);
      }
    } else {
      crgMsgPrint(dCrgMsgLevelWarn, "calcRefLine: xInfoSize = %ld, yInfoSize = %ld\n", crgData->channelX.info.size,
                  crgData->channelY.info.size);
      crgData->channelX.info.last = crgData->channelX.data[crgData->channelX.info.size - 1];
      crgData->channelY.info.last = crgData->channelY.data[crgData->channelY.info.size - 1];
    }
    return;
  }

  crgMsgPrint(dCrgMsgLevelNotice, "calcRefLine: Reference line will be generated by integration of phi values.\n");

  if ((crgData->admin.defMask & dCrgDataDefXEnd) && (crgData->admin.defMask & dCrgDataDefYEnd)) {
    crgMsgPrint(dCrgMsgLevelDebug, "calcRefLine: endpoint x/y = %.6f / %.6f\n", crgData->channelX.info.last,
                crgData->channelY.info.last);

    /* --- perform forward / backward integration, first backward --- */
    crgData->channelX.data[crgData->channelX.info.size - 1] = crgData->channelX.info.last;
    crgData->channelY.data[crgData->channelY.info.size - 1] = crgData->channelY.info.last;

    for (i = crgData->channelPhi.info.size - 1lu; i > 0lu; --i) {
      crgData->channelX.data[i - 1lu] =
          crgData->channelX.data[i] - crgData->channelU.info.inc * cos(crgData->channelPhi.data[i]);
      crgData->channelY.data[i - 1lu] =
          crgData->channelY.data[i] - crgData->channelU.info.inc * sin(crgData->channelPhi.data[i]);
    }

    dx = crgData->channelX.info.first - crgData->channelX.data[0];
    dy = crgData->channelY.info.first - crgData->channelY.data[0];

    if (sqrt(dx * dx + dy * dy) > dCrgEps * (crgData->channelU.info.last - crgData->channelU.info.first)) {
      crgMsgPrint(dCrgMsgLevelWarn, "calcRefLine: Reference line integration error after backward integration:\n");
      crgMsgPrint(dCrgMsgLevelWarn, "             x[m] = %10.4f\n", dx);
      crgMsgPrint(dCrgMsgLevelWarn, "             y[m] = %10.4f\n", dy);
    }

    crgData->channelX.data[0] = crgData->channelX.info.first;
    crgData->channelY.data[0] = crgData->channelY.info.first;

    /* --- now do weighted forward integration --- */
    for (i = 0; i < crgData->channelPhi.info.size - 1; i++) {
      /* integrate with constant slope or with explicit values of slope channel */
      double fraction = (1.0 * (i + 1)) / (1.0 * (crgData->channelPhi.info.size - 1));

      crgData->channelX.data[i + 1] =
          (1.0 - fraction) *
              (crgData->channelX.data[i] + crgData->channelU.info.inc * cos(crgData->channelPhi.data[i + 1])) +
          fraction * crgData->channelX.data[i + 1];
      crgData->channelY.data[i + 1] =
          (1.0 - fraction) *
              (crgData->channelY.data[i] + crgData->channelU.info.inc * sin(crgData->channelPhi.data[i + 1])) +
          fraction * crgData->channelY.data[i + 1];
    }

    /*
    crgMsgPrint( dCrgMsgLevelNotice, "calcRefLine: resulting reference line:\n" );
    for ( i = 0; i < crgData->channelPhi.info.size; i++ )
        crgMsgPrint( dCrgMsgLevelNotice, "calcRefLine: phi[%d] = %.6f, x[%d] = %.6f / y[%d] = %.6f\n",
                                         i, crgData->channelPhi.data[i], i, crgData->channelX.data[i], i,
    crgData->channelY.data[i] );
    */
  } else {
    crgMsgPrint(dCrgMsgLevelDebug, "calcRefLine: using simplified (forward) algorithm.\n");

    /* --- integrate phi -> (x,y) from start to end by simple Euler steps, using simple forward integration --- */
    for (i = 0; i < crgData->channelPhi.info.size - 1; i++) {
      crgData->channelX.data[i + 1] =
          crgData->channelX.data[i] + crgData->channelU.info.inc * cos(crgData->channelPhi.data[i + 1]);
      crgData->channelY.data[i + 1] =
          crgData->channelY.data[i] + crgData->channelU.info.inc * sin(crgData->channelPhi.data[i + 1]);
    }

    crgData->channelX.info.last = crgData->channelX.data[crgData->channelX.info.size - 1];
    crgData->channelY.info.last = crgData->channelY.data[crgData->channelY.info.size - 1];
  }

  /* --- check for contradictory x and y channel definition --- */
  if (crgData->channelX.info.defined) /* @todo: evaluate later-on */
    crgMsgPrint(dCrgMsgLevelWarn, "calcRefLine: Reference line x and y channels not checked/used.\n");

  /* --- x and y channels are valid after this operation --- */
  crgData->channelX.info.valid = 1;
  crgData->channelY.info.valid = 1;
}

static void calcRefLineZ(CrgDataStruct* crgData) {
  size_t i;
  double slope;
  double zError;
  double fraction;

  /* --- calculate z only if slope channel or constant start slope is given --- */
  if ((crgData->channelSlope.info.first == 0.0) && !crgData->channelSlope.info.defined) return;

  crgMsgPrint(dCrgMsgLevelNotice, "calcRefLineZ: Reference line z will be generated by integration of slope values.\n");

  /* -- perform backward/forward integration if reference line z at end is given --- */
  if ((crgData->admin.defMask & dCrgDataDefZEnd)) {
    /* --- integrate slope -> z from end to start by backward integration --- */
    crgData->channelRefZ.data[crgData->channelRefZ.info.size - 1] = crgData->channelRefZ.info.last;

    for (i = crgData->channelRefZ.info.size - 1; i > 0; i--) {
      /* integrate with constant slope or with explicit values of slope channel */
      slope = (crgData->channelSlope.info.defined) ? crgData->channelSlope.data[i] : crgData->channelSlope.info.first;

      crgData->channelRefZ.data[i - 1] = crgData->channelRefZ.data[i] - slope * crgData->channelU.info.inc;
    }

    /* --- check for z error at the start of the reference line --- */
    zError = fabs(crgData->channelRefZ.data[0] - crgData->channelRefZ.info.first);

    if (zError > (crgData->channelU.info.last - crgData->channelU.info.first) * dCrgEps)
      crgMsgPrint(dCrgMsgLevelWarn,
                  "calcRefLineZ: inconsistent z on reference line after slope backward integration, error = %.5f\n",
                  zError);

    /* --- go forward with weighted interpolation --- */
    crgData->channelRefZ.data[0] = crgData->channelRefZ.info.first;

    for (i = 0; i < crgData->channelRefZ.info.size - 2; i++) {
      /* integrate with constant slope or with explicit values of slope channel */
      slope =
          (crgData->channelSlope.info.defined) ? crgData->channelSlope.data[i + 1] : crgData->channelSlope.info.first;
      fraction = (1.0 * i) / (1.0 * (crgData->channelRefZ.info.size - 1));

      crgData->channelRefZ.data[i + 1] =
          (1.0 - fraction) * (crgData->channelRefZ.data[i] + slope * crgData->channelU.info.inc) +
          fraction * crgData->channelRefZ.data[i + 1];
    }

    /*
    crgMsgPrint( dCrgMsgLevelNotice, "calcRefLineZ: resulting reference line:\n" );
    for ( i = 0; i < crgData->channelRefZ.info.size; i++ )
        crgMsgPrint( dCrgMsgLevelNotice, "calcRefLineZ: z[%d] = %.6f, slope = %.6f\n", i, crgData->channelRefZ.data[i],
    crgData->channelSlope.data[i] );
    */
  } else {
    /* --- integrate slope -> z from start to end by forward integration --- */
    crgData->channelRefZ.data[0] = crgData->channelRefZ.info.first;

    for (i = 0; i < crgData->channelRefZ.info.size - 1; i++) {
      /* integrate with constant slope or with explicit values of slope channel */
      slope = crgData->channelSlope.info.defined ? crgData->channelSlope.data[i + 1] : crgData->channelSlope.info.first;

      crgData->channelRefZ.data[i + 1] = crgData->channelRefZ.data[i] + slope * crgData->channelU.info.inc;

      crgMsgPrint(dCrgMsgLevelDebug, "calcRefLineZ: crgData->channelRefZ.data[%d] = %.5f\n", i,
                  crgData->channelRefZ.data[i]);
    }
    /* --- remember last value --- */
    crgData->channelRefZ.info.last = crgData->channelRefZ.data[crgData->channelRefZ.info.size - 1];
  }

  /* --- do not delete slope data since it may be modified by modifier options --- */

  /* --- reference line z channel is valid after this operation --- */
  crgData->channelRefZ.info.valid = 1;
}

static void normalizeRefLine(CrgDataStruct* crgData) {
  double xOff;
  double yOff;
  double hdg;
  double cHdg;
  double sHdg;
  size_t i;

  /** @todo: disabled on May 1, 2009 */
  return;

  /* start u-v-system at (0,0) */
  crgData->channelU.info.last = crgData->channelU.info.last - crgData->channelU.info.first;
  crgData->channelU.info.first = 0.0;

  /* start x/y/heading at (0,0,0) */
  if (!crgData->channelX.info.size) return;

  xOff = crgData->channelX.data[0];
  yOff = crgData->channelY.data[0];

  for (i = 0; i < crgData->channelX.info.size; i++) {
    crgData->channelX.data[i] -= xOff;
    crgData->channelY.data[i] -= yOff;
  }

  /* correct for the heading angle at the beginning */
  hdg = atan2(crgData->channelY.data[1] - crgData->channelY.data[0],
              crgData->channelX.data[1] - crgData->channelX.data[0]);

  cHdg = cos(hdg);
  sHdg = sin(hdg);

  for (i = 0; i < crgData->channelX.info.size; i++) {
    double x = crgData->channelX.data[i];
    double y = crgData->channelY.data[i];

    crgData->channelX.data[i] = x * cHdg + y * sHdg; /* @todo: check */
    crgData->channelY.data[i] = -x * sHdg + y * cHdg;
  }
}

static void normalizeZ(CrgDataStruct* crgData) {
  double zMean = 0.0;
  size_t i, j;
  size_t nValues = 0;

  /* make mean elevation at first cross section = 0.0 */
  /* note: prepare may be called multiple times, so take old mean value into account */
  /* only use values not being NaNs! */
  for (i = 0; i < crgData->channelV.info.size; i++)
    if (!crgIsNanf(&(crgData->channelZ[i].data[0]))) {
      zMean += crgData->channelZ[i].data[0] + crgData->channelZ[i].info.mean;
      nValues++;
    }

  if (nValues) zMean /= nValues;

  zMean = 0.0; /** @todo: continue here */

  /* @todo: check size of all channels! */
  for (i = 0; i < crgData->channelV.info.size; i++) {
    for (j = 0; j < crgData->channelZ[i].info.size; j++)
      if (!crgIsNanf(&(crgData->channelZ[i].data[j])))
        crgData->channelZ[i].data[j] = crgData->channelZ[i].data[j] + (float)(crgData->channelZ[i].info.mean - zMean);
    crgData->channelZ[i].info.mean = zMean;
  }
}

static void smoothenRefLine(CrgDataStruct* crgData) {
  size_t nU;
  size_t i;
  double val;
  double hdg;
  double xEnd;
  double yEnd;
  double x;
  double y;
  double frac;
  CrgChannelStruct tmpChannel;

  if (!crgData->channelX.info.valid || !crgData->channelX.info.defined) return;

  /* --- no smoothing required if centerline is defined by angles --- */
  /*
  if ( crgData->channelPhi.info.valid )
       return;
  */

  /* --- allocate space for curvature data --- */
  tmpChannel.info.size = crgData->channelPhi.info.size;
  if (!(tmpChannel.data = (double*)crgCalloc(tmpChannel.info.size, sizeof(double)))) {
    crgMsgPrint(dCrgMsgLevelFatal, "calcCurvature: could not allocate space for curvature data.\n");
    return;
  }

  /* ok, we may use this data */
  tmpChannel.info.valid = 1;

  /* --- get number of u intervals at least 0.5m long --- */

  nU = (size_t)(0.5 / crgData->channelU.info.inc);

  if (nU < 1) nU = 1;

  /*  calculate curvature by using cross product of two
   *  consecutive road sections defined by three points
   *  P0: iu0 = iu - nu: (X0, Y0)
   *  P1: iu1 = iu     : (X1, Y1)
   *  P2: iu2 = iu + nu: (X2, Y2)
   *  curv = dphi/ds = (P1-P0)x(P2-P1) / |P1-P0|**3
   */
  val = 1.0 / pow(crgData->channelU.info.inc * nU, 3.0);

  for (i = nU; i < tmpChannel.info.size - nU; i++) {
    double dx0 = crgData->channelX.data[i] - crgData->channelX.data[i - nU];
    double dx1 = crgData->channelX.data[i + nU] - crgData->channelX.data[i];
    double dy0 = crgData->channelY.data[i] - crgData->channelY.data[i - nU];
    double dy1 = crgData->channelY.data[i + nU] - crgData->channelY.data[i];

    tmpChannel.data[i] = (dx0 * dy1 - dy0 * dx1) * val;
  }

  /* extrapolate curvature at start and end linearly to zero */
  for (i = 0; i < nU - 1; i++) {
    tmpChannel.data[i] = i * tmpChannel.data[nU] / nU;
    tmpChannel.data[tmpChannel.info.size - i - 1] = i * tmpChannel.data[tmpChannel.info.size - nU - 1] / nU;
  }

  /* integrate: curv -> heading, store at same memory location */
  hdg = 0.0;

  /** @todo: why is first heading altered??? shift by 1? **/

  for (i = 0; i < tmpChannel.info.size; i++) {
    hdg += tmpChannel.data[i] * crgData->channelU.info.inc;
    tmpChannel.data[i] = hdg;
  }

  /* save old value */
  xEnd = crgData->channelX.data[crgData->channelX.info.size - 1];
  yEnd = crgData->channelY.data[crgData->channelY.info.size - 1];

  /* integrate: heading -> position from start to end */
  x = 0.0;
  y = 0.0;

  for (i = 0; i < tmpChannel.info.size; i++) {
    hdg = tmpChannel.data[i];
    crgData->channelX.data[i] = x;
    crgData->channelX.data[i] = y;

    x += crgData->channelU.info.inc * cos(hdg);
    y += crgData->channelU.info.inc * sin(hdg);
  }

  /* integrate: heading -> position from saved end to start
   *  do weightened interpolation of both integration runs */
  x = xEnd;
  y = yEnd;

  for (i = tmpChannel.info.size - 1; i <= 0; i--) {
    hdg = tmpChannel.data[i];
    frac = (1.0 * i) / (1.0 * (tmpChannel.info.size - 1));
    crgData->channelX.data[i] = (1.0 - frac) * crgData->channelX.data[i] + frac * x;
    crgData->channelY.data[i] = (1.0 - frac) * crgData->channelY.data[i] + frac * y;
    x -= crgData->channelU.info.inc * cos(hdg);
    y -= crgData->channelU.info.inc * sin(hdg);
  }

  /* --- free temporary memory --- */
  crgFree(tmpChannel.data);
}

void crgLoaderPrepareData(CrgDataStruct* crgData) {
  /* --- first: eliminate NaNs --- */
  /** @note: removed: NaNs should be handled by applying modifiers explicitly!
  crgLoaderHandleNaNs( crgData, dCrgGridNaNKeepLast, 0.0 );
  crgMsgPrint( dCrgMsgLevelDebug, "crgLoaderPrepareData: handleNaNs() done.\n" );
  */

  /* --- calculate the reference line --- */
  calcRefLine(crgData);
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderPrepareData: calcRefLine() done.\n");

  /* --- calculate elevation profile of the reference line --- */
  calcRefLineZ(crgData);
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderPrepareData: calcRefLineZ() done.\n");

  /* --- calculate statistics --- */
  /* crgPrintElevData( crgData ); */
  crgCalcStatistics(crgData);
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderPrepareData: crgCalcStatistics() done.\n");

  /* --- normalize reference line data --- */
  normalizeRefLine(crgData);
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderPrepareData: normalizeRefLine() done.\n");

  /* --- normalize z data --- */
  normalizeZ(crgData);
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderPrepareData: normalizeZ() done.\n");

  /* --- smoothen the reference line --- */
  smoothenRefLine(crgData);
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderPrepareData: smoothenRefLine() done.\n");

  /* --- prepare some data for higher performance of evaluations --- */
  crgCalcUtilityData(crgData);
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderPrepareData: crgCalcUtilityData() done.\n");
}

int crgCheck(int dataSetId) {
  CrgDataStruct* crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgCheck: invalid data set id <%d>.\n", dataSetId);
    return 0;
  }

  /* @todo: move to one of the following sub check routines */
  /* --- check if closed refline option is valid --- */
  if (crgData->util.uIsClosed && crgOptionHasValueInt(&(crgData->options), dCrgRefLineCloseTrack, 1)) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgCheckData: refline cannot be closed.\n");
    return 0;
  }

  if (!crgCheckOpts(crgData)) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: failed to validate option settings.\n");
    return 0;
  }

  if (!crgCheckMods(crgData)) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: failed to validate modifier settings.\n");
    return 0;
  }

  return 1;
}

int crgCheckOpts(CrgDataStruct* crgData) {
  int optAsInt;
  double optAsDouble, optAsDoubleTemp;
  double midinc, mininc;
  double ceps, cinc, ctol;

  /* --- check singular value ranges --- */

  /* CRG elevation grid border modes in u and v directions */
  if (crgOptionGetInt(&crgData->options, dCrgCpOptionBorderModeU, &optAsInt)) {
    if (optAsInt < 0 || optAsInt > 4) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"border_mode_u\": %d\n", optAsInt);
      return 0;
    }
  }

  if (crgOptionGetInt(&crgData->options, dCrgCpOptionBorderModeV, &optAsInt)) {
    if (optAsInt < 0 || optAsInt > 4) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"border_mode_v\": %d\n", optAsInt);
      return 0;
    }
  }

  if (crgOptionGetDouble(&crgData->options, dCrgCpOptionSmoothUBegin, &optAsDouble)) {
    if (optAsDouble < 0) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"border_smooth_ubeg\": %f\n", optAsDouble);
      return 0;
    }
  }

  if (crgOptionGetDouble(&crgData->options, dCrgCpOptionSmoothUEnd, &optAsDouble)) {
    if (optAsDouble < 0) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"border_smooth_uend\": %f\n", optAsDouble);
      return 0;
    }
  }

  if (crgOptionGetInt(&crgData->options, dCrgCpOptionRefLineContinue, &optAsInt)) {
    if (optAsInt < 0 || optAsInt > 1) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"refline_continuation\": %d\n", optAsInt);
      return 0;
    }
  }

  /* CRG reference line search strategy */
  optAsDoubleTemp = 0.;
  if (crgOptionGetDouble(&crgData->options, dCrgCpOptionRefLineFar, &optAsDoubleTemp)) {
    if (optAsDoubleTemp < 0) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"refline_search_far\": %f\n", optAsDoubleTemp);
      return 0;
    }
  }

  if (crgOptionGetDouble(&crgData->options, dCrgCpOptionRefLineClose, &optAsDouble)) {
    if (optAsDouble < 0 ||
        (crgOptionIsSet(&crgData->options, dCrgCpOptionRefLineFar) && optAsDouble >= optAsDoubleTemp)) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"refline_search_close\": %f\n", optAsDouble);
      return 0;
    }
  }

  /* CRG message options */
  if (crgOptionGetInt(&crgData->options, dCrgCpOptionWarnMsgs, &optAsInt)) {
    if (optAsInt < -1) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"warn_msgs\": %d", optAsInt);
      return 0;
    }
  }

  /* not implemented: warn_curv_local  */
  /* not implemented: warn_curv_global */
  /* not implemented: log_msgs         */
  /* not implemented: log_eval         */
  /* not implemented: log_eval_freq    */
  /* not implemented: log_hist         */
  /* not implemented: log_hist_freq    */
  /* not implemented: log_stat         */
  /* not implemented: log_stat_freq    */

  /* CRG check options */
  if (crgOptionGetDouble(&crgData->options, dCrgCpOptionCheckEps, &ceps)) {
    if (ceps < 1.0e-6 || ceps > 1.0e-2) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"check_eps\": %f\n", ceps);
      return 0;
    }
  } else {
    crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: missing default option data \"check_eps\": %f\n", ceps);
    return 0;
  }

  midinc = 0.001;
  mininc = midinc * (1 - ceps); /* dCRGCpOptionCheckEps always exists -> is default option */
  if (crgOptionGetDouble(&crgData->options, dCrgCpOptionCheckInc, &cinc)) {
    if (cinc < mininc ||
        fabs(((int)(cinc / midinc + 0.5)) * midinc - cinc) > ceps * ((midinc > cinc) ? midinc : cinc)) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"check_inc\": %f\n", cinc);
      return 0;
    }
  } else {
    crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: missing default option data \"check_inc\": %f\n", ceps);
    return 0;
  }

  if (crgOptionGetDouble(&crgData->options, dCrgCpOptionCheckTol, &ctol)) {
    if (ctol < ceps * cinc || ctol > 0.5 * cinc) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal option data \"check_tol\": %d", ctol);
      return 0;
    }
  } else {
    crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: missing default option data \"check_tol\": %f\n", ceps);
    return 0;
  }

  return 1;
}

int crgCheckMods(CrgDataStruct* crgData) {
  int modAsInt = 0;
  int byoff = 0;
  int byref = 0;
  double modAsDouble;

  /* --- check singular value ranges */

  /* CRG scaling */
  if (crgOptionGetDouble(&crgData->modifiers, dCrgModScaleLength, &modAsDouble)) {
    if (modAsDouble <= 0) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckMods: illegal modifier \"scale_length\": %f\n", modAsDouble);
      return 0;
    }
  }

  if (crgOptionGetDouble(&crgData->modifiers, dCrgModScaleWidth, &modAsDouble)) {
    if (modAsDouble <= 0) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckMods: illegal modifier \"scale_width\": %f\n", modAsDouble);
      return 0;
    }
  }

  /* CRG elevation grid NaN handling */
  if (crgOptionGetInt(&crgData->modifiers, dCrgModGridNaNMode, &modAsInt)) {
    if (modAsInt < 0 || modAsInt > 2) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckOpts: illegal modifier \"grid_nan_mode\": %d", modAsInt);
      return 0;
    }
  }

  if (crgOptionGetDouble(&crgData->modifiers, dCrgModGridNaNOffset, &modAsDouble)) {
    if (!crgOptionGetInt(&crgData->modifiers, dCrgModGridNaNMode, &modAsInt)) {
      crgOptionSetInt(&crgData->modifiers, dCrgModGridNaNMode, dCrgGridNaNKeepLast); /* default */
    }
    if (modAsInt == dCrgGridNaNKeep) /* check for useless offset setting */
    {
      crgMsgPrint(dCrgMsgLevelFatal, "crgCheckMods: inconsistent grid_nan_mod=%d with grid_nan_offset=%f\n", modAsInt,
                  modAsDouble);
      return 0;
    }
  }

  /* CRG re-positioning: refline by offset (default: "by refpoint") */
  byoff |= crgOptionIsSet(&crgData->modifiers, dCrgModRefLineOffsetPhi);
  byoff |= crgOptionIsSet(&crgData->modifiers, dCrgModRefLineOffsetX);
  byoff |= crgOptionIsSet(&crgData->modifiers, dCrgModRefLineOffsetY);
  byoff |= crgOptionIsSet(&crgData->modifiers, dCrgModRefLineOffsetZ);

  if (byoff) {
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefLineOffsetPhi))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefLineOffsetPhi, 0.);
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefLineOffsetX))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefLineOffsetX, 0.);
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefLineOffsetY))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefLineOffsetY, 0.);
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefLineOffsetZ))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefLineOffsetZ, 0.);
  }

  /* CRG re-positioning: refline by refpoint (overwrites "by offset") */
  byref |= crgOptionIsSet(&crgData->modifiers, dCrgModRefPointUFrac);
  byref |= crgOptionIsSet(&crgData->modifiers, dCrgModRefPointU);
  byref |= crgOptionIsSet(&crgData->modifiers, dCrgModRefPointVFrac);
  byref |= crgOptionIsSet(&crgData->modifiers, dCrgModRefPointV);
  byref |= crgOptionIsSet(&crgData->modifiers, dCrgModRefPointX);
  byref |= crgOptionIsSet(&crgData->modifiers, dCrgModRefPointY);
  byref |= crgOptionIsSet(&crgData->modifiers, dCrgModRefPointZ);
  byref |= crgOptionIsSet(&crgData->modifiers, dCrgModRefPointPhi);

  if (byref && byoff) {
    crgOptionRemove(&crgData->modifiers, dCrgModRefLineOffsetPhi);
    crgOptionRemove(&crgData->modifiers, dCrgModRefLineOffsetX);
    crgOptionRemove(&crgData->modifiers, dCrgModRefLineOffsetY);
    crgOptionRemove(&crgData->modifiers, dCrgModRefLineOffsetZ);
    crgMsgPrint(
        dCrgMsgLevelFatal,
        "crgCheckMods: CRG re-positioning modifiers: refline \"by refpoint\" overwrites \"by offset\" setting\n");
    return 0;
  }

  if (byref) {
    if (crgOptionIsSet(&crgData->modifiers, dCrgModRefPointUFrac) &&
        crgOptionIsSet(&crgData->modifiers, dCrgModRefPointU)) {
      crgMsgPrint(dCrgMsgLevelFatal,
                  "crgCheckMods: only one of \"refpoint_u_fraction\" and \"refpoint_u\" may be defined\n");
      return 0;
    }
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefPointUFrac) &&
        !crgOptionIsSet(&crgData->modifiers, dCrgModRefPointU)) {
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefPointUFrac, 0.);
      crgMsgPrint(dCrgMsgLevelNotice, "crgCheckMods: setting value of modifier \"refpoint_u_fraction\" to 0.0.\n");
    }
    if (crgOptionIsSet(&crgData->modifiers, dCrgModRefPointVFrac) &&
        crgOptionIsSet(&crgData->modifiers, dCrgModRefPointV)) {
      crgMsgPrint(dCrgMsgLevelFatal,
                  "crgCheckMods: only one of \"refpoint_v_fraction\" and \"refpoint_v\" may be defined\n");
      return 0;
    }
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefPointVFrac) &&
        !crgOptionIsSet(&crgData->modifiers, dCrgModRefPointV)) {
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefPointVFrac, 0.);
      crgMsgPrint(dCrgMsgLevelNotice, "crgCheckMods: setting value of modifier \"refpoint_v_fraction\" to 0.0.\n");
    }
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefPointUOffset))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefPointUOffset, 0.);
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefPointVOffset))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefPointVOffset, 0.);
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefPointX))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefPointX, 0.);
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefPointY))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefPointY, 0.);
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefPointZ))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefPointZ, 0.);
    if (!crgOptionIsSet(&crgData->modifiers, dCrgModRefPointPhi))
      crgOptionSetDouble(&crgData->modifiers, dCrgModRefPointPhi, 0.);
  }

  return 1;
}

static int readDouble(char* dataPtr, double* tgt) {
  int j;
  int compValue[2];
  char* valPtr = (char*)tgt;

  if (!dataPtr || !tgt) return 0;

  if (mCrgBigEndian)
    memcpy(valPtr, dataPtr, 8 * sizeof(char));
  else
    for (j = 0; j < 8; j++) memcpy(&valPtr[7 - j], &dataPtr[j], sizeof(char));

  /* check for NaN and make sure it can be identified later-on */
  memcpy(compValue, tgt, sizeof(double));

  if (mCrgBigEndian) {
    if (compValue[0] >= 0x7ff80000) return -1;
  } else if (compValue[1] >= 0x7ff80000)
    return -1;

  return 1;
}

static int readFloat(char* dataPtr, float* tgt) {
  int compValue;
  int j;
  char* valPtr = (char*)tgt;

  if (!dataPtr || !valPtr) return 0;

  if (mCrgBigEndian)
    memcpy(valPtr, dataPtr, 4 * sizeof(char));
  else
    for (j = 0; j < 4; j++) memcpy(&valPtr[3 - j], &dataPtr[j], sizeof(char));

  /* check for NaN and make sure it can be identified later-on */
  memcpy(&compValue, tgt, sizeof(float));

  if ((compValue & 0x7fc00000) >= 0x7fc00000) return -1;

  return 1;
}

static int isLittleEndian(void) {
  int i = 0;

  ((char*)(&i))[0] = 1;

  return (i == 1);
}

int crgLoaderReadFile(const char* filename) {
  CrgDataStruct* crgData = NULL;

  /* --- initialize the loader before reading the file --- */
  crgLoaderInit();

  /* --- set file level to base level (reading primary file ) --- */
  mFileLevel = 0;

  if (!crgLoaderAddFile(filename, &crgData)) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgLoaderReadFile: error loading <%s>\n", filename);
    terminateReader(crgData, 0);
    return 0;
  }

  /* --- data available? --- */
  if (!crgData->channelV.info.size) {
    crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderReadFile: no data available. Terminating reader\n");
    return terminateReader(crgData, 0);
  }

  /* --- prepare the data read from file --- */
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderReadFile: preparing data\n");
  crgLoaderPrepareData(crgData);

  /* --- initialize data-set specific history --- */
  crgDataSetHistory(crgData->admin.id, dCrgHistoryStdSize);

  crgMsgPrint(dCrgMsgLevelNotice, "crgLoaderReadFile: finished reading file <%s>\n", filename);

  /* --- clear temporary data and declare success --- */
  if (!terminateReader(crgData, 1)) return 0;

  return crgData->admin.id;
}

static int crgLoaderAddFile(const char* filename, CrgDataStruct** crgRetData) {
  size_t noBytesRead;
  struct stat fileStat;
  char* bufPtr;
  size_t nBytesLeft;
  FILE* fPtr = NULL;
  CrgDataStruct* crgData = *crgRetData;

  /* --- open the file --- */
  if ((fPtr = fopen(filename, "rb")) == NULL) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgLoaderAddFile: could not open <%s>\n", filename);
    return 0;
  }

  /* --- check for endianness --- */
  if (isLittleEndian())
    crgMsgPrint(dCrgMsgLevelInfo, "crgLoaderAddFile: reading little endian encoding\n");
  else {
    crgMsgPrint(dCrgMsgLevelInfo, "crgLoaderAddFile: reading big endian encoding\n");
    mCrgBigEndian = 1;
  }

  /* --- get a new data structure and initialize it --- */
  if (!crgData) {
    if (!(crgData = crgDataSetCreate())) {
      crgMsgPrint(dCrgMsgLevelFatal, "crgLoaderAddFile: could not create data set\n");
      fclose(fPtr);
      return 0;
    }

    *crgRetData = crgData;
    initData(crgData);
  }

  /* --- memory map the file for faster access --- */
  stat(filename, &fileStat);
  crgData->admin.fileBuffer = (char*)crgCalloc(1, fileStat.st_size + 1);

  if (!crgData->admin.fileBuffer) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgLoaderAddFile: cannot allocate memory for file data\n");
    fclose(fPtr);
    return 0;
  }

  noBytesRead = fread(crgData->admin.fileBuffer, 1, fileStat.st_size, fPtr);
  fclose(fPtr);

  if (noBytesRead < (size_t)fileStat.st_size) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgLoaderAddFile: read error: only got %lld of %lld bytes\n", noBytesRead,
                fileStat.st_size);
    return 0;
  }

  /* --- copy basic file parameters for subsequent alteration --- */
  bufPtr = crgData->admin.fileBuffer;
  nBytesLeft = noBytesRead;

  /* --- parse the header of the file --- */
  if (!parseFileHeader(crgData, &bufPtr, &nBytesLeft)) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgLoaderAddFile: this file contains no data section.\n");
    return 1;
  }

  /* --- check the data consistency (i.e. header information) --- */
  if (!checkHeaderConsistency(crgData)) return 0;

  /* --- prepare some data according to the way v data is defined in the file --- */
  if ((crgData->admin.defMask & dCrgDataDefVPos)) {
    if (!prepareFromPosDef(crgData)) return 0;
  }

  if ((crgData->admin.defMask & dCrgDataDefVIndex)) {
    if (!prepareFromIndexDef(crgData)) return 0;
  }

  /* --- store the pointer to the data section of the file --- */
  crgData->admin.dataSection = bufPtr;
  crgData->admin.dataSize = nBytesLeft;

  /* --- the header seems to be ok, now let's start reading the actual data  --- */
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderAddFile: parsing center line\n");
  if (!parseCenterLine(crgData, bufPtr, nBytesLeft)) return 0;

  /* --- allocate space for actual crg data --- */
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderAddFile: allocating channels\n");
  if (!allocateChannels(crgData)) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgLoaderAddFile: could not allocate data.\n");
    return 0;
  }

  /* --- read the actual CRG data --- */
  crgMsgPrint(dCrgMsgLevelDebug, "crgLoaderAddFile: reading actual data\n");
  readData(crgData);

  /* --- clear temporary data and declare success --- */
  return 1;
}

static int crgStrBeginsWithStrNoCase(const char* str1, const char* str2) {
  char* c1 = (char*)str1;
  char* c2 = (char*)str2;
  unsigned int i;

  if (!str1 || !str2) return 0;

  if (strlen(str1) < strlen(str2)) return 0;

  for (i = 0; i < strlen(str2); i++) {
    if (tolower(*c1) != tolower(*c2)) return 0;

    c1++;
    c2++;
  }
  return 1;
}

static void crgLoaderInit(void) {
  static int firstTime = 1;

  /* --- do this only once! --- */
  if (!firstTime) return;
}

static int decodeIncludeFile(CrgDataStruct* crgData, const char* buffer, int code) {
  static char filename[1024];
  static char envVar[256];
  static int firstTime = 1;
  int result;

  /* --- initialize the structures? --- */
  if (firstTime) {
    firstTime = 0;
    memset(filename, 0, 1024 * sizeof(char));
  }

  switch (code) {
    case dOpcodeIncludeItem: {
      char* bufPtr = (char*)buffer;
      char* bufPtrEnd = NULL;
      char* bufPtrEnv = NULL;
      char* bufPtrEnvEnd = NULL;

      /* --- remove leading spaces --- */
      while (bufPtr) {
        if ((*bufPtr != ' ') || (*bufPtr == '\0')) break;

        bufPtr++;
      }

      /* --- read until next space or newline character --- */
      bufPtrEnd = bufPtr;

      while (bufPtrEnd) {
        if ((*bufPtrEnd == ' ') || (*bufPtrEnd == '\n') || (*bufPtrEnd == '\r') || (*bufPtrEnd == '\0') ||
            (*bufPtrEnd == '!'))
          break;
        bufPtrEnd++;
      }

      /* terminate the string */
      *bufPtrEnd = '\0';

      /* --- attach all characters between bufPtr and bufPtrEnd to filename, and don't forget
             to replace environment variables with their proper contents --- */

      while (*bufPtr != '\0') {
        /* is an environment variable hidden in the filename? */
        bufPtrEnv = (char*)findToken(bufPtr, "$");

        if (bufPtrEnv) {
          /* attach all characters before the environment variable to the filename */
          strncat(filename, bufPtr, (bufPtrEnv - bufPtr - 1));

          /* retrieve the environment variable */
          bufPtrEnvEnd = bufPtrEnv;

          while (bufPtrEnvEnd) {
            if ((*bufPtrEnvEnd == '\0') || (*bufPtrEnvEnd == '/')) break;

            bufPtrEnvEnd++;
          }

          memset(envVar, 0, 256 * sizeof(char));

          strncpy(envVar, bufPtrEnv, (bufPtrEnvEnd - bufPtrEnv));

          /* attach contents of environment variable to filename */
          if (getenv(envVar))
            strcat(filename, getenv(envVar));
          else
            crgMsgPrint(dCrgMsgLevelWarn, "decodeIncludeFile: undefined environment variable <%s>\n", envVar);

          /* go on after environment variable */
          bufPtr = bufPtrEnvEnd;
        } else {
          strncat(filename, bufPtr, bufPtrEnd - bufPtr);
          bufPtr = bufPtrEnd;
        }
      }
    }
      return 1;
      break;

    case dOpcodeIncludeDone: {
      CrgAdminStruct adminBackup;

      crgMsgPrint(dCrgMsgLevelNotice, "--------------------------------------\n", filename);
      crgMsgPrint(dCrgMsgLevelNotice, "decodeIncludeFile: importing file <%s>\n", filename);

      /* hold a copy of the administration structure */
      memcpy(&adminBackup, &(crgData->admin), sizeof(CrgAdminStruct));
      crgData->admin.sectionType = dFileSectionNone;

      /* load the include file and set the file level accordingly */
      mFileLevel++;

      result = crgLoaderAddFile(filename, &crgData);

      mFileLevel--;

      if (!result) return 0;

      /* restore the administration structure */
      memcpy(&(crgData->admin), &adminBackup, sizeof(CrgAdminStruct));

      crgMsgPrint(dCrgMsgLevelNotice, "--------------------------------------\n", filename);
      crgMsgPrint(dCrgMsgLevelNotice, "decodeIncludeFile: continuing with previous file\n");

      /* reset the filename for successive read operations */
      memset(filename, 0, sizeof(filename));

      return 1;

      /** @todo: check if following lines are still needed */
      /* --- the following lines are only required if endOfSection may also be placed --- */
      /* --- in a line starting with whitespace characters                            --- */
      /* careful, this is either the end of the section or an environment variable, both may begin with a $ sign */
      /*
      if ( buffer[0] == '$' )
      {
          fprintf( stderr, "decodeIncludeFile: importing file <%s>\n", filename );
          return 1;
      }
      else
          return decodeIncludeFile( crgData, buffer, dOpcodeIncludeItem );
      */
    } break;
  }

  return 0;
}
