/* Define data types and macros needed for programs that will work with VS Solvers
   using the VS API.

  July 2017, J. Schudel. Added explicit enum values and cautionary note
  July 2017, M. Sayers. Removed definitions not needed by wrapper programs
*/

#ifndef VS_DEFTYPES_H_
#define VS_DEFTYPES_H_

  #ifndef PI // avoid conflict in NI CVI
    #define PI 3.141592653589793
  #endif
  #define PI_HALF 1.5707963267948966

  // Floating point and boolean types
  typedef double vs_real;
  typedef int vs_bool;

  /***
    CAUTION:

      Editing these enums improperly can produce unpredictable errors in behavior.
      Addition of new entries to enumerators should be done in sequence, please
      use the next integer in sequence regardless of position. Please note, that
      the next integer in sequence may in fact not be located at the end of the
      enumerator.
  ***/

  // Types of objects that can be accessed with a keyword, including new commands
  typedef enum {
    VS_KEYWORD_NULL = 100, VS_KEYWORD_SYM = 101, VS_KEYWORD_ISYM = 102,
    VS_KEYWORD_FUNC0 = 103, VS_KEYWORD_FUNC1 = 104, VS_KEYWORD_CLOSURE = 105,
    VS_KEYWORD_OUTVAR = 106, VS_KEYWORD_WRT = 107, VS_KEYWORD_EXP = 108,
    VS_KEYWORD_ANI = 109, VS_KEYWORD_IMP = 110, VS_KEYWORD_TAB_1D = 111,
    VS_KEYWORD_TAB_2D = 112,
    VS_KEYWORD_TAB_GAIN = 113, VS_KEYWORD_TAB_OFFSET = 114, VS_KEYWORD_TAB_CONST = 115,
    VS_KEYWORD_TAB_COEF = 116, VS_KEYWORD_TAB_GROUP_1D = 117,
    VS_KEYWORD_TAB_GROUP_2D = 118,
    VS_KEYWORD_TAB_GROUP_GAIN = 119, VS_KEYWORD_TAB_GROUP_OFFSET = 120,
    VS_KEYWORD_TAB_GROUP_CONST = 121, VS_KEYWORD_TAB_GROUP_COEF = 122,
    VS_KEYWORD_CALC = 123,
    VS_KEYWORD_TAB_GROUP_START_XROW = 124, VS_KEYWORD_TAB_START_XROW = 125,
    VS_KEYWORD_TAB_GROUP_SCALE_XROW = 126, VS_KEYWORD_TAB_SCALE_XROW = 127,
    VS_KEYWORD_TAB_GROUP_EQ = 128, VS_KEYWORD_TAB_EQ = 129,
    VS_KEYWORD_TAB_GROUP_START_XCOL = 130, VS_KEYWORD_TAB_START_XCOL = 131,
    VS_KEYWORD_TAB_GROUP_SCALE_XCOL = 132, VS_KEYWORD_TAB_SCALE_XCOL = 133,
    VS_KEYWORD_TAB_GROUP_COMBINE = 134, VS_KEYWORD_TAB_COMBINE = 135,
    VS_KEYWORD_TAB_GROUP_ID = 136, VS_KEYWORD_TAB_ID = 137
  } vs_keyword_type;

  // Locations used for calling external functions (e.g. external_echo,
  // external_calc, external_status, etc.).
  typedef enum {
    VS_EXT_ECHO_TOP = 0, VS_EXT_ECHO_SYPARS = 1, VS_EXT_ECHO_PARS = 2,
    VS_EXT_ECHO_END = 3, VS_EXT_EQ_INIT = 4, VS_EXT_EQ_IN = 5, VS_EXT_EQ_OUT = 6,
    VS_EXT_EQ_END = 7, VS_EXT_EQ_PRE_INIT = 8, VS_EXT_EQ_INIT2 = 9,
    VS_EXT_EQ_SAVE = 10, VS_EXT_EQ_FULL_STEP = 11,  VS_EXT_AFTER_READ = 12,
    VS_EXT_LOC_UNSPECIFIED = 13, VS_EXT_EQ_DYN = 14
  } vs_ext_loc;

  // Attributes that can be set for an internal symbolic structure (sym). Used
  // in VS API functions.
  // Any new entry added here should also be installed in sFindGenIdType()
  typedef enum {  // start with attributes for outputs & exports
    OUTVAR_SHORT_NAME = 0, OUTVAR_LONG_NAME = 1, OUTVAR_GEN_NAME = 2,
    OUTVAR_BODY_NAME = 3, OUTVAR_REAL = 4, OUTVAR_VALUE = 5, OUTVAR_UNITS = 6,
    OUTVAR_COMP = 7, OUTVAR_ECHO_DESC = 8, IMP_KEYWORD = 9, IMP_UNITS = 10,
    IMP_DESC = 11, IMP_COMP = 12, IMP_NATIVE = 13, IMP_REAL = 14, IMP_VISIBLE = 15,
    SV_UNITS = 16, SV_DERIVATIVE = 17, // state variable derivative
    ISYM_KEYWORD = 18, ISYM_DIMENSIONS = 19,
    ISYM_LIMIT_1 = 20, ISYM_LIMIT_2 = 21, ISYM_LIMIT_3 = 22, ISYM_LIMIT_4 = 23,
    ISYM_LIMIT_5 = 24, ISYM_INDEX_1 = 25, ISYM_INDEX_2 = 26, ISYM_INDEX_3 = 27,
    ISYM_INDEX_4 = 28, ISYM_INDEX_5 = 29,
    SYS_PAR_KEYWORD = 30, SYS_PAR_UNITS = 31, SYS_PAR_DESC = 32, SYS_PAR_REAL = 33,
    SYS_PAR_INTEGER = 34, SYS_PAR_VALUE = 35, SYS_PAR_INIT = 36, SYS_PAR_VISIBLE = 37,
    PAR_KEYWORD = 38, PAR_UNITS = 39, PAR_DESC = 40, PAR_REAL = 41, PAR_INTEGER = 42,
    PAR_VALUE = 43, PAR_INIT = 44, PAR_VISIBLE = 45,
    ANI_SHORT_NAME = 46, ANI_REAL = 47, ANI_REAL_SCALED = 48, // for live ani
    OUTVAR_ERD_NAME = 49, OUTVAR_ERD_UNITS = 50, OUTVAR_EXTRA_DOC = 51,
    ANI_NAME = 52, PAR_CHECK_VALUE = 53, PAR_MIN_VALUE = 54, PAR_MAX_VALUE = 55,
    SYS_PAR_CHECK_VALUE = 56, SYS_PAR_MIN_VALUE = 57, SYS_PAR_MAX_VALUE = 58
  } vs_sym_attr_type;

  typedef enum {
    UNKNOWN_TYPE = -1,  /* should not occur */
    OUTVAR_TYPE = 0,
    IMP_TYPE = 1,
    SV_TYPE = 2,
    ISYM_TYPE = 3,
    SYS_PAR_TYPE = 4,
    PAR_TYPE = 5,
    ANI_TYPE = 6
  } vs_gen_sym_attr_type;

  /// Definition of an external status update callback function type.
  typedef int (*status_func_t)(vs_ext_loc where, int statusCode, vs_real statusValue, void* userData);

#endif  // end block for _VS_DEFTYPES_H
