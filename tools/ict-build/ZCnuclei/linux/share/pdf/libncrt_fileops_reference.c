/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

----------------------------------------------------------------------
Licensing information

Licensor:                 SEGGER Software GmbH
Licensed to:              Nuclei System Technology Co., Ltd., Room 101, No. 500 Bibo Road, Pilot Free Trade Zone, Shanghai, P. R. China
Licensed SEGGER software: emRun RISC-V
License number:           RTL-00126
License model:            License and Service Agreement, signed August 27, 2021
Licensed platform:        RISC-V based Processor Units designed, manufactured marketed and branded by LICENSEE based on the RV32 architecture
-------------------------- END-OF-HEADER -----------------------------
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_Int.h"
#include "stdio.h"

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

__SEGGER_RTL_PUBLIC_API struct __SEGGER_RTL_FILE_impl {
  int handle;  // At least one field required (but unused) to ensure
               // the three file descriptors have unique addresses.
};

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

int metal_tty_putc(int c);  // UART output function
int metal_tty_getc(void);   // UART input function

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static FILE __SEGGER_RTL_stdin  = { 0 };
static FILE __SEGGER_RTL_stdout = { 1 };
static FILE __SEGGER_RTL_stderr = { 2 };

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

FILE __SEGGER_RTL_PUBLIC_API *stdin  = &__SEGGER_RTL_stdin;
FILE __SEGGER_RTL_PUBLIC_API *stdout = &__SEGGER_RTL_stdout;
FILE __SEGGER_RTL_PUBLIC_API *stderr = &__SEGGER_RTL_stderr;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/


