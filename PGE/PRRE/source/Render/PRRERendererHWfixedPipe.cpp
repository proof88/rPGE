/*
    ###################################################################################
    PRRERendererHWfixedPipe.cpp
    This file is part of PRRE.
    Fixed-Pipeline HW Renderer class.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "PRREbaseIncludes.h"  // PCH
#include "../../include/external/Render/PRRERendererHWfixedPipe.h"
#include <cassert>
#include "../../include/external/Math/PRREVector.h"
#include "../../include/external/Math/PRRETransformMatrix.h"
#include "../../include/external/Object3D/PRREObject3DManager.h"
#include "../../include/internal/PRREGLsafeFuncs.h"
#include "../../include/internal/PRREGLsnippets.h"
#include "../../include/internal/PRREGLextensionFuncs.h"
#include "../../include/internal/PRREpragmas.h"

using namespace std;

/*
   PRRERendererHWfixedPipeImpl
   ###########################################################################
*/

// TODO: this should also implement PRREIIncrementalRenderPipeline
class PRRERendererHWfixedPipeImpl :
    public PRRERendererHWfixedPipe
{
public:
    static TPRREbool OQ_AUTO_UPDATE_OCCLUDER_STATES;
    static TPRREbool OQ_ZPASS_FOR_OCCLUDERS;

    // ---------------------------------------------------------------------------

    virtual ~PRRERendererHWfixedPipeImpl();       /**< Calls shutdown(). */

    TPRREuint initialize(
        TPRREuint width, TPRREuint height,
        TPRRE_DISPLAY_MODES dmode,
        TPRREuint freq, TPRREint cdepth,
        TPRREint zdepth, TPRREint stencil,
        TPRREint samples,
        HWND window = NULL);         
    TPRREbool shutdown();            
    TPRREbool isInitialized() const;    

    void SetManagers(
        PRREObject3DManager* _objmgr,
        PRREuiManager* _uimgr,
        PRRECamera* _cam );

    void RenderScene();  

    const TPRRE_RENDER_HINT& getRenderHints();
    void  SetRenderHints(const TPRRE_RENDER_HINT& hints);

    void ResetStatistics();
    
    void WriteStats() const;       
    void CheckConsistency() const;  

private:
    TPRRE_RENDER_HINT    renderHints;  /**< Render hints. */
    PRREWindow&          wnd;          /**< Our window, where we draw to, singleton. */
    PRREhwInfo&          hwInfo;       /**< Hardware infos, singleton. */
    PRREScreen&          screen;       /**< Our screen, singleton. */
    PRREObject3DManager* pObject3DMgr; /**< ObjectManager, at least 1 instance. */
    PRRECamera*          pCamera;      /**< Camera, at least 1 instance. */
    PRREuiManager*       pUImgr;       /**< UI manager, singleton. */

    HGLRC                rc;           /**< Rendering context, initially NULL. Determines whether the renderer is initialized or not. */
    GLfloat mat4x4Identity[4][4];

    TPRREuint nObjectsTotal;
    TPRREuint nObjectVisible;
    TPRREuint nObjectsOccluder;
    TPRREuint nObjectsOcclusionTested;
    TPRREuint nObjectsOccluded;
    TPRREuint nObjectsNonOccluded;

    // ---------------------------------------------------------------------------

    PRRERendererHWfixedPipeImpl();                /**< NULLs members only. */
    PRRERendererHWfixedPipeImpl(
        PRREWindow& _wnd,
        PRREScreen& _scr,
        PRREhwInfo& _hwinfo );
    PRRERendererHWfixedPipeImpl(const PRRERendererHWfixedPipeImpl&);
    PRRERendererHWfixedPipeImpl& operator=(const PRRERendererHWfixedPipeImpl&);

    void myLookAt(const PRREVector& pos,
        const PRREVector& target,
        const PRREVector& up) const;

    void printOGLerrorBrief();           /**< Writes OpenGL error to console only if there is really an error. */
    void printOGLerrorFull();            /**< Writes OpenGL error to console even the error is no error. */

    TPRREbool initializeOpenGL(HDC dc);  /**< Initializes OpenGL. */
    TPRREbool shutdownOpenGL();          /**< Shuts down OpenGL. */
    void      SetBasicThingsInOpenGL();  /**< Sets some basic things in OpenGL. */

    void RenderObject(PRREObject3D& object);

    void BeginRendering();                                               /**< Sets viewport size and clears buffers. */
    void SwitchToPerspectiveProjection();                                /**< Sets perspective projection. */
    void SwitchToOrtographicProjection();                                /**< Sets orthographic projection. */
    void OrderObjectContainersByZdistance();                             /**< Orders objects by Z-distance relative to camera view. */
    void Draw3DObjects_Legacy(PRREIRenderer& renderer);                  /**< Draws 3D objects, the legacy PR00FPS way. */
    void Draw3DObjects_Ordered_Containers(PRREIRenderer& renderer);      /**< Draws 3D objects using separate Object3D containers. */
    void Draw3DObjects_OcclusionQuery(
        PRREIRenderer& renderer,
        TPRREbool bASyncQuery);                                          /**< Draws 3D objects with occlusion query. */
    void Draw2DObjects(PRREIRenderer& renderer);                         /**< Draws 2D objects. */
    void FinishRendering();                /**< Forces pending tasks to be finished and displays the rendered picture. */

    friend class PRRERendererHWfixedPipe;

}; // PRRERendererHWfixedPipeImpl


// ############################### PUBLIC ################################

TPRREbool PRRERendererHWfixedPipeImpl::OQ_AUTO_UPDATE_OCCLUDER_STATES = true;
TPRREbool PRRERendererHWfixedPipeImpl::OQ_ZPASS_FOR_OCCLUDERS = false;


