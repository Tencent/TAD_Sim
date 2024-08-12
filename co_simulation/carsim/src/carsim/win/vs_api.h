/* VS API functions in a VS DLL. Listed more or less how they appear in the VS API
   reference manual.

  Log:
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

#include <windows.h>
#include "vs_deftypes.h"


/** Specify a custom printf-style function (callback) to handle error messages.
  This will override the default error message handler. Pass in a pointer to an
  alternate message handling function (e.g. printf) to change how functions in
  vs_get_api.c will report error messages.
*/
void vs_set_custom_api_error_message_function( int( *errorMsgFunc ) (const char *format, ...) );

// Use these two functions to load the VS DLL and get all of the API functions
int vs_get_dll_path (const char *simfile, char *pathDLL);
int vs_get_api (HMODULE dll, const char *dll_fname);

int vs_get_api_basic (HMODULE dll, const char *dll_fname); // legacy
int vs_get_api_extend (HMODULE dll, const char *dll_fname); // legacy
int vs_get_api_road (HMODULE dll, const char *dll_fname); // legacy
int vs_get_api_install_external (HMODULE dll, const char *dll_fname); // legacy

// simple run function (chapter 2)
int      (__cdecl *vs_run) (const char *simfile);

// managing import/export arrays (chapter 4)
int      (__cdecl *vs_statement) (const char *key, const char *buffer, int stopError);
void     (__cdecl *vs_copy_export_vars) (vs_real *exports);
void     (__cdecl *vs_copy_import_vars) (vs_real *imports);
void     (__cdecl *vs_copy_io) (vs_real *imports, vs_real *exports);
int      (__cdecl *vs_integrate_io) (vs_real t, vs_real *imports, vs_real *exports);
int      (__cdecl *vs_integrate_IO) (vs_real t, vs_real *imports, vs_real *exports);
void     (__cdecl *vs_read_configuration) (const char *simfile, int *n_import,
                                  int *n_export, vs_real *tstart, vs_real *tstop,
                                  vs_real *tstep);
void     (__cdecl *vs_scale_import_vars) (void);
void     (__cdecl *vs_terminate_run) (vs_real t);

// utility functions: conditions (chapter 5)
int      (__cdecl *vs_during_event) (void);
vs_bool  (__cdecl *vs_error_occurred) (void);
vs_real  (__cdecl *vs_get_tstep) (void);
vs_bool  (__cdecl *vs_opt_pause)(void);

// utility functions: messages (chapter 5)
void     (__cdecl *vs_clear_error_message) (void);
void     (__cdecl *vs_clear_output_message) (void);

char    *(__cdecl *vs_get_echofile_name) (void);
char    *(__cdecl *vs_get_endfile_name) (void);
char    *(__cdecl *vs_get_erdfile_name) (void);
char    *(__cdecl *vs_get_error_message) (void);
char    *(__cdecl *vs_get_infile_name) (void);
char    *(__cdecl *vs_get_logfile_name) (void);
char    *(__cdecl *vs_get_output_message) (void);
char    *(__cdecl *vs_get_simfile_name) (void);
char    *(__cdecl *vs_get_version_model) (void);
char    *(__cdecl *vs_get_version_product) (void);
char    *(__cdecl *vs_get_version_vs) (void);
void     (__cdecl *vs_printf) (const char *format, ...);
void     (__cdecl *vs_printf_error) (const char *format, ...);

// installation of callback functions (chapter 6)
void     (__cdecl *vs_install_calc_function) (void (*calc) (vs_real time, vs_ext_loc where));
void     (__cdecl *vs_install_echo_function) (void (*echo) (vs_ext_loc where));
void     (__cdecl *vs_install_setdef_function) (void (*setdef) (void));
void     (__cdecl *vs_install_scan_function) (vs_bool (*scan) (char *, char *));
void     (__cdecl *vs_install_free_function) (void (*free) (void));
void     (__cdecl *vs_install_calc_function2) (void (*calc) (vs_real time, vs_ext_loc where, void* userData), void* userData);
void     (__cdecl *vs_install_echo_function2) (void (*echo) (vs_ext_loc where, void* userData), void* userData);
void     (__cdecl *vs_install_setdef_function2) (void (*setdef) (void* userData), void* userData);
void     (__cdecl *vs_install_scan_function2) (void (*scan) (char *, char *, void* userData), void* userData);
void     (__cdecl *vs_install_free_function2) (void (*func) (void* userData), void* userData);

