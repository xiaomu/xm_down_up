/*****************************************************************************
 *  Copyright          :  All Rights Reserved.
 *
 *  Date               :  2012-09-11 16:44:35
 *  Author/Corporation :  Dengzhaoqun
 *  Email              :  dengzhaoqun@163.com
 *****************************************************************************/
#ifndef _XM_CMN_UTIL_H_
#define _XM_CMN_UTIL_H_

char *xm_char_add(char *str, char *str2);
char *xm_vsprintf_ex(int len, char *fmt, ... );
int debug_printf(char *format, ...);
int help(char *menu[]);
int equal_or_bnull(char *str, char *str2);

#define DATE_TIME_LEN 20
typedef char DATE_TIME_T[DATE_TIME_LEN];
void date_time(time_t myt, DATE_TIME_T *dt);

char* itoa(int val, int base);
char *strcat_ex(const char *str, const char *str2)

void *xm_malloc(FILE *out, const int len) ;
FILE *xm_fopen(FILE *out, const char *path, const char *mode);
#endif