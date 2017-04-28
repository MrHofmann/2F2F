#ifndef HIGH_PASS_H
#define HIGH_PASS_H

#include "filter.h"

class High_Pass:public Filter
{
public:
    High_Pass(int len);
};

#endif // HIGH_PASS_H
