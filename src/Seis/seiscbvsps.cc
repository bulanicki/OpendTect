/*+
 * COPYRIGHT: (C) dGB Beheer B.V.
 * AUTHOR   : A.H. Bril
 * DATE     : 21-1-1998
-*/

static const char* rcsID = "$Id: seiscbvsps.cc,v 1.4 2005-01-01 12:56:17 bert Exp $";

#include "seiscbvsps.h"
#include "seispsioprov.h"
#include "seiscbvs.h"
#include "seisbuf.h"
#include "filepath.h"
#include "filegen.h"
#include "survinfo.h"
#include "sortedlist.h"
#include "dirlist.h"
#include "iopar.h"

class CBVSSeisPSIOProvider : public SeisPSIOProvider
{
public:
			CBVSSeisPSIOProvider() : SeisPSIOProvider("CBVS") {}
    SeisPSReader*	makeReader( const char* dirnm ) const
			{ return new SeisCBVSPSReader(dirnm); }
    SeisPSWriter*	makeWriter( const char* dirnm ) const
			{ return new SeisCBVSPSWriter(dirnm); }
    static int		factid;
};

// This adds the CBVS type pre-stack seismics data storage to the factory
int CBVSSeisPSIOProvider::factid = SPSIOPF().add( new CBVSSeisPSIOProvider );


SeisCBVSPSIO::SeisCBVSPSIO( const char* dirnm )
    	: dirnm_(dirnm)
{
    BufferString& sm = const_cast<BufferString&>( selmask_ );
    sm = "*."; sm += CBVSSeisTrcTranslator::sKeyDefExtension;
}


SeisCBVSPSIO::~SeisCBVSPSIO()
{
}


SeisCBVSPSReader::SeisCBVSPSReader( const char* dirnm )
    	: SeisCBVSPSIO(dirnm)
    	, inls_(*new SortedList<int>(false))
{
    if ( !File_isDirectory(dirnm_) )
    {
	errmsg_ = "Directory '"; errmsg_ += dirnm_;
	errmsg_ += "' does not exist";
	return;
    }

    DirList dl( dirnm_, DirList::FilesOnly, selmask_.buf() );
    for ( int idx=0; idx<dl.size(); idx++ )
    {
	BufferString fnm( dl.get(idx) );
	char* ptr = fnm.buf();
	while ( *ptr && !isdigit(*ptr) ) ptr++;
	while ( *ptr && isdigit(*ptr) ) ptr++;
	*ptr = '\0';
	if ( fnm == "" ) continue;
	inls_ += atoi( ptr );
    }

    if ( inls_.size() < 1 )
    {
	errmsg_ = "Directory '"; errmsg_ += dirnm_;
	errmsg_ += "' contains no usable pre-stack data files";
	return;
    }
}


SeisCBVSPSReader::~SeisCBVSPSReader()
{
    delete &inls_;
}


bool SeisCBVSPSReader::getGather( const BinID& bid, SeisTrcBuf& gath ) const
{
    int inlidx = inls_.indexOf( bid.inl );
    if ( inlidx < 0 )
	{ errmsg_ = "Inline not present"; return false; }

    FilePath fp( dirnm_ );
    BufferString fnm = inls_[inlidx]; fnm += ext();
    fp.add( fnm );

    errmsg_ = "";
    CBVSSeisTrcTranslator* tr = CBVSSeisTrcTranslator::make( fp.fullPath(),
	    					false, false, &errmsg_ );
    if ( !tr )
	return false;

    if ( !tr->goTo( BinID(bid.crl,0) ) )
	{ errmsg_ = "Crossline not present"; return false; }

    const Coord coord = SI().transform( bid );
    while ( true )
    {
	SeisTrc* trc = new SeisTrc;
	if ( !tr->read(*trc) )
	{
	    delete trc;
	    errmsg_ = tr->errMsg();
	    return errmsg_ == "";
	}
	else if ( trc->info().binid.inl != bid.crl )
	    { delete trc; return true; }

	trc->info().nr = trc->info().binid.crl;
	trc->info().binid = bid; trc->info().coord = coord;
	gath.add( trc );
    }

    // Not reached
    return true;
}


SeisCBVSPSWriter::SeisCBVSPSWriter( const char* dirnm )
    	: SeisCBVSPSIO(dirnm)
    	, reqdtype_(DataCharacteristics::Auto)
    	, tr_(0)
    	, prevbid_(*new BinID(mUndefIntVal,mUndefIntVal))
	, nringather_(1)
{
    if ( !File_isDirectory(dirnm_) )
    {
	if ( File_exists(dirnm_) )
	{
	    errmsg_ = "Existing file '";
	    errmsg_ += dirnm_; errmsg_ += "'. Remove or rename.";
	    return;
	}
	File_createDir(dirnm_,0);
    }
}


SeisCBVSPSWriter::~SeisCBVSPSWriter()
{
    close();
    delete &prevbid_;
}


void SeisCBVSPSWriter::close()
{
    delete tr_; tr_ = 0;
    prevbid_ = BinID( mUndefIntVal, mUndefIntVal );
    nringather_ = 1;
}


void SeisCBVSPSWriter::usePar( const IOPar& iopar )
{
    const char* res = iopar.find( CBVSSeisTrcTranslator::sKeyDataStorage );
    if ( res && *res )
	reqdtype_ = (DataCharacteristics::UserType)(*res-'0');
}


bool SeisCBVSPSWriter::newInl( const SeisTrc& trc )
{
    if ( mIsUndefInt(prevbid_.inl) )
    {
	if ( reqdtype_ == DataCharacteristics::Auto )
	    dc_ = trc.data().getInterpreter()->dataChar();
	else
	    dc_ = DataCharacteristics( reqdtype_ );
    }

    const BinID& trcbid = trc.info().binid;
    BufferString fnm = trcbid.inl; fnm += ext();
    FilePath fp( dirnm_ ); fp.add( fnm );
    fnm = fp.fullPath();

    if ( tr_ ) delete tr_;
    tr_ = CBVSSeisTrcTranslator::getInstance();
    if ( !tr_->initWrite(new StreamConn(fnm,Conn::Write),trc) )
    {
	errmsg_ = "Trying to start writing to '";
	errmsg_ += fnm; errmsg_ += "':\n";
	errmsg_ += tr_->errMsg();
	return false;
    }
    ObjectSet<SeisTrcTranslator::TargetComponentData>& ci= tr_->componentInfo();
    for ( int idx=0; idx<ci.size(); idx++ )
	ci[idx]->datachar = dc_;

    return true;
}


bool SeisCBVSPSWriter::put( const SeisTrc& trc )
{
    SeisTrcInfo& ti = const_cast<SeisTrcInfo&>( trc.info() );
    const BinID trcbid = ti.binid;
    if ( trcbid.inl != prevbid_.inl )
    {
	if ( !newInl(trc) )
	    return false;
    }
    if ( trcbid.crl != prevbid_.crl )
	nringather_ = 1;
    prevbid_ = trcbid;

    ti.binid = BinID( trcbid.crl, nringather_ );
    bool res = tr_->write( trc );
    ti.binid = trcbid;
    if ( !res )
	errmsg_ = tr_->errMsg();
    else
	nringather_++;

    return res;
}
