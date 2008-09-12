#ifndef uigmtsymbolpars_h
#define uigmtsymbolpars_h

/*+
________________________________________________________________________

 CopyRight:	(C) dGB Beheer B.V.
 Author:	Raman Singh
 Date:		Sept 2008
 RCS:		$Id: uigmtsymbolpars.h,v 1.1 2008-09-12 11:32:30 cvsraman Exp $
________________________________________________________________________

-*/

#include "uigroup.h"

class IOPar;
class uiCheckBox;
class uiColorInput;
class uiComboBox;
class uiGenInput;

class uiGMTSymbolPars : public uiGroup
{
public:

    			uiGMTSymbolPars(uiParent*);

    bool		fillPar(IOPar&) const;
    bool		usePar(const IOPar&);

protected:

    uiComboBox*		shapefld_;
    uiGenInput*		sizefld_;
    uiCheckBox*		fillfld_;
    uiColorInput*	outcolfld_;
    uiColorInput*	fillcolfld_;

    void		fillSel(CallBacker*);
    void		fillShapes();
};


#endif
