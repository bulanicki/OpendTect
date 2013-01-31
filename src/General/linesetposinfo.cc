/*+
 * (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 * AUTHOR   : Bert
 * DATE     : July 2005 / Mar 2008
-*/

static const char* rcsID mUsedVar = "$Id$";

#include "linesetposinfo.h"
#include "survinfo.h"
#include "statruncalc.h"
#include "binidvalset.h"


PosInfo::LineSet2DData::IR::~IR()
{
    delete posns_;
}


const PosInfo::Line2DData* PosInfo::LineSet2DData::getLineData(
					const char* lnm ) const
{
    PosInfo::LineSet2DData::Info* li = findLine( lnm );
    return li ? &li->pos_ : 0;
}


PosInfo::LineSet2DData::Info* PosInfo::LineSet2DData::findLine(
			const char* lnm ) const
{
    if ( !lnm || !*lnm ) return 0;

    for ( int idx=0; idx<data_.size(); idx++ )
    {
	if ( data_[idx]->lnm_ == lnm )
	    return const_cast<PosInfo::LineSet2DData::Info*>(data_[idx]);
    }

    return 0;
}


PosInfo::Line2DData& PosInfo::LineSet2DData::addLine( const char* lnm )
{
    PosInfo::LineSet2DData::Info* li = findLine( lnm );
    if ( li ) return li->pos_;

    li = new Info;
    li->lnm_ = lnm;
    li->pos_.setLineName( lnm );
    data_ += li;
    return li->pos_;
}


void PosInfo::LineSet2DData::removeLine( const char* lnm )
{
    PosInfo::LineSet2DData::Info* li = findLine( lnm );
    if ( li )
    {
	data_ -= li;
	delete li;
    }
}


void PosInfo::LineSet2DData::intersect( const BinIDValueSet& bivset,
			ObjectSet<PosInfo::LineSet2DData::IR>& resultset ) const
{
    BinIDValueSet* globalbivset = new BinIDValueSet( bivset.nrVals(), true );
    for ( int idx=0; idx<nrLines(); idx++ )
    {
	BinIDValueSet* newbivset = new BinIDValueSet( bivset.nrVals(), true );
	BinID prevbid(-1,-1);
	for ( int idy=0; idy<lineData(idx).positions().size(); idy++ )
	{
	    BinID bid = SI().transform( lineData(idx).positions()[idy].coord_ );
	    if ( bid == prevbid ) continue;
	    prevbid = bid;
	    if ( bivset.includes(bid) )
	    {
		BinIDValueSet::Pos pos = bivset.findFirst(bid);

		while ( true )
		{
		    BinIDValues bidvalues;
		    bivset.get(pos,bidvalues);
		    if ( !globalbivset->areBinidValuesThere( bidvalues ) )
		    {
			newbivset->add(bidvalues);
			globalbivset->add(bidvalues);
		    }
		    bivset.next( pos );
		    if ( bid != bivset.getBinID(pos) )
			break;
		}
	    }
	}

	if ( newbivset->totalSize() > 0 )
	{
	    IR* result = new IR;
	    result->lnm_ = lineName(idx);
	    result->posns_ = newbivset;
	    resultset += result;
	}
	else
	    delete newbivset;
    }
}


float PosInfo::LineSet2DData::getDistBetwTrcs( bool ismax,
					       const char* linenm ) const
{
    Stats::CalcSetup rcsetup;
    if ( ismax )
	rcsetup.require( Stats::Max );
    else
	rcsetup.require( Stats::Median );

    Stats::RunCalc<float> stats( rcsetup );
    for ( int idx=0; idx<trcdiststatsperlines_.size(); idx++ )
    {
	if ( BufferString(linenm) == trcdiststatsperlines_[idx].linename_ )
	    return ismax ? trcdiststatsperlines_[idx].maxdist_
			 : trcdiststatsperlines_[idx].mediandist_;
	else
	    stats += ismax ? trcdiststatsperlines_[idx].maxdist_
			   : trcdiststatsperlines_[idx].mediandist_;
    }

    return ismax ? stats.max() : stats.median();
}


void PosInfo::LineSet2DData::compDistBetwTrcsStats()
{
    Stats::CalcSetup rcsetup;
    rcsetup.require( Stats::Max );
    rcsetup.require( Stats::Median );
    for ( int lidx=0; lidx<nrLines(); lidx++ )
    {
	Stats::RunCalc<float> stats( rcsetup );
	const TypeSet<PosInfo::Line2DPos>& posns = lineData(lidx).positions();
	for ( int pidx=1; pidx<posns.size(); pidx++ )
	{
	    const double distsq =
			posns[pidx].coord_.sqDistTo( posns[pidx-1].coord_ );

	    stats += (float)Math::Sqrt(distsq);
	}

	LineTrcDistStats ltrcdiststats( lineName( lidx ), stats.median(),
					stats.max() );

	trcdiststatsperlines_ += ltrcdiststats;
    }
}