/**
    Calls shutdown().
*/
PRRERendererHWfixedPipeImpl::~PRRERendererHWfixedPipeImpl()
{
    shutdown();
} // ~PRRERendererHWfixedPipeImpl()


TPRREuint PRRERendererHWfixedPipeImpl::initialize(
    TPRREuint width, TPRREuint height,
    TPRRE_DISPLAY_MODES dmode, TPRREuint freq,
    TPRREint cdepth, TPRREint zdepth, TPRREint stencil,
    TPRREint samples,
    HWND window)
{
    TPRREint st = stencil;

    getConsole().OLn("PRRERendererHWfixedPipe::initialize(%d, %d, dmode, %d, %d, %d, %d, %d)",
                      width, height, freq, cdepth, zdepth, st, samples);
    if ( isInitialized() )
    {
        getConsole().SOLn("  > already initialized!");
        return 1;
    }

    getConsole().OI();

    // we accept only 0 or 8 for stencil bits
    if ( st != 0 )
    {
        st = 8;
        getConsole().OLn("WARNING: forcing non-0 stencil to %d !", st);
    }

    TPRREbool bRequestAdvancedPixelFormat = false;  // will be set to true in the loop AFTER 1st GL init if antialiasing is requested AND if advanced formats available
    TPRREbool bLoopNeeded = true;  // will be set to false in the loop AFTER 1st GL init if antialiasing is NOT requested OR advanced formats are NOT available

    /*
        Initialize OpenGL once or twice.
        2nd init is needed if advanced pixel format is needed, eg. antialiasing is requested. That's the reason of the loop here.
        This comes from the nature of OpenGL: it must be initialized to query available advanced pixel formats, then it may be reinitialized again with an
        advanced pixel format, thus a renderer must handle screen, window and hwinfo instances init during its init.
    */
    while ( bLoopNeeded )
    {
        getConsole().OLn("");
        getConsole().OLn("#### STAGE 1/3 ####");

            getConsole().OLn("screen settings ...");
            screen.SetResolution(width, height);
            screen.SetFullScreened( dmode != PRRE_WINDOWED );  // == PRRE_FULLSCREEN nem j�, mert lesz 3. FS m�d is ...
            screen.SetFreq( freq );
            screen.SetColorBits( cdepth );
            screen.SetDepthBits( zdepth );
            screen.SetStencilBits( st );
            screen.SetFSAAlevel( samples );
      
            getConsole().OLn("new PRREWindow...");
            /* 
                A window may be created only once, even if advanced pixel format is requested, since a DC is required to be created only once.
		        This would mean that window instance can be created before renderer initialization. But based on SetPixelFormat() documentation, a
		        pixel format shouldn't be set to a window multiple times. This means that we actually have to recreate a window if an advanced pixel
		        format is to be set!
            */
            if ( !wnd.initialize(screen.getResWidth(), screen.getResHeight(), dmode != PRRE_WINDOWED, "Window", window) )
            {
                getConsole().EOLnOO("ERROR: wnd.initialize() failed!");
                shutdown();
                return 1;
            }
            getConsole().SOLn("done!");

            if ( !screen.applyDisplaySettings(wnd.getWndDC(), bRequestAdvancedPixelFormat ? PRRE_SCREEN_PF_ADVANCED : PRRE_SCREEN_PF_SIMPLE) )
            {
                getConsole().EOLnOO("ERROR: applyDisplaySettings() failed!");
                shutdown();
                return 1;
            }

        getConsole().OLn("");
        getConsole().OLn("#### STAGE 2/3 ####");
            if ( !initializeOpenGL(wnd.getWndDC()) )
            {
                getConsole().OO();
                shutdown();
                return 1;
            }

            hwInfo.Initialize(
                rc, wnd.getWndDC(),
                screen.getResWidth(), screen.getResHeight(),
                screen.getColorBits(), screen.getDepthBits(),
                screen.getStencilBits(), screen.getFSAAlevel() );

            if ( hwInfo.getVideo().isAdvancedPixelFormatSetSupported() )
            {
                if ( (samples > 0) && !bRequestAdvancedPixelFormat )
                {
                    getConsole().OLn("Advanced pixel format requested, trying ...");
                    // following call can be removed after screen.applyDisplaySettings() is modified to automatically
                    // query advanced pixel formats at first call ... see PGE-History.txt's relevant RFR task!
                    screen.applyDisplaySettings( wnd.getWndDC() );
                    bRequestAdvancedPixelFormat = true;
                    shutdown();
                }
                else
                {
                    bLoopNeeded = false;
                    getConsole().OLn("Could set advanced pixel format but not requested or already set.");
                }
            }
            else
            {
                getConsole().OLn("Advanced pixel formats not available.");
                bLoopNeeded = false;
            }
    } // while bLoopNeeded

    getConsole().OLn("");
    getConsole().OLn("#### STAGE 3/3 ####");
    getConsole().OLn("");

        printOGLerrorFull();
        SetBasicThingsInOpenGL();
        printOGLerrorFull();

        // GFX card drivers' default setting in 2015: off (if undefined by application), so we also set it to false initially
        screen.SetVSyncEnabled(false);

    getConsole().OLn("");
    getConsole().SOLn("> Fixed Pipeline HW Renderer initialized");
    getConsole().OLnOO("");

    return 0;

} // initialize()


