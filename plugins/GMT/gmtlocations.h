#ifndef gmtloc_h
#define gmtloc_h

/*+
________________________________________________________________________

 CopyRight:	(C) dGB Beheer B.V.
 Author:	Raman Singh
 Date:		July 2008
 RCS:		$Id: gmtlocations.h,v 1.2 2008-08-06 09:58:20 cvsraman Exp $
________________________________________________________________________

-*/

#include "gmtpar.h"


class GMTLocations : public GMTPar
{
public:

    static void		initClass();

    			GMTLocations(const char* nm)
			    : GMTPar(nm)	{}
			GMTLocations(const IOPar& par)
			    : GMTPar(par) {}

    virtual bool	execute(std::ostream&,const char*);
    virtual const char* userRef() const;
    bool		fillLegendPar(IOPar&) const;

protected:

    static GMTPar*	createInstance(const IOPar&);
    static int		factoryid_;
};


class GMTPolyline : public GMTPar
{
public:

    static void		initClass();

    			GMTPolyline(const char* nm)
			    : GMTPar(nm)	{}
			GMTPolyline(const IOPar& par)
			    : GMTPar(par) {}

    virtual bool	execute(std::ostream&,const char*);
    virtual const char* userRef() const;
    bool		fillLegendPar(IOPar&) const;

protected:

    static GMTPar*	createInstance(const IOPar&);
    static int		factoryid_;
};

#endif
