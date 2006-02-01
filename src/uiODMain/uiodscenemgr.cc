/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        A.H. Bril
 Date:          Dec 2003
 RCS:           $Id: uiodscenemgr.cc,v 1.56 2006-02-01 21:56:51 cvskris Exp $
________________________________________________________________________

-*/

#include "uiodscenemgr.h"
#include "uiodapplmgr.h"
#include "uiodmenumgr.h"
#include "uiodtreeitemimpl.h"
#include "uiempartserv.h"
#include "uivispartserv.h"
#include "uiattribpartserv.h"

#include "uilabel.h"
#include "uislider.h"
#include "uidockwin.h"
#include "uisoviewer.h"
#include "uilistview.h"
#include "uiworkspace.h"
#include "uistatusbar.h"
#include "uithumbwheel.h"
#include "uigeninputdlg.h"
#include "uitreeitemmanager.h"
#include "uimsg.h"

#include "ptrman.h"
#include "survinfo.h"
#include "scene.xpm"

#define mPosField	0
#define mValueField	1
#define mNameField	2
#define mStatusField	3

static const int cWSWidth = 600;
static const int cWSHeight = 300;
static const int cMinZoom = 1;
static const int cMaxZoom = 150;
static const char* scenestr = "Scene ";

#define mWSMCB(fn) mCB(this,uiODSceneMgr,fn)
#define mDoAllScenes(memb,fn,arg) \
    for ( int idx=0; idx<scenes_.size(); idx++ ) \
	scenes_[idx]->memb->fn( arg )



uiODSceneMgr::uiODSceneMgr( uiODMain* a )
    	: appl_(*a)
    	, wsp_(new uiWorkSpace(a,"OpendTect work space"))
	, vwridx(0)
    	, lasthrot_(0), lastvrot_(0), lastdval_(0)
    	, tifs_(new uiTreeFactorySet)
{

    tifs_->addFactory( new uiODInlineTreeItemFactory, 1000 );
    tifs_->addFactory( new uiODCrosslineTreeItemFactory, 2000 );
    tifs_->addFactory( new uiODTimesliceTreeItemFactory, 3000 );
    tifs_->addFactory( new uiODRandomLineTreeItemFactory, 4000 );
    tifs_->addFactory( new uiODPickSetTreeItemFactory, 5000 );
#ifdef __debug__
    tifs_->addFactory( new uiODBodyTreeItemFactory, 5500 );
#endif
    tifs_->addFactory( new uiODHorizonTreeItemFactory, 6000);
#ifdef __debug__
    tifs_->addFactory( new uiODFaultTreeItemFactory, 7000 );
#endif
    tifs_->addFactory( new uiODWellTreeItemFactory, 8000 );

    wsp_->setPrefWidth( cWSWidth );
    wsp_->setPrefHeight( cWSHeight );

    uiGroup* leftgrp = new uiGroup( &appl_, "Left group" );

    uiThumbWheel* dollywheel = new uiThumbWheel( leftgrp, "Dolly", false );
    dollywheel->wheelMoved.notify( mWSMCB(dWheelMoved) );
    dollywheel->wheelPressed.notify( mWSMCB(anyWheelStart) );
    dollywheel->wheelReleased.notify( mWSMCB(anyWheelStop) );
    uiLabel* dollylbl = new uiLabel( leftgrp, "Mov" );
    dollylbl->attach( centeredBelow, dollywheel );

    uiLabel* dummylbl = new uiLabel( leftgrp, "" );
    dummylbl->attach( centeredBelow, dollylbl );
    dummylbl->setStretch( 0, 2 );
    uiThumbWheel* vwheel = new uiThumbWheel( leftgrp, "vRotate", false );
    vwheel->wheelMoved.notify( mWSMCB(vWheelMoved) );
    vwheel->wheelPressed.notify( mWSMCB(anyWheelStart) );
    vwheel->wheelReleased.notify( mWSMCB(anyWheelStop) );
    vwheel->attach( centeredBelow, dummylbl );

    uiLabel* rotlbl = new uiLabel( &appl_, "Rot" );
    rotlbl->attach( centeredBelow, leftgrp );

    uiThumbWheel* hwheel = new uiThumbWheel( &appl_, "hRotate", true );
    hwheel->wheelMoved.notify( mWSMCB(hWheelMoved) );
    hwheel->wheelPressed.notify( mWSMCB(anyWheelStart) );
    hwheel->wheelReleased.notify( mWSMCB(anyWheelStop) );
    hwheel->attach( leftAlignedBelow, wsp_ );

    zoomslider_ = new uiSliderExtra( &appl_, "Zoom" );
    zoomslider_->sldr()->valueChanged.notify( mWSMCB(zoomChanged) );
    zoomslider_->sldr()->setMinValue( cMinZoom );
    zoomslider_->sldr()->setMaxValue( cMaxZoom );
    zoomslider_->setStretch( 0, 0 );
    zoomslider_->attach( rightAlignedBelow, wsp_ );

    leftgrp->attach( leftOf, wsp_ );
}