// more detailed control of run (chapter 6)
int      (__cdecl *vs_bar_graph_update) (int *);
void     (__cdecl *vs_free_all) (void);
void     (__cdecl *vs_initialize) (vs_real t,
                          void (*ext_calc) (vs_real, vs_ext_loc),
                          void (*ext_echo) (vs_ext_loc));
int      (__cdecl *vs_integrate) (vs_real *t,
                         void (*ext_eq_in) (vs_real, vs_ext_loc));
vs_bool  (__cdecl *vs_integrate_io_2) (vs_real t, vs_real *imports, vs_real *exports,
                                          void (*ext_calc) (vs_real, vs_ext_loc));
void     (__cdecl *vs_setdef) (void(*ext_setdef) (void));
vs_real  (__cdecl *vs_read) (const char *simfile, vs_bool (*ext_scan) (char *key, char *buffer));
vs_real  (__cdecl *vs_setdef_and_read) (const char *simfile, void (*ext_setdef) (void),
                               int (*ext_scan) (char *, char *));
int      (__cdecl *vs_stop_run) (void);
void     (__cdecl *vs_terminate) (vs_real t, void (*ext_echo) (vs_ext_loc));

// functions for interacting with the VS math model (chapter 7)
int      (__cdecl *vs_define_import) (char *keyword, char *desc, vs_real *real, char *);
int      (__cdecl *vs_define_indexed_parameter_array) (char *keyword);
int      (__cdecl *vs_define_output) (char *shortname, char *longname, vs_real *real, char *);
int      (__cdecl *vs_define_parameter) (char *keyword, char *desc, vs_real *, char *);
int      (__cdecl *vs_define_parameter_int) (char *keyword, char *desc, int *);
void   (__cdecl *vs_add_echo_header) (const char *text);
void     (__cdecl *vs_define_units) (char *desc, vs_real gain);
int      (__cdecl *vs_define_variable) (char *keyword, char *desc, vs_real *);
int      (__cdecl *vs_get_sym_attribute) (int id, vs_sym_attr_type type, void **att);
int      (__cdecl *vs_get_var_id) (char *keyword, vs_sym_attr_type *type);
vs_real *(__cdecl *vs_get_var_ptr) (char *keyword);
int     *(__cdecl *vs_get_var_ptr_int) (char *keyword);
vs_bool  (__cdecl *vs_have_keyword_in_database) (char *keyword);
vs_real  (__cdecl *vs_import_result) (int id, vs_real native);
void     (__cdecl *vs_install_calc_func) (char *name, void *func); // obsolete
void     (__cdecl *vs_install_symbolic_func) (char *name, void *func, int n_args);
int      (__cdecl *vs_install_keyword_alias) (char *existing, char *alias);
void     (__cdecl *vs_read_next_line) (char *buffer, int n);
void     (__cdecl *vs_set_stop_run) (vs_real stop_gt_0, const char *format, ...);
int      (__cdecl *vs_set_sym_attribute) (int id, vs_sym_attr_type type, const void *att);
int      (__cdecl *vs_set_sym_int) (int id, vs_sym_attr_type dataType, int value);
int      (__cdecl *vs_set_sym_real) (int id, vs_sym_attr_type dataType, vs_real value);
void     (__cdecl *vs_set_units) (char *var_keyword, char *units_keyword);
char    *(__cdecl *vs_string_copy_internal) (char **target, char *source);
void     (__cdecl *vs_write_f_to_echo_file) (char *key, vs_real , char *doc);
void     (__cdecl *vs_write_header_to_echo_file) (char *buffer);
void     (__cdecl *vs_write_i_to_echo_file) (char *key, int , char *doc);
void     (__cdecl *vs_write_to_echo_file) (const char *format, ...);
void     (__cdecl *vs_write_to_logfile) (int level, const char *format, ...);

// 3D road properties (chapter 7)
void    (__cdecl *vs_get_dzds_dzdl) (vs_real s, vs_real l, vs_real *dzds, vs_real *dzdl);
void    (__cdecl *vs_get_dzds_dzdl_i) (vs_real s, vs_real l, vs_real *dzds,
                                       vs_real *dzdl, vs_real inst);
