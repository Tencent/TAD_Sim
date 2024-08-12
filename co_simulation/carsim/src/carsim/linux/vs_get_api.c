/* Functions to load a VS API DLL using GetProcAddress. The functions declared
   in vs_api.h are global, so this only works for a single loaded DLL.

   Log:
   Aug 31, 16. Changes for CarSim 2017, including Linux support.
   Nov 13, 15. Changes for CarSim 2016
   Sep 24, 14. Added new function for CarSim 9.0.
   May 17, 10. M. Sayers. Complete re-write with vs_get_api, better error handling.
   May 18, 09. M. Sayers. Include vs_get_api_install_external for CarSim 8.0.
   Jun 13, 08. M. Sayers. Created for the release of CarSim 7.1.
   */

// Standard C headers.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "vs_deftypes.h" // VS types and definitions
#include "vs_api.h"      // VS API definitions as prototypes

#ifdef _WIN64
#define BITNESS_SUFFIX "_64"
#elif _WIN32
#define BITNESS_SUFFIX "_32"
#else
#define BITNESS_SUFFIX ""
#endif

#if (defined(_WIN32) || defined(_WIN64))
#define LIBRARYTYPESTR "DLL"
#else
#define LIBRARYTYPESTR "SharedLibrary"
#include <dlfcn.h>
#endif

#if defined(_MSC_VER)
  // Defining _CRT_SECURE_NO_WARNINGS here doesn't seem to work, so we disable this warning directly:
  #pragma warning(disable: 4996)
#endif

/// Default error message handler function.
static int sDefaultPrintf(const char *format, ...) {
  int	numberOfCharactersPrinted;
  char tmpstr[FILENAME_MAX + 100];

  va_list args;
  va_start (args, format);
  numberOfCharactersPrinted = vsprintf(tmpstr, format, args);
  va_end (args);

#if (defined(_WIN32) || defined(_WIN64))
  MessageBoxA (NULL, tmpstr, "Sorry", MB_ICONERROR);
#else
  printf(tmpstr, args);
  printf("\n");
#endif

  return numberOfCharactersPrinted;
}


/** The API-error print function pointer. Initially, it points to our default
  error message handler, sDefaultPrintf(), but this can be overridden
  by using vs_set_error_message_function().
*/
static int( *sPrintfError ) (const char *format, ...) = sDefaultPrintf;

void vs_set_error_message_function (int(*errorMsgFunc) (const char *format, ...)) {
  sPrintfError = errorMsgFunc;
}

#if (defined(_WIN32) || defined(_WIN64))
HMODULE vs_load_library(char* pathDLL) {
  return LoadLibraryA(pathDLL);
}
void vs_free_library(HMODULE dll) {
  FreeLibrary(dll);
}
#else
void* vs_load_library(char* pathDLL) {
  char tempPath[FILENAME_MAX] = "";
  char *errorString;

  // RTLD_DEEPBIND - Forces the solver to use its own symbols first, even if
  //   we have samely named symbols in the program that loaded it.
  //   This fixes crashes when the calling program is linked with "-rdynamic".
  int loadFlags = RTLD_NOW | RTLD_DEEPBIND;

  // Clear error string.
  dlerror();
  errorString = NULL;

  printf("Loading library: %s\n", pathDLL);
  void* libPtr = dlopen(pathDLL, loadFlags);
  errorString = dlerror();

  if (libPtr == NULL || (errorString != NULL)) {
    // Also try local folder.
    sprintf(tempPath, "./%s", pathDLL);

    // Clear error string.
    dlerror();
    errorString = NULL;

    libPtr = dlopen(tempPath, loadFlags);
    errorString = dlerror();
  }

  if (libPtr == NULL || (errorString != NULL))
    printf("Error loading library: %s\n", errorString);

  return libPtr;
}

void vs_free_library(HMODULE dll) {
  dlclose(dll);
}
#endif

