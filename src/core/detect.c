#include "detect.h"
#include <stdlib.h>
#include <stdio.h>

int isvalid(struct hand *hand) {
	if (hand->groups[3].tile != -1)
		return 1;
	else
		return 0;
}

