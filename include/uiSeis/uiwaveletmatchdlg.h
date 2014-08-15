#ifndef uiwaveletmatchdlg_h
#define uiwaveletmatchdlg_h
/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Nanne Hemstra
 Date:          June 2014
 RCS:           $Id$
________________________________________________________________________

-*/

#include "uiseismod.h"

#include "multiid.h"
#include "uidialog.h"

class uiIOObjSel;
class uiSeisWaveletSel;

mExpClass(uiSeis) uiWaveletMatchDlg : public uiDialog
{ mODTextTranslationClass(uiWaveletMatchDlg)
public:
			uiWaveletMatchDlg(uiParent*);

    const MultiID&	getMultiID() const	{ return wvltid_; }

protected:

    bool		acceptOK(CallBacker*);

    uiSeisWaveletSel*	wvlt0fld_;
    uiSeisWaveletSel*	wvlt1fld_;
    uiIOObjSel*		outwvltfld_;

    MultiID		wvltid_;
};

#endif