void    (__cdecl *vs_get_road_contact) (vs_real y, vs_real x, int inst, vs_real *z,
                                vs_real *dzdy, vs_real *dzdx, vs_real *mu);
void    (__cdecl *vs_get_road_contact_sl) (vs_real s, vs_real l, int inst, vs_real *z,
                             vs_real *dzds, vs_real *dzdl, vs_real *mu);
void    (__cdecl *vs_get_road_start_stop) (vs_real *start, vs_real *stop);
void    (__cdecl *vs_get_road_xyz) (vs_real s, vs_real l, vs_real *x, vs_real *y,
                            vs_real *z);
vs_real (__cdecl *vs_road_curv_i) (vs_real s, vs_real inst);
vs_real (__cdecl *vs_road_l) (vs_real x, vs_real y);
vs_real (__cdecl *vs_road_l_i) (vs_real x, vs_real y, vs_real inst);
vs_real (__cdecl *vs_road_pitch_sl_i) (vs_real s, vs_real l, vs_real yaw, vs_real inst);
vs_real (__cdecl *vs_road_roll_sl_i) (vs_real s, vs_real l, vs_real yaw, vs_real inst);
vs_real (__cdecl *vs_road_s) (vs_real x, vs_real y);
vs_real (__cdecl *vs_road_s_i) (vs_real x, vs_real y, vs_real inst);
vs_real (__cdecl *vs_road_x) (vs_real s);
vs_real (__cdecl *vs_road_x_i) (vs_real sy, vs_real inst);
vs_real (__cdecl *vs_road_x_sl_i) (vs_real s, vs_real l, vs_real inst);
vs_real (__cdecl *vs_road_y) (vs_real s);
vs_real (__cdecl *vs_road_y_i) (vs_real sy, vs_real inst);
vs_real (__cdecl *vs_road_y_sl_i) (vs_real s, vs_real l, vs_real inst);
vs_real (__cdecl *vs_road_yaw) (vs_real sta, vs_real direction);
vs_real (__cdecl *vs_road_yaw_i) (vs_real sta, vs_real directiony, vs_real inst);
vs_real (__cdecl *vs_road_z) (vs_real x, vs_real y);
vs_real (__cdecl *vs_road_z_i) (vs_real x, vs_real yy, vs_real inst);
vs_real (__cdecl *vs_road_z_sl_i) (vs_real s, vs_real l, vs_real inst);
vs_real (__cdecl *vs_s_loop) (vs_real s);
vs_real (__cdecl *vs_target_l) (vs_real s);
vs_real (__cdecl *vs_target_heading) (vs_real s);

// moving objects and sensors (chapter 7)
int     (__cdecl *vs_define_moving_objects) (int n);
int     (__cdecl *vs_define_sensors) (int n);
void    (__cdecl *vs_free_sensors_and_objects) (void);

// functions to get number of export variables for sensors in Simulink
int     (__cdecl *vs_get_n_export_sensor) (int *max_connections);
int     (__cdecl *vs_get_sensor_connections) (vs_real *connect);

// configurable table functions (chapter 7)
int     (__cdecl *vs_define_table) (char *root, int ntab, int ninst);
vs_real (__cdecl *vs_table_calc) (int index, vs_real xcol, vs_real x, int itab, int inst);
int     (__cdecl *vs_table_index) (char *name);
int     (__cdecl *vs_table_ntab) (int index);
int     (__cdecl *vs_table_ninst) (int index);

void    (__cdecl *vs_copy_table_data) (vs_tab_group *tabg);
int     (__cdecl *vs_install_keyword_tab_group) (vs_tab_group *tabs);
void    (__cdecl *vs_malloc_table_data) (vs_table *tab, int type, int nx, int ny);

// saving and restoring the model state (chapter 8)
void    (__cdecl *vs_free_saved_states) (void);
int     (__cdecl *vs_get_request_to_restore) (void);
int     (__cdecl *vs_get_request_to_save) (void);
vs_real (__cdecl *vs_restore_state) (void);
void    (__cdecl *vs_save_state) (void);
void    (__cdecl *vs_set_request_to_restore) (vs_real t);
void    (__cdecl *vs_start_save_timer) (vs_real t);
void    (__cdecl *vs_stop_save_timer) (void);
vs_real (__cdecl *vs_get_saved_state_time) (vs_real t);

