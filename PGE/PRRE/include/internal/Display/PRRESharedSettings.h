#pragma once

/*
    ########################################################################################
    PRRESharedSettings.h
    This file is part of PRRE.
    Internal header.
    PRRE shared settings class.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ########################################################################################
*/

#include "../../external/PRREallHeaders.h"
#include "../../external/PRREBaseClass.h"

/**
    Shared settings enumerator.
*/
enum TPRRE_SHARED_SETTINGS
{
    PRRE_SSET_MONITORPOWERSAVE,  /**< Monitor powersave. */
    PRRE_SSET_SCREENSAVER,       /**< Screensaver. */
    PRRE_SSET_STANDBY,           /**< Computer standby mode. */
    PRRE_SSET_VSYNC              /**< VSync. */
}; // TPRRE_SHARED_SETTINGS


/**
    Contains PRRE shared settings.
    These settings are used by a few classes, but we store these in one place (here), to avoid
    redundancy and inconsistency. Other classes reach these settings through pointers. One class
    may be responsible for properly initializing a setting, while other class may be responsible
    for providing get/set access to the user.
    For a user, this class is not known. Users can manipulate shared settings through public
    classes.
    This class is singleton, of course.
    IMPORTANT: the main PRRE object should initialize this class before any other class attempts
    to use it.

    This class doesn't use any API directly.
*/
class PRRESharedSettings :
    public PRREBaseClass
{
#ifdef PRRE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  PRRESharedSettings is included")
#endif

public:

    /**
        Creates and gets the singleton instance.
    */
    static PRRESharedSettings& createAndGet();


    // ---------------------------------------------------------------------------


    /**
        Gets the specified shared setting.
    */
    TPRREbool get(TPRRE_SHARED_SETTINGS s) const;


    /**
        Sets the specified shared setting.
    */
    void Set(TPRRE_SHARED_SETTINGS s, TPRREbool value);

protected:

    // ---------------------------------------------------------------------------

private:

    // ---------------------------------------------------------------------------

    TPRREbool bMonitorPowerSave;      /**< Is monitor powersaving enabled? Used by Window, accessed through Screen from outside. */
    TPRREbool bScreenSaver;           /**< Is screensaver enabled? Used by Window, accessed through Screen from outside. */
    TPRREbool bStandby;               /**< Is computer standby allowed? Used by Window, accessed through Screen from outside. */
    TPRREbool bVSyncSupported;        /**< Is VSync supported? Set by HWInfo, accessed through Screen from outside. */

    // ---------------------------------------------------------------------------

    PRRESharedSettings();

    PRRESharedSettings(const PRRESharedSettings&);
    PRRESharedSettings& operator=(const PRRESharedSettings&);
    
    virtual ~PRRESharedSettings();

}; // class PRRESharedSettings