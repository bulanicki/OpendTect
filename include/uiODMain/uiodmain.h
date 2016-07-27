#ifndef uiodmain_h
#define uiodmain_h
/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        A.H. Bril
 Date:          Dec 2003
________________________________________________________________________

-*/

#include "uiodmainmod.h"
#include "uimainwin.h"
#include "multiid.h"

class CtxtIOObj;
class IOObj;
class ODSession;
class Timer;
class uiMain;
class uiODApplMgr;
class uiODMain;
class uiODMenuMgr;
class uiODSceneMgr;
class uiODViewer2DMgr;
class uiToolBar;
class uiVisColTabEd;


mGlobal(uiODMain) uiODMain* ODMainWin();
//!< Top-level access for plugins


/*!
\brief OpendTect application top level object
*/

mExpClass(uiODMain) uiODMain : public uiMainWin
{mODTextTranslationClass(uiODMain);
public:

			uiODMain(uiMain&);
			~uiODMain();

    bool		go();
    void		exit(bool doconfirm=true);

    uiODApplMgr&	applMgr()	{ return *applmgr_; }
    uiODMenuMgr&	menuMgr()	{ return *menumgr_; } //!< + toolbar
    uiODSceneMgr&	sceneMgr()	{ return *scenemgr_; }
    uiODViewer2DMgr&	viewer2DMgr()	{ return *viewer2dmgr_; }
    uiVisColTabEd&	colTabEd()	{ return *ctabed_; }
    uiToolBar*		colTabToolBar()	{ return ctabtb_; }

    Notifier<uiODMain>	sessionSave;	//!< Put data in pars
    Notifier<uiODMain>	sessionRestoreEarly; //!< Get data from pars, before vis
    Notifier<uiODMain>	sessionRestore;	//!< Get data from pars
    IOPar&		sessionPars();	//!< On session save or restore
					//!< notification, to get/put data

    Notifier<uiODMain>	justBeforeGo;	//!< Scenes inited, auto-plugins loaded
    uiString		getProgramName();

    bool		askStore(bool& askedanything,
				 const uiString& actiontype);
			/*!< Asks user if session, picksets or attributesets
			     need to be stored. */
    bool		askStoreAttribs(bool,bool& askedanything);
			/*!< Asks user if attributesets (2D or 3D )
			  need to be stored. */
    bool		hasSessionChanged(); /*!< Compares current session with
						  last saved. */
    void		saveSession();	//!< pops up the save session dialog
    void		restoreSession(); //!< pops up the restore session dlg
    void		autoSession(); //!< pops up the auto session dlg
    bool		isRestoringSession()	{ return restoringsess_; }
    void		setProgramName(const char*);
			//Default is "OpendTect"

protected:

    uiODApplMgr*	applmgr_;
    uiODMenuMgr*	menumgr_;
    uiODSceneMgr*	scenemgr_;
    uiODViewer2DMgr*	viewer2dmgr_;
    uiVisColTabEd*	ctabed_;
    uiToolBar*		ctabtb_;
    uiMain&		uiapp_;
    ODSession*		cursession_;
    ODSession&		lastsession_;
    bool		restoringsess_;
    BufferString	programname_;

    MultiID		cursessid_;
    bool		failed_;

    virtual bool	closeOK();
    void		closeApplication();
    void		afterStartupCB(CallBacker*);
    void		afterSurveyChgCB(CallBacker*);
    void		handleStartupSession();
    void		restoreSession(const IOObj*);

private:

    mGlobal(uiODMain) friend int		ODMain(int,char**);

    bool		ensureGoodDataDir();
    bool		ensureGoodSurveySetup();
    bool		buildUI();
    void		initScene();

    CtxtIOObj*		getUserSessionIOData(bool);
    bool		updateSession();
    void		doRestoreSession();

    Timer&		sesstimer_;
    Timer&		memtimer_;
    Timer&		newsurvinittimer_;
    bool		neednewsurvinit_;
    void		autoSaveFail(CallBacker*);
    void		sessTimerCB(CallBacker*);
    void		memTimerCB(CallBacker*);
    void		newSurvInitTimerCB(CallBacker*);

    uiString		getProgramString() const;
    void		translateText();

public:

    bool		sceneMgrAvailable() const	{ return scenemgr_; }
    bool		menuMgrAvailable() const	{ return menumgr_; }
    bool		viewer2DMgrAvailable() const	{ return viewer2dmgr_; }
    void		updateCaption();

};

#endif
