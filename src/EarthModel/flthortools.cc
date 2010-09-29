/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Nanne Hemstra
 Date:		October 2008
________________________________________________________________________

-*/
static const char* rcsID = "$Id: flthortools.cc,v 1.26 2010-09-29 03:56:25 satyaki Exp $";

#include "flthortools.h"

#include "binidvalset.h"
#include "emfaultstickset.h"
#include "emfault3d.h"
#include "emmanager.h"
#include "executor.h"
#include "explfaultsticksurface.h"
#include "explplaneintersection.h"
#include "faultsticksurface.h"
#include "ioman.h"
#include "ioobj.h"
#include "posinfo2d.h"
#include "seis2dline.h"
#include "survinfo.h"
#include "surv2dgeom.h"
#include "trigonometry.h"


int FaultTrace::nextID( int previd ) const
{ return previd >= -1 && previd < coords_.size()-1 ? previd + 1 : -1; }

void FaultTrace::setIndices( const TypeSet<int>& indices )
{ coordindices_ = indices; }

const TypeSet<int>& FaultTrace::getIndices() const
{ return coordindices_; }

int FaultTrace::add( const Coord3& pos )
{
    lock_.lock();
    coords_ += pos;
    lock_.unLock();
    return coords_.size() - 1;
}


int FaultTrace::add( const Coord3& pos, int trcnr )
{
    lock_.lock();
    coords_ += pos;
    trcnrs_ += trcnr;
    lock_.unLock();
    return coords_.size() - 1;
}


Coord3 FaultTrace::get( int idx ) const
{ return idx >= 0 && idx < coords_.size() ? coords_[idx] : Coord3::udf(); }

int FaultTrace::getTrcNr( int idx ) const
{ return idx >= 0 && idx < trcnrs_.size() ? trcnrs_[idx] : -1; }

void FaultTrace::set( int idx, const Coord3& pos )
{
    lock_.lock();
    if ( idx >= 0 && idx < coords_.size() )
	coords_[idx] = pos;

    lock_.unLock();
}


void FaultTrace::set( int idx, const Coord3& pos, int trcnr )
{
    lock_.lock();
    if ( idx >= 0 && idx < coords_.size() )
    {
	coords_[idx] = pos;
	if ( idx < trcnrs_.size() )
	    trcnrs_[idx] = trcnr;
    }

    lock_.unLock();
}


void FaultTrace::remove( int idx )
{
    lock_.lock();
    coords_.remove( idx );
    lock_.unLock();
}


bool FaultTrace::isDefined( int idx ) const
{ return idx >= 0 && idx < coords_.size() && coords_[idx] != Coord3::udf(); }

FaultTrace* FaultTrace::clone()
{
    FaultTrace* newobj = new FaultTrace;
    newobj->coords_ = coords_;
    newobj->trcnrs_ = trcnrs_;
    return newobj;
}


float FaultTrace::getZValFor( const BinID& bid ) const
{
    const StepInterval<float>& zrg = SI().zRange( false );
    Coord pt1( isinl_ ? bid.crl : bid.inl, zrg.start * SI().zFactor() );
    Coord pt2( isinl_ ? bid.crl : bid.inl, zrg.stop * SI().zFactor() );
    Line2 line( pt1, pt2 );
    TypeSet<float> intersections;
    for ( int idx=1; idx<coordindices_.size(); idx++ )
    {
	if ( coordindices_[idx] < 0 )
	{
	    idx += 2;
	    continue;
	}

	const Coord3& pos1 = get( coordindices_[idx-1] );
	const Coord3& pos2 = get( coordindices_[idx] );

	const Coord posbid1 = SI().binID2Coord().transformBackNoSnap( pos1 );
	const Coord posbid2 = SI().binID2Coord().transformBackNoSnap( pos2 );

	Coord nodepos1( isinl_ ? posbid1.y : posbid1.x,
			pos1.z * SI().zFactor() );
	Coord nodepos2( isinl_ ? posbid2.y : posbid2.x,
			pos2.z * SI().zFactor() );
	Line2 fltseg( nodepos1, nodepos2 );
	Coord interpos = line.intersection( fltseg );
	if ( interpos != Coord::udf() )
	    intersections += interpos.y;
    }

    return intersections.size() == 1 ? intersections[0] : mUdf(float);
}


