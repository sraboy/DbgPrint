#ifndef FLUSHER_H
#define FLUSHER_H

#include <stdio.h>
#include <Windows.h>
#include <Winbase.h> // for SECURITY_ATTRIBUTES structure for file creation
#include <memory.h>  // for memccpy  

//#define BUF     65536
//#define BUF      4

extern ULONG BUF;

void bufInit();
void InsideBufferNull();
int FlushFileBuffered( HANDLE  hFile);
int WriteFileBuffer( HANDLE  hFile, char* OB, DWORD OBsize );

#endif // FLUSHER_H