// Get solver path and program directory from simfile
int vs_get_dll_path(const char* simfile, char* pathDLL)
{
  FILE* fp;
  char *key, *rest, tmpstr[FILENAME_MAX], tempSimfilePath[FILENAME_MAX],
       progDir[FILENAME_MAX], dllDir[FILENAME_MAX],
       vehicleCode[4096], libraryName[4096], productName[128], productVersion[128];

#if !defined(_WIN32) && !defined(_WIN64)
  char *tempSimfilePathPtr = NULL;
#endif

  strcpy(tempSimfilePath, simfile);

  // JMM.2018.09.12. This may look a step backwards and maybe it is. The goal of vs_get_api.c is it
  // used and copied to many projects. Some of those project make use of PathHelper.c/h. In those projects
  // there is already defined "convert_path_delimiters" and when linking we will get a multiple definition link
  // error. As such we don't really need a local function that is only used once.
  // The below code snippet converts "\" to "/" when running on "not windows".
  // Ideally we should be using the PathHelper.h and its "convert_path_delimiters" that does exactly that.
  // However, under time constraints I am just removing the function from file. I am assuming it was not
  // a coincidence that we had two function doing basically the same thing with the same name and the original
  // author had a reason to copy the function instead of including PathHelpers.h.
#if !defined(_WIN32) && !defined(_WIN64)
  tempSimfilePathPtr = &tempSimfilePath[0];
  while (*tempSimfilePathPtr != '\0')
  {
    if (*tempSimfilePathPtr == '\\')
      *tempSimfilePathPtr = '/';
    tempSimfilePathPtr++;
  }
#endif

  if ((fp = fopen(tempSimfilePath, "r")) == NULL)
  {
    sPrintfError("This program needs a simfile to obtain other file names. The "
                 "file \"%s\" either does not exist or could not be opened.\n",
                 tempSimfilePath);
    return -1;
  }

  pathDLL[0] = progDir[0] = dllDir[0] = vehicleCode[0] = libraryName[0] = productName[0] = productVersion[0] = 0;

  // scan simfile one line at a time
  while (fgets(tmpstr, FILENAME_MAX, fp))
  {
    key = (char *)strtok(tmpstr, " \t\r\n");
    rest = (char *)strtok(NULL, "\r\n\t");

    if (!key) // skip empty lines
      continue;

    // get DLL if specified directly
#if defined(_WIN64) || defined(_WIN32)
    if (!strcmp(key, "DLLFILE") && rest && rest[0])
    {
      strcpy(pathDLL, rest);
      break;
    }
#else
    if (!strcmp(key, "SOFILE") && rest && rest[0])
    {
      strcpy(pathDLL, rest);
      break;
    }
#endif
    if (!strcmp(key, "VEHICLE_CODE")) // get vehicle code
      strcpy(vehicleCode, rest);
    else if (!strcmp(key, "PROGDIR")) // get program directory
      strcpy(progDir, rest);
    else if (!strcmp(key, "PRODUCT_ID")) // get product
      strcpy(productName, rest);
    else if (!strcmp(key, "PRODUCT_VER")) // get version
      strcpy(productVersion, rest);
    else if (!strcmp(key, "END"))
      break;
  }
  fclose(fp);

  if (strstr(vehicleCode, "tire") != NULL)
    sprintf(libraryName, "%s%s", vehicleCode, BITNESS_SUFFIX);
  else if (strcmp(productName, "CarSim") == 0)
    sprintf(libraryName, "carsim%s", BITNESS_SUFFIX);
  else if (strcmp(productName, "TruckSim") == 0)
    sprintf(libraryName, "trucksim%s", BITNESS_SUFFIX);
  else
    sprintf(libraryName, "%s%s", vehicleCode, BITNESS_SUFFIX);

  // Quit if DLL was not specified and neither was vehicle code
  if ((pathDLL[0] == 0) && (libraryName[0] == 0))
  {
    sPrintfError("Unable to determine Solver type from SIMFILE.\n");
    return -1;
  }

  // if DLLFILE was not used, resolve the standard location
  if (pathDLL[0] == 0 && progDir[0] != 0 && libraryName[0] != 0)
  {
#if (defined(_WIN32) || defined(_WIN64))
    sprintf(pathDLL, "%s\\Programs\\solvers\\%s.dll", progDir, libraryName);
#else
    if (productVersion[0] == 0)
    {
      sprintf(pathDLL, "lib%s.so.%s", libraryName, PRODUCT_VERSION);
    }
    else
    {
      sprintf(pathDLL, "lib%s.so.%s", libraryName, productVersion);
    }
#endif
  }

  return 0;
}

// try to get the address of an API function from a DLL
static int sGetApi (void *api_func, HMODULE dll, const char *func, const char *dll_name,
                    const char *get_func) {
#if !defined(_WIN32) && !defined(_WIN64)
  char *errorString;
#endif

  if (dll == NULL)
    return -1;

#if (defined(_WIN32) || defined(_WIN64))
  (*(void**)api_func) = (void *)GetProcAddress(dll, func); // cast to work

  if (*(void**)api_func)
    return 0;
#else

  dlerror();  // Clear any existing error, so we can check it afterwards.

  (*(void**)api_func) = dlsym(dll, func);

  if ((errorString = dlerror()) != NULL)
    sPrintfError( "dlerror(): %s", errorString);
  else
    return 0;
#endif

  sPrintfError( "The function %s could not get the requested VS API function \"%s\" "
                "from the " LIBRARYTYPESTR ": \"%s\".\n", get_func, func, dll_name);
  return -2;
}

// utility to handle error if a DLL isn't there.
static int sPrintErrorNoDll (const char *where, const char *dll_name) {
  sPrintfError("The function %s was not given a valid " LIBRARYTYPESTR ". "
                              "\"%s\" did not load.\n", where, dll_name);
  return -1;
}


