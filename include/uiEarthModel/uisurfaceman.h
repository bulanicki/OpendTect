#ifndef uisurfaceman_h
#define uisurfaceman_h
/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        N. Hemstra
 Date:          April 2002
 RCS:           $Id: uisurfaceman.h,v 1.14 2007-08-06 08:51:43 cvsjaap Exp $
________________________________________________________________________

-*/

#include "uiobjfileman.h"
class uiListBox;
class BufferStringSet;


class uiSurfaceMan : public uiObjFileMan
{
public:
			uiSurfaceMan(uiParent*,const char* typ);
			~uiSurfaceMan();

protected:

    uiListBox*		attribfld;

    bool		isCur2D() const;

    void		copyCB(CallBacker*);
    void		setRelations(CallBacker*);
    void		removeAttribCB(CallBacker*);
    void		renameAttribCB(CallBacker*);

    void		mkFileInfo();
    void		fillAttribList(const BufferStringSet&);
};


#endif
