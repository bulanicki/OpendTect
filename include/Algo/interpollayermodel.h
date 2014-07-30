#ifndef interpollayermodel_h
#define interpollayermodel_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Nanne Hemstra
 Date:		January 2014
 RCS:		$Id$
________________________________________________________________________

-*/


#include "algomod.h"
#include "cubesampling.h"
#include "factory.h"
#include "typeset.h"

class TaskRunner;

mExpClass(Algo) InterpolationLayerModel
{
public:
			mDefineFactoryInClass(InterpolationLayerModel,factory)
    virtual		~InterpolationLayerModel();

    virtual int		nrLayers() const				= 0;
    virtual float	getZ(const BinID&,int layer) const		= 0;
    virtual float	getLayerIndex(const BinID&,float z) const	= 0;
    virtual void	getAllZ(const BinID&,TypeSet<float>&) const	= 0;
    virtual bool	hasPosition(const BinID&) const			= 0;
    virtual bool	prepare(TaskRunner*)				= 0;

    static const char*	sKeyModelType();

    virtual void	fillPar(IOPar&) const;
    virtual bool	usePar(const IOPar&)		{ return true; }

};


mExpClass(Algo) ZSliceInterpolationModel : public InterpolationLayerModel
{
public:
			mDefaultFactoryInstantiation(
				InterpolationLayerModel,
				ZSliceInterpolationModel,
				"ZSlices",
				"Z Slices")

    void		setCubeSampling(const CubeSampling&);

    int			nrLayers() const;
    float		getZ(const BinID&,int layer) const;
    float		getLayerIndex(const BinID&,float z) const;
    void		getAllZ(const BinID&,TypeSet<float>&) const;
    bool		hasPosition(const BinID&) const;
    bool		prepare(TaskRunner*)	{ return true; }

protected:

    CubeSampling	cs_;
};

#endif
