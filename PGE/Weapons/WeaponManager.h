#pragma once

/*
    ###################################################################################
    WeaponManager.h
    This file is part of PGE.
    External header.
    PR00F's Game Engine Weapon Manager
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <map>
#include <set>
#include <string>
#include <vector>

#include "../PGEallHeaders.h"
#include "../PGEcfgFile.h"
#include "../PRRE/include/external/PR00FsReducedRenderingEngine.h"

/**
    Weapon class for PR00F's Game Engine Weapon Manager
*/
class Weapon : public PGEcfgFile
{
#ifdef PGE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  Weapon is included")   
#endif

public:
    static const char* getLoggerModuleName();          /**< Returns the logger module name of this class. */

    // ---------------------------------------------------------------------------

    explicit Weapon(const char* fname);
    virtual ~Weapon();

    CConsole&   getConsole() const;                    /**< Returns access to console preset with logger module name as this class. */

protected:

private:

    static std::set<std::string> m_WpnAcceptedVars;

    // ---------------------------------------------------------------------------

    Weapon();

}; // class Weapon

/**
    PR00F's Game Engine Weapon Manager
*/
class WeaponManager
{
#ifdef PGE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  Weapon Manager is included")   
#endif

public:
    static const char* getLoggerModuleName();          /**< Returns the logger module name of this class. */

    // ---------------------------------------------------------------------------

    WeaponManager(PR00FsReducedRenderingEngine& gfx);
    virtual ~WeaponManager();

    CConsole&   getConsole() const;                    /**< Returns access to console preset with logger module name as this class. */

    bool load(const char* fname);
    const std::vector<Weapon>& getWeapons() const;
    void Clear();

protected:

    WeaponManager(const WeaponManager&) :
        m_gfx(m_gfx)
    {}

    WeaponManager& operator=(const WeaponManager&)
    {
        return *this;
    }

private:

    PR00FsReducedRenderingEngine& m_gfx;
    std::vector<Weapon> m_weapons;

    // ---------------------------------------------------------------------------

    WeaponManager();

}; // class WeaponManager