#ifndef visvolumedisplay_h
#define visvolumedisplay_h

/*+
________________________________________________________________________

 CopyRight:	(C) de Groot-Bril Earth Sciences B.V.
 Author:	N. Hemstra
 Date:		August 2002
 RCS:		$Id: visvolumedisplay.h,v 1.21 2003-05-15 09:36:02 nanne Exp $
________________________________________________________________________


-*/


#include "visobject.h"
#include "vissurvobj.h"
#include "ranges.h"

class AttribSelSpec;
class CubeSampling;
class AttribSliceSet;

namespace visBase
{
    class CubeView;
    class VisColorTab;
    class Material;
};

namespace visSurvey
{

class Scene;

/*!\brief
    VolumeDisplay is a 3DTexture with 3 planes that display seismics or attributes.
*/

class VolumeDisplay :  public visBase::VisualObject,
			  public visSurvey::SurveyObject
{
public:

    static VolumeDisplay*	create()
				mCreateDataObj(VolumeDisplay);

    void			setCenter( const Coord3& );
    Coord3			center() const;
    Coord3			manipCenter() const;
    void			setWidth( const Coord3& );
    Coord3			width() const;
    Coord3			manipWidth() const;

    void			showBox(bool yn);
    void			resetManip();
    void			getPlaneIds(int&,int&,int&);
    float			getPlanePos(int);

    bool			updateAtNewPos();
    AttribSelSpec&		getAttribSelSpec();
    const AttribSelSpec&	getAttribSelSpec() const;
    void			setAttribSelSpec(const AttribSelSpec&);
    CubeSampling&		getCubeSampling(bool manippos=true);
    const CubeSampling&		getCubeSampling(bool manippos=true) const
				{ return const_cast<VolumeDisplay*>(this)->
				    	getCubeSampling(manippos); }
    void			setCubeSampling(const CubeSampling&);
    bool			putNewData( AttribSliceSet* );
    const AttribSliceSet*	getPrevData() const;
    float			getValue(const Coord3&) const;

    void			turnOn(bool);
    bool			isOn() const;

    void			setColorTab(visBase::VisColorTab&);
    visBase::VisColorTab&	getColorTab();
    const visBase::VisColorTab&	getColorTab() const;
    const TypeSet<float>& 	getHistogram() const;

    void                        setMaterial( visBase::Material* );
    				/*!< Does not affect the volren */
    visBase::Material*		getMaterial();
    				/*!< Does not affect the volren */
    const visBase::Material*	getMaterial() const;
    				/*!< Does not affect the volren */

    int				getVolRenId() const;
    bool			isVolRenShown() const;
    SoNode*			getData();

    virtual void		fillPar( IOPar&, TypeSet<int>& ) const;
    virtual int			usePar( const IOPar& );

    Notifier<VolumeDisplay>	moved;
    Notifier<VolumeDisplay>	slicemoving;


protected:
				~VolumeDisplay();

    void			initSlice(int);
    void			manipMotionFinishCB(CallBacker*);
    void			sliceMoving(CallBacker*);

    visBase::CubeView*		cube;
    AttribSliceSet*		cache;

    AttribSelSpec&		as;

    int				inlid;
    int				crlid;
    int				tslid;

    static const char*		volumestr;
    static const char*		inlinestr;
    static const char*		crosslinestr;
    static const char*		timestr;
    static const char*		volrenstr;
    static const char*		inlineposstr;
    static const char*		crosslineposstr;
    static const char*		timeposstr;
    static const char*		inlineshowstr;
    static const char* 		crosslineshowstr;
    static const char* 		timeshowstr;

};

};


#endif