bool FaultTrace::isCrossing( const BinID& bid1, float z1,
			     const BinID& bid2, float z2  ) const
{
    if ( !getSize() )
	return false;

    const bool is2d = trcnrs_.size();
    if ( trcnrs_.size() != getSize() )
	return false;

    z1 *= SI().zFactor();
    z2 *= SI().zFactor();
    if ( ( isinl_ && (bid1.inl != nr_ || bid2.inl != nr_) )
	    || ( !isinl_ && (bid1.crl != nr_ || bid2.crl != nr_) ) )
	return false;

    Coord pt1( isinl_ ? bid1.crl : bid1.inl, z1 );
    Coord pt2( isinl_ ? bid2.crl : bid2.inl, z2 );
    Line2 line( pt1, pt2 );

    for ( int idx=1; idx<coordindices_.size(); idx++ )
    {
	const int curidx = coordindices_[idx];
	const int previdx = coordindices_[idx-1];
	if ( curidx < 0 || previdx < 0 )
	{
	    idx += 2;
	    continue;
	}

	const Coord3& pos1 = get( previdx );
	const Coord3& pos2 = get( curidx );

	Coord nodepos1, nodepos2;
	if ( is2d )
	{
	    nodepos1.setXY( getTrcNr(previdx), pos1.z * SI().zFactor() );
	    nodepos2.setXY( getTrcNr(curidx), pos2.z * SI().zFactor() );
	}
	else
	{
	    Coord posbid1 = SI().binID2Coord().transformBackNoSnap( pos1 );
	    Coord posbid2 = SI().binID2Coord().transformBackNoSnap( pos2 );
	    nodepos1.setXY( isinl_ ? posbid1.y : posbid1.x,
		    	   pos1.z * SI().zFactor() );
	    nodepos2.setXY( isinl_ ? posbid2.y : posbid2.x,
		    	   pos2.z * SI().zFactor() );
	}

	Line2 fltseg( nodepos1, nodepos2 );
	Coord interpos = line.intersection( fltseg );
	if ( interpos != Coord::udf() )
	    return true;
    }

    return false;
}



FaultTraceExtractor::FaultTraceExtractor( EM::Fault* flt,
					  int nr, bool isinl,
					  const BinIDValueSet* bvset )
  : fault_(flt)
  , nr_(nr), isinl_(isinl)
  , bvset_(bvset)
  , flttrc_(0)
  , is2d_(false)
{
    fault_->ref();
}


FaultTraceExtractor::FaultTraceExtractor( EM::Fault* flt,
					  const char* linenm, int sticknr,
					  const BinIDValueSet* bvset )
  : fault_(flt)
  , sticknr_(sticknr)
  , nr_(0),isinl_(true)
  , linenm_(linenm)
  , bvset_(bvset)
  , flttrc_(0)
  , is2d_(true)
{
    fault_->ref();
}



FaultTraceExtractor::~FaultTraceExtractor()
{
    fault_->unRef();
    if ( flttrc_ ) flttrc_->unRef();
}


bool FaultTraceExtractor::execute()
{
    if ( flttrc_ )
    {
	flttrc_->unRef();
	flttrc_ = 0;
    }

    if ( is2d_ )
	return get2DFaultTrace();

    EM::SectionID fltsid( 0 );
    mDynamicCastGet(EM::Fault3D*,fault3d,fault_)
    Geometry::IndexedShape* fltsurf = new Geometry::ExplFaultStickSurface(
	    	fault3d->geometry().sectionGeometry(fltsid), SI().zFactor() );
    fltsurf->setCoordList( new FaultTrace, new FaultTrace, 0 );
    if ( !fltsurf->update(true,0) )
	return false;

    CubeSampling cs;
    BinID start( isinl_ ? nr_ : cs.hrg.start.inl,
	    	 isinl_ ? cs.hrg.start.crl : nr_ );
    BinID stop( isinl_ ? nr_ : cs.hrg.stop.inl,
	    	isinl_ ? cs.hrg.stop.crl : nr_ );
    Coord3 p0( SI().transform(start), cs.zrg.start );
    Coord3 p1( SI().transform(start), cs.zrg.stop );
    Coord3 p2( SI().transform(stop), cs.zrg.stop );
    Coord3 p3( SI().transform(stop), cs.zrg.start );
    TypeSet<Coord3> pts;
    pts += p0; pts += p1; pts += p2; pts += p3;
    const Coord3 normal = (p1-p0).cross(p3-p0).normalize();

    Geometry::ExplPlaneIntersection* insectn =
					new Geometry::ExplPlaneIntersection;
    insectn->setShape( *fltsurf );
    insectn->addPlane( normal, pts );
    Geometry::IndexedShape* idxdshape = insectn;
    idxdshape->setCoordList( new FaultTrace, new FaultTrace, 0 );
    if ( !idxdshape->update(true,0) )
	return false;

    Coord3List* clist = idxdshape->coordList();
    mDynamicCastGet(FaultTrace*,flttrc,clist);
    if ( !flttrc ) return false;

    const Geometry::IndexedGeometry* idxgeom = idxdshape->getGeometry()[0];
    if ( !idxgeom ) return false;

    flttrc_ = flttrc->clone();
    flttrc_->ref();
    flttrc_->setIndices( idxgeom->coordindices_ );
    flttrc_->setIsInl( isinl_ );
    flttrc_->setLineNr( nr_ );
    if ( bvset_ )
	useHorizons();

    delete fltsurf;
    delete insectn;
    return true;
}
    

