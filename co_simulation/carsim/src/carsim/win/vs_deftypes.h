/* Define data types and macros for VS solver programs.

   Revision Log
   ------------
   Jul 10, 2015, M. Sayers. Changed vs_func1.function type from void to int
   Feb 25, 2014, M. Sayers. Removed obsolete vs_sym_attr_type enums
*/

#ifndef VS_DEFTYPES_H_
#define VS_DEFTYPES_H_

  #if (defined(_WIN32) || defined(_WIN64)) && !defined(_LABVIEW_RT)
    #define _CRTDBG_MAP_ALLOC  // look for memory leaks
    #include <stdlib.h>
    #ifdef _MSC_VER
      #include <crtdbg.h>
    #endif
    #define VS_API_EXPORT  __declspec(dllexport) // declaring public function in a DLL
  #else
    #define VS_API_EXPORT
  #endif

  #include <stdio.h>
  #include "vs_target.h"
  #include "vsm_project.h"

  #define TRUE 1
  #define FALSE 0
  #ifndef PI // avoid conflict in NI CVI
    #define PI 3.141592653589793
  #endif
  #define PI_HALF 1.5707963267948966

  // Floating point and boolean types
  typedef double vs_real;
  typedef int vs_bool;

  // Types of objects that can be accessed with a keyword, including new commands
  typedef enum {
    VS_KEYWORD_NULL = 100, VS_KEYWORD_SYM, VS_KEYWORD_ISYM, VS_KEYWORD_FUNC0,
    VS_KEYWORD_FUNC1, VS_KEYWORD_OUTVAR, VS_KEYWORD_WRT, VS_KEYWORD_EXP,
    VS_KEYWORD_ANI, VS_KEYWORD_IMP, VS_KEYWORD_TAB_1D, VS_KEYWORD_TAB_2D,
    VS_KEYWORD_TAB_GAIN, VS_KEYWORD_TAB_OFFSET, VS_KEYWORD_TAB_CONST,
    VS_KEYWORD_TAB_COEF, VS_KEYWORD_TAB_GROUP_1D, VS_KEYWORD_TAB_GROUP_2D,
    VS_KEYWORD_TAB_GROUP_GAIN, VS_KEYWORD_TAB_GROUP_OFFSET,
    VS_KEYWORD_TAB_GROUP_CONST, VS_KEYWORD_TAB_GROUP_COEF, VS_KEYWORD_CALC,
    VS_KEYWORD_TAB_GROUP_START_XROW, VS_KEYWORD_TAB_START_XROW,
    VS_KEYWORD_TAB_GROUP_SCALE_XROW, VS_KEYWORD_TAB_SCALE_XROW,
    VS_KEYWORD_TAB_GROUP_EQ, VS_KEYWORD_TAB_EQ,
    VS_KEYWORD_TAB_GROUP_START_XCOL, VS_KEYWORD_TAB_START_XCOL,
    VS_KEYWORD_TAB_GROUP_SCALE_XCOL, VS_KEYWORD_TAB_SCALE_XCOL,
    VS_KEYWORD_TAB_GROUP_COMBINE, VS_KEYWORD_TAB_COMBINE,
    VS_KEYWORD_TAB_GROUP_ID, VS_KEYWORD_TAB_ID
  } vs_keyword_type;

  // locations used for calling functions external_echo and external_calc (VS API).
  typedef enum {
    VS_EXT_ECHO_TOP, VS_EXT_ECHO_SYPARS, VS_EXT_ECHO_PARS, VS_EXT_ECHO_END,
    VS_EXT_EQ_INIT, VS_EXT_EQ_IN, VS_EXT_EQ_OUT, VS_EXT_EQ_END,
    VS_EXT_EQ_PRE_INIT, VS_EXT_EQ_INIT2, VS_EXT_EQ_SAVE, VS_EXT_EQ_FULL_STEP,
    VS_EXT_AFTER_READ // added July 10, 2-15
  } vs_ext_loc;

  // Attributes that can be set for an internal symbolic structure (sym). Used
  // in VS API functions. The order must be consistent with sCheckIdRange
  typedef enum {  // start with attributes for outputs & exports
    OUTVAR_SHORT_NAME, OUTVAR_LONG_NAME, OUTVAR_GEN_NAME, OUTVAR_BODY_NAME,
    OUTVAR_REAL, OUTVAR_VALUE, OUTVAR_UNITS, OUTVAR_COMP, OUTVAR_ECHO_DESC,

    // attributes for imports
    IMP_KEYWORD, IMP_UNITS, IMP_DESC, IMP_COMP, IMP_NATIVE, IMP_REAL, IMP_VISIBLE,

    SV_UNITS, SV_DERIVATIVE, // state variable derivative

    // attributes for indexed syms, with up to 5 dimensions
    ISYM_KEYWORD, ISYM_DIMENSIONS,
    ISYM_LIMIT_1, ISYM_LIMIT_2, ISYM_LIMIT_3, ISYM_LIMIT_4, ISYM_LIMIT_5,
    ISYM_INDEX_1, ISYM_INDEX_2, ISYM_INDEX_3, ISYM_INDEX_4, ISYM_INDEX_5,

     // attributes for system parameters
    SYS_PAR_KEYWORD, SYS_PAR_UNITS, SYS_PAR_DESC, SYS_PAR_REAL, SYS_PAR_INTEGER,
    SYS_PAR_VALUE, SYS_PAR_INIT, SYS_PAR_VISIBLE,

    // regular model parameters
    PAR_KEYWORD, PAR_UNITS, PAR_DESC, PAR_REAL, PAR_INTEGER, PAR_VALUE, PAR_INIT,
    PAR_VISIBLE,

    ANI_SHORT_NAME, ANI_REAL, ANI_REAL_SCALED // variables set for live ani
  } vs_sym_attr_type;

  // Import options for use with IMP_VISIBLE. IMP_VS_* options do not affect
  // Simulink. If this enum is changed in any way, you MUST ensure that it
  // stays in sync with sImpKeyType (defined in vsp_simlibio.c).
  typedef enum {
    IMP_IGNORE, IMP_REPLACE, IMP_ADD, IMP_MULTIPLY, IMP_USER_DEFINED,
    IMP_VS_REPLACE, IMP_VS_ADD, IMP_VS_MULTIPLY
  } vs_import_option;

  // Status of a vs_units structure.
  typedef enum {
    VS_UNITS_NATIVE, VS_UNITS_NEW, VS_UNITS_MODIFIED
  } vs_units_status;

  // Structure for units, needed for some VS API functions.
  typedef struct {
    char *desc, *keyword;   // desc is case-sensitive, keyword is all-caps
    vs_real gain;           // gain: multiply to convert from internal to output
    vs_units_status status; // status of the structure
  } vs_units;

  // Types of data that can be represented with vs_table. Used in table definition.
  typedef enum {
    VS_TAB_CONST, VS_TAB_COEF, VS_TAB_EQ, // non-table options
    VS_TAB_LIN, VS_TAB_LIN_LOOP, VS_TAB_LIN_FLAT, // 1D options
    VS_TAB_STEP, VS_TAB_STEP_LOOP, VS_SPLINE, VS_SPLINE_LOOP, VS_SPLINE_FLAT,
    VS_TAB_2D, VS_TAB_2D_LOOP, VS_TAB_2D_STEP, VS_TAB_2D_FROM_ZERO, // 2D options
    VS_TAB_2D_SPLINE, VS_TAB_2D_VAR_WIDTH_STEP, VS_TAB_2D_VAR_WIDTH
  } vs_table_type;

  // Combination options for replacing 2D table with 2 1D tables
  typedef enum {
    VS_TAB_COMBINE_ADD, VS_TAB_COMBINE_MULTIPLY, // used for 2D tables
    VS_TAB_COMBINE_CHILD, // indicates 1D table is child of 2D table
    VS_TAB_COMBINE_BOTH // used to specify that both add and multiply are supported
  } vs_tab_combine_type;

  typedef struct {
    vs_real ****c;  //Coefficients for each grid square for 2d splines
  } vs_2d_spline_coef;

  /* Structure for configurable table function. Value calculated using form:
       offset + gain*constant, or
       offset + gain*X*coefficient, or
       offset + gain*F(X, itab, inst), or
       offset + gain*F(Xcol, X, itab, inst), or
       offset + gain*(F(X, itab, inst) + F2(Xcol, itab, inst)), or
       offset + gain*(F(X, itab, inst)*F2(Xcol, itab, inst)), or
       offset + gain*eval(expression).

       where X    = (xrow_m - start_xrow)/scale_xrow
             Xcol = (xcol_m - start_xcol)/scale_xcol
             xrow_m, xcol_m are independent variables in the math model
             F = main function (all options), F2 is 2nd function (1D options)
     */

  // Structure for individual table.
  typedef struct vs_table {
    void *tabs;   // parent group: must coerce. Stupid GNU!
    int     visible;      // should table be printed in echo file?
    vs_table_type type;   // used for any table or constant
    vs_real constant, coefficient; // constants used to replace a table
    vs_real *x, *y;       // arrays used for any table
    vs_real *loop;        // range of table, used for looping
    vs_real gain, offset, start_xrow, scale_xrow; // used to transform almost any table
    vs_real start_xcol, scale_xcol; // used to transform some 2D tables
    int     nx, *jx;      // primary size and last entry; used for any table
    vs_real *dydx;        // array used only for 1D table
    vs_real **fxy;        // 2D array used only for 2D tables
    int     ny, *jy, extra;   // 2nd size used only for 2D tables
    char *table_keyword, *carpet_keyword; // Keywords (if not "") for parsfiles
    char *gain_keyword, *offset_keyword, *constant_keyword, *coefficient_keyword;
    char *equation_keyword;
    char *start_xrow_keyword, *scale_xrow_keyword; // revised July 4, 2011
    char *start_xcol_keyword, *scale_xcol_keyword; // added July 4, 2011
    char *user_id_keyword; // added May 5, 2014
    int *j_inverse; // instances for inverse tables
    void **sub_tabs;  // used to hold an array of sub-tables. In variable width
                      // tables, used to hold a 2d table with the position of the
                      // lanes (in roads, S vs L for the columns). For Independent
                      // column tables, holds an array of 1D tables.
    int nSubTabs;     // Number of sub-tables in sub_tabs array.
    vs_2d_spline_coef *coef; // structure to hold the coefficients needed for
                             // bicubic interpolation. Used only in 2D_SPLINE
    void *equation; // pointer to symbolic expression; must be coerced
    char *eq_desc; // string that was read to define the equation
    vs_units *units_row, *units_out; // used for all tables
    vs_units *units_col;             // used for column variable in 2D tables
    struct vs_table *col_tab; // optional pointer to 1D table for f vs xcol
    char *combine_keyword; // optional keyword to specify how 2 1D tables are combined
    vs_tab_combine_type combine_type; // code for combining 2 1D tables
    int extrapolated; // flag: not zero if extrapolation occurred
    //int user_id; // ID number of dataset defined by user
    int set; // flag: != 0 if set by vs_read_table (reading a parsfile)
  } vs_table;

  // Structure for table group
  typedef struct vs_tab_group {
    vs_table **table; // array of table pointers
    int ntab, ninst; // number of tables and instances per table
    vs_table_type type_default; // type of 2D table that is supported
    vs_units *units_row, *units_out; // defaults used for all tables
    vs_units *units_col;             // default used for column variable in 2D tables
    char *table_keyword, *carpet_keyword; // Keywords
    char *gain_keyword, *offset_keyword, *start_xrow_keyword, *scale_xrow_keyword;
    char *start_xcol_keyword, *scale_xcol_keyword;
    char *constant_keyword, *coefficient_keyword, *root_keyword, *equation_keyword;
    char *user_id_keyword;
    char *title;
    char *desc_row; // description of the primary independent variable
    char *desc_out; // description of the dependent variable from the table
    char *desc_col; // description of column variable in 2D tables
    int  (*get_itab) (void); // function to get table id if ntab > 1
    char *(*print_id) (int); // function to print table id info if ntab > 1
    char *(*print_label) (int); // function to print table label if ntab > 1
    char *name_id; // name written in echo file for id parameters (IAXLE, etc.) if ntab > 1
    int  *id; // pointer to table index (e.g., IAXLE)
    int inverse, derivative; // is there inverse and derivative lookup?
    int vs_command; // was this defined with the VS Command DEFINE_TABLE?
    struct vs_tab_group *col_tabg; // optional pointer to 1D table group for f vs xcol
    char *combine_keyword; // optional keyword to specify how 2 1D tables are combined
    vs_tab_combine_type combine_type_default; // default code for combining 2 1D tables
  } vs_tab_group;

  // Structure for deferred function call (no argument)
  typedef struct  {
    char *keyword;     // keyword should be all-caps, no spaces.
    void (*func) (void); // pointer to function name
  } vs_func0;

  // Structure for deferred function call (1 argument)
  typedef struct {
    char *keyword;     // keyword should be all-caps, no spaces.
    int (*func) (char *); // pointer to function name
  } vs_func1;

#endif  // end block for _VS_DEFTYPES_H
