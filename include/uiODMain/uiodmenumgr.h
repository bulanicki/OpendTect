#ifndef uiodmenumgr_h
#define uiodmenumgr_h
/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        A.H. Bril
 Date:          Dec 2003
 RCS:           $Id: uiodmenumgr.h,v 1.39 2008-10-20 10:58:52 cvsnanne Exp $
________________________________________________________________________

-*/

#include "uiodapplmgr.h"

class uiMenuItem;
class uiODHelpMenuMgr;
class uiPopupMenu;
class uiToolBar;
class DirList;
class uiPopupMenu;


/*!\brief The OpendTect menu manager

  The uiODMenuMgr instance can be accessed like:
  ODMainWin()->menuMgr()

  All standard menus should be reachable directly without searching for
  the text. It is easy to add your own menu items. And tool buttons, for that
  matter.

*/

class uiODMenuMgr : public CallBacker
{

    friend class	uiODMain;
    friend class	uiODHelpMenuMgr;

public:

    // TODO: winMnu() only here for backward compatibility
    // Remove in version 3.4
    uiPopupMenu*	fileMnu()		{ return surveymnu_; }
    uiPopupMenu*	surveyMnu()		{ return surveymnu_; }
    uiPopupMenu*	analMnu()		{ return analmnu_; }
    uiPopupMenu*	procMnu()		{ return procmnu_; }
    uiPopupMenu*	winMnu()		{ return scenemnu_; }
    uiPopupMenu*	sceneMnu()		{ return scenemnu_; }
    uiPopupMenu*	viewMnu()		{ return viewmnu_; }
    uiPopupMenu*	utilMnu()		{ return utilmnu_; }
    uiPopupMenu*	helpMnu()		{ return helpmnu_; }
    uiPopupMenu*	settMnu()		{ return settmnu_; }

    uiPopupMenu*	getBaseMnu(uiODApplMgr::ActType);
    			//! < Within Survey menu
    uiPopupMenu*	getMnu(bool imp,uiODApplMgr::ObjType);
    			//! < Within Survey - Import or Export

    uiToolBar*		dtectTB()		{ return dtecttb_; }
    uiToolBar*		coinTB()		{ return cointb_; }
    uiToolBar*		manTB()			{ return mantb_; }


    			// Probably not needed by plugins
    void		updateStereoMenu();
    void		updateViewMode(bool);
    void		updateAxisMode(bool);
    bool		isSoloModeOn() const;
    void		enableMenuBar(bool);
    void		enableActButton(bool);
    void		setCameraPixmap(bool isperspective);
    void		updateSceneMenu();
//    void		updateWindowsMenu() { updateSceneMenu(); }
    			// Backward compatibility, remove in od3.4

    Notifier<uiODMenuMgr> dTectTBChanged;
    Notifier<uiODMenuMgr> dTectMnuChanged;

protected:

			uiODMenuMgr(uiODMain*);
			~uiODMenuMgr();
    void		initSceneMgrDepObjs(uiODApplMgr*,uiODSceneMgr*);

    uiODMain&		appl_;
    uiODHelpMenuMgr*	helpmgr_;

    uiPopupMenu*	surveymnu_;
    uiPopupMenu*	analmnu_;
    uiPopupMenu*	procmnu_;
    uiPopupMenu*	scenemnu_;
    uiPopupMenu*	viewmnu_;
    uiPopupMenu*	utilmnu_;
    uiPopupMenu*	impmnu_;
    uiPopupMenu*	expmnu_;
    uiPopupMenu*	manmnu_;
    uiPopupMenu*	helpmnu_;
    uiPopupMenu*	settmnu_;
    ObjectSet<uiPopupMenu> impmnus_;
    ObjectSet<uiPopupMenu> expmnus_;

    uiToolBar*		dtecttb_;
    uiToolBar*		cointb_;
    uiToolBar*		mantb_;

    void		fillSurveyMenu();
    void		fillImportMenu();
    void		fillExportMenu();
    void		fillManMenu();
    void		fillAnalMenu();
    void		fillProcMenu();
    void		fillSceneMenu();
    void		fillViewMenu();
    void		fillUtilMenu();
    void		fillDtectTB(uiODApplMgr*);
    void		fillCoinTB(uiODSceneMgr*);
    void		fillManTB();

    void		selectionMode(CallBacker*);
    void		handleToolClick(CallBacker*);
    void		handleClick(CallBacker*);
    void		dispColorBar(CallBacker*);
    void		manSeis(CallBacker*);
    void		manHor(CallBacker*);
    void		manFlt(CallBacker*);
    void		manWll(CallBacker*);
    void		manPick(CallBacker*);
    void		manWvlt(CallBacker*);
    void		manStrat(CallBacker*);
    void		updateDTectToolBar(CallBacker*);
    void		updateDTectMnus(CallBacker*);

    uiMenuItem*		stereooffitm_;
    uiMenuItem*		stereoredcyanitm_;
    uiMenuItem*		stereoquadbufitm_;
    uiMenuItem*		stereooffsetitm_;
    int			axisid_, actid_, viewid_, cameraid_, soloid_;
    int			coltabid_, polyselectid_;

    inline uiODApplMgr&	applMgr()	{ return appl_.applMgr(); }
    inline uiODSceneMgr& sceneMgr()	{ return appl_.sceneMgr(); }

    void		showLogFile();
    void		mkViewIconsMnu();
    void		addIconMnuItems(const DirList&,uiPopupMenu*,int&);
};


#endif
