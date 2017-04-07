/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Ranojay Sen
 Date:          June 2010
________________________________________________________________________

-*/


#include "uiodpseventstreeitem.h"

#include "uiioobjseldlg.h"
#include "uicolseqdisp.h"
#include "uicoltabsel.h"

#include "uimenu.h"
#include "uimsg.h"
#include "uiodmain.h"
#include "uiodapplmgr.h"
#include "uiodscenemgr.h"
#include "uitaskrunner.h"
#include "uitreeview.h"
#include "uiviscoltabed.h"
#include "uivispartserv.h"
#include "vispseventdisplay.h"

#include "ctxtioobj.h"
#include "enums.h"
#include "menuhandler.h"
#include "prestackevents.h"
#include "prestackeventtransl.h"
#include "prestackeventio.h"
#include "ptrman.h"
#include "survinfo.h"


uiODPSEventsParentTreeItem::uiODPSEventsParentTreeItem()
    : uiODSceneTreeItem( uiStrings::sPreStackEvents() )
    , child_(0)
{}


uiODPSEventsParentTreeItem::~uiODPSEventsParentTreeItem()
{}


bool uiODPSEventsParentTreeItem::showSubMenu()
{
    uiMenu mnu( getUiParent(), uiStrings::sAction() );
    mnu.insertItem( new uiAction(m3Dots(uiStrings::sAdd())), 0 );
    addStandardItems( mnu );

    const int mnusel = mnu.exec();
    if ( mnusel == 0 )
    {
	BufferString eventname;
	DBKey key;
	if ( !loadPSEvent(key,eventname) )
	    return false;

	child_ = new uiODPSEventsTreeItem( key, eventname );
	addChild( child_, true );
    }

    handleStandardItems( mnusel );
    return true;
}


bool uiODPSEventsParentTreeItem::loadPSEvent( DBKey& key,
					      BufferString& eventname )
{
    CtxtIOObj context = PSEventTranslatorGroup::ioContext();
    context.ctxt_.forread_ = true;
    uiIOObjSelDlg dlg( getUiParent(), context );
    if ( !dlg.go() )
	return false;

    eventname = dlg.ioObj()->name();
    key = dlg.chosenID();
    if ( key.isInvalid() || eventname.isEmpty() )
    {
	uiString errmsg = tr("Failed to load prestack event");
	uiMSG().error( errmsg );
	return false;
    }

    return true;
}


bool uiODPSEventsParentTreeItem::init()
{
    bool ret = uiODSceneTreeItem::init();
    if ( !ret ) return false;

    return true;
}


const char* uiODPSEventsParentTreeItem::iconName() const
{ return "tree-psevents"; }


const char* uiODPSEventsParentTreeItem::parentType() const
{ return typeid(uiODSceneTreeTop).name(); }



// uiODPSEventsTreeItem

uiODPSEventsTreeItem::uiODPSEventsTreeItem( const DBKey& key,
					    const char* eventname )
    : key_(key)
    , psem_(*new PreStack::EventManager)
    , eventname_(eventname)
    , eventdisplay_(0)
    , dir_(Coord(1,0))
    , scalefactor_(1)
    , coloritem_(new MenuItem(uiStrings::sColor(mPlural)))
    , coloridx_(0)
    , dispidx_(0)
{
    psem_.setStorageID( key, true );
}


uiODPSEventsTreeItem::~uiODPSEventsTreeItem()
{
    detachAllNotifiers();

    if ( eventdisplay_ )
    {
	uiVisPartServer* visserv = ODMainWin()->applMgr().visServer();
	visserv->removeObject( displayid_, sceneID() );
	eventdisplay_->unRef();
	eventdisplay_ = 0;
    }
}


bool uiODPSEventsTreeItem::init()
{
    updateDisplay();
    eventdisplay_->setDisplayMode( visSurvey::PSEventDisplay::ZeroOffset );
    return uiODDisplayTreeItem::init();
}


