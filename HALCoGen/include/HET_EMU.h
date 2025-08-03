/*
 * HET_EMU.h
 *
 *  Created on: Nov 28, 2012
 *      Author: a0866528
 */

#ifndef HET_EMU_H_
#define HET_EMU_H_
#define SCIRxReady (hetREG1->FLG & (1<<23))
void HetUART1PutChar(char Data);
void HetUART1PutText(char *text);
unsigned HetUART1Printf(const char *_format, ...);
char HetUART1GetChar(void);

#endif /* HET_EMU_H_ */