uiODSceneMgr::~uiODSceneMgr()
{
    cleanUp( false );
    delete tifs_;
}


void uiODSceneMgr::initMenuMgrDepObjs()
{
    if ( !scenes_.size() )
	addScene();
}


void uiODSceneMgr::cleanUp( bool startnew )
{
    while ( scenes_.size() )
	scenes_[0]->vwrGroup()->mainObject()->close();
    	// close() cascades callbacks which remove the scene from set

    visServ().deleteAllObjects();
    vwridx = 0;
    if ( startnew ) addScene();
}


uiODSceneMgr::Scene& uiODSceneMgr::mkNewScene()
{
    uiODSceneMgr::Scene& scn = *new uiODSceneMgr::Scene( wsp_ );
    scn.vwrGroup()->mainObject()->closed.notify( mWSMCB(removeScene) );
    scenes_ += &scn;
    vwridx++;
    return scn;
}


int uiODSceneMgr::addScene()
{
    Scene& scn = mkNewScene();
    int sceneid = visServ().addScene();
    scn.sovwr_->setSceneGraph( sceneid );
    BufferString title( scenestr );
    title += vwridx;
    scn.sovwr_->setTitle( title );
    visServ().setObjectName( sceneid, title );
    scn.sovwr_->display();
    scn.sovwr_->viewAll();
    scn.sovwr_->saveHomePos();
    scn.sovwr_->viewmodechanged.notify( mWSMCB(viewModeChg) );
    scn.sovwr_->pageupdown.notify( mCB(this,uiODSceneMgr,pageUpDownPressed) );
    scn.vwrGroup()->display( true, false, true );
    actMode(0);
    setZoomValue( scn.sovwr_->getCameraZoom() );
    initTree( scn, vwridx );

    if ( scenes_.size() > 1 && scenes_[0] )
    {
	scn.sovwr_->setStereoType( scenes_[0]->sovwr_->getStereoType() );
	scn.sovwr_->setStereoOffset(
		scenes_[0]->sovwr_->getStereoOffset() );
    }

    return sceneid;
}


void uiODSceneMgr::removeScene( CallBacker* cb )
{
    mDynamicCastGet(uiGroupObj*,grp,cb)
    if ( !grp ) return;
    int idxnr = -1;
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	if ( grp == scenes_[idx]->vwrGroup()->mainObject() )
	{
	    idxnr = idx;
	    break;
	}
    }
    if ( idxnr < 0 ) return;

    uiODSceneMgr::Scene* scene = scenes_[idxnr];
    scene->itemmanager_->prepareForShutdown();
    visServ().removeScene( scene->itemmanager_->sceneID() );
    appl_.removeDockWindow( scene->treeWin() );

    scene->vwrGroup()->mainObject()->closed.remove( mWSMCB(removeScene) );
    scenes_ -= scene;
    delete scene;
}


void uiODSceneMgr::storePositions()
{
    //TODO remember the scene's positions
    // mDoAllScenes(sovwr_,storePosition,);
}


void uiODSceneMgr::getScenePars( IOPar& iopar )
{
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	IOPar iop;
	scenes_[idx]->sovwr_->fillPar( iop );
	BufferString key = idx;
	iopar.mergeComp( iop, key );
    }
}


