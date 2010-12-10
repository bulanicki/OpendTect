#ifndef uitranslatedlg_h
#define uitranslatedlg_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Nanne Hemstra
 Date:		January 2010
 RCS:		$Id: uitranslatedlg.h,v 1.2 2010-12-10 06:03:58 cvsnanne Exp $
________________________________________________________________________

-*/

#include "uidialog.h"

class uiCheckBox;
class uiComboBox;
class uiGenInput;
class uiPushButton;

mClass uiTranslateDlg : public uiDialog
{
public:
			uiTranslateDlg(uiParent*);
			~uiTranslateDlg();

    bool		enabled() const;

protected:

    void		fillBox();
    bool		acceptOK(CallBacker*);
    void		googleButPushCB(CallBacker*);

    uiComboBox*		languagefld_;
    uiCheckBox*		enabbut_;
    uiGenInput*		keyfld_;
    uiPushButton*	googlebut_;

};

#endif

