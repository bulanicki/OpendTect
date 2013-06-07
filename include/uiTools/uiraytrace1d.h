#ifndef uiraytracer_h
#define uiraytracer_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Y. Liu
 Date:		January 2011
 RCS:		$Id$
________________________________________________________________________


-*/

#include "callback.h"
#include "raytrace1d.h"
#include "uigroup.h"
#include "uigeninput.h"

class uiLabeledComboBox;


mClass uiRayTracer1D : public uiGroup
{
public:

    mClass Setup 		
    {
	public:	
			Setup()
			    : convertedwaves_(false)
			    , dosourcereceiverdepth_(true)
			    , doreflectivity_(true)
			    , dooffsets_(false)
			    , offsetrg_(0,6000,100)
			    {}

	mDefSetupMemb(bool,convertedwaves);
	mDefSetupMemb(bool,dosourcereceiverdepth);
	mDefSetupMemb(bool,dooffsets);
	mDefSetupMemb(bool,doreflectivity);
	mDefSetupMemb(StepInterval<float>,offsetrg);
    };

    mDefineFactory2ParamInClass(uiRayTracer1D,uiParent*,const Setup&,factory);

    virtual bool 	usePar(const IOPar&);
    virtual void  	fillPar(IOPar&) const;

protected:
			uiRayTracer1D(uiParent*,const Setup&);

    uiGenInput*		srcdepthfld_;
    uiGenInput*		downwavefld_;
    uiGenInput*		upwavefld_;

    uiGenInput* 	offsetfld_;
    uiGenInput* 	offsetstepfld_;

    uiGenInput*		lastfld_;
    bool		doreflectivity_;

public:
    void		displayOffsetFlds(bool yn); 
    void		setOffsetRange(StepInterval<float>);
    Notifier<uiGenInput>& offsetChanged();
    bool		doOffsets() const	{ return offsetfld_; }
};


mClass uiVrmsRayTracer1D : public uiRayTracer1D
{
public:
			uiVrmsRayTracer1D(uiParent*,
					const uiRayTracer1D::Setup&);

    static uiRayTracer1D* create(uiParent* p,const uiRayTracer1D::Setup& s)
			    { return new uiVrmsRayTracer1D(p,s); }

    static void         initClass();
};


mClass uiRayTracerSel : public uiGroup
{
public:
    			uiRayTracerSel(uiParent*,const uiRayTracer1D::Setup&);

    void                usePar(const IOPar&);
    void                fillPar(IOPar&) const;

protected:

    uiLabeledComboBox*	raytracerselfld_;

    ObjectSet<uiRayTracer1D> grps_;

    void		selRayTraceCB(CallBacker*);

public:
    uiRayTracer1D*	current();
    const uiRayTracer1D* current() const;
    void		offsChangedCB(CallBacker*);
    Notifier<uiRayTracerSel>* offsetChanged();
};


#endif
