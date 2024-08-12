/* VS API functions in a VS DLL. Listed more or less how they appear in the VS API
   reference manual.

  Log:
  Aug 31, 16. M. Sayers. Changes for CarSim 2017; 184 functions
  Nov 13, 15. Changes for CarSim 2016
  Sep 24, 14. Changes for CarSim 9.0.
  Mar 16, 11. M. Sayers. added traffic, sensor, and table functions.
  May 04, 10. M. Sayers. updated to include more new functions and add __cdecl cast.
  May 20, 09. M. Sayers. updated to include vs_run and new functions for CarSim 8.0.
  Jun 06, 08. M. Sayers. created, based on previous version that used a structure.
  */

#ifndef _vs_api_h_INCLUDED
#define _vs_api_h_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

#define PRODUCT_VERSION "2018.2"

#include "vs_deftypes.h"

// ENC = External Naming Convention
#if (defined(_WIN32) || defined(_WIN64))
#include <windows.h>
#define ENC __cdecl
#else
#define HMODULE void*
#define ENC
#endif

// simple run function (chapter 2)
int (ENC *vs_run)(const char *simfile);

// more control over a run (chapter 2)
int     (ENC *vs_bar_graph_update)(int *);
vs_bool (ENC *vs_error_occurred)(void);
void    (ENC *vs_initialize) (vs_real t, void (*ext_calc) (vs_real, vs_ext_loc),
          void (*ext_echo) (vs_ext_loc));
int     (ENC *vs_integrate)(vs_real *t,
                        void (*ext_eq_in) (vs_real, vs_ext_loc));
void    (ENC *vs_set_macro)(const char *macro_name, const char* macro_value);
vs_real (ENC *vs_setdef_and_read)(const char *simfile, void (*ext_setdef) (void),
            int (*ext_scan) (char *, char *));
int   (ENC *vs_statement)(const char *key, const char *buffer, int stopError);
int   (ENC *vs_stop_run)(void);
void  (ENC *vs_terminate_run) (vs_real t);

// simulating with import/export arrays (chapter 4)
void    (ENC *vs_copy_export_vars) (vs_real *exports);
vs_real (ENC *vs_get_tstep)(void);
int     (ENC *vs_integrate_io)(vs_real t, vs_real *imports, vs_real *exports);
void    (ENC *vs_read_configuration) (const char *simfile, int *n_import,
           int *n_export, vs_real *tstart, vs_real *tstop, vs_real *tstep);

// utility functions: conditions (chapter 5)
int     (ENC *vs_during_event)(void);
vs_real (ENC *vs_get_time)(void);
vs_bool (ENC *vs_opt_pause)(void);

// utility functions: messages (chapter 5)
void  (ENC *vs_clear_error_message) (void);
void  (ENC *vs_clear_output_message) (void);
char *(ENC *vs_get_error_message)(void);
char *(ENC *vs_get_output_message)(void);
char *(ENC *vs_get_version_model)(void);
char *(ENC *vs_get_version_product)(void);
char *(ENC *vs_get_version_vs)(void);
void  (ENC *vs_printf)(const char *format, ...);
void  (ENC *vs_printf_error)(const char *format, ...);

// utility functions: file names and directories (chapter 5)
char *(ENC *vs_get_database_dir)(void);
char *(ENC *vs_get_echofile_name)(void);
char *(ENC *vs_get_endfile_name)(void);
char *(ENC *vs_get_erdfile_name)(void);
char *(ENC *vs_get_filebase_name)(void);
char *(ENC *vs_get_infile_name)(void);
char *(ENC *vs_get_logfile_name)(void);
char *(ENC *vs_get_simfile_name)(void);

// utility functions: writing documentation files (chapter 5)
void  (ENC *vs_write_doc_files) (const char *simfile, const char *root, int model,
         int import, int output);

// loading the solver: functions from vs_get_api.c (chapter 6)
HMODULE vs_load_library(char* pathDLL);
void vs_free_library (HMODULE dll);
int vs_get_dll_path(const char *simfile, char *pathDLL);
int vs_get_api(HMODULE dll, const char *dll_fname);
void vs_set_error_message_function(int(*errorMsgFunc)(const char *format, ...));

// installation of callback functions (chapter 6)
void (ENC *vs_install_calc_function) (void (*calc) (vs_real time, vs_ext_loc where));
void (ENC *vs_install_echo_function) (void (*echo) (vs_ext_loc where));
void (ENC *vs_install_setdef_function) (void (*setdef) (void));
void (ENC *vs_install_scan_function) (vs_bool (*scan)(char *, char *));
void (ENC *vs_install_free_function) (void (*free) (void));
void (ENC *vs_install_status_function) (status_func_t extStatusFunc, void* userData);
void (ENC *vs_install_calc_function2) (void (*calc) (vs_real time, vs_ext_loc where,
        void* userData), void* userData);
