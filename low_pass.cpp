#include "low_pass.h"

Low_Pass::Low_Pass(int len)
    : Filter(2000,22000, 0, len)
{
}
