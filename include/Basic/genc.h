#ifndef genc_H
#define genc_H

/*+
________________________________________________________________________

 CopyRight:	(C) dGB Beheer B.V.
 Author:	A.H. Bril
 Date:		23-10-1996
 RCS:		$Id: genc.h,v 1.35 2008-09-30 12:08:20 cvshelene Exp $
________________________________________________________________________

Some general utilities, that need to be accessible in many places:

-*/

#ifndef gendefs_H
#include "gendefs.h"
#endif

#include "string2.h"

#ifdef __cpp__
extern "C" {
#endif

const char*	GetProjectVersionName(void);
		/*!< "dTect Vx.x" */

int		GetPID();
		/*!< returns process ID */

const char*	GetLocalHostName();
		/*!< returns (as expected) local host name */

int		isProcessAlive(int pid);
		/*!< returns 1 if the process is still running */

int		ExitProgram( int ret );
		/*!< Win32: kills progam itself and ignores ret.
		     Unix: uses exit(ret).
		     Return value is convenience only, so you can use like:
		     return exitProgram( retval );
                */

typedef void	(*PtrAllVoidFn)(void);
void		NotifyExitProgram(PtrAllVoidFn);
		/*!< Function will be called on 'ExitProgram' */

void		PutIsLittleEndian(unsigned char*);
		/*!< Puts into 1 byte: 0=SunSparc/SGI (big), 1=PC (little) */

void		SwapBytes(void*,int nbytes);
		/*!< nbytes=2,4,... e.g. nbytes=4: abcd becomes cdab */

int		InSysAdmMode();
		/*!< returns 0 unless in sysadm mode */

#ifdef __cpp__
}
#else
/* C only */

typedef char	FileNameString[mMaxFilePathLength+1];
typedef char	UserIDString[mMaxUserIDLength+1];

#endif


#endif
