#ifndef attribsteering_h
#define attribsteering_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Kristofer Tingdahl
 Date:          07-10-1999
 RCS:           $Id$
________________________________________________________________________

-*/

#include "position.h"
#include "seistype.h"
//#include "commondefs.h"

#define mStd2DTrcSpacing 12.5			//tmp fix for od4.4

namespace Attrib
{

mGlobal BinID getSteeringPosition( int );
mGlobal int getSteeringIndex( const BinID& );


}; //Namespace


#endif
