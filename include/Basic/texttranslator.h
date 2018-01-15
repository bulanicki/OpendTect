#pragma once

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Nanne Hemstra
 Date:		January 2010
________________________________________________________________________

-*/


#include "basicmod.h"
#include "notify.h"
#include "uistring.h"
#include "bufstringset.h"

mFDQtclass(QTranslator)
mFDQtclass(QLocale)
mFDQtclass(QString)

class TextTranslatorLanguage;
namespace File { class Path; }

/*!Manager that keeps track of translations. By default, it only handles
   English plural, but it can be extended with more languages. */

mExpClass(Basic) TextTranslateMgr : public CallBacker
{ mODTextTranslationClass(TextTranslateMgr);
public:
				TextTranslateMgr();
				~TextTranslateMgr();

    int				nrSupportedLanguages() const;
    uiString			getLanguageUserName(int) const;
    BufferString		getLocaleName(int) const;
    uiRetVal			setLanguage(int);
    int				currentLanguage() const;

    Notifier<TextTranslateMgr>	languageChange;
				/*!<Triggers both on new languages and changed
				    languages. */

    int				changeCount() const	{ return dirtycount_; }
				//Increased every time language is changed

    const mQtclass(QTranslator)* getQTranslator(const char* appl) const;
    const mQtclass(QLocale)*	getQLocale() const;

    bool			addLanguage(TextTranslatorLanguage*);
				//!<Returns false if it was not added.

protected:
    friend class			TextTranslatorLanguage;

    void				loadUSEnglish();

    int					dirtycount_;
    ObjectSet<TextTranslatorLanguage>	languages_;
    int					currentlanguageidx_;
public: //Speicalized stuff
    static void				GetLocalizationDir(File::Path&);
    static char				cApplicationEnd() { return '_'; }
};


/*!Holds the translation for one language. Each language has its own locale
   code, (such as en-us) */
mExpClass(Basic) TextTranslatorLanguage : public RefCount::Referenced
{ mODTextTranslationClass(TextTranslatorLanguage);
public:
					TextTranslatorLanguage(
					    const char*vlocalename);

    const mQtclass(QString)&		getLanguageName() const;
    const mQtclass(QLocale)&		getLanguageLocale() const;
    BufferString			getLocaleName() const;

    bool				addFile(const char* filename);

    bool				load();

    const mQtclass(QTranslator)*	getTranslator(const char* appl) const;

protected:
                                        ~TextTranslatorLanguage();

    bool				loaded_;
    BufferString			localename_;
    mQtclass(QString)*			languagename_;
    mQtclass(QLocale)*			locale_;

    BufferStringSet			filenames_;

    ObjectSet<QTranslator>		translators_;
    BufferStringSet			applications_;
};

mGlobal(Basic) TextTranslateMgr& TrMgr();

namespace OD
{
mGlobal(Basic) void	loadLocalization();
}