TPRREbool PRRERendererHWfixedPipeImpl::shutdown()
{
    getConsole().OLn("PRRERendererHWfixedPipe::shutdown()");
    
    getConsole().OIOLn("Shutting down renderer ...");
    getConsole().OI();

    if ( !shutdownOpenGL() )
    {
        getConsole().EOLn("WARNING: shutdownOpenGL() failed!");
    }

    // only window, scren, hwinfo needs to be deinitialized,
    // other managers set by SetManagers() are taken care by who specified it at the beginning.
    wnd.Deinitialize();
    screen.ResetDisplaySettings();
    hwInfo.Deinitialize();

    getConsole().OO();

    getConsole().OLnOO("Renderer is shut down!");

    return true;
} // shutdown()


TPRREbool PRRERendererHWfixedPipeImpl::isInitialized() const
{
    return ( rc != NULL );
} // isInitialized()


void PRRERendererHWfixedPipeImpl::SetManagers(
    PRREObject3DManager* _objmgr,
    PRREuiManager* _uimgr,
    PRRECamera* _cam )
{
    pObject3DMgr = _objmgr;
    pUImgr = _uimgr;
    pCamera = _cam;
}


void PRRERendererHWfixedPipeImpl::RenderObject(PRREObject3D& object)
{
    // TODO
}


void PRRERendererHWfixedPipeImpl::RenderScene()
{
    if ( !isInitialized() )
    {
        return;
    }

    BeginRendering();
                                                                                                
    SwitchToPerspectiveProjection();
    const TPRREuint iRenderPath = BITF_READ(renderHints, PRRE_RH_RENDER_PATH_BITS, 3);
    switch (iRenderPath)
    {
    case PRRE_RH_RP_LEGACY_PR00FPS:
        Draw3DObjects_Legacy(*this);
        break;
    case PRRE_RH_RP_DISTANCE_ORDERED:
        Draw3DObjects_Ordered_Containers(*this);
        break;
    case PRRE_RH_RP_OCCLUSION_CULLED:
        {
            const TPRREuint iOcclusionQueryMethod = BITF_READ(renderHints, PRRE_RH_OQ_METHOD_BITS, 2);
            switch (iOcclusionQueryMethod)
            {
            case PRRE_RH_OQ_METHOD_SYNC:
                Draw3DObjects_OcclusionQuery(*this, false);
                break;
            case PRRE_RH_OQ_METHOD_ASYNC:
                Draw3DObjects_OcclusionQuery(*this, true);
                break;
            default:
                // later more OQ methods will come ...
                getConsole().EOLn("%s() ERROR: unhandled iOcclusionQueryMethod!", __FUNCTION__);
                assert(false);
            } // switch iOcclusionQueryMethod
        }
        break;
    default:
        // later more renderpaths will come ...
        getConsole().EOLn("%s() ERROR: unhandled iRenderPath!", __FUNCTION__);
        assert(false);
    } // switch iRenderPath

    SwitchToOrtographicProjection();
    Draw2DObjects(*this);

    FinishRendering();

    // we don't really check for GL errors in above functions, so check them here
    printOGLerrorBrief();

} // RenderScene()


const TPRRE_RENDER_HINT& PRRERendererHWfixedPipeImpl::getRenderHints()
{
    return renderHints;
} // getRenderHints()


