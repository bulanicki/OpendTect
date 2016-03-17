/*+
 * (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 * AUTHOR   : A.H. Bril
 * DATE     : Feb 2004
-*/


#include "unitofmeasure.h"
#include "ascstream.h"
#include "separstr.h"
#include "survinfo.h"
#include "od_iostream.h"
#include "file.h"
#include "filepath.h"
#include "debug.h"
#include "ioman.h"
#include "uistrings.h"

static const char* filenamebase = "UnitsOfMeasure";
static const char* sKeyUOM = "UOM";


class UnitOfMeasureCurDefsMgr : public CallBacker
{
public:

UnitOfMeasureCurDefsMgr() : iop_(crNew())
{
    IOM().surveyChanged.notify( mCB(this,UnitOfMeasureCurDefsMgr,newSrv) );
    const CallBack savedefscb( mCB(this,UnitOfMeasureCurDefsMgr,saveDefs) );
    IOM().surveyToBeChanged.notify( savedefscb );
    IOM().applicationClosing.notify( savedefscb );
}

~UnitOfMeasureCurDefsMgr()
{ delete iop_; }

void saveDefs( CallBacker* )
{ UnitOfMeasure::saveCurrentDefaults(); }

void newSrv( CallBacker* )
{ delete iop_; iop_ = crNew(); }

static IOPar* crNew()
{
    IOPar* ret = SI().pars().subselect( sKeyUOM );
    if ( !ret ) ret = new IOPar;
    ret->setName( "Current Units of Measure" );
    return ret;
}

    IOPar*	iop_;

};

static UnitOfMeasureCurDefsMgr* uomcurdefsmgr_ = 0;
	// shld be static class member but not sure about initialization then

IOPar& UnitOfMeasure::currentDefaults()
{
    if ( !uomcurdefsmgr_ )
	uomcurdefsmgr_ = new UnitOfMeasureCurDefsMgr;
    return *uomcurdefsmgr_->iop_;
}

void UnitOfMeasure::saveCurrentDefaults()
{
    SI().getPars().mergeComp( currentDefaults(), sKeyUOM );
    SI().savePars();
}


UnitOfMeasureRepository& UoMR()
{
    mDefineStaticLocalObject( PtrMan<UnitOfMeasureRepository>, umrepo, = 0 );
    if ( !umrepo )
    {
	if ( DBG::isOn() ) DBG::message( "Creating UnitOfMeasureRepository" );
	umrepo = new UnitOfMeasureRepository;
	if ( DBG::isOn() )
	{
	    BufferString msg( "Total units of measure: " );
	    msg += umrepo->all().size();
	    DBG::message( msg );
	}
    }
    return *umrepo;
}


UnitOfMeasure& UnitOfMeasure::operator =( const UnitOfMeasure& uom )
{
    if ( this != &uom )
    {
	setName( uom.name() );
	symbol_ = uom.symbol_;
	scaler_ = uom.scaler_;
	proptype_ = uom.proptype_;
	source_ = uom.source_;
    }
    return *this;
}


const UnitOfMeasure* UnitOfMeasure::getGuessed( const char* nm )
{
    const UnitOfMeasure* direct = UoMR().get( nm );
    return direct ? direct : UoMR().get( UoMR().guessedStdName(nm) );
}


const UnitOfMeasure* UnitOfMeasure::surveyDefZUnit()
{
    return SI().zIsTime() ? surveyDefTimeUnit() : surveyDefDepthUnit();
}


const UnitOfMeasure* UnitOfMeasure::surveyDefTimeUnit()
{
    return UoMR().get( "Milliseconds" );
}


const UnitOfMeasure* UnitOfMeasure::surveyDefDepthUnit()
{
    return UoMR().get( SI().depthsInFeet() ? "Feet" : "Meter" );
}


const UnitOfMeasure* UnitOfMeasure::surveyDefVelUnit()
{
    return UoMR().get( SI().depthsInFeet() ? "Feet/second" : "Meter/second" );
}


uiString UnitOfMeasure::surveyDefZUnitAnnot( bool symb, bool withparens )
{
    return zUnitAnnot( SI().zIsTime(), symb, withparens );
}


uiString UnitOfMeasure::surveyDefTimeUnitAnnot( bool symb, bool withparens )
{
    return zUnitAnnot( true, symb, withparens );
}


uiString UnitOfMeasure::surveyDefDepthUnitAnnot( bool symb, bool withparens )
{
    return zUnitAnnot( false, symb, withparens );
}

#define mCreateLbl() \
{ \
    if ( withparens ) lbl.append( "(" ); \
    lbl.append( tr(symb ? uom->symbol() : uom->name()) ); \
    if ( withparens ) lbl.append( ")" ); \
}

