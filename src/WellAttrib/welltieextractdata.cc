/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Bruno
 Date:		Jan 2009
________________________________________________________________________

-*/
static const char* rcsID = "$Id: welltieextractdata.cc,v 1.37 2011-09-28 10:35:34 cvsbruno Exp $";

#include "welltieextractdata.h"
#include "welltiegeocalculator.h"

#include "arrayndimpl.h"
#include "cubesampling.h"
#include "interpol1d.h"
#include "ioman.h"
#include "datapointset.h"
#include "linekey.h"
#include "seisbuf.h"
#include "seistrc.h"
#include "seisread.h"
#include "seisselectionimpl.h"
#include "survinfo.h"

namespace WellTie
{

SeismicExtractor::SeismicExtractor( const IOObj& ioobj ) 
	: Executor("Extracting Seismic positions")
	, rdr_(new SeisTrcReader( &ioobj ))
	, trcbuf_(new SeisTrcBuf(false))				   
	, nrdone_(0)
	, outtrc_(0)	    
	, cs_(new CubeSampling(false))	    
	, extrintv_(SI().zRange(false))
	, linekey_(0)		  
	, radius_(1)		  
{}


SeismicExtractor::~SeismicExtractor() 
{
    delete cs_;
    delete rdr_;
    delete trcbuf_;
    delete outtrc_;
}


void SeismicExtractor::setInterval( const StepInterval<float>& itv )
{
    extrintv_ = itv;
    delete outtrc_;
    outtrc_ = new SeisTrc( itv.nrSteps() + 1 );
    outtrc_->info().sampling = itv;
    outtrc_->zero();
}


void SeismicExtractor::collectTracesAroundPath() 
{
    if ( bidset_.isEmpty() ) return;

    if ( rdr_->is2D() )
    {
	cs_->hrg.setInlRange( Interval<int>(bidset_[0].inl,bidset_[0].inl) );
	cs_->hrg.setCrlRange( Interval<int>(0,SI().crlRange(true).stop) );
    }
    else
    {
	for ( int idx=0; idx<bidset_.size(); idx++ )
	{
	    BinID bid = bidset_[idx];
	    cs_->hrg.include( BinID( bid.inl + radius_, bid.crl + radius_ ) );
	    cs_->hrg.include( BinID( bid.inl - radius_, bid.crl - radius_ ) );
	}
    }
    cs_->hrg.snapToSurvey();
    cs_->zrg = extrintv_;
    Seis::RangeSelData* sd = new Seis::RangeSelData( *cs_ );
    sd->lineKey() = *linekey_;

    rdr_->setSelData( sd );
    rdr_->prepareWork();

    SeisBufReader sbfr( *rdr_, *trcbuf_ );
    sbfr.execute();
}


void SeismicExtractor::setBIDValues( const TypeSet<BinID>& bids )
{
    bidset_.erase();
    for ( int idx=0; idx<bids.size(); idx++ )
    {	
	if ( SI().isInside( bids[idx], true ) )
	    bidset_ += bids[idx];
	else if ( idx )
	    bidset_ += bids[idx-1];
    }
    collectTracesAroundPath();
}


int SeismicExtractor::nextStep()
{
    double zval = extrintv_.atIndex( nrdone_ );

    if ( zval>extrintv_.stop || nrdone_ >= extrintv_.nrSteps() 
	    || nrdone_ >= bidset_.size() )
	return Executor::Finished();

    const int datasz = extrintv_.nrSteps();

    const BinID curbid = bidset_[nrdone_];
    float val = 0; int nrtracesinradius = 0;
    int prevradius = mUdf(int);

    for ( int idx=0; idx<trcbuf_->size(); idx++ )
    {
	const SeisTrc* trc = trcbuf_->get(idx);
	BinID b = trc->info().binid;
	int xx0 = b.inl-curbid.inl; 	xx0 *= xx0; 
	int yy0 = b.crl-curbid.crl;	yy0 *= yy0;

	if ( rdr_->is2D() )
	{
	    if ( ( xx0 + yy0  ) < prevradius || mIsUdf(prevradius) )
	    {
		prevradius = xx0 + yy0;
		val = trc->get( nrdone_, 0 );
		nrtracesinradius = 1;
	    }
	}
	else
	{
	    if ( xx0 + yy0 < radius_*radius_ )
	    {
		nrtracesinradius ++;
		val += trc->get( nrdone_, 0 );
	    }
	}
    }
    if ( mIsUdf(val) ) val =0;
    outtrc_->set( nrdone_, val/nrtracesinradius, 0 );

    nrdone_ ++;
    return Executor::MoreToDo();
}

}; //namespace WellTie