void PRRERendererHWfixedPipeImpl::SetRenderHints(const TPRRE_RENDER_HINT& hints)
{
    const TPRREuint prevRenderPath = BITF_READ(renderHints, PRRE_RH_RENDER_PATH_BITS, 3);
    const TPRREuint newRenderPath = BITF_READ(hints, PRRE_RH_RENDER_PATH_BITS, 3);

    const TPRREuint prevOcclusionQueryMethod = BITF_READ(renderHints, PRRE_RH_OQ_METHOD_BITS, 2);
    const TPRREuint newOcclusionQueryMethod = BITF_READ(hints, PRRE_RH_OQ_METHOD_BITS, 2);

    const TPRREbool bPrevOcclusionDrawBoundingBoxes = ( BIT_READ(renderHints, PRRE_RH_OQ_DRAW_BOUNDING_BOXES_BIT) == 1u );
    const TPRREbool bNewOcclusionDrawBoundingBoxes = ( BIT_READ(hints, PRRE_RH_OQ_DRAW_BOUNDING_BOXES_BIT) == 1u );

    const TPRREbool bPrevOcclusionDrawIfQueryPending = ( BIT_READ(renderHints, PRRE_RH_OQ_DRAW_IF_QUERY_PENDING_BIT) == 1u );
    const TPRREbool bNewOcclusionDrawIfQueryPending = ( BIT_READ(hints, PRRE_RH_OQ_DRAW_IF_QUERY_PENDING_BIT) == 1u );

    const TPRREbool bPrevOrderByDistance = ( BIT_READ(renderHints, PRRE_RH_ORDERING_BY_DISTANCE_BIT) == 1u );
    const TPRREbool bNewOrderByDistance = ( BIT_READ(hints, PRRE_RH_ORDERING_BY_DISTANCE_BIT) == 1u );

    getConsole().OLn("PRRERendererHWfixedPipe::SetRenderHints(%u)", hints);
    getConsole().OLn("  prev renderHints: %u", renderHints);
    if ( prevRenderPath != newRenderPath )
    {
        std::string sPrevRenderPath;
        switch (prevRenderPath)
        {
        case 0: sPrevRenderPath = "Legacy PR00FPS";
            break;
        case 1: sPrevRenderPath = "Distance Ordered";
            break;
        default: sPrevRenderPath = "Occlusion Culled";
        }

        std::string sNewRenderPath;
        switch (newRenderPath)
        {
        case 0: sNewRenderPath = "Legacy PR00FPS";
            break;
        case 1: sNewRenderPath = "Distance Ordered";
            break;
        default: sNewRenderPath = "Occlusion Culled";
        }

        getConsole().OLn("  prevRenderPath: %u - %s", prevRenderPath, sPrevRenderPath.c_str());
        getConsole().OLn("  newRenderPath : %u - %s", newRenderPath, sNewRenderPath.c_str());
    }

    if ( prevOcclusionQueryMethod != newOcclusionQueryMethod )
    {
        std::string sPrevOcMethod;
        switch (prevOcclusionQueryMethod)
        {
        case 0: sPrevOcMethod = "Sync";
            break;
        default: sPrevOcMethod = "ASync";
        }

        std::string sNewOcMethod;
        switch (newOcclusionQueryMethod)
        {
        case 0: sNewOcMethod = "Sync";
            break;
        default: sNewOcMethod = "ASync";
        }

        getConsole().OLn("  prevOcclusionQueryMethod: %u - %s", prevOcclusionQueryMethod, sPrevOcMethod.c_str());
        getConsole().OLn("  newOcclusionQueryMethod : %u - %s", newOcclusionQueryMethod, sNewOcMethod.c_str());
    }

    if ( bPrevOcclusionDrawBoundingBoxes != bNewOcclusionDrawBoundingBoxes )
    {
        getConsole().OLn("  bPrevOcclusionDrawBoundingBoxes: %b", bPrevOcclusionDrawBoundingBoxes);
        getConsole().OLn("  bNewOcclusionDrawBoundingBoxes : %b", bNewOcclusionDrawBoundingBoxes);
    }

    if ( bPrevOcclusionDrawIfQueryPending != bNewOcclusionDrawIfQueryPending )
    {
        getConsole().OLn("  bPrevOcclusionDrawIfQueryPending: %b", bPrevOcclusionDrawIfQueryPending);
        getConsole().OLn("  bNewOcclusionDrawIfQueryPending : %b", bNewOcclusionDrawIfQueryPending);
    }

    if ( bPrevOrderByDistance != bNewOrderByDistance )
    {
        getConsole().OLn("  bPrevOrderByDistance: %b", bPrevOrderByDistance);
        getConsole().OLn("  bNewOrderByDistance : %b", bNewOrderByDistance);
    }

    if ( ( (prevRenderPath == PRRE_RH_RP_OCCLUSION_CULLED) && (newRenderPath != PRRE_RH_RP_OCCLUSION_CULLED) )
        ||
        ( (prevOcclusionQueryMethod == PRRE_RH_OQ_METHOD_ASYNC) && (newOcclusionQueryMethod != PRRE_RH_OQ_METHOD_ASYNC) ) )
    {
        // we are switching either render path or occlusion query method when there still might be pending queries,
        // so let's wait them to finish, otherwise some assert() may fail and/or some states might be inconsistent at next rendering!
        getConsole().OLn("");
        getConsole().O("  Force-finishing ongoing occlusion queries ... ");
        for (int i = 0; i < pObject3DMgr->getSize(); i++)
        {
            PRREObject3D* const obj = (PRREObject3D*) pObject3DMgr->getAttachedAt(i);
        
            if ( obj == PGENULL )
                continue;

            obj->ForceFinishOcclusionTest();
        }
        getConsole().OLn("done!");
    }
    getConsole().OLn("");

    renderHints = hints;
} // SetRenderHints()


void PRRERendererHWfixedPipeImpl::ResetStatistics()
{
    getConsole().OLn("PRRERendererHWfixedPipe::ResetStatistics()");
    pObject3DMgr->ResetStatistics();    
} // ResetStatistics()


void PRRERendererHWfixedPipeImpl::WriteStats() const
{
    getConsole().OLn("PRRERendererHWfixedPipe::WriteStats()");
    getConsole().L();

    const TPRREuint iRenderPath = BITF_READ(renderHints, PRRE_RH_RENDER_PATH_BITS, 3);
    const TPRREuint iOcclusionQueryMethod = BITF_READ(renderHints, PRRE_RH_OQ_METHOD_BITS, 2);
    const TPRREbool bOcclusionDrawBoundingBoxes = ( BIT_READ(renderHints, PRRE_RH_OQ_DRAW_BOUNDING_BOXES_BIT) == 1u );
    const TPRREbool bOcclusionDrawIfQueryPending = ( BIT_READ(renderHints, PRRE_RH_OQ_DRAW_IF_QUERY_PENDING_BIT) == 1u );
    const TPRREbool bOrderByDistance = ( BIT_READ(renderHints, PRRE_RH_ORDERING_BY_DISTANCE_BIT) == 1u );

    getConsole().OI();

    getConsole().OLn("iRenderPath: %u; iOcclusionQueryMethod: %u", iRenderPath, iOcclusionQueryMethod);
    getConsole().OLn("bOcclusionDrawBoundingBoxes: %b", bOcclusionDrawBoundingBoxes);
    getConsole().OLn("bOcclusionDrawIfQueryPending: %b", bOcclusionDrawIfQueryPending);
    getConsole().OLn("bOrderByDistance: %b", bOrderByDistance);
    getConsole().OLn("");

    getConsole().OO();
    
} // WriteStats()


/**
    Runs an internal consistency test, just for debug.
*/
void PRRERendererHWfixedPipeImpl::CheckConsistency() const
{
    getConsole().OLn("PRRERendererHWfixedPipe::CheckConsistency()");

    getConsole().OLn("END of consistency check!");
} // CheckConsistency()


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


/**
    NULLs members only.
*/                                                                         
PRRERendererHWfixedPipeImpl::PRRERendererHWfixedPipeImpl() :
    renderHints(DefaultHints),
    wnd( wnd ),
    hwInfo( hwInfo ),
    screen( screen ),
    pObject3DMgr( NULL ),
    pCamera( NULL ),
    pUImgr( NULL )
{
    rc = NULL;
} // PRRERendererHWfixedPipeImpl(...)