uiString UnitOfMeasure::zUnitAnnot( bool time, bool symb, bool withparens )
{
    const UnitOfMeasure* uom = time  ? surveyDefTimeUnit()
				     : surveyDefDepthUnit();
    if ( !uom )
	return uiString::emptyString();

    uiString lbl;
    mCreateLbl()

    return lbl;
}


uiString UnitOfMeasure::surveyDefVelUnitAnnot( bool symb, bool withparens )
{
    const UnitOfMeasure* uom = surveyDefVelUnit();
    if ( !uom )
	return uiString::emptyString();

    uiString lbl;
    mCreateLbl()

    return lbl;
}


bool UnitOfMeasure::isImperial() const
{
    const char* unitnm = name();
    const char* unitsymb = symbol_.buf();
    BufferStringSet needle;
    TypeSet<int> usename;

    needle.add( getDistUnitString( true, false ) ); usename += 0;
    needle.add( "Fahrenheit" ); usename += 1;
    needle.add( "Inch" ); usename += 1;
    needle.add( "psi" ); usename += 0;
    needle.add( "pounds" ); usename += 1;

    for ( int idx=0; idx<needle.size(); idx++ )
    {
	const char* haystack = (bool)usename[idx] ? unitnm : unitsymb;
	if ( firstOcc(haystack,needle[idx]->buf()) )
	    return true;
    }

    return false;
}




UnitOfMeasureRepository::UnitOfMeasureRepository()
{
    Repos::FileProvider rfp( filenamebase );
    while ( rfp.next() )
	addUnitsFromFile( rfp.fileName(), rfp.source() );
}


void UnitOfMeasureRepository::addUnitsFromFile( const char* fnm,
						Repos::Source src )
{
    od_istream strm( fnm );
    if ( !strm.isOK() )
	return;

    ascistream stream( strm, true );
    while ( !atEndOfSection( stream.next() ) )
    {
	FileMultiString fms( stream.value() );
	const int sz = fms.size();
	if ( sz < 3 ) continue;
	SeparString types( fms[0], '|' );
	const BufferString symb = fms[1];
	double fac = fms.getDValue( 2 );
	double shft = sz > 3 ? fms.getDValue( 3 ) : 0.;

	const int nrtypes = types.size();
	for ( int ityp=0; ityp<nrtypes; ityp++ )
	{
	    PropertyRef::StdType stdtype =
                PropertyRef::StdTypeDef().parse( types[ityp] );
	    UnitOfMeasure un( stream.keyWord(), symb, fac, stdtype );
	    un.setScaler( LinScaler(shft,fac) );
	    un.setSource( src );
	    add( un );
	}
    }
}


bool UnitOfMeasureRepository::write( Repos::Source src ) const
{
    Repos::FileProvider rfp( filenamebase );
    const BufferString fnm = rfp.fileName( src );

    bool havesrc = false;
    for ( int idx=0; idx<entries.size(); idx++ )
    {
	if ( entries[idx]->source() == src )
	    { havesrc = true; break; }
    }
    if ( !havesrc )
	return !File::exists(fnm) || File::remove( fnm );

    od_ostream strm( fnm );
    if ( !strm.isOK() )
    {
	BufferString msg( "Cannot write to " ); msg += fnm;
	strm.addErrMsgTo( msg ); ErrMsg( fnm );
	return false;
    }

    ascostream astrm( strm );
    astrm.putHeader( "Units of Measure" );
    for ( int idx=0; idx<entries.size(); idx++ )
    {
	const UnitOfMeasure& uom = *entries[idx];
	if ( uom.source() != src ) continue;

	FileMultiString fms( PropertyRef::toString(uom.propType()) );
	fms += uom.symbol();
	fms += uom.scaler().toString();
	astrm.put( uom.name(), fms );
    }

    return true;
}


