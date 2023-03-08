#ifndef MY_TIME_OPS_H
#define MY_TIME_OPS_H

long get_current_time_byns(void);
long get_current_time_byus(void);
long get_current_time_bymsec(void);
long get_current_time_bysec(void);
void displayCurrentTimeByYMDS(void);
void get_current_time_byYMD(struct tm* tm);

#endif