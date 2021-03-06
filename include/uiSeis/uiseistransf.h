#pragma once
/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Bert
 Date:          June 2002
________________________________________________________________________

-*/

#include "uiseismod.h"
#include "uigroup.h"
#include "seisselsetup.h"
#include "seisioobjinfo.h"

class IOObj;
class Scaler;
class Executor;
class uiScaler;
class uiGenInput;
class uiSeisSubSel;
class SeisResampler;
class uiSeis2DSubSel;
class uiSeis3DSubSel;
namespace Seis { class SelData; }


mExpClass(uiSeis) uiSeisTransfer : public uiGroup
{ mODTextTranslationClass(uiSeisTransfer);
public:

    mClass(uiSeis) Setup : public Seis::SelSetup
    {
    public:
			Setup( Seis::GeomType gt )
			    : Seis::SelSetup(gt)
			    , withnullfill_(false)	{}
				    //!< 'add null traces' (3D)
			Setup( const Seis::SelSetup& sss )
			    : Seis::SelSetup(sss)
			    , withnullfill_(false)	{}
			Setup( bool _is2d, bool _isps )
			    : Seis::SelSetup(_is2d,_isps)
			    , withnullfill_(false)	{}

	mDefSetupMemb(bool,withnullfill)
    };

			uiSeisTransfer(uiParent*,const Setup&);
    void		updateFrom(const IOObj&);

    Executor*		getTrcProc(const IOObj& from,const IOObj& to,
				   const char* executor_txt,
				   const uiString& work_txt,
				   const char* linenm2d_overrule=0) const;

    uiSeisSubSel*	selfld;
    uiScaler*		scalefld_;
    uiGenInput*		remnullfld;
    uiGenInput*		trcgrowfld_;

    uiSeis2DSubSel*	selFld2D(); //!< null when not 2D
    uiSeis3DSubSel*	selFld3D(); //!< null when not 3D

    void		setIsSteering(bool);
    void		setInput(const IOObj&);
    Seis::SelData*	getSelData() const;
    SeisResampler*	getResampler() const; //!< may return null

    Scaler*		getScaler() const;
    bool		removeNull() const;
    bool		extendTrcsToSI() const;
    bool		fillNull() const;
    int			nullTrcPolicy() const
			{ return removeNull() ? 0 : (fillNull() ? 2 : 1); }

    void		fillPar(IOPar&) const;
    static const char*	sKeyNullTrcPol()	{ return "Null trace policy"; }

    SeisIOObjInfo::SpaceInfo spaceInfo(int bps=4) const;

protected:

    Setup		setup_;
    bool		issteer_;

    void		updSteer(CallBacker*);

};