/* ----------------------------------------------------------------------------
   Get all VS API functions declared in "vs_api.h"
---------------------------------------------------------------------------- */
int vs_get_api (HMODULE dll, const char *dname) {
  char *me = "vs_get_api";
  if (dll == NULL) return sPrintErrorNoDll(me, dname);

  // basic run functions (chapter 2)
  if (sGetApi(&vs_run, dll, "vs_run", dname, me)) return -2;
  if (sGetApi(&vs_bar_graph_update, dll, "vs_bar_graph_update", dname, me)) return -2;
  if (sGetApi(&vs_error_occurred, dll, "vs_error_occurred", dname, me)) return -2;
  if (sGetApi(&vs_initialize, dll, "vs_initialize", dname, me)) return -2;
  if (sGetApi(&vs_integrate, dll, "vs_integrate", dname, me)) return -2;
  if (sGetApi(&vs_set_macro, dll, "vs_set_macro", dname, me)) return -2;
  if (sGetApi(&vs_setdef_and_read, dll, "vs_setdef_and_read", dname, me)) return -2;
  if (sGetApi(&vs_statement, dll, "vs_statement", dname, me)) return -2;
  if (sGetApi(&vs_stop_run, dll, "vs_stop_run", dname, me)) return -2;
  if (sGetApi(&vs_terminate_run, dll, "vs_terminate_run", dname, me)) return -2;

  // managing import/export arrays (chapter 4)
  if (sGetApi(&vs_copy_export_vars, dll, "vs_copy_export_vars", dname, me)) return -2;
  if (sGetApi(&vs_integrate_io, dll, "vs_integrate_io", dname, me)) return -2;
  if (sGetApi(&vs_get_tstep, dll, "vs_get_tstep", dname, me)) return -2;
  if (sGetApi(&vs_read_configuration, dll, "vs_read_configuration", dname, me)) return -2;

  // utility functions: conditons (chapter 5)
  if (sGetApi(&vs_during_event, dll, "vs_during_event", dname, me)) return -2;
  if (sGetApi(&vs_get_time, dll, "vs_get_time", dname, me)) return -2;
  if (sGetApi(&vs_opt_pause, dll, "vs_opt_pause", dname, me)) return -2;

  // utility functions: messages (chapter 5)
  if (sGetApi(&vs_clear_output_message, dll, "vs_clear_output_message", dname, me)) return -2;
  if (sGetApi(&vs_clear_error_message, dll, "vs_clear_error_message", dname, me)) return -2;
  if (sGetApi(&vs_get_error_message, dll, "vs_get_error_message", dname, me)) return -2;
  if (sGetApi(&vs_get_output_message, dll, "vs_get_output_message", dname, me)) return -2;
  if (sGetApi(&vs_get_version_model, dll, "vs_get_version_model", dname, me)) return -2;
  if (sGetApi(&vs_get_version_product, dll, "vs_get_version_product", dname, me)) return -2;
  if (sGetApi(&vs_get_version_vs, dll, "vs_get_version_vs", dname, me)) return -2;
  if (sGetApi(&vs_printf, dll, "vs_printf", dname, me)) return -2;
  if (sGetApi(&vs_printf_error, dll, "vs_printf_error", dname, me)) return -2;

  // utility functions: file names and directories (chapter 5)
  if (sGetApi(&vs_get_database_dir, dll, "vs_get_database_dir", dname, me)) return -2;
  if (sGetApi(&vs_get_echofile_name, dll, "vs_get_echofile_name", dname, me)) return -2;
  if (sGetApi(&vs_get_endfile_name, dll, "vs_get_endfile_name", dname, me)) return -2;
  if (sGetApi(&vs_get_erdfile_name, dll, "vs_get_erdfile_name", dname, me)) return -2;
  if (sGetApi(&vs_get_filebase_name, dll, "vs_get_filebase_name", dname, me)) return -2;
  if (sGetApi(&vs_get_infile_name, dll, "vs_get_infile_name", dname, me)) return -2;
  if (sGetApi(&vs_get_logfile_name, dll, "vs_get_logfile_name", dname, me)) return -2;
  if (sGetApi(&vs_get_simfile_name, dll, "vs_get_simfile_name", dname, me)) return -2;
  if (sGetApi(&vs_write_doc_files, dll, "vs_write_doc_files", dname, me)) return -2;

   // install external functions (chapter 6)
  if (sGetApi(&vs_install_calc_function, dll, "vs_install_calc_function", dname, me)) return -2;
  if (sGetApi(&vs_install_echo_function, dll, "vs_install_echo_function", dname, me)) return -2;
  if (sGetApi(&vs_install_setdef_function, dll, "vs_install_setdef_function", dname, me)) return -2;
  if (sGetApi(&vs_install_scan_function, dll, "vs_install_scan_function", dname, me)) return -2;
  if (sGetApi(&vs_install_free_function, dll, "vs_install_free_function", dname, me)) return -2;
  if (sGetApi(&vs_install_status_function, dll, "vs_install_status_function", dname, me)) return -2;
  if (sGetApi(&vs_install_calc_function2, dll, "vs_install_calc_function2", dname, me)) return -2;
  if (sGetApi(&vs_install_echo_function2, dll, "vs_install_echo_function2", dname, me)) return -2;
  if (sGetApi(&vs_install_setdef_function2, dll, "vs_install_setdef_function2", dname, me)) return -2;
  if (sGetApi(&vs_install_scan_function2, dll, "vs_install_scan_function2", dname, me)) return -2;
  if (sGetApi(&vs_install_free_function2, dll, "vs_install_free_function2", dname, me)) return -2;

  // functions for interacting with the VS math model (chapter 7)
  if (sGetApi(&vs_define_imp, dll, "vs_define_imp", dname, me)) return -2;
  if (sGetApi(&vs_define_ipar_1d, dll, "vs_define_ipar_1d", dname, me)) return -2;
  if (sGetApi(&vs_define_ipar_2d, dll, "vs_define_ipar_2d", dname, me)) return -2;
  if (sGetApi(&vs_define_ipar_3d, dll, "vs_define_ipar_3d", dname, me)) return -2;
  if (sGetApi(&vs_define_ipar_4d, dll, "vs_define_ipar_4d", dname, me)) return -2;
  if (sGetApi(&vs_define_ipar_5d, dll, "vs_define_ipar_5d", dname, me)) return -2;
  if (sGetApi(&vs_define_out, dll, "vs_define_out", dname, me)) return -2;
  if (sGetApi(&vs_define_par, dll, "vs_define_par", dname, me)) return -2;
  if (sGetApi(&vs_define_sypar, dll, "vs_define_sypar", dname, me)) return -2;
  if (sGetApi(&vs_define_sv, dll, "vs_define_sv", dname, me)) return -2;
  if (sGetApi(&vs_define_units, dll, "vs_define_units", dname, me)) return -2;
  if (sGetApi(&vs_get_numeric_value, dll, "vs_get_numeric_value", dname, me)) return -2;
  if (sGetApi(&vs_get_sym_attribute, dll, "vs_get_sym_attribute", dname, me)) return -2;
  if (sGetApi(&vs_get_var_id, dll, "vs_get_var_id", dname, me)) return -2;
  if (sGetApi(&vs_get_var_ptr, dll, "vs_get_var_ptr", dname, me)) return -2;
  if (sGetApi(&vs_get_var_ptr2, dll, "vs_get_var_ptr2", dname, me)) return -2;
  if (sGetApi(&vs_get_var_ptr_int, dll, "vs_get_var_ptr_int", dname, me)) return -2;
  if (sGetApi(&vs_get_var_ptr_int2, dll, "vs_get_var_ptr_int2", dname, me)) return -2;
  if (sGetApi(&vs_have_keyword_in_database, dll, "vs_have_keyword_in_database", dname, me)) return -2;
  if (sGetApi(&vs_import_result, dll, "vs_import_result", dname, me)) return -2;
  if (sGetApi(&vs_install_symbolic_func, dll, "vs_install_symbolic_func", dname, me)) return -2;
  if (sGetApi(&vs_install_keyword_alias, dll, "vs_install_keyword_alias", dname, me)) return -2;
  if (sGetApi(&vs_put_on_free_list, dll, "vs_put_on_free_list", dname, me)) return -2;
  if (sGetApi(&vs_set_stop_run, dll, "vs_set_stop_run", dname, me)) return -2;
  if (sGetApi(&vs_set_sym_attribute, dll, "vs_set_sym_attribute", dname, me)) return -2;
  if (sGetApi(&vs_set_sym_int, dll, "vs_set_sym_int", dname, me)) return -2;
  if (sGetApi(&vs_string_copy_internal, dll, "vs_string_copy_internal", dname, me)) return -2;
  if (sGetApi(&vs_set_opt_error_dialog, dll, "vs_set_opt_error_dialog", dname, me)) return -2;

  // working with simulation files (chapter 7)
  if (sGetApi(&vs_add_new_par_group, dll, "vs_add_new_par_group", dname, me)) return -2;
  if (sGetApi(&vs_insert_echo_blank_line, dll, "vs_insert_echo_blank_line", dname, me)) return -2;
  if (sGetApi(&vs_insert_echo_comment, dll, "vs_insert_echo_comment", dname, me)) return -2;
  if (sGetApi(&vs_get_active_par_group, dll, "vs_get_active_par_group", dname, me)) return -2;
  if (sGetApi(&vs_set_active_par_group, dll, "vs_set_active_par_group", dname, me)) return -2;

  if (sGetApi(&vs_read_next_line, dll, "vs_read_next_line", dname, me)) return -2;
  if (sGetApi(&vs_write_f_to_echo_file, dll, "vs_write_f_to_echo_file", dname, me)) return -2;
  if (sGetApi(&vs_write_header_to_echo_file, dll, "vs_write_header_to_echo_file", dname, me)) return -2;
  if (sGetApi(&vs_write_i_to_echo_file, dll, "vs_write_i_to_echo_file", dname, me)) return -2;
  if (sGetApi(&vs_write_to_echo_file, dll, "vs_write_to_echo_file", dname, me)) return -2;
  if (sGetApi(&vs_write_to_logfile, dll, "vs_write_to_logfile", dname, me)) return -2;

  // configurable table functions (chapter 7)
  if (sGetApi(&vs_define_table, dll, "vs_define_table", dname, me)) return -2;
  if (sGetApi(&vs_table_calc, dll, "vs_table_calc", dname, me)) return -2;
  if (sGetApi(&vs_table_index, dll, "vs_table_index", dname, me)) return -2;
  if (sGetApi(&vs_table_ninst, dll, "vs_table_ninst", dname, me)) return -2;
  if (sGetApi(&vs_table_ntab, dll, "vs_table_ntab", dname, me)) return -2;

  // 2D reference path properties (chapter 7)
  if (sGetApi(&vs_get_road_start_stop, dll, "vs_get_road_start_stop", dname, me)) return -2;
  if (sGetApi(&vs_path_curv_i, dll, "vs_path_curv_i", dname, me)) return -2;
  if (sGetApi(&vs_road_curv_i, dll, "vs_road_curv_i", dname, me)) return -2;
  if (sGetApi(&vs_path_l_i, dll, "vs_path_l_i", dname, me)) return -2;
  if (sGetApi(&vs_road_l, dll, "vs_road_l", dname, me)) return -2;
  if (sGetApi(&vs_road_l_i, dll, "vs_road_l_i", dname, me)) return -2;
  if (sGetApi(&vs_path_s_i, dll, "vs_path_s_i", dname, me)) return -2;
  if (sGetApi(&vs_road_s, dll, "vs_road_s", dname, me)) return -2;
  if (sGetApi(&vs_road_s_i, dll, "vs_road_s_i", dname, me)) return -2;
  if (sGetApi(&vs_path_x_i, dll, "vs_path_x_i", dname, me)) return -2;
  if (sGetApi(&vs_road_x, dll, "vs_road_x", dname, me)) return -2;
  if (sGetApi(&vs_road_x_i, dll, "vs_road_x_i", dname, me)) return -2;
  if (sGetApi(&vs_path_x_sl_i, dll, "vs_path_x_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_road_x_sl_i, dll, "vs_road_x_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_path_y_i, dll, "vs_path_y_i", dname, me)) return -2;
  if (sGetApi(&vs_road_y, dll, "vs_road_y", dname, me)) return -2;
  if (sGetApi(&vs_road_y_i, dll, "vs_road_y_i", dname, me)) return -2;
  if (sGetApi(&vs_path_y_sl_i, dll, "vs_path_y_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_road_y_sl_i, dll, "vs_road_y_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_path_yaw_i, dll, "vs_path_yaw_i", dname, me)) return -2;
  if (sGetApi(&vs_road_yaw, dll, "vs_road_yaw", dname, me)) return -2;
  if (sGetApi(&vs_road_yaw_i, dll, "vs_road_yaw_i", dname, me)) return -2;
  if (sGetApi(&vs_s_loop, dll, "vs_s_loop", dname, me)) return -2;
  if (sGetApi(&vs_s_path_loop, dll, "vs_s_path_loop", dname, me)) return -2;
  if (sGetApi(&vs_target_heading, dll, "vs_target_heading", dname, me)) return -2;
  if (sGetApi(&vs_target_l, dll, "vs_target_l", dname, me)) return -2;

  // 3D properties of current road (chapter 7)
  if (sGetApi(&vs_get_dzds_dzdl, dll, "vs_get_dzds_dzdl", dname, me)) return -2;
  if (sGetApi(&vs_get_dzds_dzdl_i, dll, "vs_get_dzds_dzdl_i", dname, me)) return -2;
  if (sGetApi(&vs_get_road_contact, dll, "vs_get_road_contact", dname, me)) return -2;
  if (sGetApi(&vs_get_road_contact_sl, dll, "vs_get_road_contact_sl", dname, me)) return -2;
  if (sGetApi(&vs_get_road_xyz, dll, "vs_get_road_xyz", dname, me)) return -2;
  if (sGetApi(&vs_road_pitch_sl_i, dll, "vs_road_pitch_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_road_roll_sl_i, dll, "vs_road_roll_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_road_z, dll, "vs_road_z", dname, me)) return -2;
  if (sGetApi(&vs_road_z_i, dll, "vs_road_z_i", dname, me)) return -2;
  if (sGetApi(&vs_road_z_sl_i, dll, "vs_road_z_sl_i", dname, me)) return -2;

  // Functions the make use of user ID for paths (chapter 7)
  if (sGetApi(&vs_path_sstart_id, dll, "vs_path_sstart_id", dname, me)) return -2;
  if (sGetApi(&vs_path_sstop_id, dll, "vs_path_sstop_id", dname, me)) return -2;
  if (sGetApi(&vs_path_length_id, dll, "vs_path_length_id", dname, me)) return -2;
  if (sGetApi(&vs_path_is_looped_id, dll, "vs_path_is_looped_id", dname, me)) return -2;
  if (sGetApi(&vs_path_s_id, dll, "vs_path_s_id", dname, me)) return -2;
  if (sGetApi(&vs_path_l_id, dll, "vs_path_l_id", dname, me)) return -2;
  if (sGetApi(&vs_path_x_id, dll, "vs_path_x_id", dname, me)) return -2;
  if (sGetApi(&vs_path_y_id, dll, "vs_path_y_id", dname, me)) return -2;
  if (sGetApi(&vs_path_dxds_id, dll, "vs_path_dxds_id", dname, me)) return -2;
  if (sGetApi(&vs_path_dyds_id, dll, "vs_path_dyds_id", dname, me)) return -2;
  if (sGetApi(&vs_path_dxdl_id, dll, "vs_path_dxdl_id", dname, me)) return -2;
  if (sGetApi(&vs_path_dydl_id, dll, "vs_path_dydl_id", dname, me)) return -2;
  if (sGetApi(&vs_path_curv_id, dll, "vs_path_curv_id", dname, me)) return -2;
  if (sGetApi(&vs_path_yaw_id, dll, "vs_path_yaw_id", dname, me)) return -2;

  // Functions that make use of user ID for roads (chapter 7)
  if (sGetApi(&vs_road_sstart_id, dll, "vs_road_sstart_id", dname, me)) return -2;
  if (sGetApi(&vs_road_sstop_id, dll, "vs_road_sstop_id", dname, me)) return -2;
  if (sGetApi(&vs_road_length_id, dll, "vs_road_length_id", dname, me)) return -2;
  if (sGetApi(&vs_road_is_looped_id, dll, "vs_road_is_looped_id", dname, me)) return -2;
  if (sGetApi(&vs_road_s_id, dll, "vs_road_s_id", dname, me)) return -2;
  if (sGetApi(&vs_road_l_id, dll, "vs_road_l_id", dname, me)) return -2;
  if (sGetApi(&vs_road_x_id, dll, "vs_road_x_id", dname, me)) return -2;
  if (sGetApi(&vs_road_y_id, dll, "vs_road_y_id", dname, me)) return -2;
  if (sGetApi(&vs_road_dxds_id, dll, "vs_road_dxds_id", dname, me)) return -2;
  if (sGetApi(&vs_road_dyds_id, dll, "vs_road_dyds_id", dname, me)) return -2;
  if (sGetApi(&vs_road_dxdl_id, dll, "vs_road_dxdl_id", dname, me)) return -2;
  if (sGetApi(&vs_road_dydl_id, dll, "vs_road_dydl_id", dname, me)) return -2;
  if (sGetApi(&vs_road_curv_id, dll, "vs_road_curv_id", dname, me)) return -2;
  if (sGetApi(&vs_road_z_id, dll, "vs_road_z_id", dname, me)) return -2;
  if (sGetApi(&vs_road_dzds_id, dll, "vs_road_dzds_id", dname, me)) return -2;
  if (sGetApi(&vs_road_dzdl_id, dll, "vs_road_dzdl_id", dname, me)) return -2;

  // moving objects and sensors (chapter 7)
  if (sGetApi(&vs_define_moving_objects, dll, "vs_define_moving_objects", dname, me)) return -2;
  if (sGetApi(&vs_define_sensors, dll, "vs_define_sensors", dname, me)) return -2;
  if (sGetApi(&vs_free_sensors_and_objects, dll, "vs_free_sensors_and_objects", dname, me)) return -2;

  // saving and restoring the model state (chapter 8)
  if (sGetApi(&vs_free_saved_states, dll, "vs_free_saved_states", dname, me)) return -2;
  if (sGetApi(&vs_get_request_to_restore, dll, "vs_get_request_to_restore", dname, me)) return -2;
  if (sGetApi(&vs_get_request_to_save, dll, "vs_get_request_to_save", dname, me)) return -2;
  if (sGetApi(&vs_get_saved_state_time, dll, "vs_get_saved_state_time", dname, me)) return -2;
  if (sGetApi(&vs_restore_state, dll, "vs_restore_state", dname, me)) return -2;
  if (sGetApi(&vs_save_state, dll, "vs_save_state", dname, me)) return -2;
  if (sGetApi(&vs_set_request_to_restore, dll, "vs_set_request_to_restore", dname, me)) return -2;
  if (sGetApi(&vs_start_save_timer, dll, "vs_start_save_timer", dname, me)) return -2;
  if (sGetApi(&vs_stop_save_timer, dll, "vs_stop_save_timer", dname, me)) return -2;

  // managing arrays to support restarts (chapter 8)
  if (sGetApi(&vs_copy_all_state_vars_from_array, dll, "vs_copy_all_state_vars_from_array", dname, me)) return -2;
  if (sGetApi(&vs_copy_all_state_vars_to_array, dll, "vs_copy_all_state_vars_to_array", dname, me)) return -2;
  if (sGetApi(&vs_copy_differential_state_vars_from_array, dll, "vs_copy_differential_state_vars_from_array", dname, me)) return -2;
  if (sGetApi(&vs_copy_differential_state_vars_to_array, dll, "vs_copy_differential_state_vars_to_array", dname, me)) return -2;
  if (sGetApi(&vs_copy_extra_state_vars_from_array, dll, "vs_target_l", dname, me)) return -2;
  if (sGetApi(&vs_copy_extra_state_vars_to_array, dll, "vs_copy_extra_state_vars_to_array", dname, me)) return -2;
  if (sGetApi(&vs_get_export_names, dll, "vs_get_export_names", dname, me)) return -2;
  if (sGetApi(&vs_get_import_names, dll, "vs_get_import_names", dname, me)) return -2;
  if (sGetApi(&vs_get_state_q, dll, "vs_get_state_q", dname, me)) return -2;
  if (sGetApi(&vs_n_derivatives, dll, "vs_n_derivatives", dname, me)) return -2;
  if (sGetApi(&vs_n_extra_state_variables, dll, "vs_n_extra_state_variables", dname, me)) return -2;

  // undocumented functions to support legacy Sensor Ports in Simulink Gen-1
  if (sGetApi(&vs_get_n_export_sensor, dll, "vs_get_n_export_sensor", dname, me)) return -2;
  if (sGetApi(&vs_get_sensor_connections, dll, "vs_get_sensor_connections", dname, me)) return -2;

  // undocumented
  if (sGetApi(&vs_get_lat_pos_of_edge, dll, "vs_get_lat_pos_of_edge", dname, me)) return -2;
  if (sGetApi(&vs_scale_export_vars, dll, "vs_scale_export_vars", dname, me)) return -2;

 // legacy stuff (Appendix)
  if (sGetApi(&vs_add_echo_header, dll, "vs_add_echo_header", dname, me)) return -2;
  if (sGetApi(&vs_copy_import_vars, dll, "vs_copy_import_vars", dname, me)) return -2;
  if (sGetApi(&vs_copy_io, dll, "vs_copy_io", dname, me)) return -2;
  if (sGetApi(&vs_define_import, dll, "vs_define_import", dname, me)) return -2;
  if (sGetApi(&vs_define_indexed_parameter_array, dll, "vs_define_indexed_parameter_array", dname, me)) return -2;
  if (sGetApi(&vs_define_output, dll, "vs_define_output", dname, me)) return -2;
  if (sGetApi(&vs_define_parameter, dll, "vs_define_parameter", dname, me)) return -2;
  if (sGetApi(&vs_define_parameter_int, dll, "vs_define_parameter_int", dname, me)) return -2;
  if (sGetApi(&vs_define_variable, dll, "vs_define_variable", dname, me)) return -2;
  if (sGetApi(&vs_free_all, dll, "vs_free_all", dname, me)) return -2;
  if (sGetApi(&vs_integrate_IO, dll, "vs_integrate_IO", dname, me)) return -2;
  if (sGetApi(&vs_integrate_io_2, dll, "vs_integrate_io_2", dname, me)) return -2;
  if (sGetApi(&vs_opt_all_write, dll, "vs_opt_all_write", dname, me)) return -2;
  if (sGetApi(&vs_scale_import_vars, dll, "vs_scale_import_vars", dname, me)) return -2;
  if (sGetApi(&vs_set_sym_real, dll, "vs_set_sym_real", dname, me)) return -2;
  if (sGetApi(&vs_set_units, dll, "vs_set_units", dname, me)) return -2;
  if (sGetApi(&vs_terminate, dll, "vs_terminate", dname, me)) return -2;

  if (sGetApi(&vs_setdef, dll, "vs_setdef", dname, me)) return -2;
  if (sGetApi(&vs_read, dll, "vs_read", dname, me)) return -2;
  if (sGetApi(&vs_install_calc_func, dll, "vs_install_calc_func", dname, me)) return -2;
  return 0;
}

