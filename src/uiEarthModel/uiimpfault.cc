/*+
________________________________________________________________________

 CopyRight:     (C) de Groot-Bril Earth Sciences B.V.
 Author:        Nanne Hemstra
 Date:          May 2002
 RCS:           $Id: uiimpfault.cc,v 1.8 2003-09-26 21:40:31 bert Exp $
________________________________________________________________________

-*/

#include "uiimpfault.h"

#include "emmanager.h"
#include "emfault.h"
#include "emfaulttransl.h"
#include "ctxtioobj.h"
#include "ioobj.h"
#include "uiioobjsel.h"
#include "strmdata.h"
#include "strmprov.h"
#include "uiexecutor.h"
#include "uifileinput.h"
#include "uigeninput.h"
#include "filegen.h"
#include "uimsg.h"
#include "uiscaler.h"
#include "uibinidsubsel.h"
#include "scaler.h"
#include "survinfo.h"

#include "gridread.h"
#include "valgridtr.h"
#include "streamconn.h"

#include <fstream>


uiImportLMKFault::uiImportLMKFault( uiParent* p )
	: uiDialog(p,uiDialog::Setup("Import Landmark Fault",
				     "Specify fault parameters","104.1.2"))
	, ctio(*new CtxtIOObj(EMFaultTranslator::ioContext()))
{
    infld = new uiFileInput( this, "Input Landmark file" );
    infld->setDefaultSelectionDir(
	    IOObjContext::getDataDirName(IOObjContext::Surf) );

    formatfilefld = new uiFileInput( this, "Input Landmark formatfile",
	    			     uiFileInput::Setup()
				     .filter("*.fault_fmt;*") );
    formatfilefld->setDefaultSelectionDir(
	    IOObjContext::getDataDirName(IOObjContext::Surf) );
    formatfilefld->attach( alignedBelow, infld );

    ctio.ctxt.forread = false;
    outfld = new uiIOObjSel( this, ctio, "Output Fault" );
    outfld->attach( alignedBelow, formatfilefld );
}


uiImportLMKFault::~uiImportLMKFault()
{
    delete &ctio;
}


#define mWarnRet(s) { uiMSG().warning(s); return false; }
#define mErrRet(s) { uiMSG().error(s); return false; }

bool uiImportLMKFault::handleAscii()
{
    const char* faultnm = outfld->getInput();

    EM::EMManager& em = EM::EMM();
    MultiID key = em.add( EM::EMManager::Fault, faultnm );
    mDynamicCastGet( EM::Fault*, fault, em.getObject( key ) );
    if ( !fault )
	mErrRet( "Cannot create fault" );

    fault->ref();

    lmkEMFaultTranslator translator;
    ifstream* stream = new ifstream( infld->fileName(), ios::in | ios::binary );
    Conn* conn = new StreamConn( stream );

    PtrMan<Executor> exec =
	translator.reader( *fault,conn,formatfilefld->fileName()); 

    if ( !exec )
    {
	fault->unRef();
	mErrRet( "Cannot import fault" );
    }

    uiExecutor dlg( this, *exec );
    if ( !dlg.go() )
    {
	fault->unRef();
	mErrRet( dlg.lastMsg() );
    }

    PtrMan<Executor> saveexec = fault->saver();
    uiExecutor savedlg( this, *saveexec );
    if ( !savedlg.go() )
    {
	fault->unRef();
	mErrRet( savedlg.lastMsg() );
    }

    fault->unRef();

    return true;
}


bool uiImportLMKFault::acceptOK( CallBacker* )
{
    bool ret = checkInpFlds() && handleAscii();
    return ret;
}


bool uiImportLMKFault::checkInpFlds()
{
    if ( ! *infld->fileName() )
	mWarnRet( "Please select the input file" )
    else if ( !File_exists(infld->fileName()) )
	mWarnRet( "Input file does not exist" )

    if ( ! *formatfilefld->fileName() )
	mWarnRet( "Please select the format file" )
    else if ( !File_exists(formatfilefld->fileName()) )
	mWarnRet( "Format file does not exist" )

    if ( !outfld->commitInput( true ) )
	mWarnRet( "Please select the output" )

    return true;
}