void uiODSceneMgr::useScenePars( const IOPar& sessionpar )
{
    for ( int idx=0; ; idx++ )
    {
	BufferString key = idx;
	PtrMan<IOPar> scenepar = sessionpar.subselect( key );
	if ( !scenepar || !scenepar->size() )
	{
	    if ( !idx ) continue;
	    break;
	}

	Scene& scn = mkNewScene();
  	if ( !scn.sovwr_->usePar( *scenepar ) )
	{
	    removeScene( scn.vwrGroup() );
	    continue;
	}
    
	int sceneid = scn.sovwr_->sceneId();
	BufferString title( scenestr );
	title += vwridx;
  	scn.sovwr_->setTitle( title );
	scn.sovwr_->display();
	scn.sovwr_->viewmodechanged.notify( mWSMCB(viewModeChg) );
	scn.sovwr_->pageupdown.notify(mCB(this,uiODSceneMgr,pageUpDownPressed));
	scn.vwrGroup()->display( true, false );
	setZoomValue( scn.sovwr_->getCameraZoom() );
	initTree( scn, vwridx );
    }

    rebuildTrees();
}


void uiODSceneMgr::viewModeChg( CallBacker* cb )
{
    if ( !scenes_.size() ) return;

    mDynamicCastGet(uiSoViewer*,sovwr_,cb)
    if ( sovwr_ ) setToViewMode( sovwr_->isViewing() );
}


void uiODSceneMgr::setToViewMode( bool yn )
{
    mDoAllScenes(sovwr_,setViewing,yn);
    visServ().setViewMode( yn );
    menuMgr().updateViewMode( yn );
}


void uiODSceneMgr::actMode( CallBacker* )
{
    setToViewMode( false );
}


void uiODSceneMgr::viewMode( CallBacker* )
{
    setToViewMode( true );
    appl_.statusBar()->message( "", mPosField );
    appl_.statusBar()->message( "", mValueField );
    appl_.statusBar()->message( "", mNameField );
    appl_.statusBar()->message( "", mStatusField );
    appl_.statusBar()->setBGColor( mStatusField,
	    			   appl_.statusBar()->getBGColor(mPosField) );
}


void uiODSceneMgr::pageUpDownPressed( CallBacker* cb )
{
    mCBCapsuleUnpack(bool,up,cb);
    applMgr().pageUpDownPressed( up );
}


void uiODSceneMgr::updateStatusBar()
{
    Coord3 xytpos = visServ().getMousePos(true);
    BufferString msg;
    if ( !Values::isUdf( xytpos.x ) )
    {
	BinID bid( SI().transform( Coord(xytpos.x,xytpos.y) ) );
	msg = bid.inl; msg += "/"; msg += bid.crl;
	msg += "   (";
	msg += mNINT(xytpos.x); msg += ",";
	msg += mNINT(xytpos.y); msg += ",";
	msg += SI().zIsTime() ? mNINT(xytpos.z * 1000) : xytpos.z; msg += ")";
    }

    appl_.statusBar()->message( msg, mPosField );

    BufferString valstr = visServ().getMousePosVal();
    msg = valstr == "" ? "" : "Value = "; msg += valstr;
    appl_.statusBar()->message( msg, mValueField );

    msg = visServ().getMousePosString();
    appl_.statusBar()->message( msg, mNameField );

    const bool ispicking = visServ().isPicking();

    appl_.statusBar()->message( ispicking ? "Picking" : 0, mStatusField );

    appl_.statusBar()->setBGColor( mStatusField, ispicking ?
	    Color( 255, 0, 0 ) : appl_.statusBar()->getBGColor(mPosField) );
}


void uiODSceneMgr::setKeyBindings()
{
    if ( !scenes_.size() ) return;

    BufferStringSet keyset;
    scenes_[0]->sovwr_->getAllKeyBindings( keyset );

    StringListInpSpec* inpspec = new StringListInpSpec( keyset );
    inpspec->setText( scenes_[0]->sovwr_->getCurrentKeyBindings(), 0 );
    uiGenInputDlg dlg( &appl_, "Select Mouse Controls", "Select", inpspec );
    if ( dlg.go() )
	mDoAllScenes(sovwr_,setKeyBindings,dlg.text());
}


int uiODSceneMgr::getStereoType() const
{
    return scenes_.size() ? (int)scenes_[0]->sovwr_->getStereoType() : 0;
}


void uiODSceneMgr::setStereoType( int type )
{
    if ( !scenes_.size() ) return;

    uiSoViewer::StereoType stereotype = (uiSoViewer::StereoType)type;
    const float stereooffset = scenes_[0]->sovwr_->getStereoOffset();
    for ( int ids=0; ids<scenes_.size(); ids++ )
    {
	uiSoViewer& sovwr_ = *scenes_[ids]->sovwr_;
	if ( !sovwr_.setStereoType(stereotype) )
	{
	    uiMSG().error( "No support for this type of stereo rendering" );
	    return;
	}
	if ( type )
	    sovwr_.setStereoOffset( stereooffset );
    }
}