void (ENC *vs_install_echo_function2)
        (void (*echo) (vs_ext_loc where, void* userData), void* userData);
void (ENC *vs_install_setdef_function2)
        (void (*setdef) (void* userData), void* userData);
void (ENC *vs_install_scan_function2) (void (*scan)
        (char *, char *, void* userData), void* userData);
void (ENC *vs_install_free_function2) (void (*func) (void* userData), void* userData);

// functions for interacting with the VS math model (chapter 7)
int (ENC *vs_define_imp)(char *keyword, vs_real *real, char *units, int native,
                              char *component, char *desc);
int (ENC *vs_define_ipar_1d)(char *key, int n, char *index);
int (ENC *vs_define_ipar_2d)(char *key, int n1, char *index1, int n2, char *index2);
int (ENC *vs_define_ipar_3d)(char *key, int n1, char *index1, int n2, char *index2,
       int n3, char *index3);
int (ENC *vs_define_ipar_4d)(char *key, int n1, char *index1, int n2, char *index2,
       int n3, char *index3, int n4, char *index4);
int (ENC *vs_define_ipar_5d)(char *key, int n1, char *index1, int n2, char *index2,
       int n3, char *index3, int n4, char *index4, int n5, char *index5);
int (ENC *vs_define_out)(char *shortname, char *longname, vs_real *real,
                              char *units, char *genname, char *bodyname, char *comp);
int (ENC *vs_define_par)(char *keyword, void *par, vs_real value, char *units,
                              int visible, int init, char *desc);
int (ENC *vs_define_sypar)(char *keyword, void *par, vs_real value, char *units,
       int visible,  char *desc);
int (ENC *vs_define_sv)(char *keyword, vs_real *real, vs_real value, char *units,
       char *desc);
void  (ENC *vs_define_units) (char *desc, vs_real gain);

vs_real (ENC *vs_get_numeric_value)(char *buffer);
int     (ENC *vs_get_sym_attribute)(int id, vs_sym_attr_type type, void **att);
int     (ENC *vs_get_var_id)(char *keyword, vs_sym_attr_type *type);
vs_real *(ENC *vs_get_var_ptr)(char *keyword);
vs_real *(ENC *vs_get_var_ptr2)(const char *keyword, vs_keyword_type *outopt_keywordType, vs_bool allowErrors);
int     *(ENC *vs_get_var_ptr_int)(char *keyword);
int     *(ENC *vs_get_var_ptr_int2)(const char *keyword, vs_keyword_type* outopt_keywordType, vs_bool allowErrors);
vs_bool (ENC *vs_have_keyword_in_database)(char *keyword);
vs_real (ENC *vs_import_result)(int id, vs_real native);
int   (ENC *vs_import_type)(int id);
void  (ENC *vs_install_symbolic_func) (char *name, void *func, int n_args);
int   (ENC *vs_install_keyword_alias)(char *existing, char *alias);
void  (ENC *vs_put_on_free_list) (void *thing);
void  (ENC *vs_set_stop_run) (vs_real stop_gt_0, const char *format, ...);
int   (ENC *vs_set_sym_attribute)(int id, vs_sym_attr_type type, const void *att);
int   (ENC *vs_set_sym_int)(int id, vs_sym_attr_type dataType, int value);
char *(ENC *vs_string_copy_internal)(char **target, char *source);
void  (ENC *vs_set_opt_error_dialog)(int value);
void  (ENC *vs_set_opt_abortive_socket_close)(int value);

// working with simulation files (chapter 7)
int  (ENC *vs_add_new_par_group)(char *text);
int  (ENC *vs_insert_echo_blank_line)(void);
int  (ENC *vs_insert_echo_comment)(char *desc);
int  (ENC *vs_get_active_par_group)(void);
void (ENC *vs_set_active_par_group) (int i);

void (ENC *vs_read_next_line) (char *buffer, int n);
void (ENC *vs_write_f_to_echo_file) (char *key, vs_real , char *doc);
void (ENC *vs_write_header_to_echo_file) (char *buffer);
void (ENC *vs_write_i_to_echo_file) (char *key, int , char *doc);
void (ENC *vs_write_to_echo_file) (const char *format, ...);
void (ENC *vs_write_to_logfile) (int level, const char *format, ...);

// configurable table functions (chapter 7)
int     (ENC *vs_define_table)(char *root, int ntab, int ninst);
vs_real (ENC *vs_table_calc)(int index, vs_real xcol, vs_real x, int itab, int inst);
int     (ENC *vs_table_index)(char *name);
int     (ENC *vs_table_ninst)(int index);
int     (ENC *vs_table_ntab)(int index);