#define mAddPSMenuItems( mnu, func, midx, enab ) \
    mnu->removeItems(); \
    items = eventdisplay_->func(); \
    if ( items.isEmpty() ) return; \
    mnu->createItems( items ); \
    for ( int idx=0; idx<items.size(); idx++ ) \
    {  mnu->getItem(idx)->checkable = true; \
       mnu->getItem(idx)->enabled = !idx || enab; \
    } \
    mnu->getItem(midx)->checked = true; \
    items.setEmpty(); \

void uiODPSEventsTreeItem::createMenu( MenuHandler* menu, bool istb )
{
    uiODDisplayTreeItem::createMenu( menu, istb );
    if ( istb || !eventdisplay_ || !menu || menu->menuID()!=displayID() )
	return;

    mAddMenuItem( menu, coloritem_, true, false );
    uiStringSet items;
    mAddPSMenuItems( coloritem_, markerColorNames, coloridx_, true );
    mAddMenuItem( menu, &displaymnuitem_, true, false );
    MenuItem* item = &displaymnuitem_;
    const bool enabled = eventdisplay_->supportsDisplay();
    mAddPSMenuItems( item, displayModeNames, dispidx_, enabled )
}


void uiODPSEventsTreeItem::handleMenuCB( CallBacker* cb )
{
    uiODDisplayTreeItem::handleMenuCB(cb);
    mCBCapsuleUnpackWithCaller( int, menuid, caller, cb );
    mDynamicCastGet(MenuHandler*,menu,caller);
    if ( !eventdisplay_ || menu->isHandled()
	|| menu->menuID()!=displayID() || menuid==-1 )
	return;

    if ( displaymnuitem_.id != -1 && displaymnuitem_.itemIndex(menuid) != -1 )
    {
	dispidx_ = displaymnuitem_.itemIndex( menuid );
	MouseCursorChanger cursorchanger( MouseCursor::Wait );
	eventdisplay_->setDisplayMode(
	    (visSurvey::PSEventDisplay::DisplayMode) dispidx_ );
	menu->setIsHandled( true );
    }
    else if ( coloritem_->id!=-1 && coloritem_->itemIndex(menuid)!=-1 )
    {
	coloridx_ = coloritem_->itemIndex( menuid );
	MouseCursorChanger cursorchanger( MouseCursor::Wait );
	updateColorMode( coloridx_ );
	menu->setIsHandled( true );
    }
}


void uiODPSEventsTreeItem::updateColorMode( int mode )
{
    eventdisplay_->setMarkerColor(
	(visSurvey::PSEventDisplay::MarkerColor) mode );
    updateColumnText( uiODSceneMgr::cColorColumn() );
}


void uiODPSEventsTreeItem::updateDisplay()
{
    if ( !eventdisplay_ )
    {
	eventdisplay_ = new visSurvey::PSEventDisplay;
	eventdisplay_->ref();
        uiVisPartServer* visserv = ODMainWin()->applMgr().visServer();
	visserv->addObject( eventdisplay_, sceneID(), false );
	displayid_ = eventdisplay_->id();
	eventdisplay_->setName( toUiString(eventname_) );
	eventdisplay_->setLineStyle( OD::LineStyle(OD::LineStyle::Solid,4) );
	eventdisplay_->setEventManager( &psem_ );

	const ColTab::Sequence& cseq = uiCOLTAB().sequence();
	mAttachCB( cseq.objectChanged(), uiODPSEventsTreeItem::coltabChangeCB );
	eventdisplay_->setColTabSequence( 0, cseq, 0 );
    }
}


bool uiODPSEventsTreeItem::anyButtonClick( uiTreeViewItem* lvm )
{
    applMgr()->updateColorTable( displayid_, 0 );
    displayMiniColTab();
    return true;
}


void uiODPSEventsTreeItem::coltabChangeCB( CallBacker* cb )
{
    displayMiniColTab();
}


void uiODPSEventsTreeItem::updateColumnText( int col )
{
    uiODDisplayTreeItem::updateColumnText( col );
    displayMiniColTab();
}


void uiODPSEventsTreeItem::displayMiniColTab()
{
    if ( eventdisplay_->getMarkerColor() == visSurvey::PSEventDisplay::Single )
	return;

    const ColTab::Sequence& seq = eventdisplay_->getColTabSequence(0);
    setPixmap( uiODSceneMgr::cColorColumn(), &seq );
}