PRRERendererHWfixedPipeImpl::PRRERendererHWfixedPipeImpl(
    PRREWindow& _wnd,
    PRREScreen& _scr,
    PRREhwInfo& _hwinfo ) :
    renderHints(DefaultHints),
    wnd( _wnd ),
    hwInfo( _hwinfo ),
    screen( _scr ),
    pObject3DMgr( NULL ),
    pCamera( NULL ),
    pUImgr( NULL )
{
    rc = NULL;
} // PRRERendererHWfixedPipeImpl(...)


PRRERendererHWfixedPipeImpl::PRRERendererHWfixedPipeImpl(const PRRERendererHWfixedPipeImpl&) :
    renderHints(DefaultHints),
    wnd( PRREWindow::createAndGet() ),
    hwInfo( PRREhwInfo::get() ),
    screen( PRREScreen::createAndGet() ),
    pUImgr( NULL ),
    pObject3DMgr( NULL ),
    pCamera( NULL )
{
}


PRRERendererHWfixedPipeImpl& PRRERendererHWfixedPipeImpl::operator=(const PRRERendererHWfixedPipeImpl&)
{
    return *this;
}


/**
    Writes OpenGL error to console only if there is really an error.
*/
void PRRERendererHWfixedPipeImpl::printOGLerrorBrief()
{
    while ( PRREGLsnippets::isGLErrorPresent() )
        getConsole().EOLn("Last error: %s",
                           PRREGLsnippets::getGLErrorTextFromEnum( PRREGLsnippets::getLastSavedGLError() ));   
} // printOGLerrorBrief()


/**
    Writes OpenGL error to console even the error is no error.
*/
void PRRERendererHWfixedPipeImpl::printOGLerrorFull()
{
    if ( PRREGLsnippets::isGLErrorPresent() )
        getConsole().EOLn("Last error: %s",
                           PRREGLsnippets::getGLErrorTextFromEnum( PRREGLsnippets::getLastSavedGLError() ));
    else
        getConsole().SOLn("Last error: %s",
                           PRREGLsnippets::getGLErrorTextFromEnum( PRREGLsnippets::getLastSavedGLError() ));
} // printOGLerrorFull()


/**
    Initializes OpenGL.
    Updates member rc.
*/
TPRREbool PRRERendererHWfixedPipeImpl::initializeOpenGL(HDC dc)
{
    getConsole().OLn("initializing OpenGL subsystem ...");

    GLint row, col;
    for (row = 0; row < 4; row++)
        for (col = 0; col < 4 ; col++)
            mat4x4Identity [row][col] = (row == col);

    if ( !( rc = wglCreateContext(dc) ) )
    {
        getConsole().EOLn("ERROR: wglCreateContext() failed!");
        return false;
    }
    getConsole().SOLn("wglCreateContext() passed");

    if ( !wglMakeCurrent(dc, rc) )
    {
        getConsole().EOLn("ERROR: wglMakeCurrent() failed!");
        shutdownOpenGL();
        return false;
    }
    getConsole().SOLn("wglMakeCurrent() passed");

    printOGLerrorFull();
 
    getConsole().SOLn("done!");
    return true;
} // initializeOpenGL()


/**
    Shuts down OpenGL.
    Updates member rc.
*/
TPRREbool PRRERendererHWfixedPipeImpl::shutdownOpenGL()
{
    if ( !rc )
        return true;

    getConsole().OLnOI("Shutting down OpenGL ...");
    if ( !wglMakeCurrent(NULL, NULL) )
    {
        getConsole().EOLnOO("ERROR: wglMakeCurrent(NULL, NULL) failed!");
        return false;
    }
    getConsole().SOLn("wglMakeCurrent(NULL, NULL) okay.");

    bool b;
    if ( b = (wglDeleteContext(rc) == TRUE) )
        getConsole().SOLn("wglDeleteContext(rc) okay."); 
    else
        getConsole().EOLn("ERROR: wglDeleteContext(rc) failed.");
        
    rc = NULL;
    getConsole().OO();
    return b;
} // shutdownOpenGL()


/**
    Sets some basic things in OpenGL.
*/
void PRRERendererHWfixedPipeImpl::SetBasicThingsInOpenGL()
{
    glEnable(GL_DEPTH_TEST);  // some objects like sticked objects or 2-sided AND wireframed objects may temporarily modify this
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);  // permanent setting, don't change in middle of rendering to get use of depth buffer optimisations
    glDepthRange(0, 1);    // permanent
    glCullFace(GL_BACK);      // 2-sided objects may temporarily modify this
    glFrontFace(GL_CCW);      // usually this is true, however some negative-scaled objects may temporarily modify this
    glEnable(GL_SCISSOR_TEST);
    pglHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // permanent setting
    glShadeModel(GL_SMOOTH);
    //glShadeModel(GL_FLAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // instead of the default 4-byte boundary, we use 1 to support textures like 2x2
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat white[4] = {1.0f, 1.0f, 1.0f, 1.0};
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 127);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glFlush();
    glFinish();
} // SetBasicThingsInOpenGL()


