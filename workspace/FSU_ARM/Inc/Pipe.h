/*
 * Pipe.h
 *
 *  Created on: 2016-6-14
 *      Author: lcz
 */

#ifndef PIPE_H_
#define PIPE_H_

void notifyCamera();
void notifyPPP(const char * pn, const char * msg);
bool getPipeNotify(const char * file, const char * o);
void checkRebootSignal();

#endif /* PIPE_H_ */
