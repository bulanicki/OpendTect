#ifndef visobject_h
#define visobject_h

/*+
________________________________________________________________________

 CopyRight:	(C) dGB Beheer B.V.
 Author:	Kris Tingdahl
 Date:		Jan 2002
 RCS:		$Id: visobject.h,v 1.21 2004-01-05 09:43:47 kristofer Exp $
________________________________________________________________________


-*/

#include "visdata.h"
#include "color.h"

class SoSeparator;
class SoNode;
class SoSwitch;

namespace visBase
{
class Material;
class Transformation;

/*!\brief
    Base class for all objects that are visual on the scene.
*/

class VisualObject : public DataObject
{
public:
    virtual void		turnOn(bool)				= 0;
    virtual bool		isOn() const				= 0;

    virtual void		setMaterial( Material* )		= 0;
    virtual Material*		getMaterial()				= 0;

    void			setSelectable( bool yn ) { isselectable=yn; }
    bool			selectable() const { return isselectable; }
    NotifierAccess*		selection() { return &selnotifier; }
    NotifierAccess*		deSelection() { return &deselnotifier; }

    virtual int			usePar( const IOPar& iopar )
				{ return DataObject::usePar(iopar); }
    virtual void		fillPar( IOPar& iopar,
	    				 TypeSet<int>& saveids ) const
				{ DataObject::fillPar( iopar, saveids );}

protected:
    void		triggerSel()
    			{ if (isselectable) selnotifier.trigger(); }

    void		triggerDeSel()
    			{ if (isselectable) deselnotifier.trigger(); }

			VisualObject(bool selectable=false);
			~VisualObject();

private:
    bool			isselectable;
    Notifier<VisualObject>	selnotifier;
    Notifier<VisualObject>	deselnotifier;
};


class VisualObjectImpl : public VisualObject
{
public:
    void		turnOn(bool);
    bool		isOn() const;

    void		setMaterial( Material* );
    const Material*	getMaterial() const { return material; }
    Material*		getMaterial() { return material; }

    SoNode*		getInventorNode();

    virtual int		usePar( const IOPar& iopar );
    virtual void	fillPar( IOPar& iopar, TypeSet<int> & ) const;
protected:
    void		addChild( SoNode* );
    void		insertChild( int pos, SoNode* );
    void		removeChild( SoNode* );


			VisualObjectImpl(bool selectable=false);
    virtual		~VisualObjectImpl();

    SoSwitch*		onoff;
    Material*		material;

private:
    
    SoSeparator*	root;

    static const char*	materialidstr;
    static const char*	isonstr;

};

};


#endif
