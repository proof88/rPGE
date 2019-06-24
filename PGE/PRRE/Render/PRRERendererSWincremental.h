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

#include "PRREIRenderer.h"
#include "../PRREScreen.h"
#include "../PRREWindow.h"
#include "../PRREhwInfo.h"

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
    static PRRERendererSWincremental& createAndGet(
        PRREWindow& _wnd,
        PRREScreen& _scr,
        PRREhwInfo& _hwinfo );    /**< Creates and gets the singleton implementation instance. */

    // ---------------------------------------------------------------------------


    /**
        Only deviations from the original PRREIRenderer documentation are mentioned here.
 
        @param zdepth  The Z-buffer-depth to be set. Currently Z-buffering is not implemented by this renderer.
        @param stencil The stencil buffer depth to be set. Currently Z-buffering is not implemented by this renderer.
        @param samples The level of antialiasing. Currently Z-buffering is not implemented by this renderer.

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