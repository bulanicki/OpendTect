/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Bert Bril
 Date:          April 2002
________________________________________________________________________

-*/
static const char* rcsID mUsedVar = "$Id$";

#include "uitextfile.h"
#include "uitextedit.h"
#include "uitable.h"
#include "uimenu.h"
#include "uifiledlg.h"
#include "uimsg.h"
#include "filepath.h"
#include "tableconvimpl.h"
#include "uistrings.h"
#include "perthreadrepos.h"
#include "od_iostream.h"

#define mTxtEd() (txted_ ? (uiTextEditBase*)txted_ : (uiTextEditBase*)txtbr_)

void uiTextFile::init( uiParent* p )
{
    txted_ = 0; txtbr_ = 0; tbl_ = 0;
    const CallBack modifcb( mCB(this,uiTextFile,valChg) );

    if ( setup_.style_ == File::Table )
    {
	uiTable::Setup tsu;
	tsu.rowdesc("Row").coldesc("Col").fillrow(false).fillcol(true)
	    .defcollbl(true).defrowlbl(true);
	tbl_ = new uiTable( p, tsu, filename_ );
	tbl_->setTableReadOnly( !setup_.editable_ );
	tbl_->valueChanged.notify( modifcb );
	tbl_->setStretch( 2, 2 );
	tbl_->setPrefHeight( 200 );
    }
    else if ( !setup_.editable_ || setup_.style_ == File::Bin )
	txtbr_ = new uiTextBrowser( p, filename_, setup_.maxnrlines_,
				    true, setup_.style_ == File::Log );
    else
    {
	txted_ = new uiTextEdit( p, filename_ );
	txted_->textChanged.notify( modifcb );
    }

    open( filename_ );
    ismodified_ = false;
}


void uiTextFile::valChg( CallBacker* )
{
    ismodified_ = true;
}


class uiTableExpHandler : public Table::ExportHandler
{
public:

uiTableExpHandler( uiTable* t, int ml )
    : Table::ExportHandler(od_ostream::nullStream())
    , tbl_(t)
    , nrlines_(0)
    , maxlines_(ml)
{
}

bool init()
{
    tbl_->clearTable();
    tbl_->setNrRows(0);
    tbl_->setNrCols(0);
    return true;
}

void finish()
{
    tbl_->setCurrentCell( RowCol(0,0) );
}

const char* putRow( const BufferStringSet& bss )
{
    RowCol rc( tbl_->nrRows(), 0 );
    tbl_->insertRows( rc.row(), 1 );
    if ( bss.size() >= tbl_->nrCols() )
	tbl_->insertColumns( tbl_->nrCols(), bss.size() - tbl_->nrCols() );

    for ( ; rc.col()<bss.size(); rc.col()++ )
	tbl_->setText( rc, bss.get(rc.col()) );

    nrlines_++;
    if ( nrlines_ >= maxlines_ )
    {
	rc.row()++; rc.col() = 0;
	tbl_->insertRows( rc.row(), 1 );
	tbl_->setText( rc, "[...]" );
	return "";
    }
    return 0;
}

    uiTable*	tbl_;
    int		maxlines_;
    int		nrlines_;
    BufferString msg_;

};


bool uiTextFile::open( const char* fnm )
{
    uiObj()->setName( fnm );
    if ( txted_ )
	txted_->readFromFile( fnm );
    else if ( txtbr_ )
	txtbr_->setSource( fnm );
    else
    {
	od_istream strm( fnm );
	if ( !strm.isOK() )
	    return false;

	Table::WSImportHandler imphndlr( strm );
	uiTableExpHandler exphndlr( tbl_, setup_.maxnrlines_ );
	Table::Converter cnvrtr( imphndlr, exphndlr );
	cnvrtr.execute();
    }


    setFileName( fnm );
    return true;
}


void uiTextFile::setFileName( const char* fnm )
{
    if ( filename_ != fnm )
	{ filename_ = fnm; fileNmChg.trigger(); }
}


bool uiTextFile::reLoad()
{
    return open( filename_ );
}


bool uiTextFile::save()
{
    return saveAs( filename_ );
}


bool uiTextFile::saveAs( const char* fnm )
{
    if ( mTxtEd() )
    {
	if ( !mTxtEd()->saveToFile( fnm ) )
	    return false;
    }
    else
    {
	od_ostream strm( fnm );
	if ( !strm.isOK() )
	    return false;
	strm << text();
	if ( !strm.isOK() )
	    return false;
    }

    setFileName( fnm );
    ismodified_ = false;
    return true;
}


int uiTextFile::nrLines() const
{
    if ( txtbr_ )
	return txtbr_->nrLines();
    else if ( tbl_ )
	return tbl_->nrRows();
    else
    {
	BufferString txt( text() );
	return txt.isEmpty() ? 0 : txt.count( '\n' );
    }
}


void uiTextFile::toLine( int lnr )
{
    const int lastln = nrLines() - 1;
    if ( lastln < 0 ) return;
    if ( lnr > lastln ) lnr = lastln;

    if ( mTxtEd() )
    {
	//TODO implement correctly
	if ( lnr > 1 )
	    mTxtEd()->scrollToBottom();
    }
    else
	tbl_->setCurrentCell( RowCol(lnr,0) );
}


