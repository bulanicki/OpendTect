#ifndef vismaterial_h
#define vismaterial_h

/*+
________________________________________________________________________

 CopyRight:	(C) dGB Beheer B.V.
 Author:	Kristofer Tingdahl
 Date:		4-11-2002
 RCS:		$Id: vismaterial.h,v 1.8 2004-01-05 09:43:47 kristofer Exp $
________________________________________________________________________


-*/

#include "visdata.h"

class SoMaterial;
class Color;

namespace visBase
{

/*!\brief


*/

class Material : public DataObject
{
public:
    static Material*	create()
			mCreateDataObj(Material);

    void		setColor( const Color& nc );
    const Color&	getColor() const { return color; }

    void		setAmbience( float );
			/*!< Should be between 0 and 1 */
    float		getAmbience() const { return ambience; }

    void		setDiffIntensity( float );
			/*!< Should be between 0 and 1 */
    float		getDiffIntensity() const { return diffuseintencity; }

    void		setSpecIntensity( float );
			/*!< Should be between 0 and 1 */
    float		getSpecIntensity() const { return specularintensity; }

    void		setEmmIntensity( float );
			/*!< Should be between 0 and 1 */
    float		getEmmIntensity() const { return emmissiveintensity; }

    void		setShininess( float );
			/*!< Should be between 0 and 1 */
    float		getShininess() const { return shininess; }

    void		setTransparency( float );
			/*!< Should be between 0 and 1 */
    float		getTransparency() const { return transparency; }

    SoNode*		getInventorNode();
    int			usePar( const IOPar& );
    void		fillPar( IOPar&, TypeSet<int>& ) const;

protected:
			~Material();

    void		updateMaterial();

    Color&		color;
    float               ambience;
    float               diffuseintencity;
    float               specularintensity;
    float               emmissiveintensity;
    float               shininess;
    float               transparency;

    SoMaterial*		material;

    static const char*	colorstr;
    static const char*	ambiencestr;
    static const char*	diffintensstr;
    static const char*	specintensstr;
    static const char*	emmintensstr;
    static const char*	shininessstr;
    static const char*	transpstr;
};

}; // Namespace


#endif