/**
    Sets viewport size and clears buffers.
*/
void PRRERendererHWfixedPipeImpl::BeginRendering()
{
    const GLint vpX = (GLint) pCamera->getViewport().pos.x;
    const GLint vpY = (GLint) ( wnd.getClientHeight() - (pCamera->getViewport().size.height + pCamera->getViewport().pos.y) );
    const GLint vpW = (GLint) pCamera->getViewport().size.width;
    const GLint vpH = (GLint) pCamera->getViewport().size.height;
    
    glViewport(vpX, vpY, vpW, vpH);
    glScissor(vpX, vpY, vpW, vpH);

    /*
        "A common error when using glDepthMask(GL_FALSE) is that you forget to enable it again before the next glClear;
         if it's still disabled, you will have last frame's depth information still in the buffer."
        https://www.reddit.com/r/opengl/comments/66bnsn/strange_behavior_with_depth_test_and_mask/
    */
    glDepthMask(GL_TRUE);

    switch (pCamera->getClearMode())
    {
    case PRRE_CLEAR_ZBUFFER_COLORBUFFER:
        glClearColor(pCamera->getBackgroundColor().getRedAsFloat(),
            pCamera->getBackgroundColor().getGreenAsFloat(),
            pCamera->getBackgroundColor().getBlueAsFloat(),
            pCamera->getBackgroundColor().getAlphaAsFloat() );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        break;
    case PRRE_CLEAR_ZBUFFER:
        glClear(GL_DEPTH_BUFFER_BIT);
        break;
    }
} // BeginRendering()


/**
    Sets perspective projection.
*/
void PRRERendererHWfixedPipeImpl::SwitchToPerspectiveProjection()
{
    PRRETransformMatrix mProj;
    mProj.SetFrustumByFovY(pCamera->getFieldOfView(), pCamera->getAspectRatio(), pCamera->getNearPlane(), pCamera->getFarPlane());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(mProj.getTransposed());

    /*switch (pCamera->getAspectRatioMode())
    {
    case PRRE_AM_FIX:     gluPerspective(pCamera->getFieldOfView(),
                                         pCamera->getAspectRatio(),
                                         pCamera->getNearPlane(), pCamera->getFarPlane());
                          break;
    case PRRE_AM_DYNAMIC: gluPerspective(pCamera->getFieldOfView(),
                                         pCamera->getViewport().size.width / pCamera->getViewport().size.height,
                                         pCamera->getNearPlane(), pCamera->getFarPlane());
                          break;
    } */

    
    PRRETransformMatrix mView;
    mView.SetLookAt(pCamera->getPosVec(), pCamera->getTargetVec(), pCamera->getUpVec());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mView.getTransposed());
} // SwitchToPerspectiveProjection()


/**
    Sets orthographic projection.
*/
void PRRERendererHWfixedPipeImpl::SwitchToOrtographicProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(*mat4x4Identity);
    //glLoadIdentity(); 
    glOrtho(
        0, pCamera->getViewport().size.width,
        0, pCamera->getViewport().size.height,
       -1, 1);
    glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    glLoadMatrixf(*mat4x4Identity);
    // initial translation so 2D objects will be placed correctly on screen
    glTranslatef(pCamera->getViewport().size.width/2.f, pCamera->getViewport().size.height/2.f, 0.f);
} // SwitchToOrtographicProjection()


struct CompareByZdistance
{
    enum Order
    {
        NEAREST_TO_FAREST,
        FAREST_TO_NEAREST
    };
    
    PRREVector vPosCam;
    PRREVector vFwdCam;
    Order order;

    // std::sort() uses this to determine if a is less than b
    bool operator()(PRREObject3D* a, PRREObject3D* b) const
    {
        const PRREVector vDistanceToCam_a = a->getPosVec() - vPosCam;
        const PRREVector vDistanceToCam_b = b->getPosVec() - vPosCam;
        const TPRREfloat fZdistanceToCam_a = vDistanceToCam_a.getDotProduct(vFwdCam);
        const TPRREfloat fZdistanceToCam_b = vDistanceToCam_b.getDotProduct(vFwdCam);

        return (order == NEAREST_TO_FAREST) ? (fZdistanceToCam_a < fZdistanceToCam_b) : (fZdistanceToCam_a > fZdistanceToCam_b);
    }
}; // struct CompareByZdistance


/**
    Orders objects by Z-distance relative to camera view.
    No effect if PRRE_RH_ORDERING_BY_DISTANCE_BIT is not set.
*/
void PRRERendererHWfixedPipeImpl::OrderObjectContainersByZdistance()
{
    if ( BIT_READ(renderHints, PRRE_RH_ORDERING_BY_DISTANCE_BIT) == 1u )
    {
        static CompareByZdistance comparator;
        comparator.vPosCam = pCamera->getPosVec();
        comparator.vFwdCam = pCamera->getTargetVec();

        // this would be less complicated with C++11 lambda ...
        // std::sort() sorts in non-descending order, if comparator also returns true if element 'a' is less than element 'b',
        // so in our case it is nearest to farest ordering
        comparator.order = CompareByZdistance::Order::NEAREST_TO_FAREST;
        std::sort(pObject3DMgr->getOccluders().begin(), pObject3DMgr->getOccluders().end(), comparator);
        std::sort(pObject3DMgr->get3dOpaqueOccludees().begin(), pObject3DMgr->get3dOpaqueOccludees().end(), comparator);

        comparator.order = CompareByZdistance::Order::FAREST_TO_NEAREST;
        std::sort(pObject3DMgr->get3dBlendedOccludees().begin(), pObject3DMgr->get3dBlendedOccludees().end(), comparator);
    }
} // OrderObjectContainersByZdistance()


/**
    Draws 3D objects, the legacy PR00FPS way.
*/
void PRRERendererHWfixedPipeImpl::Draw3DObjects_Legacy(PRREIRenderer& renderer)
{    
    bool blended = true;
    for (int iBlend = 1; iBlend < 3; iBlend++)
    {
        blended = !blended;

        for (int i = 0; i < pObject3DMgr->getSize(); i++)
        {
            PRREObject3D* const obj = (PRREObject3D*) pObject3DMgr->getAttachedAt(i);
        
            if ( obj == PGENULL )
                continue;
        
            if ( (blended == PRREMaterial::isBlendFuncReallyBlending(obj->getMaterial(false).getSourceBlendFunc(), obj->getMaterial(false).getDestinationBlendFunc()))
                 &&
                 ( obj->isVisible() )
                 &&
                 ( !obj->isStickedToScreen() )
               )
            {
                glPushMatrix();
                obj->Draw(PRRE_RPASS_NORMAL, false, false);
                glPopMatrix();
            }
        } // for i    
    } // for iBlend
    
} // Draw3DObjects_Legacy