// 2D reference path properties (chapter 7)
void (ENC *vs_get_road_start_stop) (vs_real *start, vs_real *stop);
vs_real (ENC *vs_path_curv_i)(vs_real s, vs_real inst);
vs_real (ENC *vs_road_curv_i)(vs_real s, vs_real inst);
vs_real (ENC *vs_path_l_i)(vs_real x, vs_real y, vs_real inst);
vs_real (ENC *vs_road_l)(vs_real x, vs_real y);
vs_real (ENC *vs_road_l_i)(vs_real x, vs_real y, vs_real inst);
vs_real (ENC *vs_path_s_i)(vs_real x, vs_real y, vs_real inst);
vs_real (ENC *vs_road_s)(vs_real x, vs_real y);
vs_real (ENC *vs_road_s_i)(vs_real x, vs_real y, vs_real inst);
vs_real (ENC *vs_path_x_i)(vs_real sy, vs_real inst);
vs_real (ENC *vs_road_x)(vs_real s);
vs_real (ENC *vs_road_x_i)(vs_real sy, vs_real inst);
vs_real (ENC *vs_path_x_sl_i)(vs_real s, vs_real l, vs_real inst);
vs_real (ENC *vs_road_x_sl_i)(vs_real s, vs_real l, vs_real inst);
vs_real (ENC *vs_path_y_i)(vs_real s, vs_real inst);
vs_real (ENC *vs_road_y)(vs_real s);
vs_real (ENC *vs_road_y_i)(vs_real sy, vs_real inst);
vs_real (ENC *vs_path_y_sl_i)(vs_real s, vs_real l, vs_real inst);
vs_real (ENC *vs_road_y_sl_i)(vs_real s, vs_real l, vs_real inst);
vs_real (ENC *vs_path_yaw_i)(vs_real sta, vs_real directiony, vs_real inst);
vs_real (ENC *vs_road_yaw)(vs_real sta, vs_real direction);
vs_real (ENC *vs_road_yaw_i)(vs_real sta, vs_real directiony, vs_real inst);
vs_real (ENC *vs_s_loop)(vs_real s);
vs_real (ENC *vs_s_path_loop)(vs_real s);
vs_real (ENC *vs_target_heading)(vs_real s);
vs_real (ENC *vs_target_l)(vs_real s);

// 3D properties of current road (chapter 7)
void (ENC *vs_get_dzds_dzdl) (vs_real s, vs_real l, vs_real *dzds, vs_real *dzdl);
void (ENC *vs_get_dzds_dzdl_i) (vs_real s, vs_real l, vs_real *dzds,
        vs_real *dzdl, vs_real inst);
void (ENC *vs_get_road_contact) (vs_real y, vs_real x, int inst, vs_real *z,
        vs_real *dzdy, vs_real *dzdx, vs_real *mu);
void (ENC *vs_get_road_contact_sl) (vs_real s, vs_real l, int inst, vs_real *z,
        vs_real *dzds, vs_real *dzdl, vs_real *mu);
void (ENC *vs_get_road_xyz) (vs_real s, vs_real l, vs_real *x, vs_real *y, vs_real *z);
vs_real (ENC *vs_road_pitch_sl_i)(vs_real s, vs_real l, vs_real yaw, vs_real inst);
vs_real (ENC *vs_road_roll_sl_i)(vs_real s, vs_real l, vs_real yaw, vs_real inst);
vs_real (ENC *vs_road_z)(vs_real x, vs_real y);
vs_real (ENC *vs_road_z_i)(vs_real x, vs_real yy, vs_real inst);
vs_real (ENC *vs_road_z_sl_i)(vs_real s, vs_real l, vs_real inst);

// Functions the make use of user ID for paths (chapter 7)
vs_real (ENC *vs_path_sstart_id)(vs_real user_id);
vs_real (ENC *vs_path_sstop_id)(vs_real user_id);
vs_real (ENC *vs_path_length_id)(vs_real user_id);
vs_real (ENC *vs_path_is_looped_id)(vs_real user_id);
vs_real (ENC *vs_path_s_id)(vs_real x, vs_real y, vs_real user_id, vs_real inst);
vs_real (ENC *vs_path_l_id)(vs_real x, vs_real y, vs_real user_id, vs_real inst);
vs_real (ENC *vs_path_x_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_path_y_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_path_dxds_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_path_dyds_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_path_dxdl_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_path_dydl_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_path_curv_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_path_yaw_id)(vs_real s, vs_real path_id, vs_real ltarg_id, vs_real inst);

