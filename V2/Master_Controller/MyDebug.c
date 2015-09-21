#include "MyDebug.h"
#include "string.h"
void ERROR(char cErrorMessage[])
{
	char cErrorMessagePrint[80]=0 ;
	strcpy(cErrorMessagePrint,cErrorMessage);
	while(1);  //stop here
}
