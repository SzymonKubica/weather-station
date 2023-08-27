#ifndef DATE_TIME_H
#define DATE_TIME_H

struct DateTime {
    struct tm *date_time;
    struct tm *date_time_utc;
};

extern struct DateTime system_time;

void update_time();

#endif