void uiODSceneMgr::tile()		{ wsp_->tile(); }
void uiODSceneMgr::tileHorizontal()	{ wsp_->tileHorizontal(); }
void uiODSceneMgr::tileVertical()	{ wsp_->tileVertical(); }
void uiODSceneMgr::cascade()		{ wsp_->cascade(); }


void uiODSceneMgr::layoutScenes()
{
    const int nrgrps = scenes_.size();
    if ( nrgrps == 1 && scenes_[0] )
	scenes_[0]->vwrGroup()->display( true, false, true );
    else if ( scenes_[0] )
	tile();
}


void uiODSceneMgr::toHomePos( CallBacker* )
{ mDoAllScenes(sovwr_,toHomePos,); }
void uiODSceneMgr::saveHomePos( CallBacker* )
{ mDoAllScenes(sovwr_,saveHomePos,); }
void uiODSceneMgr::viewAll( CallBacker* )
{ mDoAllScenes(sovwr_,viewAll,); }
void uiODSceneMgr::align( CallBacker* )
{ mDoAllScenes(sovwr_,align,); }

void uiODSceneMgr::viewX( CallBacker* )
{ mDoAllScenes(sovwr_,viewPlane,uiSoViewer::X); }
void uiODSceneMgr::viewY( CallBacker* )
{ mDoAllScenes(sovwr_,viewPlane,uiSoViewer::Y); }
void uiODSceneMgr::viewZ( CallBacker* )
{ mDoAllScenes(sovwr_,viewPlane,uiSoViewer::Z); }
void uiODSceneMgr::viewInl( CallBacker* )
{ mDoAllScenes(sovwr_,viewPlane,uiSoViewer::Inl); }
void uiODSceneMgr::viewCrl( CallBacker* )
{ mDoAllScenes(sovwr_,viewPlane,uiSoViewer::Crl); }


void uiODSceneMgr::showRotAxis( CallBacker* )
{
    mDoAllScenes(sovwr_,showRotAxis,);
    if ( scenes_.size() )
	menuMgr().updateAxisMode( scenes_[0]->sovwr_->rotAxisShown() );
}


void uiODSceneMgr::mkSnapshot( CallBacker* )
{
    if ( scenes_.size() )
	scenes_[0]->sovwr_->renderToFile();
}


void uiODSceneMgr::soloMode( CallBacker* )
{
    TypeSet< TypeSet<int> > dispids;
    int selectedid;
    for ( int idx=0; idx<scenes_.size(); idx++ )
	dispids += scenes_[idx]->itemmanager_->getDisplayIds( selectedid,  
						 !menuMgr().isSoloModeOn() );
    
    visServ().setSoloMode( menuMgr().isSoloModeOn(), dispids, selectedid );
}


void uiODSceneMgr::setZoomValue( float val )
{
    zoomslider_->sldr()->setValue( val );
}


void uiODSceneMgr::zoomChanged( CallBacker* )
{
    const float zmval = zoomslider_->sldr()->getValue();
    mDoAllScenes(sovwr_,setCameraZoom,zmval);
}


void uiODSceneMgr::anyWheelStart( CallBacker* )
{ mDoAllScenes(sovwr_,anyWheelStart,); }
void uiODSceneMgr::anyWheelStop( CallBacker* )
{ mDoAllScenes(sovwr_,anyWheelStop,); }


void uiODSceneMgr::wheelMoved( CallBacker* cb, int wh, float& lastval )
{
    mDynamicCastGet(uiThumbWheel*,wheel,cb)
    if ( !wheel ) { pErrMsg("huh") ; return; }

    const float whlval = wheel->lastMoveVal();
    const float diff = lastval - whlval;

    if ( diff )
    {
	for ( int idx=0; idx<scenes_.size(); idx++ )
	{
	    if ( wh == 1 )
		scenes_[idx]->sovwr_->rotateH( diff );
	    else if ( wh == 2 )
		scenes_[idx]->sovwr_->rotateV( -diff );
	    else
		scenes_[idx]->sovwr_->dolly( diff );
	}
    }

    lastval = whlval;
}