bool FaultTraceExtractor::get2DFaultTrace()
{
    EM::SectionID fltsid( 0 );
    mDynamicCastGet(const EM::FaultStickSet*,fss,fault_)
    if ( !fss ) return false;

    const int nrknots = fss->geometry().nrKnots( fltsid, sticknr_ );
    const Geometry::FaultStickSet* fltgeom =
	fss->geometry().sectionGeometry( fltsid );
    if ( !fltgeom || nrknots < 2 )
	return false;

    const MultiID* lsetid = fss->geometry().lineSet( fltsid, sticknr_ );
    PtrMan<IOObj> ioobj = IOM().get( *lsetid );
    if ( !ioobj )
	return false;

    Seis2DLineSet lset( *ioobj );
    LineKey lk( linenm_.buf(), LineKey::sKeyDefAttrib() );
    int lidx = lset.indexOf( lk );
    if ( lidx < 0 ) lidx = lset.indexOfFirstOccurrence( linenm_.buf() );
    if ( lidx < 0 ) return false;

    PosInfo::POS2DAdmin().setCurLineSet( lset.name() );
    PosInfo::Line2DData linegeom( lk.lineName() );
    if ( !PosInfo::POS2DAdmin().getGeometry(linegeom) )
	return false;

    flttrc_ = new FaultTrace;
    flttrc_->ref();
    flttrc_->setIsInl( true );
    flttrc_->setLineNr( 0 );
    StepInterval<int> colrg = fltgeom->colRange( sticknr_ );
    for ( int idx=colrg.start; idx<=colrg.stop; idx+=colrg.step )
    {
	const Coord3 knot = fltgeom->getKnot( RowCol(sticknr_,idx) );
	PosInfo::Line2DPos pos;
	if ( !linegeom.getPos(knot,pos,1.0) )
	    break;

	flttrc_->add( knot, pos.nr_ );
    }

    return flttrc_->getSize() > 1;
}