const char* UnitOfMeasureRepository::guessedStdName( const char* nm )
{
    const FixedString fsnm( nm );
    if ( fsnm.isEmpty() )
	return 0;

    switch ( *nm )
    {
    case 'F': case 'f':
	if ( caseInsensitiveEqual(nm,"F",0)
	  || caseInsensitiveEqual(nm,"FT",0)
	  || caseInsensitiveEqual(nm,"FEET",0) )
	    return "ft";
	else if ( caseInsensitiveEqual(nm,"F/S",0)
	       || caseInsensitiveEqual(nm,"F/SEC",0) )
	    return "ft/s";
	else if ( (fsnm.endsWith("/S",CaseInsensitive)
		    || fsnm.endsWith("/SEC",CaseInsensitive))
		&& (fsnm.startsWith("FT",CaseInsensitive)
		    || fsnm.startsWith("FEET",CaseInsensitive)) )
	    return "ft/s";
    break;
    case 'K' : case 'k':
	if ( fsnm.startsWith("kg/m2s",CaseInsensitive) )
	    return "m/s x kg/m3";
	if ( fsnm.startsWith("kg/m2us",CaseInsensitive) )
	    return "kg/m3 / us/m";
    break;
    case 'G' : case 'g':
	if ( caseInsensitiveEqual(nm,"gAPI",0) )
	    return "API";
	if ( fsnm.startsWith("G/cm2s",CaseInsensitive) )
	    return "m/s x g/cc";
	if ( fsnm.startsWith("G/C",CaseInsensitive)
	  || fsnm.startsWith("GM/C",CaseInsensitive)
	  || fsnm.startsWith("GR/C",CaseInsensitive) )
	    return "g/cc";
    break;
    case 'P' : case 'p':
	if ( caseInsensitiveEqual(nm,"PU",0) )
	    return "%";
    break;
    case 'U' : case 'u':
	if ( fsnm.startsWith("USEC/F",CaseInsensitive)
	  || fsnm.startsWith("US/F",CaseInsensitive) )
	    return "us/ft";
	else if ( fsnm.startsWith("USEC/M",CaseInsensitive) )
	    return "us/m";
    break;
    case 'm':
	if ( caseInsensitiveEqual(nm,"m3/m3",0) ) // Petrel special
	    return "";
    break;
    }

    return 0;
}


const UnitOfMeasure* UnitOfMeasureRepository::get( const char* nm ) const
{
    return findBest( entries, nm );
}


const UnitOfMeasure* UnitOfMeasureRepository::get( PropertyRef::StdType typ,
						   const char* nm ) const
{
    ObjectSet<const UnitOfMeasure> uns; getRelevant( typ, uns );
    return findBest( uns, nm );
}


const UnitOfMeasure* UnitOfMeasureRepository::findBest(
	const ObjectSet<const UnitOfMeasure>& uns, const char* nm ) const
{
    const FixedString fsnm( nm );
    if ( fsnm.isEmpty() )
	return 0;

    if ( fsnm.startsWith( "FRAC", CaseInsensitive )
      || fsnm.startsWith( "DEC", CaseInsensitive )
      || fsnm.startsWith( "UNITLESS", CaseInsensitive ) )
	nm = "Fraction";

    if ( fsnm.startsWith( "RAT", CaseInsensitive ) )
	nm = "Ratio";

    for ( int idx=0; idx<uns.size(); idx++ )
    {
	if ( uns[idx]->name().isEqual(nm,CaseInsensitive) )
	    return uns[idx];
    }
    for ( int idx=0; idx<uns.size(); idx++ )
    {
	if ( FixedString(uns[idx]->symbol()).isEqual(nm,CaseInsensitive) )
	    return uns[idx];
    }

    return 0;
}


bool UnitOfMeasureRepository::add( const UnitOfMeasure& uom )
{
    if ( get(uom.name()) || get(uom.symbol()) )
	return false;

    entries += new UnitOfMeasure( uom );
    return true;
}


void UnitOfMeasureRepository::getRelevant(
		PropertyRef::StdType typ,
		ObjectSet<const UnitOfMeasure>& ret ) const
{
    for ( int idx=0; idx<entries.size(); idx++ )
    {
	if ( typ == PropertyRef::Other || entries[idx]->propType() == typ )
	    ret += entries[idx];
    }
}


const UnitOfMeasure* UnitOfMeasureRepository::getInternalFor(
		PropertyRef::StdType st ) const
{
    ObjectSet<const UnitOfMeasure> candidates;
    getRelevant( st, candidates );
    for ( int idx=0; idx<candidates.size(); idx++ )
    {
	if ( candidates[idx]->scaler().isEmpty() )
	    return candidates[idx];
    }
    return 0;
}


const UnitOfMeasure* UnitOfMeasureRepository::getCurDefaultFor(
		const char* ky ) const
{
    const char* res = UnitOfMeasure::currentDefaults().find( ky );
    return res && *res ? get( res ) : 0;
}


const UnitOfMeasure* UnitOfMeasureRepository::getDefault( const char* ky,
					PropertyRef::StdType st ) const
{
    const UnitOfMeasure* ret = getCurDefaultFor( ky );
    if ( !ret )
	ret = getCurDefaultFor( PropertyRef::toString(st) );
    return ret ? ret : getInternalFor( st );
}