// managing arrays to support restarts (chapter 8)
void    (__cdecl *vs_copy_all_state_vars_from_array) (vs_real *array);
void    (__cdecl *vs_copy_all_state_vars_to_array) (vs_real *array);
void    (__cdecl *vs_copy_differential_state_vars_from_array) (vs_real *array);
void    (__cdecl *vs_copy_differential_state_vars_to_array) (vs_real *array);
void    (__cdecl *vs_copy_extra_state_vars_from_array) (vs_real *array);
void    (__cdecl *vs_copy_extra_state_vars_to_array) (vs_real *array);
int     (__cdecl *vs_get_export_names)(char **expNames);
int     (__cdecl *vs_get_import_names)(char **impNames);
int     (__cdecl *vs_n_derivatives) (void);
int     (__cdecl *vs_n_extra_state_variables) (void);

// necessary but undocumented
int     (__cdecl *vs_get_lat_pos_of_edge) (int edge, vs_real s, int opt_road, vs_real *l);
void    (__cdecl *vs_scale_export_vars) (void);

// more: added with CarSim 9.0
int      (__cdecl *vs_define_imp) (char *keyword, vs_real *real, char *units, int native,
                                   char *component, char *desc);
int      (__cdecl *vs_define_out) (char *shortname, char *longname, vs_real *real,
                                   char *units, char *genname, char *bodyname, char *comp);
int      (__cdecl *vs_define_par) (char *keyword, void *par, vs_real value, char *units,
                                   int visible, int init, char *desc);
int      (__cdecl *vs_define_sypar) (char *keyword, void *par, vs_real value, char *units,
                                     int visible,  char *desc);
int      (__cdecl *vs_define_ipar_1d) (char *key, int n, char *index);
int      (__cdecl *vs_define_ipar_2d) (char *key, int n1, char *index1, int n2, char *index2);
int      (__cdecl *vs_define_ipar_3d) (char *key, int n1, char *index1, int n2, char *index2,
                                       int n3, char *index3);
int      (__cdecl *vs_define_ipar_4d) (char *key, int n1, char *index1, int n2, char *index2,
                                       int n3, char *index3, int n4, char *index4);
int      (__cdecl *vs_define_ipar_5d) (char *key, int n1, char *index1, int n2, char *index2,
                                       int n3, char *index3, int n4, char *index4, int n5,
                                       char *index5);
int      (__cdecl *vs_define_sv) (char *keyword, vs_real *real, vs_real value, char *units,
                                  char *desc);

// New in CarSim 9 (paths and roads)
vs_real (__cdecl *vs_path_sstart_id) (vs_real user_id);
vs_real (__cdecl *vs_path_sstop_id) (vs_real user_id);
vs_real (__cdecl *vs_path_length_id) (vs_real user_id);
vs_real (__cdecl *vs_path_is_looped_id) (vs_real user_id);
vs_real (__cdecl *vs_path_s_id) (vs_real x, vs_real y, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_path_l_id) (vs_real x, vs_real y, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_path_x_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_path_y_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_path_dxds_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_path_dyds_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_path_dxdl_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_path_dydl_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_path_curv_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);

vs_real (__cdecl *vs_road_sstart_id) (vs_real user_id);
vs_real (__cdecl *vs_road_sstop_id) (vs_real user_id);
vs_real (__cdecl *vs_road_length_id) (vs_real user_id);
vs_real (__cdecl *vs_road_is_looped_id) (vs_real user_id);
vs_real (__cdecl *vs_road_s_id) (vs_real x, vs_real y, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_l_id) (vs_real x, vs_real y, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_x_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_y_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_dxds_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_dyds_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_dxdl_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_dydl_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_curv_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_z_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_dzds_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);
vs_real (__cdecl *vs_road_dzdl_id) (vs_real s, vs_real l, vs_real user_id, vs_real inst);

// added after CarSim 9.0
void  (__cdecl *vs_opt_all_write) (int arg); // Jan 20, 2015

// DEPRECATED in CarSim 9 (no more J in path lookup tables)
// void    (__cdecl *vs_get_road_xy_j) (vs_real s, vs_real l, vs_real *x, vs_real *y, int *j);
// vs_real (__cdecl *vs_road_curv_j) (vs_real s, int *j);
// vs_real (__cdecl *vs_road_yaw_j) (vs_real sta, vs_real direction, int *j);


#ifdef __cplusplus
}
#endif


#endif  // _vs_api_h_INCLUDED