void FaultTraceExtractor::useHorizons()
{
    if ( flttrc_->getSize() < 2 )
	return;

    const Coord3 toppos = flttrc_->get( 0 );
    const Coord3 botpos = flttrc_->get( flttrc_->getSize() - 1 );

    const BinID topbid = SI().transform( toppos );
    const BinID botbid = SI().transform( botpos );
    const BinIDValueSet::Pos topbvspos = bvset_->findFirst( topbid );
    const BinIDValueSet::Pos botbvspos = bvset_->findFirst( botbid );
    float topz=mUdf(float), botz=mUdf(float), dummyz;
    BinID dummy;
    if ( topbvspos.valid() )
	bvset_->get( topbvspos, dummy, topz, dummyz );
    if ( botbvspos.valid() )
	bvset_->get( botbvspos, dummy, dummyz, botz );

    if ( !mIsUdf(topz) && toppos.z > topz )
    {
	const Coord posbid = SI().binID2Coord().transformBackNoSnap( toppos );
	const Coord3 nextpos = flttrc_->get( 1 );
	const Coord nextposbid =
	    		SI().binID2Coord().transformBackNoSnap( nextpos );

	Coord nodepos1( isinl_ ? posbid.y : posbid.x,
			toppos.z * SI().zFactor() );
	Coord nodepos2( isinl_ ? nextposbid.y : nextposbid.x,
			nextpos.z * SI().zFactor() );
	Line2 fltseg( nodepos1, nodepos2 );
	fltseg.start_ = Coord::udf();
	fltseg.stop_ = Coord::udf();

	const bool isinc = isinl_ ? posbid.y > nextposbid.y
	    			  : posbid.x > nextposbid.x;
	const BinID incbid( isinl_ ? 0 : (isinc ? 1 : -1),
			    isinl_ ? (isinc ? 1 : -1) : 0 );
	BinID start = topbid - incbid;
	for ( int idx=0; idx<1024; idx++ )
	{
	    const BinID stop = start + incbid;
	    const BinIDValueSet::Pos startpos = bvset_->findFirst( start );
	    const BinIDValueSet::Pos stoppos = bvset_->findFirst( stop );
	    float starttopz=mUdf(float), stoptopz=mUdf(float);
	    if ( startpos.valid() )
		bvset_->get( startpos, dummy, starttopz, dummyz );
	    if ( stoppos.valid() )
		bvset_->get( stoppos, dummy, stoptopz, dummyz );

	    if ( mIsUdf(stoptopz) )
	    {
		const Coord3 newstartpos( SI().transform(start), starttopz );
		flttrc_->set( 0, newstartpos );
		break;
	    }

	    const Coord starttop( isinl_ ? start.crl : start.inl,
		    		  starttopz * SI().zFactor() );
	    const Coord stoptop( isinl_ ? stop.crl : stop.inl,
		    		 stoptopz * SI().zFactor() );
	    start = stop;
	    Line2 topseg( starttop, stoptop );
	    Coord interpos = topseg.intersection( fltseg );
	    if ( interpos != Coord::udf() )
	    {
		Coord interbid( isinl_ ? nr_ : interpos.x,
				isinl_ ? interpos.x : nr_ );
		Coord newpos = SI().binID2Coord().transform( interbid );
		Coord3 newpos3( newpos, interpos.y / SI().zFactor() );
		flttrc_->set( 0, newpos3 );
		break;
	    }
	}
    }

    if ( !mIsUdf(botz) && botpos.z < botz )
    {
	const Coord posbid = SI().binID2Coord().transformBackNoSnap( botpos );
	const Coord3 nextpos = flttrc_->get( flttrc_->getSize() - 2 );
	const Coord nextposbid =
	    		SI().binID2Coord().transformBackNoSnap( nextpos );

	Coord nodepos1( isinl_ ? posbid.y : posbid.x,
			botpos.z * SI().zFactor() );
	Coord nodepos2( isinl_ ? nextposbid.y : nextposbid.x,
			nextpos.z * SI().zFactor() );
	Line2 fltseg( nodepos1, nodepos2 );
	fltseg.start_ = Coord::udf();
	fltseg.stop_ = Coord::udf();

	const bool isinc = isinl_ ? posbid.y > nextposbid.y
	    			  : posbid.x > nextposbid.x;
	const BinID incbid( isinl_ ? 0 : (isinc ? 1 : -1),
			    isinl_ ? (isinc ? 1 : -1) : 0 );
	BinID start = botbid - incbid;
	for ( int idx=0; idx<1024; idx++ )
	{
	    const BinID stop = start + incbid;
	    const BinIDValueSet::Pos startpos = bvset_->findFirst( start );
	    const BinIDValueSet::Pos stoppos = bvset_->findFirst( stop );
	    float startbotz=mUdf(float), stopbotz=mUdf(float);
	    if ( startpos.valid() )
		bvset_->get( startpos, dummy, startbotz, dummyz );
	    if ( stoppos.valid() )
		bvset_->get( stoppos, dummy, stopbotz, dummyz );

	    if ( mIsUdf(stopbotz) )
	    {
		const Coord3 newstoppos( SI().transform(start), stopbotz );
		flttrc_->set( flttrc_->getSize() - 1, newstoppos );
		break;
	    }

	    const Coord startbot( isinl_ ? start.crl : start.inl,
		    		  startbotz * SI().zFactor() );
	    const Coord stopbot( isinl_ ? stop.crl : stop.inl,
		    		 stopbotz * SI().zFactor() );
	    start = stop;
	    Line2 botseg( startbot, stopbot );
	    Coord interpos = botseg.intersection( fltseg );
	    if ( interpos != Coord::udf() )
	    {
		Coord interbid( isinl_ ? nr_ : interpos.x,
				isinl_ ? interpos.x : nr_ );
		Coord newpos = SI().binID2Coord().transform( interbid );
		Coord3 newpos3( newpos, interpos.y / SI().zFactor() );
		flttrc_->set( flttrc_->getSize() - 1, newpos3 );
		break;
	    }
	}
    }
}
