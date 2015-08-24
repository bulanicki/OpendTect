/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Nanne Hemstra
 Date:          January 2005
________________________________________________________________________

-*/
static const char* rcsID mUsedVar = "$Id$";

#include "uiodannottreeitem.h"
#include "randcolor.h"
#include "survinfo.h"

#include "uibutton.h"
#include "uicolor.h"
#include "uifiledlg.h"
#include "uigeninput.h"
#include "uigeninputdlg.h"
#include "uimenu.h"
#include "uimenuhandler.h"
#include "uiodapplmgr.h"
#include "uislider.h"
#include "uitextedit.h"
#include "uitreeview.h"
#include "uivispartserv.h"
#include "vislocationdisplay.h"
#include "vissurvscene.h"

#include "ctxtioobj.h"
#include "ioobj.h"
#include "ioman.h"
#include "iopar.h"
#include "keystrs.h"
#include "pickset.h"
#include "picksettr.h"
#include "ptrman.h"
#include "uicolor.h"
#include "uiioobjseldlg.h"
#include "uimsg.h"


//namespace Annotations
//{


uiODAnnotParentTreeItem::uiODAnnotParentTreeItem()
    : uiTreeItem("Annotations")
{}


uiODAnnotParentTreeItem::~uiODAnnotParentTreeItem()
{}


bool uiODAnnotParentTreeItem::rightClick( uiTreeViewItem* itm )
{
    if ( itm == uitreeviewitem_ && !uitreeviewitem_->isOpen() )
        uitreeviewitem_->setOpen( true );

    return uiTreeItem::rightClick( itm );
}


int uiODAnnotParentTreeItem::sceneID() const
{
    int sceneid;
    if ( !getProperty<int>(uiODTreeTop::sceneidkey(),sceneid) )
	return -1;
    return sceneid;
}


bool uiODAnnotParentTreeItem::init()
{
    bool ret = uiTreeItem::init();
    if ( !ret ) return false;

    addChild( new ArrowParentItem(), true );
    addChild( new ImageParentItem(), true );
    addChild( new ScaleBarParentItem(), true );
    getItem()->setOpen( false );
    return true;
}


const char* uiODAnnotParentTreeItem::parentType() const
{ return typeid(uiODTreeTop).name(); }


// TreeItemFactory +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uiTreeItem* uiODAnnotTreeItemFactory::create( int visid,
					      uiTreeItem* treeitem ) const
{
    visBase::DataObject* dataobj =
	ODMainWin()->applMgr().visServer()->getObject(visid);
    if ( !dataobj ) return 0;

    mDynamicCastGet(visSurvey::LocationDisplay*,ld, dataobj );
    if ( !ld ) return 0;

    if ( treeitem->findChild( visid ) )
	return 0;

    const MultiID mid = ld->getMultiID();
    const char* factoryname = 0;

    Pick::SetMgr& mgr = Pick::SetMgr::getMgr( factoryname );
    int setidx = mgr.indexOf(mid);
    if ( setidx==-1 )
    {
	PtrMan<IOObj> ioobj = IOM().get( mid );
	Pick::Set* ps = new Pick::Set;
	BufferString bs;
	PickSetTranslator::retrieve(*ps,ioobj,true,bs);
	mgr.set( mid, ps );

	setidx = mgr.indexOf(mid);

    }

    return 0;
}


// Base uiODAnnotTreeItem ++++++++++++++++++++++++++++++++++++++++++++++++++++++

uiODAnnotTreeItem::uiODAnnotTreeItem( const uiString& type_ )
    : uiODTreeItem(type_)
    , typestr_(type_.getFullString())
{ }


uiODAnnotTreeItem::~uiODAnnotTreeItem()
{}


const char* uiODAnnotTreeItem::parentType() const
{ return typeid(uiODAnnotParentTreeItem).name(); }


bool uiODAnnotTreeItem::init()
{
    Pick::SetMgr& mgr = Pick::SetMgr::getMgr( managerName() );
    mgr.setToBeRemoved.notify( mCB(this,uiODAnnotTreeItem,setRemovedCB));
    return true;
}


void uiODAnnotTreeItem::prepareForShutdown()
{
    Pick::SetMgr::getMgr( managerName() ).removeCBs( this );
}


