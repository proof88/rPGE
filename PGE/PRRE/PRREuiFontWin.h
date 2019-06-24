#pragma once

/*
    ###################################################################################
    PRREuiFontWin.h
    This file is part of PRRE.
    Internal header.
    PRRE User Interface Windows Font class.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "PRREallHeaders.h"
#include "PRREBaseClass.h"

/**
    PRRE User Interface Windows Font class.

    This class API usage?
*/
class PRREuiFontWin :
    public PRREBaseClass
{
#ifdef PRRE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  PRREuiFontWin is included")
#endif

public:

    // ---------------------------------------------------------------------------

    PRREuiFontWin();

    PRREuiFontWin(
        const std::string& fontFaceName, int height, bool bold, bool italic, bool underline, bool strikeout,
        HDC wnd_dc );

    PRREuiFontWin(const PRREuiFontWin& uiFont);
    PRREuiFontWin& PRREuiFontWin::operator=(const PRREuiFontWin& uiFont);

    virtual ~PRREuiFontWin();

    const std::string& getFontFaceName() const;

    int  getHeight() const;
    bool getBold() const;
    bool getItalic() const;
    bool getUnderline() const;
    bool getStrikeOut() const;
    unsigned int getListBase() const;

    long getTextWidth(const std::string& text) const;
    long getTextHeight(const std::string& text) const;

protected:

    // ---------------------------------------------------------------------------

private:

    // ---------------------------------------------------------------------------

    HDC hDC;
    HFONT hFont;
    int nHeight;
    int nWeight;
    DWORD bItalic;
    DWORD bUnderline;
    DWORD bStrikeOut;
    std::string sFontFaceName;
    unsigned int base;

};
