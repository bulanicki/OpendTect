#ifndef uisegyimpdlg_h
#define uisegyimpdlg_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Bert
 Date:          Sep 2008
 RCS:           $Id$
________________________________________________________________________

-*/

#include "uisegycommon.h"
#include "uisegyreaddlg.h"
#include "uistring.h"
class uiSeisSel;
class CtxtIOObj;
class uiCheckBox;
class uiSeisTransfer;
class uiBatchJobDispatcherSel;


/*!\brief Dialog to import SEG-Y files after basic setup. */

mExpClass(uiSEGY) uiSEGYImpDlg : public uiSEGYReadDlg
{ mODTextTranslationClass(uiSEGYImpDlg);
public :

			uiSEGYImpDlg(uiParent*,const Setup&,IOPar&);

    virtual void	use(const IOObj*,bool force);
    virtual MultiID	outputID() const;


protected:

    uiSeisTransfer*	transffld_;
    uiSeisSel*		seissel_;
    uiCheckBox*		morebut_;
    uiBatchJobDispatcherSel* batchfld_;

    virtual bool	doWork(const IOObj&);

    friend class	uiSEGYImpSimilarDlg;
    bool		impFile(const IOObj&,const IOObj&,const char*);

};


#endif