/* ----------------------------------------------------------------------------
   Legacy get functions for subsets of the API functions.
---------------------------------------------------------------------------- */
int vs_get_api_basic (HMODULE dll, const char *dname) {
  char *me = "vs_get_api_basic";
  if (dll == NULL) return sPrintErrorNoDll(me, dname);

  if (sGetApi(&vs_bar_graph_update, dll, "vs_bar_graph_update", dname, me)) return -2;
  if (sGetApi(&vs_copy_io, dll, "vs_copy_io", dname, me)) return -2;
  if (sGetApi(&vs_error_occurred, dll, "vs_error_occurred", dname, me)) return -2;
  if (sGetApi(&vs_free_all, dll, "vs_free_all", dname, me)) return -2;
  if (sGetApi(&vs_get_error_message, dll, "vs_get_error_message", dname, me)) return -2;
  if (sGetApi(&vs_get_output_message, dll, "vs_get_output_message", dname, me)) return -2;
  if (sGetApi(&vs_get_tstep, dll, "vs_get_tstep", dname, me)) return -2;
  if (sGetApi(&vs_get_version_product, dll, "vs_get_version_product", dname, me)) return -2;
  if (sGetApi(&vs_get_version_vs, dll, "vs_get_version_vs", dname, me)) return -2;
  if (sGetApi(&vs_initialize, dll, "vs_initialize", dname, me)) return -2;
  if (sGetApi(&vs_integrate, dll, "vs_integrate", dname, me)) return -2;
  if (sGetApi(&vs_integrate_io, dll, "vs_integrate_io", dname, me)) return -2;
  if (sGetApi(&vs_opt_pause, dll, "vs_opt_pause", dname, me)) return -2;
  if (sGetApi(&vs_read_configuration, dll, "vs_read_configuration", dname, me)) return -2;
  if (sGetApi(&vs_set_macro, dll, "vs_set_macro", dname, me)) return -2;
  if (sGetApi(&vs_setdef_and_read, dll, "vs_setdef_and_read", dname, me)) return -2;
  if (sGetApi(&vs_stop_run, dll, "vs_stop_run", dname, me)) return -2;
  if (sGetApi(&vs_terminate, dll, "vs_terminate", dname, me)) return -2;

  return 0;
}


