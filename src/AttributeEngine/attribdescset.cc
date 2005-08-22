/*+
 * COPYRIGHT: (C) dGB Beheer B.V.
 * AUTHOR   : A.H. Bril
 * DATE     : Sep 2003
-*/

static const char* rcsID = "$Id: attribdescset.cc,v 1.26 2005-08-22 15:33:53 cvsnanne Exp $";

#include "attribdescset.h"
#include "attribstorprovider.h"
#include "attribparam.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "bufstringset.h"
#include "separstr.h"
#include "iopar.h"
#include "ioman.h"
#include "ioobj.h"
#include "separstr.h"
#include "gendefs.h"

namespace Attrib
{

DescSet* DescSet::clone() const
{
    DescSet* descset = new DescSet();
    for ( int idx=0; idx<nrDescs(); idx++ )
    {
	Desc* nd = descs[idx]->clone();
	nd->setDescSet( descset );
	descset->addDesc( nd, ids[idx] );
    }

    descset->updateInputs();
    return descset;
}


void DescSet::updateInputs()
{
    for ( int idx=0; idx<nrDescs(); idx++ )
    {
	for ( int inpidx=0; inpidx<descs[idx]->nrInputs(); inpidx++ )
	{
	    const Desc* oldinpdesc = descs[idx]->getInput( inpidx );
	    if ( !oldinpdesc ) continue;
	    Desc* newinpdesc = getDesc( oldinpdesc->id() );
	    descs[idx]->setInput( inpidx, newinpdesc );
	}
    }
}


DescID DescSet::addDesc( Desc* nd, DescID id )
{
    nd->setDescSet( this );
    nd->ref();
    descs += nd;
    const DescID newid = id < 0 ? getFreeID() : id;
    ids += newid;
    return newid;
}


Desc* DescSet::getDesc( const DescID& id )
{
    const int idx = ids.indexOf(id);
    if ( idx==-1 ) return 0;
    return descs[idx];
}


const Desc* DescSet::getDesc( const DescID& id ) const
{
    return const_cast<DescSet*>(this)->getDesc(id);
}


int DescSet::nrDescs() const { return descs.size(); }


DescID DescSet::getID( const Desc& desc ) const
{
    const int idx = descs.indexOf( &desc );
    return idx==-1 ? DescID::undef() : ids[idx];
}


DescID DescSet::getID( int idx ) const
{
    if ( idx < 0 || idx >= ids.size() ) return DescID::undef();
    return ids[idx];
}


void DescSet::getIds( TypeSet<DescID>& attribids ) const
{
    attribids = ids;
}


DescID DescSet::getID( const char* str, bool isusrref ) const
{
    if ( !str || !*str ) return DescID::undef();

    for ( int idx=0; idx<nrDescs(); idx++ )
    {
	if ( isusrref && descs[idx]->isIdentifiedBy(str) )
	    return ids[idx];
	else if ( !isusrref )
	{
	    BufferString defstr;
	    descs[idx]->getDefStr( defstr );
	    if ( defstr == str )
		return ids[idx];
	}
    }

    return DescID::undef();
}


void DescSet::removeDesc( const DescID& id )
{
    const int idx = ids.indexOf(id);
    if ( idx==-1 ) return;

    if ( descs[idx]->descSet()==this )
	descs[idx]->setDescSet(0);

    descs[idx]->unRef();
    descs.remove(idx);
    ids.remove(idx);
}


void DescSet::removeAll()
{ while ( ids.size() ) removeDesc(ids[0]); }


void DescSet::fillPar( IOPar& par ) const
{
    int maxid = 0;

    for ( int idx=0; idx<descs.size(); idx++ )
    {
	IOPar apar;
	BufferString defstr;
	if ( !descs[idx]->getDefStr(defstr) ) continue;
	apar.set( definitionStr(), defstr );

	BufferString userref( descs[idx]->userRef() );
	apar.set( userRefStr(), userref );

	apar.setYN( hiddenStr(), descs[idx]->isHidden() );

	for ( int input=0; input<descs[idx]->nrInputs(); input++ )
	{
	    if ( !descs[idx]->getInput(input) ) continue;

	    const char* key = IOPar::compKey( inputPrefixStr(), input );
	    apar.set( key, getID( *descs[idx]->getInput(input) ).asInt() );
	}

	BufferString subkey = ids[idx].asInt();
	par.mergeComp( apar, subkey );

	if ( ids[idx]>maxid ) maxid = ids[idx].asInt();
    }

    par.set( highestIDStr(), maxid );
}


#define mHandleParseErr( str ) \
{ \
    errmsg = str; \
    if ( !errmsgs ) \
	return false; \
\
    (*errmsgs) += new BufferString(errmsg); \
    continue; \
}

bool DescSet::usePar( const IOPar& par, BufferStringSet* errmsgs )
{
    removeAll();
    if ( errmsgs ) deepErase( *errmsgs );

    int maxid = 1024;
    par.get( highestIDStr(), maxid );
    ObjectSet<Desc> newsteeringdescs;
    IOPar copypar(par);

    for ( int id=0; id<=maxid; id++ )
    {
	const BufferString idstr( id );
	PtrMan<IOPar> descpar = par.subselect(idstr);
	if ( !descpar ) continue;

	//Look for type (old format)
	const char* typestr = descpar->find("Type");
	if ( typestr && !strcmp(typestr,"Stored" ) )
	{
	    const char* olddef = descpar->find(definitionStr());
	    if ( !olddef ) continue;
	    BufferString newdef = StorageProvider::attribName();
	    newdef += " ";
	    newdef += Attrib::StorageProvider::keyStr();
	    newdef += "=";
	    newdef +=olddef;
	    descpar->set(definitionStr(),newdef);
	}

	int steeringdescid = -1;
	const IOPar* steeringpar = descpar->subselect("Steering");
	if ( steeringpar )
	{
	    const char* defstring = descpar->find(definitionStr());
	    if ( !defstring )
		mHandleParseErr("No attribute definition string specified");
	    if ( !createSteeringDesc(*steeringpar, defstring, newsteeringdescs,
					steeringdescid ) )
	    {
	        BufferString err = "Cannot create steering desc ";
	        mHandleParseErr(err);
	    }
	}

	BufferString defstring = descpar->find(definitionStr());
	if ( !defstring.size() )
	    mHandleParseErr("No attribute definition string specified");

	BufferString attribname;
	if ( !Desc::getAttribName( defstring.buf(), attribname ) )
	    mHandleParseErr("Cannot find attribute name");

	if ( attribname == "RefTime" )
	{
	    attribname = "Reference";
	    defstring = attribname;
	    descpar->set("Selected Attrib","2");
	}
	
	RefMan<Desc> desc;
	desc = PF().createDescCopy( attribname );
	if ( !desc )
	{
	    BufferString err = "Cannot find factory-entry for ";
	    err += attribname;
	    mHandleParseErr(err);
	}

	if ( !desc->parseDefStr(defstring.buf()) )
	{
	    if ( !desc->isStored() )
	    {
		BufferString err = "Cannot parse: ";
		err += defstring;
		mHandleParseErr(err);
	    }
	}

	const char* userref = descpar->find( userRefStr() );
	if ( userref ) desc->setUserRef( userref );

	bool ishidden = false;
	descpar->getYN( hiddenStr(), ishidden );
	desc->setHidden( ishidden );

	int selout = 0;
	if ( descpar->get("Selected Attrib",selout) )
	    desc->selectOutput(selout);

	if ( steeringpar )
	{
	    for ( int idx=0; idx<desc->nrInputs(); idx++ )
	    {
		BufferString inputstr = IOPar::compKey( "Input", idx );
		if ( !strcmp(descpar->find(inputstr),"-1") )
		{
		    BufferString newinput = id;
		    newinput = IOPar::compKey( newinput, inputstr );
		    copypar.set( newinput, maxid + steeringdescid +1 );
		}
	    }
	}

	desc->updateParams();
	addDesc( desc, DescID(id,true) );
    }

    for( int idx=0 ; idx<newsteeringdescs.size() ; idx++ )
	addDesc( newsteeringdescs[idx], DescID( maxid+idx+1, true ) );

    for ( int idx=0; idx<ids.size()-newsteeringdescs.size(); idx++ )
    {
	const BufferString idstr( ids[idx].asInt() );
	PtrMan<IOPar> descpar = copypar.subselect(idstr);
	if ( !descpar )
	    { pErrMsg("Huh?"); continue; }

	for ( int input=0; input<descs[idx]->nrInputs(); input++ )
	{
	    const char* key = IOPar::compKey( inputPrefixStr(), input );

	    int inpid;
	    if ( !descpar->get(key,inpid) ) continue;

	    Desc* inpdesc = getDesc( DescID(inpid,true) );
	    if ( !inpdesc ) continue;

	    descs[idx]->setInput( input, inpdesc );
	}
	if ( descs[idx]->isSatisfied() == Desc::Error )
	{
	    BufferString err = "inputs or parameters are not satisfied for ";
	    err += descs[idx]->attribName();
	    mHandleParseErr(err);
	}
    }

    return true;
}


#define mHandleSteeringParseErr( str ) \
{ \
    errmsg = str; \
    if ( !errmsgs ) \
	return false; \
\
    (*errmsgs) += new BufferString(errmsg); \
}


bool DescSet::createSteeringDesc( const IOPar& steeringpar, 
				  BufferString defstring,
				  ObjectSet<Desc>& newsteeringdescs, int& id,
				  BufferStringSet* errmsgs )
{
    BufferString steeringtype = steeringpar.find("Type");
    BufferString steeringdef = steeringtype;
    steeringdef += " stepout=";
    const char* stepoutstr = steeringpar.find( "Stepout" );
    if ( stepoutstr )
	steeringdef += stepoutstr;
    else
    {
	BufferString pos0val;
	BufferString stepoutval;
	if ( Desc::getParamString(defstring,"stepout",stepoutval) )
	{
	    BinIDParam param("stepout");
	    if ( stepoutval.size() == 1 )
	    {
		BufferString saveval = stepoutval;
		stepoutval += ","; stepoutval += saveval;
	    }

	    param.setCompositeValue( stepoutval );
	    BinID bid( param.getIntValue(0), param.getIntValue(1) );
	    steeringdef += abs(bid.inl); steeringdef += ",";
	    steeringdef += abs(bid.crl);
	}
	else if ( Desc::getParamString(defstring,"pos0",pos0val) )
	{
	    BufferString pos1val;
	    Desc::getParamString( defstring, "pos1", pos1val );
	    BinIDParam pos0("pos0");
	    BinIDParam pos1("pos1");
	    pos0.setCompositeValue(pos0val);
	    pos1.setCompositeValue(pos1val);
	    BinID pos0bid, pos1bid;
	    pos0bid.inl = pos0.getIntValue(0);
	    pos0bid.crl = pos0.getIntValue(1);
	    pos1bid.inl = pos1.getIntValue(0);
	    pos1bid.crl = pos1.getIntValue(1);
	    
	    int outputinl = mMAX( abs(pos0bid.inl), abs(pos1bid.inl) );
	    int outputcrl = mMAX( abs(pos0bid.crl), abs(pos1bid.crl) );
	    steeringdef += outputinl; steeringdef += ","; 
	    steeringdef += outputcrl;
	}
	else
	    steeringdef += "5,5";
    }

    if ( steeringtype == "ConstantSteering" )
    {
	steeringdef += " ";
	steeringdef += "dip=";
	steeringdef += steeringpar.find("AppDip");
	steeringdef += " ";
	steeringdef += "azi=";
	steeringdef += steeringpar.find("Azimuth");
    }
    else
    {   
	steeringdef += " phlock=";
	bool phaselock = false;
	steeringpar.getYN( "PhaseLock", phaselock );
	steeringdef += phaselock ? "Yes" : "No";
	if ( phaselock )
	{
	    steeringdef += " aperture=";
	    const char* aperture = steeringpar.find("Aperture");
	    steeringdef += aperture ? aperture : "-5,5";
	}
    }

    BufferString attribname;
    if ( !Desc::getAttribName(steeringdef,attribname) )
	mHandleSteeringParseErr("Cannot find attribute name");

    RefMan<Desc> stdesc = PF().createDescCopy(attribname);
    if ( !stdesc )
    {
	BufferString err = "Cannot find factory-entry for ";
	err += attribname;
	mHandleSteeringParseErr(err);
    }

    if ( !stdesc->parseDefStr(steeringdef) )
    {
	BufferString err = "Cannot parse: ";
	err += steeringdef;
	mHandleSteeringParseErr(err);
    }

    BufferString usrrefstr = "steering input ";
    usrrefstr += newsteeringdescs.size();
    stdesc->setUserRef( usrrefstr );
    stdesc->setSteering(true);
    stdesc->setHidden(true);
    
    const char* inldipstr = steeringpar.find("InlDipID");
    if ( inldipstr )
    {
	DescID inldipid( atoi(inldipstr), true );
	stdesc->setInput( 0, getDesc(inldipid) );
    }

    const char* crldipstr = steeringpar.find("CrlDipID");
    if ( crldipstr )
    {
	DescID crldipid( atoi(crldipstr), true );
	stdesc->setInput( 1, getDesc(crldipid) );
    }	

//TODO see what's going on for the phase input	
    for ( int idx=0; idx<newsteeringdescs.size(); idx++ )
    {
	if ( stdesc->isIdenticalTo(*newsteeringdescs[idx]) )
	{
	    id = idx;
	    return true;
	}
    }
    stdesc->ref();
    newsteeringdescs += stdesc;
    id = newsteeringdescs.size()-1;
    
    return true;
}


const char* DescSet::errMsg() const
{ return errmsg[0] ? (const char*) errmsg : 0; }


DescID DescSet::getFreeID() const
{
    DescID id(0,true);
    while ( ids.indexOf(id)!=-1 )
	id.asInt()++;

    return id;
}


bool DescSet::is2D() const
{
    for ( int idx=0; idx<descs.size(); idx++ )
    {
	if ( descs[idx]->is2D() )
	    return true;
    }

    return false;
}


DescID DescSet::getStoredID( const char* lk, int selout, bool create )
{
    for ( int idx=0; idx<descs.size(); idx++ )
    {
	const Desc* desc = descs[idx];
	if ( !desc->isStored() || desc->selectedOutput()!=selout )
	    continue;

	const ValParam* keypar = desc->getValParam( StorageProvider::keyStr() );
	const char* curlk = keypar->getStringValue();
	if ( !strcmp(lk,curlk) ) return desc->id();
    }

    if ( !create ) return DescID::undef();

    LineKey newlk( lk );
    MultiID mid = newlk.lineName().buf();
    PtrMan<IOObj> ioobj = IOM().get( mid );
    if ( !ioobj ) return DescID::undef();

    Desc* newdesc = PF().createDescCopy( StorageProvider::attribName() );
    if ( !newdesc ) return DescID::undef(); // "Cannot create desc"

    BufferString userref = LineKey( ioobj->name(), newlk.attrName() );
    newdesc->setUserRef( userref );
    newdesc->selectOutput( selout );
    ValParam* keypar = newdesc->getValParam( StorageProvider::keyStr() );
    keypar->setValue( lk );
    return addDesc( newdesc );
}


DescSet* DescSet::optimizeClone( const DescID& targetnode ) const
{
    TypeSet<DescID> needednodes( 1, targetnode );
    return optimizeClone( needednodes );
}


DescSet* DescSet::optimizeClone( const TypeSet<DescID>& targets ) const
{
    DescSet* res = new DescSet;
    TypeSet<DescID> needednodes = targets;
    while ( needednodes.size() )
    {
	const DescID needednode = needednodes[0];
	needednodes.remove( 0 );
	const Desc* desc = getDesc( needednode );
	if ( !desc )
	{
	    delete res;
	    return 0;
	}

	Desc* nd = desc->clone();
	nd->setDescSet( res );
	res->addDesc( nd, needednode );

	for ( int idx=0; idx<desc->nrInputs(); idx++ )
	{
	    const Desc* inpdesc = desc->getInput(idx);
	    const DescID inputid = inpdesc ? inpdesc->id() : DescID::undef();
	    if ( inputid!=DescID::undef() && !res->getDesc(inputid) )
		needednodes += inputid;
	}
    }

    if ( res->nrDescs() == 0 )
	{ delete res; res = clone(); }

    res->updateInputs();
    return res;
}


bool DescSet::isAttribUsed( const DescID& id ) const
{
    for ( int idx=0; idx<nrDescs(); idx++ )
    {
	const Desc* ad = descs[idx];
	for ( int inpnr=0; inpnr<ad->nrInputs(); inpnr++ )
	{
	    if ( ad->inputId(inpnr) == id )
		return true;
	}
    }

    return false;
}


int DescSet::removeUnused( bool remstored )
{
    TypeSet<DescID> torem;

    while ( true )
    {
	int count = 0;
	for ( int descidx=0; descidx<nrDescs(); descidx++ )
	{
	    DescID descid = getID( descidx );
	    if ( torem.indexOf(descid) >= 0 ) continue;

	    const Desc* ad = getDesc( descid );
	    bool iscandidate = false;
	    if ( ad->isStored() )
	    {
		const ValParam* keypar = 
		    	ad->getValParam( StorageProvider::keyStr() );
		PtrMan<IOObj> ioobj = IOM().get( keypar->getStringValue() );
		if ( remstored || !ioobj || !ioobj->implExists(true) )
		    iscandidate = true;
	    }
	    else if ( ad->isHidden() )
		iscandidate = true;

	    if ( iscandidate )
	    {
		if ( !isAttribUsed(descid) )
		    { torem += descid; count++; }
	    }
	}

	if ( count == 0 ) break;
    }

    const int sz = torem.size();
    for ( int idx=sz-1; idx>=0; idx-- )
	removeDesc( torem[idx] );

    return sz;
}


bool DescSet::getFirstStored( Pol2D p2d, MultiID& key ) const
{
    for ( int idx=0; idx<nrDescs(); idx++ )
    {
	const Desc* ad = descs[idx];
	if ( !ad->isStored() ) continue;
	if ( (ad->is2D() && p2d != No2D) || (!ad->is2D() && p2d != Only2D) )
	{
	    const ValParam* keypar = ad->getValParam(StorageProvider::keyStr());
	    key = keypar->getStringValue();
	    return true;
	}
    }

    return false;
}


Desc* DescSet::getFirstStored( Pol2D p2d )
{
    for ( int idx=0; idx<nrDescs(); idx++ )
    {
	Desc* ad = descs[idx];
	if ( !ad->isStored() ) continue;

	if ( (ad->is2D() && p2d != No2D) || (!ad->is2D() && p2d != Only2D) )
	    return ad;
    }
    return 0;
}

/*
bool DescSet::setRanges( IOPar& iop, bool incstepout ) const
{
    CubeSampling cs;
    if ( !getInputRanges(cs) )
	return false;

    if ( incstepout )
    {
	BinID stepout = getMaximumStepout();
	cs.hrg.start.inl += stepout.inl * cs.hrg.step.inl;
	cs.hrg.stop.inl -= stepout.inl * cs.hrg.step.inl;
	cs.hrg.start.crl += stepout.crl * cs.hrg.step.crl;
	cs.hrg.stop.crl -= stepout.crl * cs.hrg.step.crl;
    }

    iop.set( sKeyMaxInlRg, cs.hrg.start.inl, cs.hrg.stop.inl, cs.hrg.step.inl );
    iop.set( sKeyMaxCrlRg, cs.hrg.start.crl, cs.hrg.stop.crl, cs.hrg.step.crl );
	return true;
}


bool DescSet::getInputRanges( CubeSampling& cs, const char* lk ) const
{
    cs.init();
    if ( is2D() )
    {
	cs.hrg.start.inl = cs.hrg.start.crl = 0;
	cs.hrg.stop.inl = cs.hrg.stop.crl = mUndefIntVal;
	cs.hrg.step.inl = cs.hrg.step.crl = 1;
    }
    
    bool foundone = false;
    for ( int idx=0; idx<ids.size(); idx++ )
    {
	const Desc* desc = getDesc( getID(ids[idx]) );
	if ( !desc.isStored() ) continue;

	CubeSampling lcs( cs );
	if ( desc->getDataLimits(lcs,lk) )
	{
	    if ( !foundone )
	    {
		cs = lcs;
		foundone = true;
	    }
	    else
	    {
		if ( ad->is2D() )
		    lcs.zrg.step = cs.zrg.step;
		cs.limitTo( lcs );
	    }
	}
    }

    return foundone;
}


BinID DescSet::getMaximumStepout() const
{
    BinID res(0,0);

    for ( int idx=0; idx<ids.size(); idx++ )
    {
	TypeSet<int>    localstepoutids;
	TypeSet<BinID>  localstepout;
	if ( !getMaximumStepout( ids[idx], BinID(0,0),
		    localstepoutids, localstepout ) )
	    continue;

	for ( int idy=0; idy<localstepoutids.size(); idy++ )
	{
	    res = BinID( mMAX(localstepout[idy].inl, res.inl),
			 mMAX(localstepout[idy].crl, res.crl));
	}
    }

    return res;
}
//missing another getMaxStepout
*/
}; // namespace Attrib
