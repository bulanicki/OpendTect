#ifndef uifrequencyattrib_h
#define uifrequencyattrib_h

/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        Nanne Hemstra
 Date:          July 2001
 RCS:           $Id: uifrequencyattrib.h,v 1.2 2005-08-22 15:33:53 cvsnanne Exp $
________________________________________________________________________

-*/

#include "uiattrdesced.h"

namespace Attrib { class Desc; };
class uiGenInput;
class uiImagAttrSel;

/*! \brief Frequency Attribute description editor */

class uiFrequencyAttrib : public uiAttrDescEd
{
public:

			uiFrequencyAttrib(uiParent*);

    void		getEvalParams(TypeSet<EvalParam>&) const;

protected:

    uiImagAttrSel*	inpfld;
    uiGenInput*         gatefld;
    uiGenInput*		normfld;
    uiGenInput*		winfld;
    uiGenInput*		outpfld;

    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);
    bool		setOutput(const Attrib::Desc&);

    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);
    bool		getOutput(Attrib::Desc&);
};

#endif