void uiODAnnotTreeItem::addPickSet( Pick::Set* ps )
{
    if ( !ps ) return;

    uiTreeItem* item = createSubItem( -1, *ps );
    addChild( item, true );
}


void uiODAnnotTreeItem::removePickSet( Pick::Set* ps )
{
    if ( !ps ) return;
    uiVisPartServer* visserv = applMgr()->visServer();

    for ( int idx=0; idx<children_.size(); idx++ )
    {
	mDynamicCastGet(uiODDisplayTreeItem*,itm,children_[idx])
	    if ( !itm ) continue;

	const int displayid = itm->displayID();
	mDynamicCastGet(visSurvey::LocationDisplay*,ld,
	    visserv->getObject(displayid));
	if ( !ld ) continue;

	if ( ld->getSet() == ps )
	{
	    applMgr()->visServer()->removeObject( displayid, sceneID() );
	    uiTreeItem::removeChild( itm );
	    return;
	}
    }
}


void uiODAnnotTreeItem::setRemovedCB( CallBacker* cb )
{
    mDynamicCastGet(Pick::Set*,ps,cb)
    if ( !ps ) return;

    for ( int idx=0; idx<children_.size(); idx++ )
    {
	mDynamicCastGet(uiODAnnotSubItem*,itm,children_[idx])
	    if ( !itm ) continue;
	if ( itm->getSet() == ps )
	{
	    applMgr()->visServer()->removeObject( itm->displayID(), sceneID() );
	    uiTreeItem::removeChild( itm );
	    return;
	}
    }

}


bool uiODAnnotTreeItem::showSubMenu()
{
    mDynamicCastGet(visSurvey::Scene*,scene,
		    applMgr()->visServer()->getObject(sceneID()));
    if ( scene && scene->getZAxisTransform() )
    {
	uiMSG().message( tr("Cannot add Annotations to this scene (yet)") );
	return false;
    }

    uiMenu mnu( getUiParent(), uiStrings::sAction() );
    BufferString addtxt = "New "; addtxt += typestr_; addtxt += " group ...";
    mnu.insertItem( new uiAction(addtxt), 0 );
    mnu.insertItem(new uiAction(m3Dots(uiStrings::sAdd())),1);

    const int mnusel = mnu.exec();
    if ( mnusel < 0 ) return false;

    if ( mnusel == 0 )
    {
	BufferString title = typestr_; title += " Annotations";
	uiGenInputDlg dlg( getUiParent(), title, "Group name",
			   new StringInpSpec );
	dlg.setCaption( "Annotations" );

	while ( true )
	{
	    if ( !dlg.go() ) return false;

	    const char* txt = dlg.text();
	    if ( !txt || !*txt ) continue;

	    if ( uiODAnnotSubItem::doesNameExist( txt ) &&
	         !uiMSG().askOverwrite(
		 tr("An object with that name already "
		    "exists.\nDo you wish to overwrite it?")))
		continue;

	    MultiID mid;
	    if ( uiODAnnotSubItem::createIOEntry(txt,true,mid,managerName())!=1)
		return false;

	    Pick::Set* set = new Pick::Set(txt);
	    set->disp_.color_ = getRandStdDrawColor();
	    if ( defScale()!=-1 ) set->disp_.pixsize_ = defScale();
	    Pick::SetMgr& mgr = Pick::SetMgr::getMgr( managerName() );
	    mgr.set( mid, set );
	    uiTreeItem* item = createSubItem( -1, *set );
	    addChild( item, true );
	    break;
	}
    }
    else if ( mnusel == 1 )
    {
	Pick::Set* ps = new Pick::Set;
	if ( !readPicks(*ps) ) { delete ps; return false; }
    }

    return true;
}

#define mDelCtioRet { delete ctio->ioobj; delete ctio; return false; }


