#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "tsp.h"
#include "io.h"

int main(int argc, char *argv[])
{
	(void)printf("hello world");
    Tsp *tsp = import_tsp("d198.tsp");
	return EX_OK;
}