int vs_get_api_extend (HMODULE dll, const char *dname) {
  char *me = "vs_get_api_extend";
  if (dll == NULL) return sPrintErrorNoDll(me, dname);

  if (sGetApi(&vs_define_import, dll, "vs_define_import", dname, me)) return -2;
  if (sGetApi(&vs_define_output, dll, "vs_define_output", dname, me)) return -2;
  if (sGetApi(&vs_define_parameter, dll, "vs_define_parameter", dname, me)) return -2;
  if (sGetApi(&vs_define_units, dll, "vs_define_units", dname, me)) return -2;
  if (sGetApi(&vs_define_variable, dll, "vs_define_variable", dname, me)) return -2;
  if (sGetApi(&vs_get_var_ptr, dll, "vs_get_var_ptr", dname, me)) return -2;
  if (sGetApi(&vs_get_var_ptr2, dll, "vs_get_var_ptr2", dname, me)) return -2;
  if (sGetApi(&vs_get_var_ptr_int, dll, "vs_get_var_ptr_int", dname, me)) return -2;
  if (sGetApi(&vs_get_var_ptr_int2, dll, "vs_get_var_ptr_int2", dname, me)) return -2;
  if (sGetApi(&vs_set_units, dll, "vs_set_units", dname, me)) return -2;
  if (sGetApi(&vs_install_calc_func, dll, "vs_install_calc_func", dname, me)) return -2;
  if (sGetApi(&vs_printf, dll, "vs_printf", dname, me)) return -2;
  if (sGetApi(&vs_printf_error, dll, "vs_printf_error", dname, me)) return -2;
  if (sGetApi(&vs_set_sym_int, dll, "vs_set_sym_int", dname, me)) return -2;
  if (sGetApi(&vs_set_sym_real, dll, "vs_set_sym_real", dname, me)) return -2;
  if (sGetApi(&vs_set_sym_attribute, dll, "vs_set_sym_attribute", dname, me)) return -2;
  if (sGetApi(&vs_read_next_line, dll, "vs_read_next_line", dname, me)) return -2;
  if (sGetApi(&vs_write_to_echo_file, dll, "vs_write_to_echo_file", dname, me)) return -2;
  if (sGetApi(&vs_write_header_to_echo_file, dll, "vs_write_header_to_echo_file", dname, me)) return -2;
  if (sGetApi(&vs_write_f_to_echo_file, dll, "vs_write_f_to_echo_file", dname, me)) return -2;
  if (sGetApi(&vs_write_i_to_echo_file, dll, "vs_write_i_to_echo_file", dname, me)) return -2;
  if (sGetApi(&vs_get_sym_attribute, dll, "vs_get_sym_attribute", dname, me)) return -2;
  if (sGetApi(&vs_define_parameter_int, dll, "vs_define_parameter_int", dname, me)) return -2;
  if (sGetApi(&vs_set_opt_error_dialog, dll, "vs_set_opt_error_dialog", dname, me)) return -2;

  return 0;
}

