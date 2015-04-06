/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	J.C. Glas
 Date:		01/12/2014
________________________________________________________________________

-*/
static const char* rcsID mUsedVar = "$Id$";

#include "settingsaccess.h"
#include "envvars.h"


const char* SettingsAccess::sKeyIcons()
{ return "dTect.Icons"; }

const char* SettingsAccess::sKeyShowInlProgress()
{ return "dTect.Show inl progress"; }

const char* SettingsAccess::sKeyShowCrlProgress()
{ return "dTect.Show crl progress"; }

const char* SettingsAccess::sKeyShowRdlProgress()
{ return "dTect.Show rdl progress"; }

const char* SettingsAccess::sKeyUseSurfShaders()
{ return "dTect.Use surface shaders"; }

const char* SettingsAccess::sKeyUseVolShaders()
{ return "dTect.Use volume shaders"; }

const char* SettingsAccess::sKeyTexResFactor()
{ return "dTect.Default texture resolution factor"; }


SettingsAccess::SettingsAccess()
    : settings_( Settings::common() )
{}


SettingsAccess::SettingsAccess( Settings& settings )
    : settings_( settings )
{}


bool SettingsAccess::doesUserWantShading( bool forvolume ) const
{
    if ( GetEnvVarYN("DTECT_MULTITEXTURE_NO_SHADERS") )
	return false;

    bool userwantsshading = true;
    if ( forvolume )
	settings_.getYN( sKeyUseVolShaders(), userwantsshading );
    else
	settings_.getYN( sKeyUseSurfShaders(), userwantsshading );

    return userwantsshading;
}


static int validResolution( int res, int nrres )
{ return mMAX( 0, mMIN(res, nrres-1) ); }


static int defaultTexResFactorFromEnvVar()
{
    const char* envvar = GetEnvVar( "OD_DEFAULT_TEXTURE_RESOLUTION_FACTOR" );
    return envvar && iswdigit(*envvar) ? toInt(envvar) : -1;
}


bool SettingsAccess::systemHasDefaultTexResFactor()
{ return defaultTexResFactorFromEnvVar() >= 0; }


#define mSystemDefaultTexResSetting -1

int SettingsAccess::getDefaultTexResAsIndex( int nrres ) const
{
    int res = 0;
    if ( settings_.get(sKeyTexResFactor(),res) )
    {
	if ( res == mSystemDefaultTexResSetting )
	    res = systemHasDefaultTexResFactor() ? nrres++ : 0;
    }

    return validResolution( res, nrres );
}


void SettingsAccess::setDefaultTexResAsIndex( int idx, int nrres )
{
    int res = validResolution( idx, nrres );
    if ( idx == nrres )
	res = mSystemDefaultTexResSetting;

    settings_.set( sKeyTexResFactor(), res );
}


int SettingsAccess::getDefaultTexResFactor( int nrres ) const
{
    int res = 0;
    if ( settings_.get(sKeyTexResFactor(),res) )
    {
	if ( res != mSystemDefaultTexResSetting )
	    return validResolution( res, nrres );
    }

    res = defaultTexResFactorFromEnvVar();
    return validResolution( res, nrres );
}
