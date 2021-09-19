#pragma once

/*
    ###################################################################################
    PRRERendererSWincremental.h
    This file is part of PRRE.
    External header.
    Incremental SW Renderer class.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../../../../../../CConsole/CConsole/src/CConsole.h"
#include "PRREIRenderer.h"
#include "../Display/PRREScreen.h"
#include "../Display/PRREWindow.h"
#include "../Hardware/PRREhwInfo.h"


/**
    Incremental software renderer path.
    Only deviations from the original PRREIRenderer documentation are mentioned here.
*/
class PRRERendererSWincremental :
    public PRREIRenderer
{
#ifdef PRRE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  PRRERendererSWincremental is included")
#endif

public:
    /** Default render hints for PRRERendererSWincremental. */
    static const TPRRE_RENDER_HINT DefaultHints = 0;

    static PRRERendererSWincremental& createAndGet(
        PRREWindow& _wnd,
        PRREScreen& _scr,
        PRREhwInfo& _hwinfo );    /**< Creates and gets the singleton implementation instance. */

    static const char* getLoggerModuleName();          /**< Returns the logger module name of this class. */

    // ---------------------------------------------------------------------------

    CConsole&  getConsole() const;                    /**< Returns access to console preset with logger module name as this class. */

    /**
        Initializes the software renderer.
 
        @param width   Same as described in PRREIRenderer::initialize().
        @param height  Same as described in PRREIRenderer::initialize().
        @param dmode   Same as described in PRREIRenderer::initialize().
        @param freq    Same as described in PRREIRenderer::initialize().
        @param cdepth  Same as described in PRREIRenderer::initialize().
        @param zdepth  The Z-buffer-depth to be set. Currently Z-buffering is not implemented by this renderer.
        @param stencil The stencil buffer depth to be set. Currently Z-buffering is not implemented by this renderer.
        @param samples The level of antialiasing. Currently Z-buffering is not implemented by this renderer.
        @param window  Same as described in PRREIRenderer::initialize().

        @return The result of the initialization. 0 on success, 1 otherwise.
    */
    virtual TPRREuint initialize(
        TPRREuint width, TPRREuint height,
        TPRRE_DISPLAY_MODES dmode,
        TPRREuint freq, TPRREint cdepth,
        TPRREint zdepth, TPRREint stencil,
        TPRREint samples,
        HWND window = NULL) = 0;

    /**
        Not implemented.
        Use RenderScene() to render all objects.
    */
    virtual void RenderObject(PRREObject3D& object) = 0;


}; // PRRERendererHWfixedPipe