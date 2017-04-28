#include "high_pass.h"

High_Pass::High_Pass(int len)
    :Filter(0,12000, 0, len)
{

}
