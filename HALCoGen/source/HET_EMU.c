#include "HET_EMU.h"
#include "HL_het.h"
#include <stdio.h>

/** @fn void HetUART1PutChar(unsigned char Data)
*   @brief Sends a character of characters over HET emulated SCI
*
*   @param[in]  Data		The character to send
*/
void HetUART1PutChar(char Data)
{
  unsigned int Tmp = Data;

  Tmp <<= 1;                                    // Shift in start bit (0)
  Tmp |= 0x00000200;                            // Add stop bit (1)
  while(hetRAM1->Instruction[2].Data != 0);
  hetRAM1->Instruction[4].Data = Tmp << 7;  // Load TX buffer
  hetRAM1->Instruction[2].Data =  10 << 7;  // Load bit count
}

/** @fn void HetUART1PutText(unsigned char *text)
*   @brief Sends a string of characters over HET emulated SCI
*
*   @param[in]  text		The string of characters to send
*/
void HetUART1PutText(char *text)
{
	while(*text != 0)
	{
		HetUART1PutChar(*text++);
	}
}

/** @fn unsigned HetUART1Printf(const char *_format, ...)
*   @brief sends data to terminal (HET emulated SCI)
*
*   @param[in]  _format - string with format argument
*   @return      length of sent string
*
*   Sends formated string to terminal on HET emulated SCI
*/

unsigned HetUART1Printf(const char *_format, ...)
{
   char str[128];
   int length = -1, k = 0;

   va_list argList;
   va_start( argList, _format );

   length = vsnprintf(str, sizeof(str), _format, argList);

   va_end( argList );

   if (length > 0)
   {
      for(k=0; k<length; k++)
      {
    	  HetUART1PutChar(str[k]);
      }
   }

   return (unsigned)length;
}


/** @fn char HetUART1GetChar()
*   @brief		Gets a character from the HET Emulated SCI Receive Buffer if one is available
*
*	@return		The character in the receive buffer, if one is available. If not, 0.
*/
char HetUART1GetChar(void)
{
	unsigned int HetFlag;
	HetFlag = hetREG1->FLG;
	if(HetFlag & (1<<23))
	{
		hetREG1->FLG = (1<<23); // clear this bit
		return((char)(hetRAM1->Instruction[25].Data));
	}
	else
		return 0;
}