bool uiODAnnotTreeItem::readPicks( Pick::Set& ps )
{
    CtxtIOObj* ctio = mMkCtxtIOObj(PickSet);
    ctio->ctxt.forread = true;
    ctio->ctxt.toselect.require_.set( sKey::Type(), managerName(), oldSelKey());
    uiIOObjSelDlg dlg( getUiParent(), *ctio );
    if ( !dlg.go() || !dlg.ioObj() )
	mDelCtioRet;

    if ( defScale()!=-1 )
	ps.disp_.pixsize_= defScale();

    BufferString bs;
    if ( !PickSetTranslator::retrieve(ps,dlg.ioObj(),true,bs) )
    { uiMSG().error( bs ); mDelCtioRet; }

    Pick::SetMgr& mgr = Pick::SetMgr::getMgr( managerName() );
    if ( mgr.indexOf(dlg.ioObj()->key() ) == -1 )
    {
	mgr.set( dlg.ioObj()->key(), &ps );
	const int setidx = mgr.indexOf( ps );
	mgr.setUnChanged( setidx );
	addPickSet( &ps );
    }
    return true;
}

// SubItem ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uiODAnnotSubItem::uiODAnnotSubItem( Pick::Set& set, int displayid )
    : set_( &set )
    , defscale_(mCast(float,set.disp_.pixsize_))
    , scalemnuitem_("Size ...")
    , storemnuitem_(uiStrings::sSave())
    , storeasmnuitem_(m3Dots(uiStrings::sSaveAs()))
{
    name_ = set_->name();
    displayid_ = displayid;

    storemnuitem_.iconfnm = "save";
    storeasmnuitem_.iconfnm = "saveas";
}


uiODAnnotSubItem::~uiODAnnotSubItem()
{
}


void uiODAnnotSubItem::prepareForShutdown()
{
    Pick::SetMgr& mgr = Pick::SetMgr::getMgr( managerName() );
    const int setidx = mgr.indexOf( *set_ );
    if ( mgr.isChanged(setidx) )
    {
	uiString msg = tr("The annotation group %1 "
			  "is not saved.\n\nDo you want to save it?")
		     .arg(name());
	if ( uiMSG().askSave(msg,false) )
	    store();
    }

    removeStuff();
}


bool uiODAnnotSubItem::init()
{
    mDynamicCastGet(visSurvey::LocationDisplay*,ld,
		    visserv_->getObject(displayid_));
    if ( ld )
    {
	ld->setSetMgr( &Pick::SetMgr::getMgr( managerName()) );
	ld->setSet( set_ );
    }

    return uiODDisplayTreeItem::init();
}


void uiODAnnotSubItem::createMenu( MenuHandler* menu, bool istb )
{
    if ( !menu || menu->menuID()!=displayID() )
	return;

    const bool islocked = visserv_->isLocked( displayid_ );
    uiODDisplayTreeItem::createMenu( menu, istb );
    if ( hasScale() )
	mAddMenuOrTBItem(istb,0,menu,&scalemnuitem_,!islocked,false);

    Pick::SetMgr& mgr = Pick::SetMgr::getMgr( managerName() );
    const int setidx = mgr.indexOf( *set_ );
    mAddMenuOrTBItem(istb,menu,menu,&storemnuitem_,
		     mgr.isChanged(setidx),false);
    mAddMenuOrTBItem(istb,0,menu,&storeasmnuitem_,true,false);
}


void uiODAnnotSubItem::handleMenuCB( CallBacker* cb )
{
    uiODDisplayTreeItem::handleMenuCB(cb);
    mCBCapsuleUnpackWithCaller(int,mnuid,caller,cb);
    mDynamicCastGet(MenuHandler*,menu,caller);
    if ( menu->isHandled() || menu->menuID()!=displayID() )
	return;

    mDynamicCastGet(visSurvey::LocationDisplay*,ld,
		    visserv_->getObject(displayid_));
    if ( !ld ) return;

    if ( mnuid==scalemnuitem_.id )
    {
	menu->setIsHandled(true);
	uiDialog dlg( getUiParent(), uiDialog::Setup(tr("Set Size"),tr("Size"),
						     mNoHelpKey) );
	uiSlider* sliderfld = new uiSlider( &dlg,
			uiSlider::Setup(tr("Size")).nrdec(1).logscale(true),
			"Size" );
	sliderfld->setMinValue( 0.1 );
	sliderfld->setMaxValue( 10 );
	sliderfld->setValue( 1 );
	sliderfld->valueChanged.notify( mCB(this,uiODAnnotSubItem,scaleChg) );
	dlg.go();
	defscale_ *= sliderfld->getValue();
    }
    else if ( mnuid==storemnuitem_.id )
    {
	menu->setIsHandled(true);
	store();
    }
    else if ( mnuid==storeasmnuitem_.id )
    {
	menu->setIsHandled(true);
	storeAs();
	updateColumnText( 0 );
    }
}


