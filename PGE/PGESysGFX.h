#pragma once

/*
    ###################################################################################
    PGESysGFX.h
    This file is part of PGE.
    Internal header.
    PR00F's Game Engine graphics subsystem
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "PGEallHeaders.h"
#include "PURE/include/external/PR00FsUltimateRenderingEngine.h"
// #include "PURE/PURE.h"

/**
    PR00F's Game Engine graphics handler class.
*/
class PGESysGFX
{
#ifdef PGE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  PGESysGFX is included")   
#endif

public:
    PGESysGFX();
    virtual ~PGESysGFX();

    bool initSysGFX(
        unsigned int width, unsigned int height, TPURE_DISPLAY_MODES dmode, unsigned int freq,
        int cdepth, int zdepth, int stencil, int samples );

    bool destroySysGFX(void);

private:
    PR00FsUltimateRenderingEngine& engine;

    // ---------------------------------------------------------------------------

    PGESysGFX(const PGESysGFX&); 
    PGESysGFX& operator=(const PGESysGFX&);

}; // class PGESysGFX