int vs_get_api_road (HMODULE dll, const char *dname) {
  char *me = "vs_get_api_road";
  if (dll == NULL) return sPrintErrorNoDll(me, dname);

  if (sGetApi(&vs_road_s, dll, "vs_road_s", dname, me)) return -2;
  if (sGetApi(&vs_road_l, dll, "vs_road_l", dname, me)) return -2;
  if (sGetApi(&vs_road_x, dll, "vs_road_x", dname, me)) return -2;
  if (sGetApi(&vs_road_y, dll, "vs_road_y", dname, me)) return -2;
  if (sGetApi(&vs_road_z, dll, "vs_road_z", dname, me)) return -2;
  if (sGetApi(&vs_road_yaw, dll, "vs_road_yaw", dname, me)) return -2;
  if (sGetApi(&vs_s_loop, dll, "vs_s_loop", dname, me)) return -2;
  if (sGetApi(&vs_get_dzds_dzdl, dll, "vs_get_dzds_dzdl", dname, me)) return -2;
  if (sGetApi(&vs_get_road_start_stop, dll, "vs_get_road_start_stop", dname, me)) return -2;
  if (sGetApi(&vs_get_road_xyz, dll, "vs_get_road_xyz", dname, me)) return -2;
  if (sGetApi(&vs_get_road_contact, dll, "vs_get_road_contact", dname, me)) return -2;
  if (sGetApi(&vs_target_l, dll, "vs_target_l", dname, me)) return -2;
  if (sGetApi(&vs_get_dzds_dzdl_i, dll, "vs_get_dzds_dzdl_i", dname, me)) return -2;
  if (sGetApi(&vs_get_road_contact_sl, dll, "vs_get_road_contact_sl", dname, me)) return -2;
  if (sGetApi(&vs_road_curv_i, dll, "vs_road_curv_i", dname, me)) return -2;
  if (sGetApi(&vs_road_l_i, dll, "vs_road_l_i", dname, me)) return -2;
  if (sGetApi(&vs_road_pitch_sl_i, dll, "vs_road_pitch_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_road_roll_sl_i, dll, "vs_road_roll_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_road_s_i, dll, "vs_road_s_i", dname, me)) return -2;
  if (sGetApi(&vs_road_x_i, dll, "vs_road_x_i", dname, me)) return -2;
  if (sGetApi(&vs_road_y_i, dll, "vs_road_y_i", dname, me)) return -2;
  if (sGetApi(&vs_road_yaw_i, dll, "vs_road_yaw_i", dname, me)) return -2;
  if (sGetApi(&vs_road_z_i, dll, "vs_road_z_i", dname, me)) return -2;
  if (sGetApi(&vs_road_z_sl_i, dll, "vs_road_z_sl_i", dname, me)) return -2;

  if (sGetApi(&vs_path_x_i, dll, "vs_path_x_i", dname, me)) return -2;
  if (sGetApi(&vs_path_y_i, dll, "vs_path_y_i", dname, me)) return -2;
  if (sGetApi(&vs_path_x_sl_i, dll, "vs_path_x_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_path_y_sl_i, dll, "vs_path_y_sl_i", dname, me)) return -2;
  if (sGetApi(&vs_s_path_loop, dll, "vs_s_path_loop", dname, me)) return -2;

  return 0;
}

int vs_get_api_install_external (HMODULE dll, const char *dname) {
  char *me = "vs_get_api_install_external";
  if (dll == NULL) return sPrintErrorNoDll(me, dname);

  if (sGetApi(&vs_run, dll, "vs_run", dname, me)) return -2;

  if (sGetApi(&vs_install_calc_function, dll, "vs_install_calc_function", dname, me)) return -2;
  if (sGetApi(&vs_install_echo_function, dll, "vs_install_echo_function", dname, me)) return -2;
  if (sGetApi(&vs_install_setdef_function, dll, "vs_install_setdef_function", dname, me)) return -2;
  if (sGetApi(&vs_install_scan_function, dll, "vs_install_scan_function", dname, me)) return -2;
  if (sGetApi(&vs_install_free_function, dll, "vs_install_free_function", dname, me)) return -2;
  if (sGetApi(&vs_install_status_function, dll, "vs_install_status_function", dname, me)) return -2;
  if (sGetApi(&vs_install_calc_function2, dll, "vs_install_calc_function2", dname, me)) return -2;
  if (sGetApi(&vs_install_echo_function2, dll, "vs_install_echo_function2", dname, me)) return -2;
  if (sGetApi(&vs_install_setdef_function2, dll, "vs_install_setdef_function2", dname, me)) return -2;
  if (sGetApi(&vs_install_scan_function2, dll, "vs_install_scan_function2", dname, me)) return -2;
  if (sGetApi(&vs_install_free_function2, dll, "vs_install_free_function2", dname, me)) return -2;

  return 0;
}
