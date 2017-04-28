#ifndef LOW_PASS_H
#define LOW_PASS_H

#include"filter.h"

class Low_Pass:public Filter
{
public:
    Low_Pass(int len);
};

#endif // LOW_PASS_H