/**
    Draws 3D objects using separate Object3D containers.
    The difference compared to Draw3DObjects_Legacy():
     - iterating separate Object3D containers instead of Object3DManager's base manageds container;
     - these separate containers are ordered by Z-distance from camera, so hopefully it makes less overdraw.
*/
void PRRERendererHWfixedPipeImpl::Draw3DObjects_Ordered_Containers(PRREIRenderer& renderer)
{    
    static TPRREuint frameCntr = 0;

    OrderObjectContainersByZdistance();

    for (auto it = pObject3DMgr->getOccluders().begin(); it != pObject3DMgr->getOccluders().end(); it++)
    {
        glPushMatrix();
        (*it)->Draw(PRRE_RPASS_NORMAL, false, false);
        glPopMatrix();
    }

    for (auto it = pObject3DMgr->get3dOpaqueOccludees().begin(); it != pObject3DMgr->get3dOpaqueOccludees().end(); it++)
    {
        glPushMatrix();
        (*it)->Draw(PRRE_RPASS_NORMAL, false, false);
        glPopMatrix();
    }

    for (auto it = pObject3DMgr->get3dBlendedOccludees().begin(); it != pObject3DMgr->get3dBlendedOccludees().end(); it++)
    {
        glPushMatrix();
        (*it)->Draw(PRRE_RPASS_NORMAL, false, false);
        glPopMatrix();
    }

    frameCntr++;

    if ( OQ_AUTO_UPDATE_OCCLUDER_STATES )
    {
        if ( (frameCntr % 100) == 0 )
        {
           pObject3DMgr->UpdateOccluderStates();
        }
    }   
} // Draw3DObjects_Ordered_Containers


/**
    Draws 3D objects with occlusion query.
    The difference compared to Draw3DObjects_Ordered_Containers():
     - occlusion query is started for occludees, to decide if they should be really rendered.

    @param bASyncQuery If true, the result of occlusion queries are not waited for in the same frame, instead they will be
                       evaulated in a future frame whenever they are available.
                       If false, all occlusion queries must finish in the frame, to decide if we should really render the occludee.
*/
void PRRERendererHWfixedPipeImpl::Draw3DObjects_OcclusionQuery(PRREIRenderer& renderer, TPRREbool bASyncQuery)
{
    static TPRREuint frameCntr = 0;

    const TPRREbool bRenderIfQueryPending = ( BIT_READ(renderHints, PRRE_RH_OQ_DRAW_IF_QUERY_PENDING_BIT) == 1u );

    OrderObjectContainersByZdistance();

    if ( OQ_ZPASS_FOR_OCCLUDERS )
    {
        // first render the occluders into Z-buffer only
        PRREGLsnippets::SetZPassRendering(true);

        for (auto it = pObject3DMgr->getOccluders().begin(); it != pObject3DMgr->getOccluders().end(); it++)
        {
            glPushMatrix();
            (*it)->Draw(PRRE_RPASS_Z_ONLY, bASyncQuery, bRenderIfQueryPending);
            glPopMatrix();
        }

        PRREGLsnippets::SetZPassRendering(false);
    } // zpass for occluders

    // first render the occluders normally
    for (auto it = pObject3DMgr->getOccluders().begin(); it != pObject3DMgr->getOccluders().end(); it++)
    {
        glPushMatrix();
        (*it)->Draw(PRRE_RPASS_NORMAL, bASyncQuery, bRenderIfQueryPending);
        glPopMatrix();
    }

    // then render the occludees with occlusion query
    for (/*TPRRE_RENDER_PASS*/ int iRenderPass = PRRE_RPASS_START_OCCLUSION_QUERY; iRenderPass <= PRRE_RPASS_NORMAL; iRenderPass++)
    {
        PRREGLsnippets::SetGLBoundingBoxRendering(iRenderPass == PRRE_RPASS_START_OCCLUSION_QUERY);

        for (auto it = pObject3DMgr->get3dOpaqueOccludees().begin(); it != pObject3DMgr->get3dOpaqueOccludees().end(); it++)
        {
            glPushMatrix();
            (*it)->Draw((TPRRE_RENDER_PASS)iRenderPass, bASyncQuery, bRenderIfQueryPending);
            glPopMatrix();
        }

        for (auto it = pObject3DMgr->get3dBlendedOccludees().begin(); it != pObject3DMgr->get3dBlendedOccludees().end(); it++)
        {
            glPushMatrix();
            (*it)->Draw((TPRRE_RENDER_PASS)iRenderPass, bASyncQuery, bRenderIfQueryPending);
            glPopMatrix();
        }

    } // occludess iRenderPass

    frameCntr++;

    if ( OQ_AUTO_UPDATE_OCCLUDER_STATES )
    {
        if ( (frameCntr % 100) == 0 )
        {
           pObject3DMgr->UpdateOccluderStates();
        }
    }

    if ( (frameCntr % 1000) == 0 )
    {
        frameCntr = 0;

        nObjectsTotal = 0;
        nObjectVisible = 0;
        nObjectsOccluder = 0;
        nObjectsOcclusionTested = 0;
        nObjectsOccluded = 0;
        nObjectsNonOccluded = 0;
        // this separate statistics loop wont be needed after the above loop will be simpler: ordering of objects simplified!
        for (int i = 0; i < pObject3DMgr->getSize(); i++)
        {
            PRREObject3D* const obj = (PRREObject3D*) pObject3DMgr->getAttachedAt(i);
        
            if ( obj == PGENULL )
                continue;
        
            nObjectsTotal++;
        
            if ( !obj->isVisible() )
                continue;
        
            nObjectVisible++;

            if ( obj->isOccluder() )
                nObjectsOccluder++;
        
            if ( obj->isOcclusionTested() )
            {
                nObjectsOcclusionTested++;
                if ( obj->isOccluded() )
                {
                    nObjectsOccluded++;
                }
                else
                {
                    nObjectsNonOccluded++;
                }
            }
        } // for i    
        
        //getConsole().SetLoggingState(PRRERendererHWfixedPipe::getLoggerModuleName(), true);
        //getConsole().OLn("Number of total objects: %d", nObjectsTotal);
        //getConsole().OLn(" - of which visible: %d", nObjectVisible);
        //getConsole().OLn("Number of occluders: %d",nObjectsOccluder );
        //getConsole().OLn("Number of occlusion-tested objects: %d", nObjectsOcclusionTested);
        //getConsole().OLn(" - of which occluded: %d", nObjectsOccluded);
        //getConsole().OLn(" - of which not occluded: %d", nObjectsNonOccluded);
        //getConsole().OLn("");

        //pObject3DMgr->getConsole().SetLoggingState(PRREObject3DManager::getLoggerModuleName(), true);
        //pObject3DMgr->WriteList();
        //pObject3DMgr->getConsole().SetLoggingState(PRREObject3DManager::getLoggerModuleName(), false);
        
        getConsole().OLn("");
        getConsole().SetLoggingState(PRRERendererHWfixedPipe::getLoggerModuleName(), false);
    } // frameCntr

    if ( BIT_READ(renderHints, PRRE_RH_OQ_DRAW_BOUNDING_BOXES_BIT) == 1u )
    {
        PRREGLsnippets::glPrepareBeforeDrawBoundingBox();

        for (auto it = pObject3DMgr->getOccluders().begin(); it != pObject3DMgr->getOccluders().end(); it++)
        {
            glPushMatrix();
            (*it)->Draw(PRRE_RPASS_BOUNDING_BOX_DEBUG_FOR_OCCLUSION_QUERY, bASyncQuery, bRenderIfQueryPending);
            glPopMatrix();
        }

        for (auto it = pObject3DMgr->get3dOpaqueOccludees().begin(); it != pObject3DMgr->get3dOpaqueOccludees().end(); it++)
        {
            glPushMatrix();
            (*it)->Draw(PRRE_RPASS_BOUNDING_BOX_DEBUG_FOR_OCCLUSION_QUERY, bASyncQuery, bRenderIfQueryPending);
            glPopMatrix();
        }

        for (auto it = pObject3DMgr->get3dBlendedOccludees().begin(); it != pObject3DMgr->get3dBlendedOccludees().end(); it++)
        {
            glPushMatrix();
            (*it)->Draw(PRRE_RPASS_BOUNDING_BOX_DEBUG_FOR_OCCLUSION_QUERY, bASyncQuery, bRenderIfQueryPending);
            glPopMatrix();
        }
    } // OQ_RENDER_BOUNDING_BOXES
    
} // Draw3DObjects_OcclusionQuery