void uiODAnnotSubItem::store() const
{
    Pick::SetMgr& mgr = Pick::SetMgr::getMgr( managerName() );

    const int setidx = mgr.indexOf( *set_ );
    PtrMan<IOObj> ioobj = IOM().get( mgr.id(setidx) );
    if ( !ioobj )
    {
	storeAs( true );
	return;
    }

    ioobj->pars().set( sKey::Type(), managerName() );
    IOM().commitChanges( *ioobj );

    fillStoragePar( set_->pars_ );
    BufferString bs;
    if ( !PickSetTranslator::store( *set_, ioobj, bs ) )
	uiMSG().error(bs);
    else
	mgr.setUnChanged( setidx );
}


bool uiODAnnotSubItem::doesNameExist( const char* nm )
{
    IOM().to( PickSetTranslatorGroup::ioContext().getSelKey() );
    PtrMan<IOObj> local =
	IOM().getLocal( nm, 0 );
    return local;
}


char uiODAnnotSubItem::createIOEntry( const char* nm, bool overwrite,
				    MultiID& mid, const char* mannm )
{
    if ( !overwrite && doesNameExist(nm) )
	return 0;

    CtxtIOObj ctio( PickSetTranslatorGroup::ioContext() );
    ctio.ctxt.forread = false;
    ctio.ctxt.toselect.require_.set( sKey::Type(), mannm );
    ctio.setName( nm );
    ctio.fillObj();
    if ( !ctio.ioobj )
	return -1;

    mid = ctio.ioobj->key();
    delete ctio.ioobj;
    return 1;
}


void uiODAnnotSubItem::storeAs( bool trywitoutdlg ) const
{
    Pick::SetMgr& mgr = Pick::SetMgr::getMgr( managerName() );
    const int setidx = mgr.indexOf( *set_ );

    const char* nm = set_->name();
    MultiID mid;

    if ( trywitoutdlg )
    {
	const char res = createIOEntry( nm, false, mid, managerName() );
	if ( res==-1 )
	    return;
	if ( res==0 )
	{
	    storeAs( false );
	    return;
	}
    }
    else
    {
	CtxtIOObj ctio( PickSetTranslatorGroup::ioContext() );
	ctio.ctxt.forread = false;
	ctio.ctxt.toselect.require_.set( sKey::Type(), managerName() );
	ctio.setName( nm );
	uiIOObjSelDlg dlg( getUiParent(), ctio );
	if ( !dlg.go() )
	    { delete ctio.ioobj; return; }
	mid = dlg.chosenID( 0 );
    }

    mgr.setID( setidx, mid );
    store();
}


void uiODAnnotSubItem::setScale( float ns )
{
    mDynamicCastGet(visSurvey::LocationDisplay*,ld,
		    visserv_->getObject(displayid_));
    if ( !ld ) return;

    const int newscale = mNINT32( ns );
    Pick::Set* set = ld->getSet();
    if ( set->disp_.pixsize_==newscale )
	return;

    set->disp_.pixsize_ = newscale;

    Pick::SetMgr::getMgr( managerName() ).reportDispChange( this, *set );
}


void uiODAnnotSubItem::setColor( Color nc )
{
    mDynamicCastGet(visSurvey::LocationDisplay*,ld,
		    visserv_->getObject(displayid_));
    if ( !ld ) return;

    Pick::Set* set = ld->getSet();
    if ( set->disp_.color_==nc )
	return;

    set->disp_.color_ = nc;

    Pick::SetMgr::getMgr( managerName() ).reportDispChange( this, *set );
}


void uiODAnnotSubItem::scaleChg( CallBacker* cb )
{
    mDynamicCastGet(uiSlider*,slider,cb);
    if ( !slider ) return;
    const float newscale = defscale_ * slider->getValue();
    setScale( newscale );
}


void uiODAnnotSubItem::removeStuff()
{
    Pick::SetMgr& mgr = Pick::SetMgr::getMgr( managerName() );
    const int setidx = mgr.indexOf( *set_ );
    if ( setidx >= 0 )
    {
	mgr.set( mgr.id(setidx), 0 );
    }
    mgr.removeCBs( this );
}
