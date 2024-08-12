// functions from vs_utility.c that can be handy
int     vs_compare_real_int(vs_real x, int i);
void    vs_do_nothing (vs_real var);
vs_real vs_fix(vs_real x);
vs_real vs_if_gt0_then(vs_real test, vs_real x_then, vs_real x_else);
vs_real vs_if_true_then(vs_real test, vs_real x_then, vs_real x_else);
vs_real vs_max(vs_real a, vs_real b);
vs_real vs_min(vs_real a, vs_real b);
int     vs_nint(vs_real a);
vs_real vs_sign(vs_real a, vs_real b);
vs_real vs_seconds_elapsed(void);
vs_real vs_step_smooth(vs_real x, vs_real x0, vs_real h0, vs_real x1, vs_real h1);
void    vs_string_duplicate (char **target, char *source);
char    *vs_string_find_replace(char *string, char *find, char *replace,
                             unsigned int size);
char    *vs_string_to_upper(char *s);
char    *vs_string_trim_trailing_space(char *str);
void    vs_time_and_date (char *t_date);