// Functions that make use of user ID for roads (chapter 7)
vs_real (ENC *vs_road_sstart_id)(vs_real user_id);
vs_real (ENC *vs_road_sstop_id)(vs_real user_id);
vs_real (ENC *vs_road_length_id)(vs_real user_id);
vs_real (ENC *vs_road_is_looped_id)(vs_real user_id);
vs_real (ENC *vs_road_s_id)(vs_real x, vs_real y, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_l_id)(vs_real x, vs_real y, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_x_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_y_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_dxds_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_dyds_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_dxdl_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_dydl_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_curv_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_z_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_dzds_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (ENC *vs_road_dzdl_id)(vs_real s, vs_real l, vs_real user_id, vs_real inst);

// moving objects and sensors (chapter 7)
int     (ENC *vs_define_moving_objects)(int n);
int     (ENC *vs_define_sensors)(int n);
void    (ENC *vs_free_sensors_and_objects)(void);

// saving and restoring the model state (chapter 8)
void    (ENC *vs_free_saved_states) (void);
int     (ENC *vs_get_request_to_restore)(void);
int     (ENC *vs_get_request_to_save)(void);
vs_real (ENC *vs_get_saved_state_time)(vs_real t);
vs_real (ENC *vs_restore_state)(void);
void    (ENC *vs_save_state) (void);
void    (ENC *vs_set_request_to_restore) (vs_real t);
void    (ENC *vs_start_save_timer) (vs_real t);
void    (ENC *vs_stop_save_timer) (void);

// managing arrays to support restarts (chapter 8)
void (ENC *vs_copy_all_state_vars_from_array) (vs_real *array);
void (ENC *vs_copy_all_state_vars_to_array) (vs_real *array);
void (ENC *vs_copy_differential_state_vars_from_array) (vs_real *array);
void (ENC *vs_copy_differential_state_vars_to_array) (vs_real *array);
void (ENC *vs_copy_extra_state_vars_from_array) (vs_real *array);
void (ENC *vs_copy_extra_state_vars_to_array) (vs_real *array);
int  (ENC *vs_get_export_names)(char **expNames);
int  (ENC *vs_get_import_names)(char **impNames);
void (ENC *vs_get_state_q) (vs_real *array);
int  (ENC *vs_n_derivatives)(void);
int  (ENC *vs_n_extra_state_variables)(void);

// undocumented functions to support legacy Sensor Ports in Simulink Gen-1
int (ENC *vs_get_n_export_sensor)(int *max_connections);
int (ENC *vs_get_sensor_connections)(vs_real *connect);

int  (ENC *vs_get_lat_pos_of_edge)(int edge, vs_real s, int opt_road, vs_real *l);
void (ENC *vs_scale_export_vars) (void);

// Alternative legacy functions (Appendix)
void (ENC *vs_add_echo_header) (const char *text);
void (ENC *vs_copy_import_vars) (vs_real *imports);
void (ENC *vs_copy_io) (vs_real *imports, vs_real *exports);
int  (ENC *vs_define_import)(char *keyword, char *desc, vs_real *real, char *);
int  (ENC *vs_define_indexed_parameter_array)(char *keyword);
int  (ENC *vs_define_output)(char *shortname, char *longname, vs_real *real, char *);
int  (ENC *vs_define_parameter)(char *keyword, char *desc, vs_real *, char *);
int  (ENC *vs_define_parameter_int)(char *keyword, char *desc, int *);
int   (ENC *vs_define_variable)(char *keyword, char *desc, vs_real *);
void (ENC *vs_free_all) (void);
vs_bool (ENC *vs_integrate_io_2)(vs_real t, vs_real *imports, vs_real *exports,
           void (*ext_calc) (vs_real, vs_ext_loc));
int   (ENC *vs_integrate_IO)(vs_real t, vs_real *imports, vs_real *exports);
void  (ENC *vs_opt_all_write) (int arg); // Jan 20, 2015
void  (ENC *vs_scale_import_vars) (void);
int   (ENC *vs_set_sym_real)(int id, vs_sym_attr_type dataType, vs_real value);
void  (ENC *vs_set_units) (char *var_keyword, char *units_keyword);
void  (ENC *vs_terminate) (vs_real t, void (*ext_echo) (vs_ext_loc));

// Legacy functions from get_api.c (Appendix)
int vs_get_api_basic(HMODULE dll, const char *dll_fname);
int vs_get_api_extend(HMODULE dll, const char *dll_fname);
int vs_get_api_road(HMODULE dll, const char *dll_fname);
int vs_get_api_install_external(HMODULE dll, const char *dll_fname);

// Deprecated functions (Appendix)
void (ENC *vs_install_calc_func) (char *name, void *func);
void (ENC *vs_setdef) (void(*ext_setdef) (void));
vs_real (ENC *vs_read)(const char *simfile,
           vs_bool (*ext_scan) (char *key, char *buffer));

#ifdef __cplusplus
}
#endif

#endif  // _vs_api_h_INCLUDED
