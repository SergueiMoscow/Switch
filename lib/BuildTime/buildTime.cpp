#include <time.h>
#include "buildTime.h"

time_t getBuildUnixTime() {
    BuildTime bt = BUILD_TIME_INITIALIZER;
    struct tm t = {0};
    t.tm_year = bt.year - 1900;
    t.tm_mon = bt.month - 1;
    t.tm_mday = bt.day;
    t.tm_hour = bt.hour;
    t.tm_min = bt.min;
    t.tm_sec = bt.sec;
    t.tm_isdst = -1;
    return mktime(&t);
}