/**
    Draws 2D objects (stickedToScreen and UI).
*/
void PRRERendererHWfixedPipeImpl::Draw2DObjects(PRREIRenderer& renderer)
{
    /* legacy 2D code begins */

    for (int i = 0; i < pObject3DMgr->getSize(); i++)
    {
        PRREObject3D* const obj = (PRREObject3D*) pObject3DMgr->getAttachedAt(i);
        if ( obj == PGENULL )
            continue;

        if ( ( obj->isVisible() )
                &&
             ( obj->isStickedToScreen() )
            )
        {
            glPushMatrix();
            obj->Draw(PRRE_RPASS_NORMAL, false, false);
            glPopMatrix();
        }
    } // for i

    /* legacy 2D code ends */

    glTranslatef( -pCamera->getViewport().size.width / 2.0f, -pCamera->getViewport().size.height / 2.0f, 0.0f );
    pUImgr->Render();
} // Draw2DObjects()


/**
    Forces pending tasks to be finished and displays the rendered picture.
*/
void PRRERendererHWfixedPipeImpl::FinishRendering()
{
    glFlush();
    SwapBuffers( wnd.getWndDC() );
} // FinishRendering()


/*
   PRRERendererHWfixedPipe
   ###########################################################################
*/


// ############################### PUBLIC ################################


/**
    Creates and gets the singleton instance.
*/
PRRERendererHWfixedPipe& PRRERendererHWfixedPipe::createAndGet(
    PRREWindow& _wnd,
    PRREScreen& _scr,
    PRREhwInfo& _hwinfo )
{
    static PRRERendererHWfixedPipeImpl inst(_wnd, _scr, _hwinfo);
    return inst;
} // createAndGet()


/**
    Returns access to console preset with logger module name as this class.
    Intentionally not virtual, so the getConsole() in derived class should hide this instead of overriding.

    @return Console instance used by this class.
*/
CConsole& PRRERendererHWfixedPipe::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
} // getConsole()


/**
    Returns the logger module name of this class.
    Intentionally not virtual, so derived class should hide this instead of overriding.
    Not even private, so user can also access this from outside, for any reason like controlling log filtering per logger module name.

    @return The logger module name of this class.
*/
const char* PRRERendererHWfixedPipe::getLoggerModuleName()
{
    return "PRRERendererHWfixedPipe";
} // getLoggerModuleName()


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