/*********************************************************************
*
*       __SEGGER_RTL_X_file_stat()
*
*  Function description
*    Get file status.
*
*  Parameters
*    stream - Pointer to file.
*
*  Additional information
*    Low-overhead test to determine if stream is valid.  If stream
*    is a valid pointer and the stream is open, this function must
*    succeed.  If stream is a valid pointer and the stream is closed,
*    this function must fail.
*
*    The implementation may optionally determine whether stream is
*    a valid pointer: this may not always be possible and is not
*    required, but may assist debugging when clients provide wild
*    pointers.
*
*  Return value
*    <  0 - Failure, stream is not a valid file.
*    >= 0 - Success, stream is a valid file.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_stat(__SEGGER_RTL_FILE *stream) {
  if (stream == stdin || stream == stdout || stream == stderr) {
    return 0;
  } else {
    return EOF;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_bufsize()
*
*  Function description
*    Get stream buffer size.
*
*  Parameters
*    stream - Pointer to file.
*
*  Additional information
*    Returns the number of characters to use for buffered I/O on
*    the file stream.  The I/O buffer is allocated on the stack
*    for the duration of the I/O call, therefore this value should
*    not be set arbitrarily large.
*
*    For unbuffered I/O, return 1.
*
*  Return value
*    Nonzero number of characters to use for buffered I/O; for
*    unbuffered I/O, return 1.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_bufsize(__SEGGER_RTL_FILE *stream) {
    return 1;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_close()
*
*  Function description
*    Close file.
*
*  Parameters
*    stream - Pointer to file.
*
*  Additional information
*    Close the file stream.  If the stream is connected to a temporary
*    file (by use of tmpfile()), the temporary file is deleted.
*
*  Return value
*    <  0 - Failure, stream is already closed.
*    >= 0 - Success, stream is closed.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_close(__SEGGER_RTL_FILE *stream) {
    return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_end()
*
*  Function description
*    Test for end-of-file condition.
*
*  Parameters
*    stream - Pointer to file.
*
*  Return value
*    <  0 - Failure, stream is closed.
*    == 0 - Success, stream is not at end of file.
*    >  0 - Success, stream is at end of file.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_end(__SEGGER_RTL_FILE *stream) {
  return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_error()
*
*  Function description
*    Test for file-error condition.
*
*  Parameters
*    stream - Pointer to file.
*
*  Return value
*    <  0 - Failure, stream is closed.
*    == 0 - Success, stream is not in error.
*    >  0 - Success, stream is in error.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_error(__SEGGER_RTL_FILE *stream) {
  return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_flush()
*
*  Function description
*    Flush unwritten data to file.
*
*  Parameters
*    stream - Pointer to file.
*
*  Return value
*    <  0 - Failure, file cannot be flushed or was not successfully flushed.
*    == 0 - Success, unwritten data is flushed.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_flush(__SEGGER_RTL_FILE *stream) {
  return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_getpos()
*
*  Function description
*    Get file position.
*
*  Parameters
*    stream - Pointer to file.
*    pos    - Pointer to object that receives the position.
*
*  Return value
*    == 0 - Position retrieved successfully.
*    <  0 - Position not retrieved successfully.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_getpos(__SEGGER_RTL_FILE *stream, fpos_t *pos) {
  return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_clrerr()
*
*  Function description
*    Clear file-error status.
*
*  Parameters
*    stream - Pointer to file.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_clrerr(__SEGGER_RTL_FILE *stream) {
  /* Pass */
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_open()
*
*  Function description
*    Open file.
*
*  Parameters
*    filename - Pointer to zero-terminated file name.
*    mode     - Pointer to zero-terminated file mode.
*
*  Return value
*    == NULL - File not opened.
*    != NULL - File opened.
*/
__SEGGER_RTL_FILE * __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_open(const char *filename, const char *mode) {
    return NULL;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_seek()
*
*  Function description
*    Set file position.
*
*  Parameters
*    stream - Pointer to file to position.
*    offset - Offset relative to anchor specified by whence.
*    whence - Where offset is relative to.
*
*  Return value
*    == 0 - Position is set.
*    != 0 - Position is not set.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_seek(__SEGGER_RTL_FILE *stream, long offset, int whence) {
  return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_remove()
*
*  Function description
*    Remove file.
*
*  Parameters
*    filename - Pointer to string denoting file name to remove.
*
*  Return value
*    == 0 - Remove succeeded.
*    != 0 - Remove failed.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_remove(const char *filename) {
  return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_rename()
*
*  Function description
*    Rename file.
*
*  Parameters
*    old - Pointer to string denoting old file name.
*    new - Pointer to string denoting new file name.
*
*  Return value
*    == 0 - Rename succeeded.
*    != 0 - Rename failed.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_rename(const char *old, const char *new) {
  return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_tmpnam()
*
*  Function description
*    Generate name for temporary file.
*
*  Parameters
*    s   - Pointer to object that receives the temporary file name,
*          or NULL indicating that a (shared) internal buffer is used
*          for the temporary name.
*    max - Maxumum number of characters acceptable in the object s.
*
*  Return value
*    == NULL - Cannot generate a unique temporary name.
*    != NULL - Pointer to temporary name generated.
*/
char *__SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_tmpnam(char *s, unsigned max) {
    return NULL;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_tmpfile()
*
*  Function description
*    Generate temporary file.
*
*  Return value
*    == NULL - Cannot generate a unique temporary file.
*    != NULL - Pointer to temporary file.
*/
__SEGGER_RTL_FILE * __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_tmpfile(void) {
  return NULL;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_read()
*
*  Function description
*    Read data from file.
*
*  Parameters
*    stream - Pointer to file to read from.
*    s      - Pointer to object that receives the input.
*    len    - Number of characters to read from file.
*
*  Return value
*    >= 0 - Success.
*    <  0 - Failure.
*
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_read(__SEGGER_RTL_FILE * stream,
                                                    char               * s,
                                                    unsigned            len) {
  if (stream == stdin)
  {
    int r = len;
    while (len > 0)
    {
      *s++ = metal_tty_getc();
      --len;
    }
    return r;
  }

  return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_write()
*
*  Function description
*    Write data to file.
*
*  Parameters
*    stream - Pointer to file to write to.
*    s      - Pointer to object to write to file.
*    len    - Number of characters to write to the file.
*
*  Return value
*    >= 0 - Success.
*    <  0 - Failure.
*
*  Additional information
*    Writing to any file other than stdout or stderr results in an error.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_write(__SEGGER_RTL_FILE *stream, const char *s, unsigned len) {
  if (stream == stdout || stream == stderr)
  {
    int r = len;
    while (len > 0)
    {
      metal_tty_putc(*s++);
      --len;
    }
    return r;
  }

  return EOF;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_unget()
*
*  Function description
*    Push character back to stream.
*
*  Parameters
*    stream - Pointer to file to push back to.
*    c      - Character to push back.
*
*  Return value
*    >= 0 - Success.
*    <  0 - Failure.
*
*  Additional information
*    As input from the UART is not supported, this function always fails.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_file_unget(__SEGGER_RTL_FILE *stream, int c) {
  return EOF;
}

/*************************** End of file ****************************/
