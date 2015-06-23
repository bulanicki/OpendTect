#ifndef uisegywriteopts_h
#define uisegywriteopts_h
/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Bert
 Date:		June 2015
 RCS:		$Id$
________________________________________________________________________

-*/

#include "uiseismod.h"
#include "uiioobjselwritetransl.h"
#include "uistring.h"

class uiGenInput;
class uiSEGYFilePars;


mExpClass(uiSeis) uiSEGYDirectVolOpts : public uiIOObjTranslatorWriteOpts
{ mODTextTranslationClass(uiSEGYDirectVolOpts);
public:

			uiSEGYDirectVolOpts(uiParent*);

    mDecluiIOObjTranslatorWriteOptsStdFns(uiSEGYDirectVolOpts);

protected:

    uiSEGYFilePars*	parsfld_;

};


mExpClass(uiSeis) uiCBVSPS3DOpts : public uiIOObjTranslatorWriteOpts
{ mODTextTranslationClass(uiCBVSPS3DOpts);
public:

			uiCBVSPS3DOpts(uiParent*);

    mDecluiIOObjTranslatorWriteOptsStdFns(uiCBVSPS3DOpts);

protected:

    uiGenInput*		stortypfld_;

};


mExpClass(uiSeis) uiSEGYDirectPS3DOpts : public uiIOObjTranslatorWriteOpts
{ mODTextTranslationClass(uiSEGYDirectPS3DOpts);
public:

			uiSEGYDirectPS3DOpts(uiParent*);

    mDecluiIOObjTranslatorWriteOptsStdFns(uiSEGYDirectPS3DOpts);

protected:

    uiSEGYFilePars*	parsfld_;
    uiGenInput*		nrinlpfilefld_;

};


#endif