void uiODSceneMgr::hWheelMoved( CallBacker* cb )
{ wheelMoved(cb,1,lasthrot_); }
void uiODSceneMgr::vWheelMoved( CallBacker* cb )
{ wheelMoved(cb,2,lastvrot_); }
void uiODSceneMgr::dWheelMoved( CallBacker* cb )
{ wheelMoved(cb,0,lastdval_); }


void uiODSceneMgr::switchCameraType( CallBacker* )
{
    ObjectSet<uiSoViewer> vwrs;
    getSoViewers( vwrs );
    if ( !vwrs.size() ) return;
    mDoAllScenes(sovwr_,toggleCameraType,);
    const bool isperspective = vwrs[0]->isCameraPerspective();
    menuMgr().setCameraPixmap( isperspective );
    zoomslider_->setSensitive( isperspective );
}


void uiODSceneMgr::getSoViewers( ObjectSet<uiSoViewer>& vwrs )
{
    vwrs.erase();
    for ( int idx=0; idx<scenes_.size(); idx++ )
	vwrs += scenes_[idx]->sovwr_;
}


void uiODSceneMgr::initTree( Scene& scn, int vwridx )
{
    BufferString capt( "Tree scene " ); capt += vwridx;
    uiDockWin* dw = new uiDockWin( &appl_, capt );
    appl_.moveDockWindow( *dw, uiMainWin::Left );
    dw->setResizeEnabled( true );

    scn.lv_ = new uiListView( dw, "d-Tect Tree" );
    scn.lv_->addColumn( "Elements" );
    scn.lv_->addColumn( "Color" );
    scn.lv_->setColumnWidthMode( cNameColumn(), uiListView::Manual );
    scn.lv_->setColumnWidthMode( cColorColumn(), uiListView::Manual );

    scn.lv_->setColumnWidth( cColorColumn(), 38 );
    scn.lv_->setColumnWidth( cNameColumn(), 152 );
    scn.lv_->setPrefWidth( 190 );

    scn.lv_->setStretch( 2, 2 );

    scn.itemmanager_ = new uiODTreeTop( scn.sovwr_, scn.lv_, &applMgr(), tifs_);

    int nradded = 0;
    int globalhighest = INT_MAX;
    while ( nradded<tifs_->nrFactories() )
    {
	int highest = INT_MIN;
	for ( int idx=0; idx<tifs_->nrFactories(); idx++ )
	{
	    const int placementidx = tifs_->getPlacementIdx(idx);
	    if ( placementidx>highest && placementidx<globalhighest )
		highest = placementidx;
	}

	for ( int idx=0; idx<tifs_->nrFactories(); idx++ )
	{
	    if ( tifs_->getPlacementIdx(idx)==highest )
	    {
		scn.itemmanager_->addChild( tifs_->getFactory(idx)->create(),
					    false );
		nradded++;
	    }
	}

	globalhighest = highest;
    }

    scn.itemmanager_->addChild( new uiODSceneTreeItem(scn.sovwr_->getTitle(),
					     scn.sovwr_->sceneId() ), false );
    scn.lv_->display();
    scn.treeWin()->display();
}


void uiODSceneMgr::updateTrees()
{
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	Scene& scene = *scenes_[idx];
	scene.itemmanager_->updateColumnText( cNameColumn() );
	scene.itemmanager_->updateColumnText( cColorColumn() );
    }
}


void uiODSceneMgr::rebuildTrees()
{
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	Scene& scene = *scenes_[idx];
	const int sceneid = scene.sovwr_->sceneId();
	TypeSet<int> visids; visServ().getChildIds( sceneid, visids );

	for ( int idy=0; idy<visids.size(); idy++ )
	    uiODDisplayTreeItem::create( scene.itemmanager_, &applMgr(), 
		    			 visids[idy] );
    }
    updateSelectedTreeItem();
}


void uiODSceneMgr::setItemInfo( int id )
{
    mDoAllScenes(itemmanager_,updateColumnText,cColorColumn());
    appl_.statusBar()->message( "", mPosField );
    appl_.statusBar()->message( "", mValueField );
    appl_.statusBar()->message( visServ().getInteractionMsg(id), mNameField );
}


