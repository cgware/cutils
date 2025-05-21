#include "loc.h"

int loc_eq(loc_t l, loc_t r)
{
	return l.off == r.off && l.len == r.len;
}
