/*
 * Log.cpp
 *
 *  Created on: 2016-6-14
 *      Author: lcz
 */

#include <iostream>
#include <string.h>
#include "stdio.h"
#include "stdlib.h"
#include <stdarg.h>

#include "Log.h"

using namespace std;
const char * log_file_name = ".FSU.log";


void log(const char *s) {
	FILE *fp;
	time_t t;
	if ((fp = fopen(log_file_name, "a")) >= 0) {
		t = time(0);
		fprintf(fp, "%s", asctime(localtime(&t)));
		fprintf(fp, "%s\n",s);
		fclose(fp);
	}
	else {
		cout << log_file_name << " open error!" << endl;
	}

}

