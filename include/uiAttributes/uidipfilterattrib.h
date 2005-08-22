#ifndef ui3dfilterattrib_h
#define ui3dfilterattrib_h

/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        Nanne Hemstra
 Date:          July 2001
 RCS:           $Id: uidipfilterattrib.h,v 1.4 2005-08-22 15:33:53 cvsnanne Exp $
________________________________________________________________________

-*/

#include "uiattrdesced.h"

class uiAttrSel;
class uiGenInput;
class uiLabeledSpinBox;

/*! \brief DipFilter Attribute description editor */

class ui3DFilterAttrib : public uiAttrDescEd
{
public:

			ui3DFilterAttrib(uiParent*);

    const char*		getAttribName() const;
    void		getEvalParams(TypeSet<EvalParam>&) const;

protected:

    uiAttrSel*          inpfld;
    uiLabeledSpinBox*	szfld;
    uiGenInput*		fltrtpfld;
    uiGenInput*		velfld;
    uiGenInput*		azifld;
    uiGenInput*		aziintfld;
    uiGenInput*		taperfld;
    uiGenInput*         kernelfld;
    uiGenInput*         shapefld;
    uiGenInput*         outpfld;

    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);
    bool		setOutput(const Attrib::Desc&);

    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);
    bool		getOutput(Attrib::Desc&);

    virtual void	set2D(bool);

    void		filtSel(CallBacker*);
    void		aziSel(CallBacker*);
    void		kernelSel(CallBacker*);
};



#endif