const char* uiTextFile::text() const
{
    if ( mTxtEd() )
	return mTxtEd()->text();

    mDeclStaticString( ret ); ret = "";
    BufferString linetxt;
    const int nrrows = tbl_->nrRows(); const int nrcols = tbl_->nrCols();
    for ( RowCol rc(0,0); rc.row()<nrrows; rc.row()++ )
    {
	linetxt = "";
	for ( rc.col()=0; rc.col()<nrcols; rc.col()++ )
	{
	    const char* celltxt = tbl_->text( rc );
	    if ( !*celltxt ) break;
	    if ( rc.col() ) linetxt += " ";
	    linetxt += celltxt;
	}
	if ( rc.row() != nrrows-1 ) linetxt += "\n";
	ret += linetxt;
    }

    return ret.buf();
}


uiObject* uiTextFile::uiObj()
{
    uiObject* ret = mTxtEd();
    if ( !ret ) ret = tbl_;
    return ret;
}


uiTextFileDlg::uiTextFileDlg( uiParent* p, const char* fnm, bool rdonly,
				bool tbl )
	: uiDialog(p,Setup(fnm))
{
    Setup dlgsetup( fnm );
    dlgsetup.allowopen(!rdonly).allowsave(!rdonly);
    uiTextFile::Setup tfsu( tbl ? File::Table : File::Text );
    tfsu.editable_ = !rdonly;
    init( dlgsetup, tfsu, fnm );
}


uiTextFileDlg::uiTextFileDlg( uiParent* p, const Setup& dlgsetup )
	: uiDialog(p,dlgsetup)
{
    init( dlgsetup, uiTextFile::Setup(), dlgsetup.wintitle_.getFullString() );
}


void uiTextFileDlg::init( const uiTextFileDlg::Setup& dlgsetup,
			  const uiTextFile::Setup& tsetup, const char* fnm )
{
    if ( caption().isEmpty() )
	setCaption( fnm );

    captionisfilename_ = FixedString(caption().getFullString()) == fnm;

    editor_ = new uiTextFile( this, fnm, tsetup );
    editor_->fileNmChg.notify( mCB(this,uiTextFileDlg,fileNmChgd) );

    uiMenu* filemnu = new uiMenu( this, uiStrings::sFile() );
    if ( dlgsetup.allowopen_ )
	filemnu->insertItem( new uiAction(uiStrings::sOpen(false),
			     mCB(this,uiTextFileDlg,open)) );
    if ( dlgsetup.allowsave_ )
    {
	if ( tsetup.editable_ )
	    filemnu->insertItem( new uiAction(uiStrings::sSave(false),
				 mCB(this,uiTextFileDlg,save)) );
	filemnu->insertItem( new uiAction(uiStrings::sSaveAs(),
			     mCB(this,uiTextFileDlg,saveAs)) );
    }
    filemnu->insertItem( new uiAction("&Quit",
			 mCB(this,uiTextFileDlg,dismiss)) );
    menuBar()->insertItem( filemnu );
}


void uiTextFileDlg::setFileName( const char* fnm )
{
    if ( captionisfilename_ )
	setCaption( fnm );
    editor_->open( fnm );
}


void uiTextFileDlg::fileNmChgd( CallBacker* )
{
    const BufferString fnm( editor_->fileName() );
    FilePath fp( fnm );
    setName( fp.fileName() );
    if ( captionisfilename_ )
	setCaption( fnm );
}


void uiTextFileDlg::open( CallBacker* )
{
    uiFileDialog dlg( this, uiFileDialog::ExistingFile,
		      editor_->fileName(), "", "Select file" );
    if ( dlg.go() )
	editor_->open( dlg.fileName() );
}


void uiTextFileDlg::save( CallBacker* )
{
    editor_->save();
}


void uiTextFileDlg::saveAs( CallBacker* )
{
    uiFileDialog dlg( this, uiFileDialog::AnyFile,
		      editor_->fileName(), "", "Select new file name" );
    if ( dlg.go() )
	editor_->saveAs( dlg.fileName() );
}


void uiTextFileDlg::dismiss( CallBacker* )
{
    accept( this );
}


bool uiTextFileDlg::rejectOK( CallBacker* cb )
{
    if ( !cancelpushed_ )
	return acceptOK( cb );

    if ( !okToExit() )
	return false;

    if ( !editor_->reLoad() )
	doMsg( "Cannot re-load file. Possibly the file no longer exists." );

    return false;
}


bool uiTextFileDlg::acceptOK( CallBacker* )
{
    return okToExit();
}


int uiTextFileDlg::doMsg( const char* msg, bool iserr )
{
    int ret = 0;

    uiMsgMainWinSetter setter( this );

    if ( iserr )
	uiMSG().error( msg );
    else
	ret = uiMSG().askGoOnAfter( msg );

    return ret;
}


bool uiTextFileDlg::okToExit()
{
    if ( !editor_->isModified() )
	return true;

    const BufferString msg( "File:\n", editor_->fileName(),
			"\nwas modified. Save now?" );
    int opt = doMsg( msg, false );
    if ( opt == 2 )
	return false;
    else if ( opt == 0 && !editor_->save() )
	{ doMsg( "Could not save.\nPlease try 'Save As'" ); return false; }

    return true;
}
