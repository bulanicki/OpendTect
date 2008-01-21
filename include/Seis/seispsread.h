#ifndef seispsread_h
#define seispsread_h

/*+
________________________________________________________________________

 CopyRight:	(C) dGB Beheer B.V.
 Author:	A.H. Bril
 Date:		Dec 2004
 RCS:		$Id: seispsread.h,v 1.8 2008-01-21 17:56:13 cvsbert Exp $
________________________________________________________________________

-*/

#include "position.h"
class IOPar;
class SeisTrc;
class SeisTrcBuf;
class BufferStringSet;
namespace PosInfo { class CubeData; class Line2DData; }


/*!\brief reads from a pre-stack seismic data store.

 Some data stores like attribute stores have a symbolic name for each sample. In
 that case, getSampleNames may return true.

*/

class SeisPSReader
{
public:

    virtual		~SeisPSReader()					{}

    virtual void	usePar(const IOPar&)				{}

    virtual const char*	errMsg() const					= 0;
    virtual SeisTrc*	getTrace(const BinID&,int nr=0) const;
    virtual bool	getGather(const BinID&,SeisTrcBuf&) const	= 0;

    virtual bool	getSampleNames(BufferStringSet&) const
    			{ return false; }

};

/*!\brief reads from a 3D pre-stack seismic data store. */

class SeisPS3DReader : public SeisPSReader
{
public:

    virtual const PosInfo::CubeData&	posData() const			= 0;

};


/*!\brief reads from a 2D pre-stack seismic data store. */

class SeisPS2DReader : public SeisPSReader
{
public:
    			SeisPS2DReader( const char* lnm )
			    : lnm_(lnm)		{}
    const char*		lineName() const	{ return lnm_.buf(); }

    			// Cannot use name overloading: seems gcc prob
    SeisTrc*		getTrc( int trcnr, int nr=0 ) const
			{ return getTrace( BinID(0,trcnr), nr ); }
    bool		getGath( int trcnr, SeisTrcBuf& b ) const
			{ return getGather( BinID(0,trcnr), b ); }

    virtual const PosInfo::Line2DData&	posData() const		= 0;

protected:

    BufferString	lnm_;

};


#endif
