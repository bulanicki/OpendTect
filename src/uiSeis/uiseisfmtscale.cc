/*+
________________________________________________________________________

 CopyRight:     (C) de Groot-Bril Earth Sciences B.V.
 Author:        Bert Bril
 Date:          May 2002
 RCS:		$Id: uiseisfmtscale.cc,v 1.7 2003-04-22 09:51:00 arend Exp $
________________________________________________________________________

-*/

#include "uiseisfmtscale.h"
#include "uiscaler.h"
#include "datachar.h"
#include "ioman.h"
#include "ioobj.h"
#include "iopar.h"

#include "uigeninput.h"


uiSeisFmtScale::uiSeisFmtScale( uiParent* p, bool wfmt )
	: uiGroup(p,"Seis format and scale")
	, imptypefld(0)
	, optimfld(0)
{
    if ( wfmt )
	imptypefld = new uiGenInput( this, "Storage",
		     StringListInpSpec(DataCharacteristics::UserTypeNames) );

    scalefld = new uiScaler( this, 0, true );
    if ( imptypefld )
	scalefld->attach( alignedBelow, imptypefld );

    if ( wfmt )
    {
	optimfld = new uiGenInput( this, "Optimize horizontal slice access",
				   BoolInpSpec() );
	optimfld->attach( alignedBelow, scalefld );
    }

    setHAlignObj( scalefld );
}


Scaler* uiSeisFmtScale::getScaler() const
{
    return scalefld->getScaler();
}


int uiSeisFmtScale::getFormat() const
{
    return imptypefld ? imptypefld->getIntValue() : 0;
}


bool uiSeisFmtScale::horOptim() const
{
    return optimfld ? optimfld->getBoolValue() : false;
}


void uiSeisFmtScale::updateFrom( const IOObj& ioobj )
{
    if ( !imptypefld ) return;

    const char* res = ioobj.pars().find( "Data storage" );
    if ( !res ) return;

    imptypefld->setValue( (int)*res );
}


void uiSeisFmtScale::updateIOObj( IOObj* ioobj ) const
{
    if ( !ioobj ) return;

    const int tp = getFormat();
    ioobj->pars().set( "Data storage", DataCharacteristics::UserTypeNames[tp] );
    const bool horoptim = horOptim();
    ioobj->pars().set( "Optimized direction",
	    		horOptim() ? "Horizontal" : "Vertical" );

    IOM().to( ioobj->key() );
    IOM().commitChanges( *ioobj );
}
