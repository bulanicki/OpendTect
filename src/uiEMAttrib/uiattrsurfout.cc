/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        Nanne Hemstra
 Date:          October 2004
________________________________________________________________________

-*/
static const char* rcsID = "$Id: uiattrsurfout.cc,v 1.28 2009-07-21 09:13:27 cvsnanne Exp $";


#include "uiattrsurfout.h"

#include "attribdesc.h"
#include "attribdescset.h"
#include "attribengman.h"
#include "attriboutput.h"
#include "attribfactory.h"
#include "emsurfacetr.h"
#include "uiattrsel.h"
#include "uiioobjsel.h"
#include "uigeninput.h"
#include "ctxtioobj.h"
#include "ioman.h"
#include "ioobj.h"
#include "iopar.h"
#include "nladesign.h"
#include "nlamodel.h"
#include "uimsg.h"
#include "ptrman.h"
#include "multiid.h"
#include "keystrs.h"


using namespace Attrib;

uiAttrSurfaceOut::uiAttrSurfaceOut( uiParent* p, const DescSet& ad,
				    const NLAModel* n, const MultiID& mid )
    : uiAttrEMOut( p, ad, n, mid, "Create surface output" )
    , ctio_(*mMkCtxtIOObj(EMHorizon3D))
{
    setHelpID( "104.4.0" );

    attrnmfld_ = new uiGenInput( uppgrp_, "Attribute name", StringInpSpec() );
    attrnmfld_->attach( alignedBelow, attrfld_ );

    ctio_.ctxt.forread = true;
    objfld_ = new uiIOObjSel( uppgrp_, ctio_, "Calculate on surface" );
    objfld_->attach( alignedBelow, attrnmfld_ );
    objfld_->selectiondone.notify( mCB(this,uiAttrSurfaceOut,objSel) );

    uppgrp_->setHAlignObj( attrfld_ );
    addStdFields( false, true );
}


uiAttrSurfaceOut::~uiAttrSurfaceOut()
{
    delete ctio_.ioobj;
    delete &ctio_;
}


void uiAttrSurfaceOut::attribSel( CallBacker* )
{
    attrnmfld_->setText( attrfld_->getInput() );
    objSel(0);
}


void uiAttrSurfaceOut::objSel( CallBacker* )
{
    if ( !objfld_->ioobj(true) ) return;
    BufferString parnm( objfld_->ioobj(true)->name() );
    parnm += " "; parnm += attrnmfld_->text();
    setParFileNmDef( parnm );
}


bool uiAttrSurfaceOut::prepareProcessing()
{
    if ( !objfld_->commitInput() )
    {
	uiMSG().error( "Please select surface" );
	return false;
    }

    return uiAttrEMOut::prepareProcessing();
}


bool uiAttrSurfaceOut::fillPar( IOPar& iopar )
{
    uiAttrEMOut::fillPar( iopar );
    BufferString outid;
    outid += ctio_.ioobj->key();
    fillOutPar( iopar, Output::surfkey(), LocationOutput::surfidkey(), outid );

    BufferString attrnm = attrnmfld_->text();
    if ( attrnm.isEmpty() ) attrnm = attrfld_->getInput();
    iopar.set( sKey::Target, attrnm );
    return true;
}