void uiODSceneMgr::updateSelectedTreeItem()
{
    const int id = visServ().getSelObjectId();

    if ( id != -1 )
    {
	setItemInfo( id );
	//applMgr().modifyColorTable( id );
	if ( !visServ().isOn(id) ) visServ().turnOn(id, true, true);
	else if ( scenes_.size() != 1 && visServ().isSoloMode() )
	    visServ().updateDisplay( true, id );
    }

    bool found = applMgr().attrServer()->attrSetEditorActive();
    bool gotoact = false;
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	Scene& scene = *scenes_[idx];
	if ( !found )
	{
	    mDynamicCastGet( const uiODDisplayTreeItem*, treeitem,
		    	     scene.itemmanager_->findChild(id) );
	    if ( treeitem )
	    {
		gotoact = treeitem->actModeWhenSelected();
		found = true;
	    }
	}

	scene.itemmanager_->updateSelection( id );
	scene.itemmanager_->updateColumnText( cNameColumn() );
	scene.itemmanager_->updateColumnText( cColorColumn() );
    }

    if ( gotoact && !applMgr().attrServer()->attrSetEditorActive() )
	actMode( 0 );
}


int uiODSceneMgr::getIDFromName( const char* str ) const
{
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	const uiTreeItem* itm = scenes_[idx]->itemmanager_->findChild( str );
	if ( itm ) return itm->selectionKey();
    }

    return -1;
}


void uiODSceneMgr::disabRightClick( bool yn )
{
    mDoAllScenes(itemmanager_,disabRightClick,yn);
}


void uiODSceneMgr::disabTrees( bool yn )
{
    for ( int idx=0; idx<scenes_.size(); idx++ )
	scenes_[idx]->lv_->setSensitive( !yn );
}


int uiODSceneMgr::addPickSetItem( const PickSet& ps, int sceneid )
{
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	Scene& scene = *scenes_[idx];
	if ( sceneid >= 0 && sceneid != scene.sovwr_->sceneId() ) continue;

	uiODPickSetTreeItem* itm = new uiODPickSetTreeItem( ps );
	scene.itemmanager_->addChild( itm, false );
	return itm->displayID();
    }

    return -1;
}


int uiODSceneMgr::addEMItem( const EM::ObjectID& emid, int sceneid )
{
    const char* type = applMgr().EMServer()->getType(emid);
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	Scene& scene = *scenes_[idx];
	if ( sceneid >= 0 && sceneid != scene.sovwr_->sceneId() ) continue;

	uiODDisplayTreeItem* itm;
	if ( !strcmp( type, "Horizon" ) ) itm = new uiODHorizonTreeItem(emid);
	else if ( !strcmp(type,"Fault" ) ) itm = new uiODFaultTreeItem(emid);
	else if ( !strcmp(type,"Horizontal Tube") )
	    itm = new uiODBodyTreeItem(emid);

	scene.itemmanager_->addChild( itm, false );
	return itm->displayID();
    }

    return -1;
}


void uiODSceneMgr::removeTreeItem( int displayid )
{
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	Scene& scene = *scenes_[idx];
	uiTreeItem* itm = scene.itemmanager_->findChild( displayid );
	if ( itm ) scene.itemmanager_->removeChild( itm );
    }
}


uiTreeItem* uiODSceneMgr::findItem( int displayid )
{
    for ( int idx=0; idx<scenes_.size(); idx++ )
    {
	Scene& scene = *scenes_[idx];
	uiTreeItem* itm = scene.itemmanager_->findChild( displayid );
	if ( itm ) return itm;
    }

    return 0;
}


uiODSceneMgr::Scene::Scene( uiWorkSpace* wsp )
        : lv_(0)
        , sovwr_(0)
    	, itemmanager_( 0 )
{
    if ( !wsp ) return;

    uiGroup* grp = new uiGroup( wsp );
    grp->setPrefWidth( 200 );
    grp->setPrefHeight( 200 );
    sovwr_ = new uiSoViewer( grp );
    sovwr_->setPrefWidth( 200 );
    sovwr_->setPrefHeight( 200 );
    sovwr_->setIcon( scene_xpm_data );
}


uiODSceneMgr::Scene::~Scene()
{
    delete sovwr_;
    delete itemmanager_;
    delete treeWin();
}


uiGroup* uiODSceneMgr::Scene::vwrGroup()
{
    return sovwr_ ? (uiGroup*)sovwr_->parent() : 0;
}


uiDockWin* uiODSceneMgr::Scene::treeWin()
{
    return lv_ ? (uiDockWin*)lv_->parent() : 0;
}

