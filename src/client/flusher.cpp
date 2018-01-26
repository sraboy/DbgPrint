/*++

Copyright (c) 2003-2004 Darja G.

Module Name:
    flusher.cpp

Abstract:
    Buffered WriteFile()

Author:
    Darja G.

Environment:
    User mode only

Notes:

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Revision History:

--*/

#include "flusher.h"

ULONG BUF=64*1024;
//char    IB[BUF];
char*   IB = NULL;
DWORD   IBsize = 0;  // inside buffer

void 
bufInit(){    
    if(!IB) {
        IB = (char*)GlobalAlloc(GMEM_FIXED, BUF);
    }
    //memset( IB, '~', BUF);
} // end bufInit()

void 
InsideBufferNull(){
    IBsize = 0;
} // end InsideBufferNull()

int 
FlushFileBuffered(HANDLE  hFile)  // handle to file in which we are write                      
{
    DWORD   written_bytes;
    if(IB && !IBsize)
        return 0;
    if ( WriteFile(
            hFile,                  // handle to file
            (void*)IB,              // data buffer
            IBsize,                 // number of bytes to write
            &written_bytes,         // number of bytes written
            NULL                    // overlapped buffer
         ) ==0) { 
            return -1;
    }
    else { 
        InsideBufferNull();
        bufInit(); 
        return 0;
    }
} // end FlushFileBuffered()


int 
WriteFileBuffer(
    HANDLE  hFile,  // handle to file in which we are write
    char*   OB,     // outside buffer - something to write into file
    DWORD   OBsize  // outside buffer length
    )
{ // if error return -1
    DWORD   NewOBsize = 0;    
    int     T;
    int     i;
    int     IBtemp;
    DWORD   written_bytes;

    if(!IB) {
        bufInit();
    }
    if ((OBsize + IBsize) < BUF){        
        memcpy( &IB[IBsize], OB, OBsize);
        IBsize = IBsize + OBsize;        
        return 0;
    }
    if ((OBsize + IBsize) >= BUF){
        NewOBsize = OBsize - (BUF - IBsize);
        IBtemp = IBsize;
        memcpy( &IB[IBsize], OB, (BUF-IBsize));   
        //Flush
        if ( WriteFile(
                hFile,                  // handle to file
                (void*)IB,              // data buffer
                BUF,                    // number of bytes to write
                &written_bytes,         // number of bytes written
                NULL                    // overlapped buffer
             ) ==0) { 
            return -1;
        }
        else { 
            InsideBufferNull();
            bufInit(); 
        }
        T = NewOBsize / BUF;
        if(T >= 1) {
            for(i=0; i<T; i++){                
                //Flush(IB)
                if ( WriteFile(
                        hFile,                                      // handle to file
                        (void*) (&OB[ (BUF-IBtemp) + BUF * i ]),    // data buffer                       
                        BUF,                                        // number of bytes to write
                        &written_bytes,                             // number of bytes written
                        NULL                                        // overlapped buffer
                     ) ==0) { 
                    return -1;
                }
                else { 
                    InsideBufferNull();
                    bufInit(); 
                }
            }
            IBsize = NewOBsize - T * BUF; // remainder after (NewOBsize/BUF)
            memcpy( IB, &OB[OBsize - IBsize], IBsize);
            return 0;
        }
        if (T < 1) {            
            memcpy( IB, &OB[OBsize - NewOBsize], NewOBsize );            
            IBsize = NewOBsize; 
            return 0;
        }
    }
    return 0;
} // end WriteFileBuffer()

#if 0
void main()
{
    #include <string.h> // for string

    HANDLE      hFile;  
    DWORD       Error;

    Error = 0;
    hFile = NULL;    
    
    hFile = 
        CreateFile(
            "dasha.txt",            // file name
            GENERIC_WRITE,          // access mode
            0,                      // share mode
            NULL,                   // Secrity 
            OPEN_ALWAYS,            // how to create - rewrite if file already exist
            FILE_ATTRIBUTE_NORMAL,  // file attributes
            NULL                    // handle to template file - in Win9x/Me must be NULL!!
        );
    if (hFile == INVALID_HANDLE_VALUE){
        Error = GetLastError();
        printf("Error (INVALID_HANDLE_VALUE)%d", Error);    
    }

    char str[16];
    bufInit();
    int i, j;
    for( i =1; i < 16; i++){
        for (j=0; j<16; j++) {
            memset( str, 'a'+j, i );
            WriteFileBuffer( hFile, str, i);            
            WriteFileBuffer( hFile, " ", 1);            
        }
        WriteFileBuffer( hFile, "\x0a", 1);            
    }    
    FlushFileBuffered( hFile);
}
#endif
