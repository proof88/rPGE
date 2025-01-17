/*
    ###################################################################################
    PureObject3D.cpp
    This file is part of PURE.
    PureObject3D class.
    Made by PR00F88
    ###################################################################################
*/

#include "PureBaseIncludes.h"  // PCH
#include "../../include/external/Object3D/PureObject3DManager.h"
#include "../../include/internal/Object3D/PureObject3DImpl.h"
#include "../../include/internal/Object3D/PureVertexTransferModeImpl.h"
#include <cassert>
#include <set>
#include "../../include/external/Hardware/PureHwInfo.h"
#include "../../include/internal/PureGLextensionFuncs.h"
#include "../../include/internal/PureGLsafeFuncs.h"
#include "../../include/internal/PureGLsnippets.h"
#include "../../include/internal/PurePragmas.h"
#include "../../../../../PFL/PFL/PFL.h"

using namespace std;

/*

    Notes

    ***************************

    Vertex Culling (Object-Space)
    *****************************

    GL_EXT_cull_vertex: https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_cull_vertex.txt
     - 1996
     - "Culling a polygon by examining its vertexes in object space can be more efficient than screen space polygon culling since
       the transformation to screen space (which may include a division by w) can be avoided for culled vertexes. Also, vertex culling
       can be computed before vertexes are assembled into primitives. This is a useful property when drawing meshes with shared vertexes,
       since a vertex can be culled once, and the resulting state can be used for all primitives which share the vertex."
     - This extension was most probably introduced for optimisation before HW T&L spread. Lack of HW T&L was present in the pre-Geforce era,
       i.e. TNT, etc.
       Desktop: was mostly supported by early integrated Intel VGA.
       Mobile: no support at all!
     - Verdict: don't use it if neither 3dfx nor other major vendors supported it before TnL!

*/

/*
   PureObject3D::PureObject3DImpl
   ###########################################################################
*/


// ############################### PUBLIC ################################


TPureUInt const PureObject3D::PureObject3DImpl::OQ_MAX_FRAMES_WO_START_QUERY_WHEN_VISIBLE  = 5;
TPureUInt const PureObject3D::PureObject3DImpl::OQ_MAX_FRAMES_WO_START_QUERY_WHEN_OCCLUDED = 0;

std::vector<PureObject3D::PureObject3DImpl::CurrentStats> PureObject3D::PureObject3DImpl::stats;

std::deque<PureObject3D*> PureObject3D::PureObject3DImpl::occluders;
std::deque<PureObject3D*> PureObject3D::PureObject3DImpl::occludees_opaque;
std::deque<PureObject3D*> PureObject3D::PureObject3DImpl::occludees_blended;
std::deque<PureObject3D*> PureObject3D::PureObject3DImpl::occludees_2d_opaque;
std::deque<PureObject3D*> PureObject3D::PureObject3DImpl::occludees_2d_blended;


PureObject3D::PureObject3DImpl::CurrentStats::CurrentStats()
{
    timeLongestGlobalWaitForSyncQueryFinish.tv_sec = 0;
    timeLongestGlobalWaitForSyncQueryFinish.tv_usec = 0;
    nFramesWaitedForOcclusionTestResultGlobalMin = UINT_MAX;
    nFramesWaitedForOcclusionTestResultGlobalMax = 0;
}


PureObject3D::PureObject3DImpl::~PureObject3DImpl()
{
    _pOwner->getManagedConsole().OLnOI("~PureObject3D() ...");

    delete[] pVerticesTransf;
    delete[] pFbBuffer;
    pVerticesTransf = PGENULL;
    pFbBuffer = PGENULL;

    // Remember: we cannot use isLevel1() or isLevel2() here because isLevel2() might return true for
    // a level-1 object that was originally a cloned object but not referring to any object anymore after
    // the original object has been deleted!
    // We might check for actual vertex data because level-1 objects never have vertex data, but I think
    // in the future even level-2 objects might also lose vertex data in their client memory if it is
    // already uploaded to host memory.
    // So, this dtor must execute operations keeping mind that the same code is executed for both level-1
    // and level-2 objects!
    // On the long run, maybe we change behavior and in case of deleting an object referred to by other
    // objects, all referrers are also get deleted automatically.

    if ( pRefersto )
    {
        pRefersto->pImpl->referrers.erase(_pOwner);
    }
    else if ( referrers.size() > 0 )
    {
        // referrers now become zombie objects: they do not refer but they dont have any subobjects either!
        // user's responsibility to get rid of them if original object is removed!
        for (auto& referrer : referrers)
            referrer->pImpl->pRefersto = PGENULL;
    }

    SetOcclusionTested(false);

    // these are way unnecessary things if we are a level-2 object, but as explained above, we have to live
    // with it - at least these are all harmless code even when executed for a level-2.
    std::set< std::deque<PureObject3D*>* > mapsEraseObjectFrom;
    // would be much nicer with Cpp11 initializer list
    mapsEraseObjectFrom.insert(&occluders);
    mapsEraseObjectFrom.insert(&occludees_opaque);
    mapsEraseObjectFrom.insert(&occludees_blended);
    mapsEraseObjectFrom.insert(&occludees_2d_opaque);
    mapsEraseObjectFrom.insert(&occludees_2d_blended);

    for (auto pMapEraseFrom : mapsEraseObjectFrom )
    {
        auto occ_it = std::find(pMapEraseFrom->begin(), pMapEraseFrom->end(), _pOwner);
        while ( occ_it != pMapEraseFrom->end() )
        {
            pMapEraseFrom->erase(occ_it);
            occ_it = std::find(pMapEraseFrom->begin(), pMapEraseFrom->end(), _pOwner);
        }
    } // end for pMapEraseFrom

    _pOwner->getManagedConsole().OI();
    _pOwner->DeleteAll();
    _pOwner->getManagedConsole().OO();

    _pOwner->getManagedConsole().SOLnOO("Done!");
} // ~PureObject3DImpl()


PureObject3D* PureObject3D::PureObject3DImpl::getReferredObject() const
{
    return pRefersto; 
} // getReferredObject()


const std::set<PureObject3D*>& PureObject3D::PureObject3DImpl::getReferrerObjects() const
{
    return referrers;
} // getReferrerObjects()


TPURE_TRANSFORMED_VERTEX* PureObject3D::PureObject3DImpl::getTransformedVertices(TPureBool implicitAccessSubobject)
{
    if ( implicitAccessSubobject && _pOwner->isLevel1() && (_pOwner->getCount() == 1) )
        return ((PureObject3D*) (_pOwner->getAttachedAt(0)))->getTransformedVertices();
    else
        return pVerticesTransf;
} // getTransformedVertices()


PureVector& PureObject3D::PureObject3DImpl::getAngleVec()
{
    return vAngle;
} // getAngleVec()


const PureVector& PureObject3D::PureObject3DImpl::getAngleVec() const
{
    return vAngle;
} // getAngleVec()


PureVector PureObject3D::PureObject3DImpl::getScaledSizeVec() const
{
    return PureVector(
        _pOwner->getSizeVec().getX() * vScaling.getX(),
        _pOwner->getSizeVec().getY() * vScaling.getY(),
        _pOwner->getSizeVec().getZ() * vScaling.getZ());
} // getScaledSizeVec()


const PureVector& PureObject3D::PureObject3DImpl::getScaling() const
{
    return vScaling;
} // getScaling()


void PureObject3D::PureObject3DImpl::SetScaling(TPureFloat value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    vScaling.Set(value, value, value);
    recalculateBiggestAreaScaled();
    if ( pBoundingBox )
    {
        pBoundingBox->SetScaling(value);
    }
} // SetScaling()


void PureObject3D::PureObject3DImpl::SetScaling(const PureVector& value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    vScaling = value;
    recalculateBiggestAreaScaled();
    if ( pBoundingBox )
    {
        pBoundingBox->SetScaling(value);
    }
} // SetScaling()


void PureObject3D::PureObject3DImpl::Scale(TPureFloat value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    vScaling.Set( vScaling.getX() * value, vScaling.getY() * value, vScaling.getZ() * value );
    recalculateBiggestAreaScaled();
    if ( pBoundingBox )
    {
        pBoundingBox->Scale(value);
    }
} // Scale()


void PureObject3D::PureObject3DImpl::Scale(const PureVector& value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    vScaling.Set( vScaling.getX() * value.getX(), vScaling.getY() * value.getY(), vScaling.getZ() * value.getZ() );
    recalculateBiggestAreaScaled();
    if ( pBoundingBox )
    {
        pBoundingBox->Scale(value);
    }
} // Scale()


TPureFloat PureObject3D::PureObject3DImpl::getBiggestAreaScaled() const
{
    return fBiggestAreaScaled;
} // getBiggestAreaScaled()


TPureBool PureObject3D::PureObject3DImpl::isRenderingAllowed() const
{
    return bVisible;
} // isRenderingAllowed()


void PureObject3D::PureObject3DImpl::SetRenderingAllowed(TPureBool state)
{
    if ( isOccluder() && !state )
    {
        _pOwner->getConsole().EOLn("%s() WARNING: occluder state is forced to false due to set hidden!", __FUNCTION__);
        SetOccluder(false);
    }

    bVisible = state;
} // SetRenderingAllowed()


void PureObject3D::PureObject3DImpl::Show()
{
    SetRenderingAllowed(true);
} // Show()


void PureObject3D::PureObject3DImpl::Hide()
{
    SetRenderingAllowed(false);
} // Hide()


TPureBool PureObject3D::PureObject3DImpl::isColliding_TO_BE_REMOVED() const
{
    return bColliding;
}


void PureObject3D::PureObject3DImpl::SetColliding_TO_BE_REMOVED(TPureBool value)
{
    bColliding = value;
}


TPURE_ROTATION_ORDER PureObject3D::PureObject3DImpl::getRotationOrder() const
{
    return rotation;
} // getRotationOrder()


void PureObject3D::PureObject3DImpl::SetRotationOrder(TPURE_ROTATION_ORDER value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    rotation = value;
} // SetRotationOrder()


TPureBool PureObject3D::PureObject3DImpl::isLit() const
{
    return bAffectedByLights;
} // isLit()


void PureObject3D::PureObject3DImpl::SetLit(TPureBool value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    bAffectedByLights = value;
} // SetLit()


TPureBool PureObject3D::PureObject3DImpl::isDoubleSided() const
{
    return bDoubleSided;
} // isDoubleSided()


void PureObject3D::PureObject3DImpl::SetDoubleSided(TPureBool value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    bDoubleSided = value;
} // SetDoubleSided()


TPureBool PureObject3D::PureObject3DImpl::isWireframed() const
{
    return bWireframe;
} // isWireframed()


void PureObject3D::PureObject3DImpl::SetWireframed(TPureBool value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    if ( value )
    {
        if ( isOccluder() )
        {
            _pOwner->getConsole().EOLn("%s() WARNING: occluder state is forced to false due to wireframed!", __FUNCTION__);
            SetOccluder(false);
        }
    }
    bWireframe = value;
} // SetWireframed()


TPureBool PureObject3D::PureObject3DImpl::isWireframedCulled() const
{
    return bWireframedCull;
} // isWireframedCulled()


void PureObject3D::PureObject3DImpl::SetWireframedCulled(TPureBool value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    bWireframedCull = value;
} // SetWireframedCulled()


TPureBool PureObject3D::PureObject3DImpl::isAffectingZBuffer() const
{
    return bAffectZBuffer;
} // isAffectingZBuffer()


void PureObject3D::PureObject3DImpl::SetAffectingZBuffer(TPureBool value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    if ( !value )
    {
        if ( isOccluder() )
        {
            _pOwner->getConsole().EOLn("%s() WARNING: occluder state is forced to false due to not affecting zbuffer!", __FUNCTION__);
            SetOccluder(false);
        }
    }
    bAffectZBuffer = value;
} // SetAffectingZBuffer()


TPureBool PureObject3D::PureObject3DImpl::isTestingAgainstZBuffer() const
{
    return bAllowZTesting;
} // isTestingAgainstZBuffer()


void PureObject3D::PureObject3DImpl::SetTestingAgainstZBuffer(TPureBool value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    bAllowZTesting = value;
} // SetTestingAgainstZBuffer()


TPureBool PureObject3D::PureObject3DImpl::isStickedToScreen() const
{
    return bStickedToScreen;
} // isStickedToScreen()


void PureObject3D::PureObject3DImpl::SetStickedToScreen(TPureBool value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    bStickedToScreen = value;

    if ( value )
    {
        if ( isOccluder() )
        {
            _pOwner->getConsole().EOLn("%s() WARNING: occluder state is forced to false due to stickedToScreen!", __FUNCTION__);
        }
    }
    // doesnt matter if it becomes sticked now or not, occluder state must change to false because:
    // - if this object now becomes sticked object, it cannot be occluder for sure;
    // - if now becomes non-sticked object, we cannot decide if this is occluder or not, UpdateOccluderStates() will decide it later, so now suspect false!
    // We need to invoke this function for sure so containers are properly updated.
    // If we dont call this function here, we should call it only if parameter "value" is true, and update containers manually if "value" is false.
    SetOccluder(false);

    SetDoubleSided(true);
    SetLit(false);
    SetTestingAgainstZBuffer(false);
} // SetStickedToScreen()


TPureBool PureObject3D::PureObject3DImpl::isOccluder() const
{
    return bOccluder;
} // isOccluder()


void PureObject3D::PureObject3DImpl::SetOccluder(TPureBool value)
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    if ( _pOwner->getManager() == PGENULL )
    {
        _pOwner->getConsole().EOLn("%s() ERROR: Object3D does not have manager associated!", __FUNCTION__);
        return;
    }

    const TPureBool bBlended = PureMaterial::isBlendFuncReallyBlending(_pOwner->getMaterial(false).getSourceBlendFunc(), _pOwner->getMaterial(false).getDestinationBlendFunc());

    std::set< std::deque<PureObject3D*>* > mapsEraseObjectFrom;

    if ( value )
    {
        if ( !isRenderingAllowed() || 
            isStickedToScreen() ||
            !(isAffectingZBuffer()) ||
            isWireframed() ||
            bBlended
        )
        {
            _pOwner->getConsole().EOLn("%s() ERROR: cannot set occluder state to true due to conflicting property!", __FUNCTION__);
            return;
        }

        // At this point we are setting this object to be occluder, however due to rare issue we have to make sure we wait for
        // its occlusion query to finish AND set its occluded state to false because occluders are always meant to be rendered.
        // This fixes an issue that can be also seen in EV2008P03 sample, when the user moves the mouse changing camera angle
        // within the 1st few frames, during when the renderer automatically decides which objects should be occluders by UpdateOccluderStates().
        ForceFinishOcclusionTest();

        auto occ_it = std::find(occluders.begin(), occluders.end(), _pOwner);
        if ( occ_it == occluders.end() )
        {
            occluders.push_back(_pOwner);
        }
        mapsEraseObjectFrom.insert(&occludees_opaque);
        mapsEraseObjectFrom.insert(&occludees_blended);
    } // value true
    else
    {
        mapsEraseObjectFrom.insert(&occluders);

        if ( bBlended )
        {
            if ( isStickedToScreen() )
            {
                auto occ_it = std::find(occludees_2d_blended.begin(), occludees_2d_blended.end(), _pOwner);
                if ( occ_it == occludees_2d_blended.end() )
                {
                    occludees_2d_blended.push_back(_pOwner);
                }
                mapsEraseObjectFrom.insert(&occludees_blended);
            }
            else
            {
                auto occ_it = std::find(occludees_blended.begin(), occludees_blended.end(), _pOwner);
                if ( occ_it == occludees_blended.end() )
                {
                    occludees_blended.push_back(_pOwner);
                }
                mapsEraseObjectFrom.insert(&occludees_2d_blended);
            }

            mapsEraseObjectFrom.insert(&occludees_opaque);
            mapsEraseObjectFrom.insert(&occludees_2d_opaque);
        } // bBlended true
        else
        {
            if ( isStickedToScreen() )
            {
                auto occ_it = std::find(occludees_2d_opaque.begin(), occludees_2d_opaque.end(), _pOwner);
                if ( occ_it == occludees_2d_opaque.end() )
                {
                    occludees_2d_opaque.push_back(_pOwner);
                }
                mapsEraseObjectFrom.insert(&occludees_opaque);
            }
            else
            {
                auto occ_it = std::find(occludees_opaque.begin(), occludees_opaque.end(), _pOwner);
                if ( occ_it == occludees_opaque.end() )
                {
                    occludees_opaque.push_back(_pOwner);
                }
                mapsEraseObjectFrom.insert(&occludees_2d_opaque);
            }

            mapsEraseObjectFrom.insert(&occludees_blended);
            mapsEraseObjectFrom.insert(&occludees_2d_blended);
        } // bBlended false
    } // value false

    for (auto pMapEraseFrom : mapsEraseObjectFrom )
    {
        auto occ_it = std::find(pMapEraseFrom->begin(), pMapEraseFrom->end(), _pOwner);
        while ( occ_it != pMapEraseFrom->end() )
        {
            pMapEraseFrom->erase(occ_it);
            occ_it = std::find(pMapEraseFrom->begin(), pMapEraseFrom->end(), _pOwner);
        }
    } // end for pMapEraseFrom

    bOccluder = value;
} // SetOccluder()


TPureBool PureObject3D::PureObject3DImpl::isOccluded() const
{
    return bOccluded;
} // isOccluded()


TPureBool PureObject3D::PureObject3DImpl::isOcclusionTested() const
{
    return nOcclusionQuery > 0;
} // isOcclusionTested()


void PureObject3D::PureObject3DImpl::SetOcclusionTested(TPureBool state)
{
    // It can happen at rare cases e.g. at engine shutdown that isLevel1() returns false due to we are cloned object NOT having the referred object anymore,
    // because it has just been deleted prior to deleting us (SetOcclusionTested(false) is being invoked by our dtor).
    // This case should be somehow identified to NOT log it since that is misleading to log this call as we were really a level-2 object!
    // I could not find a way to detect at this point that isLevel1() returns false just because our referred object has been already deleted.
    // So, instead of just checking isLevel1() right at the beginning of the function:
    // - check for isLevel1() only when state is true, because that path we definitely don't want to be executed for level-2 objects, it must be logged as error;
    //   - and it is definitely deleting objects case, since dtors set state to false, not true!
    // - do not check for isLevel1() when given state is false because it that code path doesn't have any effect when invoked for a real level-2 object.

    if ( !PureHwInfo::get().getVideo().isOcclusionQuerySupported() )
        return;

    if ( state )
    {
        if ( PGENULL == _pOwner->getManager() )
        {
            throw std::runtime_error("no manager!");
        }

        if ( !_pOwner->isLevel1() )
        {
            _pOwner->getManagedConsole().EOLn("PureObject3D::%s(true) attempt for level-2 object is ignored!", __FUNCTION__);
            return;
        } 

        if ( !pglGenQueries(1, &nOcclusionQuery) ) 
        {
            _pOwner->getConsole().EOLn("%s() ERROR: Could not generate occlusion query, this object won't be tested for occlusion!", __FUNCTION__);
            nOcclusionQuery = 0;  // just in case pglGenQueries() changed it to something else ...
            return;
        }
        
        _pOwner->getConsole().OLn("Occlusion query ID: %d", nOcclusionQuery);

        if ( getReferredObject() )
        {
            if ( getReferredObject()->pImpl->pBoundingBox )
            {
                // if we are cloned object, we should NOT create bounding box object if referred already has since we can use the box of the referred object
                // we can do this because in case of cloned object, we expect the same geometry, etc.
                _pOwner->getConsole().OLn("We are cloned and referred already has bounding box, so will use that");
                return;
            }
            else
            {
                _pOwner->getConsole().OLn("We are cloned but referred does not have bounding box, so creating it now for cloned ...");
            }
        }

        // here we specify forcing bounding box geometry in client memory because we alter vertex positions with rel pos vec, and then we upload it to host memory
        pBoundingBox = ((PureObject3DManager*)_pOwner->getManager())->createBox(_pOwner->getSizeVec().getX(), _pOwner->getSizeVec().getY(), _pOwner->getSizeVec().getZ(), PURE_VMOD_DYNAMIC, PURE_VREF_INDEXED, true);
        if ( PGENULL == pBoundingBox )
        {
            const std::string sErrMsg = "failed to create pBoundingBox!";
            throw std::runtime_error(sErrMsg);
        }

        pBoundingBox->SetName("Bounding Box for " + _pOwner->getName());
        pBoundingBox->Hide();
        // sometimes geometry is not exactly placed in mesh's [0,0,0], so we need to offset bounding box vertices based on object's relpos!
        for (TPureUInt i = 0; i < pBoundingBox->getVerticesCount(); i++)
        {
            pBoundingBox->getVertices()[i].x += _pOwner->getRelPosVec().getX();
            pBoundingBox->getVertices()[i].y += _pOwner->getRelPosVec().getY();
            pBoundingBox->getVertices()[i].z += _pOwner->getRelPosVec().getZ();
        }
        // upload bounding box geometry now to host memory with altered vertex positions
        pBoundingBox->setVertexTransferMode( pBoundingBox->selectVertexTransferMode(PURE_VMOD_STATIC, PURE_VREF_INDEXED, false) );
        pBoundingBox->DetachFrom();  // Object3DManager is no longer responsible for this but the Object3D that has the ptr to it!
    }
    else
    {
        if ( referrers.size() > 0 )
        {
            _pOwner->getConsole().OLn("%s() WARNING: implicitly setting occlusion testing off for all referring cloned objects!", __FUNCTION__);
            for (auto referrer : referrers)
                referrer->SetOcclusionTested(false);
        }

        if ( nOcclusionQuery > 0 )
        {
            glDeleteQueriesARB(1, &nOcclusionQuery);
            nOcclusionQuery = 0;
        }

        delete pBoundingBox;
        pBoundingBox = PGENULL;

        bOccluded = false;
        bOcclusionQueryStarted = false;
    }
} // SetOcclusionTested()
                                                                                                                           

const PureObject3D* PureObject3D::PureObject3DImpl::getBoundingBoxObject() const
{
    return pBoundingBox;
} // getBoundingBoxObject()


void PureObject3D::PureObject3DImpl::ForceFinishOcclusionTest()
{
    if ( !_pOwner->isLevel1() )
    {
        _pOwner->getManagedConsole().EOLn("PureObject3D::%s() attempt for level-2 object is ignored!", __FUNCTION__);
        return;
    }

    if ( !bOcclusionQueryStarted || (nOcclusionQuery == 0) )
    {
        return;
    }

    GLint queryResultAvailable;
    do {
        glGetQueryObjectivARB(nOcclusionQuery, GL_QUERY_RESULT_AVAILABLE_ARB, &queryResultAvailable);
    } while (!queryResultAvailable);

    bOcclusionQueryStarted = false;

    if ( PureHwInfo::get().getVideo().isBooleanOcclusionQuerySupported() )
    {
        GLint sampleBoolean;
        glGetQueryObjectivARB(nOcclusionQuery, GL_QUERY_RESULT_ARB, &sampleBoolean);
        bOccluded = ( sampleBoolean == GL_FALSE );
    }
    else
    {
        GLuint sampleCount;
        glGetQueryObjectuivARB(nOcclusionQuery, GL_QUERY_RESULT_ARB, &sampleCount);
        bOccluded = ( sampleCount == 0 );
    }
} // ForceFinishOcclusionTest()


TPureUInt PureObject3D::PureObject3DImpl::getUsedSystemMemory() const
{
    return (
        sizeof(*this) +
        nFbBuffer_h /* 0 for level-1 */ * sizeof(GLfloat) +
        sizeof(TPURE_TRANSFORMED_VERTEX) /* 0 for level-1 */ * _pOwner->getVertexIndicesCount() +
        (pBoundingBox /* NULL for level-2 */ ? pBoundingBox->getUsedSystemMemory() : 0)
    );
} // getUsedSystemMemory()     


TPureUInt PureObject3D::PureObject3DImpl::draw(const TPURE_RENDER_PASS& renderPass, TPureBool bASyncQuery, TPureBool bRenderIfQueryPending)
{
    // caller renderer is expected to check for GL errors, probably only once per frame, so we don't check them here

    if ( _pOwner->isLevel1() )
    {
        _pOwner->ResetLastTransferredCounts();
    }
    
    if ( !bVisible )
        return 0;

    if ( !PureHwInfo::get().getVideo().isAcceleratorDetected() )
    {
        return draw_DrawSW();
    }

    // see if we are parent or referring to another object i.e. we are cloned object
    if ( _pOwner->isLevel1() )
    {
        // Currently 3D objects are 2-level entities:
        // first level (parent) has no geometry, owns subobjects, sets basic things,
        // while second level (subobjects) own geometry, inherit basic things set by parent.

        // So transformations and other basic things are set by parent objects having subobjects.
        // Or by cloned objects which refer to another original objects but still have their own position, angle, etc.        
        if ( renderPass == PURE_RPASS_NORMAL )
        {
            if ( (nOcclusionQuery != 0) && draw_OcclusionQuery_Finish(bASyncQuery, bRenderIfQueryPending) )
            {
                return 0;
            }
            
            Draw_ApplyTransformations();
            Draw_PrepareGLBeforeDrawNormal(false);
            // continue with drawing our subobjects
        }
        else if ( renderPass == PURE_RPASS_START_OCCLUSION_QUERY )
        {
            if ( nOcclusionQuery == 0 )
            {
                // there is nothing to do in this pass
                return 0;
            }
            Draw_ApplyTransformations();
            return draw_OcclusionQuery_Start(bASyncQuery);
        }
        else if ( renderPass == PURE_RPASS_BOUNDING_BOX_DEBUG_FOR_OCCLUSION_QUERY )
        {
            if ( nOcclusionQuery == 0 )
            {
                // there is nothing to do in this pass
                return 0;
            }
            Draw_ApplyTransformations();
            return draw_RenderBoundingBox();
        }
        else if ( renderPass == PURE_RPASS_Z_ONLY )
        {
            Draw_ApplyTransformations();
            // continue with drawing our subobjects
        }

        // take care of attached objects (subobjects)
        // note that either we have our own subobjects, OR we are just a cloned object and we need to draw original object's subobjects
        PureObject3D* pWhichParent = getReferredObject() ? getReferredObject() : _pOwner;

        pWhichParent->pImpl->bParentInitiatedOperation = true;
        TPureUInt nRetSum = 0;
        for (TPureInt i = 0; i < pWhichParent->getCount(); i++)
            nRetSum += ((PureObject3D*)pWhichParent->getAttachedAt(i))->draw(renderPass, bASyncQuery, bRenderIfQueryPending);
        pWhichParent->pImpl->bParentInitiatedOperation = false;
        return nRetSum;
    }

    // if we reach this point then either our parent is drawing us as its subobject, or a cloned object is drawing us on behalf of our parent
    PureObject3D* pObjLevel1 = (PureObject3D*)_pOwner->getManager();
    const TPureBool bObjLevel1Blended = PureMaterial::isBlendFuncReallyBlending(pObjLevel1->getMaterial(false).getSourceBlendFunc(), pObjLevel1->getMaterial(false).getDestinationBlendFunc());

    // subobject must ignore draw if its Draw() was not called by its parent level-1 object but someone else from outside ...
    if ( !pObjLevel1->pImpl->bParentInitiatedOperation )
    {
        _pOwner->getManagedConsole().EOLn("Draw() of subobject called outside of its level-1 parent object, ignoring draw!");
        return 0;
    }

    if ( renderPass == PURE_RPASS_NORMAL )
    {
        PureGLsnippets::glLoadTexturesAndSetBlendState(&(_pOwner->getMaterial(false)), pObjLevel1->isStickedToScreen(), bObjLevel1Blended);
    }

    Draw_FeedbackBuffer_Start();
    const TPureUInt nTransferredVertexCount = _pOwner->transferVertices();
    Draw_FeedbackBuffer_Finish(); 

    return nTransferredVertexCount;
} // Draw()


// ############################## PROTECTED ##############################


/**
    @param owner                 The public Object3D class instance owning this pimpl object.
    @param vmod                  What vertex modifying habit to be set for the new Object3D instance.
    @param vref                  What vertex referencing mode to be set for the new Object3D instance.
    @param bForceUseClientMemory Force-select a vertex transfer mode storing geometry in client memory instead of server memory.
                                 Please note that this is considered only if dynamic modifying habit is specified.
                                 Specifying static modifying habit will always select a mode which places geometry data into server memory.
*/
PureObject3D::PureObject3DImpl::PureObject3DImpl(
    PureObject3D* owner,
    const TPURE_VERTEX_MODIFYING_HABIT& vmod,
    const TPURE_VERTEX_REFERENCING_MODE& vref,
    TPureBool bForceUseClientMemory )
{
    _pOwner = owner;
    _pOwner->getManagedConsole().OLnOI("PureObject3D() ...");

    pRefersto = NULL;
    bAffectedByLights = bAffectZBuffer = bAllowZTesting = true;
    bVisible = true;
    bParentInitiatedOperation = false;
    bColliding = bDoubleSided = false;
    bWireframe = bWireframedCull = bStickedToScreen = false;

    bOccluder = false;
    pBoundingBox = PGENULL;
    nOcclusionQuery = 0;
    bOccluded = false;
    bOcclusionQueryStarted = false;
    timeLongestWaitForSyncQueryFinish.tv_sec = LONG_MIN;
    timeLongestWaitForSyncQueryFinish.tv_usec = LONG_MIN;
    nFramesWithoutOcclusionTest = OQ_MAX_FRAMES_WO_START_QUERY_WHEN_VISIBLE /* start queries right in 1st frame */;
    nFramesWaitedForOcclusionTestResult = 0;
    nFramesWaitedForOcclusionTestResultMin = UINT_MAX;
    nFramesWaitedForOcclusionTestResultMax = 0;

    vScaling.Set(1.0f, 1.0f, 1.0f);
    fBiggestAreaScaled = 0.0f;
    rotation = PURE_YXZ;

    selectVertexTransferMode(vmod, vref, bForceUseClientMemory);
    pVerticesTransf = PGENULL;
    pFbBuffer = PGENULL;

    nFbBuffer_h = 0;

    _pOwner->getManagedConsole().SOLnOO("Done!");
} // PureObject3D()


PureObject3D::PureObject3DImpl::PureObject3DImpl(const PureObject3DImpl&)
{

}


PureObject3D::PureObject3DImpl& PureObject3D::PureObject3DImpl::operator=(const PureObject3DImpl&)
{
    return *this;
}


// ############################### PRIVATE ###############################


/**
    Set render mode to feedback and allocate buffer for transformed vertices.
    This is done only once, when the functions is called for the first time.
    Any consecutive call to this function has no effect, so pFbBuffer and nFbBuffer_h variables
    will hold the same values that have been stored during the rendereing of the 1st frame.
    This is for debugging purposes only, for breakpoint, etc.
    But in the future when HW transform and SW rasterization can be combined, this will be controlled by public API.
*/
void PureObject3D::PureObject3DImpl::Draw_FeedbackBuffer_Start()
{
    assert(_pOwner->isLevel2());

    /* We create this buffer only once at first run. */
    /* We intentionally switch to feedback mode only once since now we just use it for debugging. */
    if ( pFbBuffer != NULL )
        return;

    /* num of values can be stored in this buffer */
    /* we need to store not only the transformed values but also some extra values for grammar */
    /* in this case, the items placed in this array are polygon items (GL_POLYGON_TOKEN) which look like this:
        GL_POLYGON_TOKEN 3 (value value value value (value value value value) (value value value value))
        So for a standard cube where each side has 2 triangles, it is 12 triangles in total, meaning
        12 polygons. GL_POLYGON_TOKEN itself is 1 value, 3 itself is also 1 value, and then 3*12 values for 3 vertices.
        So the total num of values is: numOfTriangles * (2+3*12)
        Example with some real data is in later comment with debugbuffer[]. */
    
    nFbBuffer_h = GLsizei(ceil((_pOwner->getVerticesCount() / 3.0f))) * (2+3*12);
    try
    {
        /* TODO: probably in future we should rather use pVerticesTransf here as well since we already have it for that purpose, right? ;) */
        pFbBuffer = new GLfloat[nFbBuffer_h];

        /* unfortunately only the most detailed option GL_4D_COLOR_TEXTURE will give us the w-coord of vertices so we need to use that */
        if ( pglFeedbackBuffer(nFbBuffer_h, GL_4D_COLOR_TEXTURE, pFbBuffer) )
        {
            // If you want all vertices to be transformed and catched in feedback mode then dont forget to disable culling and depth testing (maybe only 1 is needed to be disabled).
            //glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE); /* otherwise only the front facing side of cube would be written to feedback buffer */

            glRenderMode(GL_FEEDBACK);
        }
    }
    catch (const std::bad_alloc&)
    {
        _pOwner->getManagedConsole().EOLn("ERROR: PureObject3D::PureObject3DImpl::Draw() failed to allocate pFbBuffer!");
    }
} // Draw_FeedbackBuffer_Start()


/**
    Set render mode to default render.
    This is for debugging purposes only, for breakpoint, etc.
    If a breakpoint is set inside this function, the coordinates of HW-transformed vertices can be examined.
*/
void PureObject3D::PureObject3DImpl::Draw_FeedbackBuffer_Finish()
{
    assert(_pOwner->isLevel2());

    /* The number of values (not vertices) transferred to the feedback buffer. */
    const GLint nFbBufferWritten_h = glRenderMode(GL_RENDER);

    GLfloat debugbuffer[500];

    if ( nFbBufferWritten_h > 0 )
    {
        // we are happy
        for (int i = 0; i < std::min(500,nFbBufferWritten_h); i++)
            debugbuffer[i] = pFbBuffer[i];
        //Sleep(1); // suitable for placing breakpoint
        /*
            debugbuffer[0] = GL_POLYGON_TOKEN;
            debugbuffer[1] = 3;
            debugbuffer[ 2.. 5] = x y z w (xyz are in window coordinates while w is clip coordinate)
            debugbuffer[ 6.. 9] = r g b a
            debugbuffer[10..13] = texture coords, probably x y u w
            debugbuffer[14..17] = x y z w (xyz are in window coordinates while w is clip coordinate)
            debugbuffer[18..21] = r g b a
            debugbuffer[22..25] = texture coords, probably x y u w
            debugbuffer[26..29] = x y z w (xyz are in window coordinates while w is clip coordinate)
            debugbuffer[30..33] = r g b a
            debugbuffer[34..37] = texture coords, probably x y u w
            debugbuffer[38] = GL_POLYGON_TOKEN;
            debugbuffer[39] = 3;

            example: gamedata\\models\\cube.obj
            assuming that nearplane is 0.1f, farplane is 100.0f, depthrange is [0, 1], fovy is 80
            box
            i=2 (3. vertex) debugbuffer[26]
                model space        -> world space       -> view/eye space    -> clip space                   -> normalized device coords     -> screen coords
            sw: [-0.5, -0.5, -0.5] -> [-0.5, -0.5, 2.5] -> [-0.5, -0.5, 2.5] -> [-0.446, -0.595, 2.348, 2.5] -> [-0.178, -0.238, 0.939, 2.5] -> [328, 228, 0.961, 2.5]
            hw:                                                                                                                                 [328, 228, 0.961, 2.5]

            i=6 (7. vertex) debugbuffer[?]
                model space        -> world space       -> view/eye space    -> clip space                   -> normalized device coords     -> screen coords
            sw: [ 0.5,  0.5,  0.5] -> [ 0.5,  0.5, 3.5] -> [ 0.5,  0.5, 3.5] -> [ 0.446,  0.595, 3.368, 3.5] -> [ 0.127,  0.170, 0.962, 3.5] -> [451, 351, 0.972, 3.5]
            hw:                                                                                                                                 [451, 351, 0.972, 3.5]

            ...
        */
    }
    else
    {
        // we were already in GL_RENDER (0) mode, OR nothing was returned, OR we were in FEEDBACK but buffer was not enough to hold all values (<0) ...
    }
} // Draw_FeedbackBuffer_Finish()


/**
    Applies transformations to the current modelview matrix based on the given object.
*/
void PureObject3D::PureObject3DImpl::Draw_ApplyTransformations() const    
{
    assert(_pOwner->isLevel1());

    /* Make sure we don't call GL functions when no accelerator is present */
    if ( !PureHwInfo::get().getVideo().isAcceleratorDetected() )
        return;

    glTranslatef(_pOwner->getPosVec().getX(), _pOwner->getPosVec().getY(), _pOwner->getPosVec().getZ());
    
    switch ( getRotationOrder() )
    {
    case PURE_XYZ:
        glRotatef(getAngleVec().getX(), 1,0,0);
        glRotatef(getAngleVec().getY(), 0,1,0);
        glRotatef(getAngleVec().getZ(), 0,0,1);
        break;
    case PURE_XZY:
        glRotatef(getAngleVec().getX(), 1,0,0);
        glRotatef(getAngleVec().getZ(), 0,0,1);
        glRotatef(getAngleVec().getY(), 0,1,0);
        break;
    case PURE_YXZ:
        glRotatef(getAngleVec().getY(), 0,1,0);
        glRotatef(getAngleVec().getX(), 1,0,0);
        glRotatef(getAngleVec().getZ(), 0,0,1);
         break;
    case PURE_YZX:
        glRotatef(getAngleVec().getY(), 0,1,0);
        glRotatef(getAngleVec().getZ(), 0,0,1);
        glRotatef(getAngleVec().getX(), 1,0,0);
        break;
    case PURE_ZXY:
        glRotatef(getAngleVec().getZ(), 0,0,1);
        glRotatef(getAngleVec().getX(), 1,0,0);
        glRotatef(getAngleVec().getY(), 0,1,0);
        break;
    case PURE_ZYX:
        glRotatef(getAngleVec().getZ(), 0,0,1);
        glRotatef(getAngleVec().getY(), 0,1,0);
        glRotatef(getAngleVec().getX(), 1,0,0);
        break;
    } // switch

    glScalef(getScaling().getX(), getScaling().getY(), getScaling().getZ());
    /*
        Well, mirror-transforming an object (scaling by negative value) makes its faces flip:
        https://www.gamedev.net/forums/topic/640616-negative-scaling-flips-face-winding-affects-backface-culling/
        I wanted to X-mirror objects loaded by tmcsgfxlib-to-prre wrapper for legacy projects, because I noticed they appeared mirrored compared to proofps engine behavior.
        So I noticed when setting negative scale on X axis, faces also flipped, so objects became inside-out.
        This is expected since vertices' winding order also flips because we still feed triangle vertices in same order even though triangles become mirrored.
        But I don't treat this as a legacy-project-only issue, since even a new project with the new engine could come up for any reason with negative scaling for any mirroring reason.
        So, this is rather a feature: we need to detect negative scaling factor and change front face winding order on-the-fly.
    */
    if ( getScaling().getX() * getScaling().getY() * getScaling().getZ() <= (0.0f-PFL::E) )
        glFrontFace(GL_CW);
    else
        glFrontFace(GL_CCW);
} // Draw_ApplyTransformations()


void PureObject3D::PureObject3DImpl::Draw_PrepareGLBeforeDrawNormal(bool bLighting) const
{
    /*AmbientLightPos[0] =  cam.getX() - obj->getPosVec().getX();
    AmbientLightPos[1] =  cam.getY() - obj->getPosVec().getY();
    AmbientLightPos[2] = -cam.getZ() - obj->getPosVec().getZ();*/
    /*AmbientLightPos[0] = cam.getX();
    AmbientLightPos[1] = cam.getY();
    AmbientLightPos[2] = -cam.getZ()-2.0f;
    AmbientLightPos[3] = 1.0f;
    glLightfv(GL_LIGHT0, GL_POSITION, AmbientLightPos);  */

    assert(_pOwner->isLevel1());

    /* Make sure we don't call GL functions when no accelerator is present */
    if ( !PureHwInfo::get().getVideo().isAcceleratorDetected() )
        return;

    if ( isDoubleSided() )
    {
        glDisable(GL_CULL_FACE);
        if ( isLit() && bLighting )
        {
            glEnable(GL_LIGHTING);
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
            //glLightfv(GL_LIGHT0, GL_POSITION, AmbientLightPos); 
        }
        else
        {
            glDisable(GL_LIGHTING);
        }
    }
    else
    {
        glEnable(GL_CULL_FACE);
        if ( isLit() && bLighting )
        {
            glEnable(GL_LIGHTING);
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
            //glLightfv(GL_LIGHT0, GL_POSITION, AmbientLightPos);
        }
        else
        {
            glDisable(GL_LIGHTING);
        }
    } // isDoubleSided()

    if ( isWireframed() )
    {
        glDisable(GL_LIGHTING);
        if ( !isWireframedCulled() )
            glDisable(GL_CULL_FACE);
        else
            glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if (_pOwner->getMaterial(false).isDecalOffsetEnabled())
        {
            glEnable(GL_POLYGON_OFFSET_LINE);
        }
        else
        {
            glDisable(GL_POLYGON_OFFSET_LINE);
        }
    }
    else
    {
        if ( isLit() && bLighting )
        {
            glEnable(GL_LIGHTING);
            //glLightfv(GL_LIGHT0, GL_POSITION, AmbientLightPos);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if (_pOwner->getMaterial(false).isDecalOffsetEnabled())
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
        }
        else
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    } // isWireframed()
    
    if ( isTestingAgainstZBuffer() )
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    // TODO: nope, we shouldnt specify color this way because glColor4f() is called per-vertex anyway later
    // getTextureEnvColor() is used by texture environment mode / function, not blending!
    // we should rather use glBlendColor() for this with GL_*_CONSTANT_* blendfunc, or modify the code not to specify color per-vertex
    
    glColor4f(
        _pOwner->getMaterial(false).getTextureEnvColor().getRedAsFloat(),
        _pOwner->getMaterial(false).getTextureEnvColor().getGreenAsFloat(),
        _pOwner->getMaterial(false).getTextureEnvColor().getBlueAsFloat(),
        _pOwner->getMaterial(false).getTextureEnvColor().getAlphaAsFloat() );   
             
    /*
        Probably later these info will be useful:

        "glEnable(GL_DEPTH_TEST) should always be enabled, depthmask relies on it"
        https://www.gamedev.net/forums/topic/87480-gldepthmask-not-working/

        "Due to an odd quirk of OpenGL, writing to the depth buffer is always inactive if GL_DEPTH_TEST is disabled, regardless of the depth mask.
         If you just want to write to the depth buffer, without actually doing a test, you must enable GL_DEPTH_TEST and use the depth function of GL_ALWAYS."
        https://paroj.github.io/gltut/Positioning/Tut05%20Overlap%20and%20Depth%20Buffering.html

        "glDepthMask(GL_FALSE);
         Note that this only has effect if depth testing is enabled."
        https://learnopengl.com/Advanced-OpenGL/Depth-testing

        "Even if the depth buffer exists and the depth mask is non-zero, the depth buffer is not updated if the depth test is disabled.
         In order to unconditionally write to the depth buffer, the depth test should be enabled and set to GL_ALWAYS."
        https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDepthFunc.xhtml
    */
    if ( PureMaterial::isBlendFuncReallyBlending(_pOwner->getMaterial(false).getSourceBlendFunc(), _pOwner->getMaterial(false).getDestinationBlendFunc()) )
    {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(PureGLsnippets::getGLBlendFromPureBlend(_pOwner->getMaterial(false).getSourceBlendFunc()), PureGLsnippets::getGLBlendFromPureBlend(_pOwner->getMaterial(false).getDestinationBlendFunc()));
        glAlphaFunc(GL_GREATER, 0.1f);
        glEnable(GL_ALPHA_TEST);
    }
    else
    {
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glDisable(GL_ALPHA_TEST);
    }
} // Draw_PrepareGLBeforeDrawNormal()


void PureObject3D::PureObject3DImpl::glBeginOcclusionQuery() const
{
    assert(_pOwner->isLevel1());

    /* Make sure we don't call GL functions when no accelerator is present */
    if ( !PureHwInfo::get().getVideo().isAcceleratorDetected() )
        return;
    
    if ( PureHwInfo::get().getVideo().isBooleanOcclusionQuerySupported() )
    {
        glBeginQueryARB(GL_ANY_SAMPLES_PASSED, nOcclusionQuery);
    }
    else
    {
        glBeginQueryARB(GL_SAMPLES_PASSED_ARB, nOcclusionQuery);
    }
} // glBeginOcclusionQuery()()


void PureObject3D::PureObject3DImpl::glEndOcclusionQuery() const
{
    assert(_pOwner->isLevel1());

    /* Make sure we don't call GL functions when no accelerator is present */
    if ( !PureHwInfo::get().getVideo().isAcceleratorDetected() )
        return;

    if ( PureHwInfo::get().getVideo().isBooleanOcclusionQuerySupported() )
    {
        glEndQueryARB(GL_ANY_SAMPLES_PASSED);
    }
    else
    {
        glEndQueryARB(GL_SAMPLES_PASSED_ARB);
    }
} // glEndOcclusionQuery()()


/**
    Sends the bounding box geometry to the graphics pipeline to draw it in the framebuffer.
    No effect if the object is not tested for occlusion.

    @return Number of transferred vertices, which are the vertices of the bounding box.
*/
TPureUInt PureObject3D::PureObject3DImpl::draw_RenderBoundingBox() const
{
    assert(_pOwner->isLevel1());

    if ( nOcclusionQuery == 0 )
    {
        return 0;
    }

    PureObject3D* const pWhichBoundingBox = pBoundingBox ? pBoundingBox : (getReferredObject() ? getReferredObject()->pImpl->pBoundingBox : PGENULL);

    assert(pWhichBoundingBox != PGENULL);
    assert(pWhichBoundingBox->getCount() > 0);

    pWhichBoundingBox->ResetLastTransferredCounts();

    if ( isOccluded() )
    {
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    }
    return ((PureObject3D*)(pWhichBoundingBox->getAttachedAt(0)))->transferVertices();
} // draw_RenderBoundingBox()


/**
    Starts occlusion query for this object if it has a query id and query should be started.
    If the occlusion query is actually started, a bounding box geometry will be sent to the graphics pipeline to find out
    if the bound geometry would be visible or not.

    @param async If true, it might delay query start for a next frame.
                 If false, it will start the query for sure.

    @return Number of transferred vertices, in case of bounding box is transferred through the graphics pipeline, false otherwise.
*/
TPureUInt PureObject3D::PureObject3DImpl::draw_OcclusionQuery_Start(TPureBool async)
{
    assert(_pOwner->isLevel1());

    if ( nOcclusionQuery == 0 )
    {
        return 0;
    }   

    PureObject3D* const pWhichBoundingBox = pBoundingBox ? pBoundingBox : (getReferredObject() ? getReferredObject()->pImpl->pBoundingBox : PGENULL);

    assert(pWhichBoundingBox != PGENULL);
    assert(pWhichBoundingBox->getCount() > 0);

    pWhichBoundingBox->ResetLastTransferredCounts();

    if ( bOcclusionQueryStarted )
    {
        // obviously we don't start it if it is already started, doesn't matter if it is sync or async,
        // however since all sync occlusion queries must finish in the same frame, it cannot happen it is already started when we are just invoked to start it!
        if ( !async )
        {
            _pOwner->getConsole().EOLn("%s() ERROR: occlusion query is already started but we are in SYNC query mode!", __FUNCTION__);
        }
        assert(async);
        return 0;
    }

    if ( async )
    {
        if ( bOccluded )
        {
            if ( nFramesWithoutOcclusionTest < OQ_MAX_FRAMES_WO_START_QUERY_WHEN_OCCLUDED )
            {
                nFramesWithoutOcclusionTest++;
                return 0;
            }
        }
        else
        {
            if ( nFramesWithoutOcclusionTest < OQ_MAX_FRAMES_WO_START_QUERY_WHEN_VISIBLE )
            {
                nFramesWithoutOcclusionTest++;
                return 0;
            }
        }
    } // async

    glBeginOcclusionQuery();
    const TPureUInt nRet = ((PureObject3D*)(pWhichBoundingBox->getAttachedAt(0)))->transferVertices();
    glEndOcclusionQuery();

    bOcclusionQueryStarted = true;
    nFramesWaitedForOcclusionTestResult = 0; // for async query

    return nRet;
} // draw_OcclusionQuery_Start()


/**
    Checks for occlusion query result and decides if object is occluded or not.

    @param bASyncQuery If true, it just checks if query is complete, but won't wait for it.
                       If false, it will wait until query is finished.
    @param bRenderIfQueryPending If true, and we do not have query result yet, the function will respond as if the object is NOT occluded.
                                 If false, and we do not have query result yet, the function will respond based on the last occlusion state of this object,
                                 meaning that: if the last finished query said the object was occluded, the function will respond as the object was occluded,
                                 otherwise it will respond as the object was not occluded.
                                 This parameter is used only with async queries.

    @return True if occluded, false if not occluded or cannot conclude.
*/
TPureBool PureObject3D::PureObject3DImpl::draw_OcclusionQuery_Finish(TPureBool bASyncQuery, TPureBool bRenderIfQueryPending)
{
    assert(_pOwner->isLevel1());

    if ( nOcclusionQuery == 0 )
    {
        return false; // let it be rendered
    }

    if ( !bOcclusionQueryStarted )
    {
        // we always end up here with legacy renderer which never starts query for any objects but
        // still invokes this function (i.e. Object3D::Draw() invokes it)
        
        if ( bASyncQuery )
        {
            if ( bOccluded && (OQ_MAX_FRAMES_WO_START_QUERY_WHEN_OCCLUDED == 0) )
            {   // if query is not started, but it should had been started if constant value says it should not wait for another frame,
                // then this is a programmer error condition
                if ( !isOccluder() )
                {   // abort only if this is really an occludee: because renderer won't start occlusion query for occluders beforehand!
                    assert(false); // this just cannot happen
                }
            }
            return bOccluded; // let it be rendered if last result says not occluded
        }
        else
        {
            // it is ok to render with either legacy renderer or when occlusion query is in sync
            // since in latter case bOcclusionQueryStarted is false only when it is not tested for occlusion
            return false; // let it be rendered
        }
    }

    // this should be done rather with chrono after we have C++11
    PFL::timeval begin, end;
    PFL::gettimeofday(&begin, 0);

    GLint queryResultAvailable;
    do {
        glGetQueryObjectivARB(nOcclusionQuery, GL_QUERY_RESULT_AVAILABLE_ARB, &queryResultAvailable);
    } while (!queryResultAvailable && !bASyncQuery);

    // update sync statistics related to wait time
    if ( !bASyncQuery )
    {
        PFL::gettimeofday(&end, 0);
        if ( PFL::updateForMaxDuration(timeLongestWaitForSyncQueryFinish, begin, end) )
        {
            PFL::updateForMaxDuration(PureObject3D::PureObject3DImpl::stats[PureObject3D::PureObject3DImpl::stats.size()-1].timeLongestGlobalWaitForSyncQueryFinish, begin, end);
        }
    } // sync stats update

    if ( queryResultAvailable == GL_FALSE )
    {
        // this is async query
        nFramesWaitedForOcclusionTestResult++;
        return bRenderIfQueryPending ? false : bOccluded;
    }

    bOcclusionQueryStarted = false;

    if ( PureHwInfo::get().getVideo().isBooleanOcclusionQuerySupported() )
    {
        GLint sampleBoolean;
        glGetQueryObjectivARB(nOcclusionQuery, GL_QUERY_RESULT_ARB, &sampleBoolean);
        bOccluded = ( sampleBoolean == GL_FALSE );
    }
    else
    {
        GLuint sampleCount;
        glGetQueryObjectuivARB(nOcclusionQuery, GL_QUERY_RESULT_ARB, &sampleCount);
        bOccluded = ( sampleCount == 0 );
    }

    // update async statistics
    if ( bASyncQuery )
    {
        if ( nFramesWaitedForOcclusionTestResult < nFramesWaitedForOcclusionTestResultMin )
        {
            nFramesWaitedForOcclusionTestResultMin = nFramesWaitedForOcclusionTestResult;
            if ( nFramesWaitedForOcclusionTestResultMin < PureObject3D::PureObject3DImpl::stats[PureObject3D::PureObject3DImpl::stats.size()-1].nFramesWaitedForOcclusionTestResultGlobalMin )
            {
                PureObject3D::PureObject3DImpl::stats[PureObject3D::PureObject3DImpl::stats.size()-1].nFramesWaitedForOcclusionTestResultGlobalMin = nFramesWaitedForOcclusionTestResultMin;
            }
        }
        if ( nFramesWaitedForOcclusionTestResult > nFramesWaitedForOcclusionTestResultMax )
        {
            nFramesWaitedForOcclusionTestResultMax = nFramesWaitedForOcclusionTestResult;
            if ( nFramesWaitedForOcclusionTestResultMax > PureObject3D::PureObject3DImpl::stats[PureObject3D::PureObject3DImpl::stats.size()-1].nFramesWaitedForOcclusionTestResultGlobalMax )
            {
                PureObject3D::PureObject3DImpl::stats[PureObject3D::PureObject3DImpl::stats.size()-1].nFramesWaitedForOcclusionTestResultGlobalMax = nFramesWaitedForOcclusionTestResultMax; 
            }
        }
        nFramesWaitedForOcclusionTestResult = 0;
        nFramesWithoutOcclusionTest = 0;
    }

    return bOccluded;
} // draw_OcclusionQuery_Finish()


/**
    @return Number of transferred vertices.
*/
TPureUInt PureObject3D::PureObject3DImpl::draw_DrawSW()
{
    if ( !bVisible )
        return 0;

    // see if we are parent or referring to another object i.e. we are cloned object
    if ( _pOwner->isLevel1() )
    {
        // Currently 3D objects are 2-level things: first level (parent) has no geometry, owns subobjects, sets basic things,
        // while second level (subobjects) own geometry, inherit basic things set by parent.

        // So transformations and other basic things are set by parent objects having subobjects.
        // Or by cloned objects which refer to another original objects but still have their own position, angle, etc.

        _pOwner->ResetLastTransferredCounts();
        Draw_ApplyTransformations();

        // take care of attached objects (subobjects)
        // note that either we have our own subobjects, OR we are just a cloned object and we need to draw original object's subobjects
        PureObject3D* pWhichParent = getReferredObject() ? getReferredObject() : _pOwner;
        TPureUInt nRetSum = 0;
        for (TPureInt i = 0; i < pWhichParent->getCount(); i++)
            nRetSum += ((PureObject3D*)pWhichParent->getAttachedAt(i))->draw(PURE_RPASS_NORMAL, false, false);
        return nRetSum;
    }

    return 0;

    // actual draw here
    // todo: wtf, we should finally decide if it is renderer's responsibility to actually render an object or object's responsibility?
} // draw_DrawSW()


TPureFloat PureObject3D::PureObject3DImpl::recalculateBiggestAreaScaled()
{
    assert(_pOwner->isLevel1());

    const PureVector vScaledSizeVec = _pOwner->getScaledSizeVec();
    const TPureFloat fAreaXY = vScaledSizeVec.getX() * vScaledSizeVec.getY();
    const TPureFloat fAreaXZ = vScaledSizeVec.getX() * vScaledSizeVec.getZ();
    const TPureFloat fAreaYZ = vScaledSizeVec.getY() * vScaledSizeVec.getZ();

    if ( fAreaXY > fAreaXZ )
    {
        fBiggestAreaScaled = fAreaXY > fAreaYZ ? fAreaXY : fAreaYZ;
    }
    else
    {
        fBiggestAreaScaled = fAreaXZ > fAreaYZ ? fAreaXZ : fAreaYZ;
    }

    return fBiggestAreaScaled;
} // recalculateBiggestAreaScaled()


/*
   PureObject3D
   ###########################################################################
*/


// ############################### PUBLIC ################################


PureObject3D::~PureObject3D()
{
    delete pImpl;
    pImpl = NULL;
} // ~PureTexture()


/**
    Returns access to console preset with logger module name as this class.
    Intentionally not virtual, so the getConsole() in derived class should hide this instead of overriding.

    @return Console instance used by this class.
*/
CConsole& PureObject3D::getManagedConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
} // getConsole()


/**
    Returns the logger module name of this class.
    Intentionally not virtual, so derived class should hide this instead of overriding.
    Not even private, so user can also access this from outside, for any reason like controlling log filtering per logger module name.

    @return The logger module name of this class.
*/
const char* PureObject3D::getLoggerModuleName()
{
    return "PureObject3D";
} // getLoggerModuleName()


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, it is definitely level-1 since only level-1 objects can be cloned, and
    although they don't have their own level-2 subobject, they use the referred level-1 object's subobjects like they were their own.
*/
TPureBool PureObject3D::isLevel1() const
{
    return getReferredObject() ? true : PureMesh3D::isLevel1();
}

/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, it is definitely level-1 since only level-1 objects can be cloned, and
    although they don't have their own level-2 subobject, they use the referred level-1 object's subobjects like they were their own.
*/
TPureBool PureObject3D::isLevel2() const
{
    return !isLevel1();
}


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
TPURE_PRIMITIVE_FORMAT PureObject3D::getPrimitiveFormat() const
{
    return getReferredObject() ? getReferredObject()->getPrimitiveFormat() : PureVertexTransfer::getPrimitiveFormat();
} // getPrimitiveFormat()


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry, but it would be misleading to
    report 0.
*/
TPureUInt PureObject3D::getVerticesCount() const
{
    return getReferredObject() ? getReferredObject()->getVerticesCount() : PureVertexTransfer::getVerticesCount();
} // getVerticesCount()


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
const TXYZ* PureObject3D::getVertices(TPureBool implicitAccessSubobject) const
{
    return getReferredObject() ? getReferredObject()->getVertices(implicitAccessSubobject) : PureVertexTransfer::getVertices(implicitAccessSubobject);
} // getVertices()


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
TXYZ* PureObject3D::getVertices(TPureBool implicitAccessSubobject)
{
    return getReferredObject() ? getReferredObject()->getVertices(implicitAccessSubobject) : PureVertexTransfer::getVertices(implicitAccessSubobject);
} // getVertices()


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry, but it would be misleading to
    report 0.
*/
TPureUInt PureObject3D::getVertexIndicesCount() const
{
    return getReferredObject() ? getReferredObject()->getVertexIndicesCount() : PureVertexTransfer::getVertexIndicesCount();
}


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
const void* PureObject3D::getVertexIndices(TPureBool implicitAccessSubobject) const
{
    return getReferredObject() ? getReferredObject()->getVertexIndices(implicitAccessSubobject) : PureVertexTransfer::getVertexIndices(implicitAccessSubobject);
}


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
unsigned int PureObject3D::getVertexIndicesType(TPureBool implicitAccessSubobject) const
{
    return getReferredObject() ? getReferredObject()->getVertexIndicesType(implicitAccessSubobject) : PureVertexTransfer::getVertexIndicesType(implicitAccessSubobject);
}


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
TPureUInt PureObject3D::getMinVertexIndex(TPureBool implicitAccessSubobject) const
{
    return getReferredObject() ? getReferredObject()->getMinVertexIndex(implicitAccessSubobject) : PureVertexTransfer::getMinVertexIndex(implicitAccessSubobject);
}

/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
TPureUInt PureObject3D::getMaxVertexIndex(TPureBool implicitAccessSubobject) const
{
    return getReferredObject() ? getReferredObject()->getMaxVertexIndex(implicitAccessSubobject) : PureVertexTransfer::getMaxVertexIndex(implicitAccessSubobject);
}

/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
TPureUInt PureObject3D::getVertexIndex(TPureUInt index, TPureBool implicitAccessSubobject) const
{
    return getReferredObject() ? getReferredObject()->getVertexIndex(index, implicitAccessSubobject) : PureVertexTransfer::getVertexIndex(index, implicitAccessSubobject);
}


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
const TXYZ* PureObject3D::getNormals(TPureBool implicitAccessSubobject) const
{
    return getReferredObject() ? getReferredObject()->getNormals(implicitAccessSubobject) : PureVertexTransfer::getNormals(implicitAccessSubobject);
} // getNormals()


/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
TPureUInt PureObject3D::getFaceCount() const
{
    return getReferredObject() ? getReferredObject()->getFaceCount() : PureVertexTransfer::getFaceCount();
}

/**
    The details of this function are described in PureMesh3D class.
    Addition to PureMesh3D behavior: if this is a cloned object, the returned value is the referred object's relevant value.
    This additional behavior is needed since a cloned object doesn't have its own geometry.
*/
TPureUInt PureObject3D::getTriangleCount() const
{
    return getReferredObject() ? getReferredObject()->getTriangleCount() : PureVertexTransfer::getTriangleCount();
}


/**
    The details of this function are described in PureVertexTransfer class.
    Addition to PureVertexTransfer behavior: if this is a cloned object, the returned value is the same as the referred object's value.
*/
TPURE_VERTEX_MODIFYING_HABIT PureObject3D::getVertexModifyingHabit() const
{
    return getReferredObject() ? getReferredObject()->getVertexModifyingHabit() : PureVertexTransfer::getVertexModifyingHabit();
} // getVertexModifyingHabit()


/**
    The details of this function are described in PureVertexTransfer class.
    Addition to PureVertexTransfer behavior: if this is a cloned object, the returned value is the same as the referred object's value.
*/
TPURE_VERTEX_REFERENCING_MODE PureObject3D::getVertexReferencingMode() const
{
    return getReferredObject() ? getReferredObject()->getVertexReferencingMode() : PureVertexTransfer::getVertexReferencingMode();
} // getVertexReferencingMode()


/**
    The details of this function are described in PureVertexTransfer class.
    Addition to PureVertexTransfer behavior: if this is a cloned object, the returned value is the same as the referred object's value.
*/
TPURE_VERTEX_TRANSFER_MODE PureObject3D::getVertexTransferMode() const
{
    return getReferredObject() ? getReferredObject()->getVertexTransferMode() : PureVertexTransfer::getVertexTransferMode();
} // getVertexTransferMode()


/**
    The details of this function are described in PureVertexTransfer class.
    Same functionality as defined originally in PureVertexTransfer::setVertexTransferMode() but extended with a check for being a cloned object or not.
    This function has no effect when called for a cloned object.
    @param  vtrans Vertex referencing mode to be set.
    @return        True on success, false otherwise, including if called for a cloned object.
*/
TPureBool PureObject3D::setVertexTransferMode(TPURE_VERTEX_TRANSFER_MODE vtrans)
{
    getManagedConsole().OLnOI("PureObject3D::setVertexTransferMode(%u)", vtrans);

    if ( getReferredObject() )
    {
        getManagedConsole().EOLnOO("PureObject3D::setVertexTransferMode(%u) ignored because we are cloned object!", vtrans);
        return false;
    }

    TPureBool b = PureVertexTransfer::setVertexTransferMode(vtrans);
    if ( isLevel1() )
    {
        recalculateBiggestAreaScaled();
    }
    getManagedConsole().OO();

    return b;
} // setVertexTransferMode()


/**
    Same functionality as defined originally in PureVertexTransfer::getLastTransferredVertexCount() but extended with awareness of:
     - optional bounding box object;
     - referred object in case of cloned object.
    So depending on the render pass and occlusion testing, if the object's bounding box was sent to the graphics pipeline by draw(), its vertices will be also counted.
    If this is a cloned object, this function returns the last transferred vertex count of the referred object since the transfer of vertices actually happens
    by the referred object, so counting also happens there, so we should return that value, otherwise it would be misleading to return cloned object's value of 0.
*/
TPureUInt PureObject3D::getLastTransferredVertexCount() const
{
    TPureUInt nTransVertexCount = 0;
    if ( getReferredObject() )
    {
        nTransVertexCount = getReferredObject()->getLastTransferredVertexCount();
    }
    else
    {
        if ( getBoundingBoxObject() )
        {
            nTransVertexCount = getBoundingBoxObject()->getLastTransferredVertexCount();
        }
        nTransVertexCount += PureVertexTransfer::getLastTransferredVertexCount();
    }

    return nTransVertexCount;
} // getLastTransferredVertexCount()


/**
    Same functionality as defined originally in PureVertexTransfer::getLastTransferredTriangleCount() but extended with awareness of:
     - optional bounding box object;
     - referred object in case of cloned object.
    So depending on the render pass and occlusion testing, if the object's bounding box was sent to the graphics pipeline by draw(), its triangles will be also counted.
    If this is a cloned object, this function returns the last transferred triangle count of the referred object since the transfer of triangles actually happens
    by the referred object, so counting also happens there, so we should return that value, otherwise it would be misleading to return cloned object's value of 0.
*/
TPureUInt PureObject3D::getLastTransferredTriangleCount() const
{
    TPureUInt nTransTriangleCount = 0;
    if ( getReferredObject() )
    {
        nTransTriangleCount = getReferredObject()->getLastTransferredTriangleCount();
    }
    else
    {
        if ( getBoundingBoxObject() )
        {
            nTransTriangleCount = getBoundingBoxObject()->getLastTransferredTriangleCount();
        }
        nTransTriangleCount += PureVertexTransfer::getLastTransferredTriangleCount();
    }

    return nTransTriangleCount;
} // getLastTransferredTriangleCount()


/**
    Gets the original object which was cloned to create this object.
    @return NULL if this is a non-cloned object, otherwise the pointer to the original object which was cloned to create this cloned object.
*/
PureObject3D* PureObject3D::getReferredObject() const
{
    return pImpl->getReferredObject();
}


/**
    Gets the cloned objects referring to this object.
    @return Cloned objects referring to this object.
*/
const std::set<PureObject3D*>& PureObject3D::getReferrerObjects() const
{
    return pImpl->getReferrerObjects();
}


/**
    Gets the pointer to transformed vertices.
    These are generated only during first rendering.
    This value is irrelevant for a level-1 object since the geometry is owned by its level-2 subobjects.
    Still the returned value for a level-1 object can be a non-NULL value, see below.

    @return Pointer to transformed vertices. If the object's own vertex count is 0 but it has exactly 1 subobject, the returned
            pointer is the subobject's transformed vertices pointer. This implicit behavior is for convenience for objects storing
            only 1 subobject like internally created objects.
*/
const TPURE_TRANSFORMED_VERTEX* PureObject3D::getTransformedVertices(TPureBool implicitAccessSubobject) const
{
    return pImpl->getTransformedVertices(implicitAccessSubobject);
} // getVertices(false)


/**
    Gets the pointer to transformed vertices.
    These are generated only during first rendering.
    This value is irrelevant for a level-1 object since the geometry is owned by its level-2 subobjects.
    Still the returned value for a level-1 object can be a non-NULL value, see below.

    @return Pointer to transformed vertices. If the object's own vertex count is 0 but it has exactly 1 subobject, the returned
            pointer is the subobject's transformed vertices pointer. This implicit behavior is for convenience for objects storing
            only 1 subobject like internally created objects.
*/
TPURE_TRANSFORMED_VERTEX* PureObject3D::getTransformedVertices(TPureBool implicitAccessSubobject)
{
    return pImpl->getTransformedVertices(implicitAccessSubobject);
} // getVertices(false)


/**
    Gets the rotation angles.
    Rotation angle is currently ignored for level-2 objects, they are rotated by the same factor as their level-1 parent object using the same pivot point.
    By default this property is (0.0f, 0.0f, 0.0f).
    @return Rotation angles vector.
*/
PureVector& PureObject3D::getAngleVec()
{
    return pImpl->getAngleVec();
} // getAngleVec()


/**
    Gets the rotation angles.
    Rotation angle is currently ignored for level-2 objects, they are rotated by the same factor as their level-1 parent object using the same pivot point.
    By default this property is (0.0f, 0.0f, 0.0f).
    @return Rotation angles vector.
*/
const PureVector& PureObject3D::getAngleVec() const
{
    return pImpl->getAngleVec();
} // getAngleVec()


/**
    Gets the mesh-local relative position.
    This position tells the offset of the vertex positions relative to the center of the mesh [0,0,0].
    For level-2 (sub)mesh, this vector is always expected to be [0,0,0] since the position of submeshes is calculated with vertex positions and size.

    Addition to PureMesh3D behavior: the Mesh3D part of a cloned object does not have its own geometry, thus even Mesh3D::RecalculateSize() would calculate it to 0.
    Thus if we are a cloned object, we need to return the relative position vector of the object we are referring to.

    @return Mesh-local relative position vector.
*/
const PureVector& PureObject3D::getRelPosVec() const
{
    return (getReferredObject() == PGENULL) ? PureMesh3D::getRelPosVec() : getReferredObject()->getRelPosVec();
} // getRelPosVec()


/**
    Gets the base sizes.

    Addition to PureMesh3D behavior: the Mesh3D part of a cloned object does not have its own geometry, thus even Mesh3D::RecalculateSize() would calculate it to 0.
    Thus if we are a cloned object, we need to return the size of the object we are referring to.

    @return Base sizes vector.
*/
const PureVector& PureObject3D::getSizeVec() const
{
    return (getReferredObject() == PGENULL) ? PureMesh3D::getSizeVec() : getReferredObject()->getSizeVec();
} // getSizeVec()


/**
    Gets the real sizes considering the geometry size calculated from vertex data and the current scaling factor.
    @return Real sizes vector.
*/
PureVector PureObject3D::getScaledSizeVec() const
{
    return pImpl->getScaledSizeVec();
} // getScaledSizeVec()


/**
    Gets the scaling factor.
    By default this property is (1.0f, 1.0f, 1.0f).
    @return Scaling.
*/
const PureVector& PureObject3D::getScaling() const
{
    return pImpl->getScaling();
} // getScaling()


/**
    Sets the scaling factor to given scalar.
    Scaling factor is currently ignored for level-2 objects, they are scaled by the same factor as their level-1 parent object.
    By default this property is (1.0f, 1.0f, 1.0f).

    @param value The desired scaling factor.
*/
void PureObject3D::SetScaling(TPureFloat value)
{
    pImpl->SetScaling(value);
} // SetScaling()


/**
    Sets the scaling factor to given vector.
    Scaling factor is currently ignored for level-2 objects, they are scaled by the same factor as their level-1 parent object.
    By default this property is (1.0f, 1.0f, 1.0f).

    @param value The desired scaling factor.
*/
void PureObject3D::SetScaling(const PureVector& value)
{
    pImpl->SetScaling(value);
} // SetScaling()


/**
    Scales by the given scalar value. 
    Scaling factor is currently ignored for level-2 objects, they are scaled by the same factor as their level-1 parent object.
    By default this property is (1.0f, 1.0f, 1.0f).

    @param value The factor to be scalen by.
*/
void PureObject3D::Scale(TPureFloat value)
{
    pImpl->Scale(value);
} // Scale()


/**
    Scales by the given vector.
    Scaling factor is currently ignored for level-2 objects, they are scaled by the same factor as their level-1 parent object.
    By default this property is (1.0f, 1.0f, 1.0f).

    @param value The vector to be scalen by.
*/
void PureObject3D::Scale(const PureVector& value)
{
    pImpl->Scale(value);
} // Scale()


/**
    Gets the biggest area of the object on either plane (XY, XZ or YZ), scaled by current scaling factor.
    Note that if you change the geometry on your own without a call to setVertexTransferMode(), this may give outdated area.
    See details at recalculateBiggestAreaScaled().
    This value is used by the engine to determine which object should be an occluder. Although occluder state
    can be turned on or off manually by SetOccluder(), the engine automatically sets this state whenever recalculateBiggestAreaScaled()
    is invoked either automatically or manually.

    @return Biggest area of the object on either plane (XY, XZ or YZ), scaled by current scaling factor.
*/
TPureFloat PureObject3D::getBiggestAreaScaled() const
{
    return pImpl->getBiggestAreaScaled();
} // getBiggestAreaScaled()


/**
    Recalculates biggest area of object on either plane (XY, XZ or YZ), scaled by current scaling factor.
    This function is automatically called when scaling factor is changed or when setVertexTransferMode() is invoked.
    However, getBiggestAreaScaled() may sometimes give you stale result, for example, after you change the mesh geometry
    (e.g. through accessing them with PureMesh3D::getVertices() ), and don't invoke setVertexTransferMode() to
    upload changed geometry to host (e.g. because you keep geometry in client memory). In such case it is recommended to
    invoke this function yourself.
    This value is used by the engine to determine which object should be an occluder. Although occluder state
    can be turned on or off manually by SetOccluder(), the engine automatically sets this state whenever recalculateBiggestAreaScaled()
    is invoked either automatically or manually.

    @return The recalculated biggest area, scaled by current scaling factor.
*/
TPureFloat PureObject3D::recalculateBiggestAreaScaled()
{
    return pImpl->recalculateBiggestAreaScaled();
} // getBiggestAreaScaled()


/**
    Gets if rendering is allowed.
    If rendering is not allowed, it is not rendered.
    This property is manual user setting, not related to any visibility testing that might be done by renderer.
    This means that even if the object is seen by the camera, not occluded by other object, it won't be rendered if this property is manually set to false.
    Similarly, if the object is occluded by other objects and isOccluded() returns true, this manual visibility setting can stay true, since it is user's preference.
    By default this property is true.

    @return True if rendering is enabled, false otherwise.
*/
TPureBool PureObject3D::isRenderingAllowed() const
{
    return pImpl->isRenderingAllowed();
} // isRenderingAllowed()


/**
    Sets if rendering is allowed.
    If rendering is not allowed, it is not rendered.
    This property is manual user setting, not related to any visibility testing that might be done by renderer.
    This means that even if the object is seen by the camera, not occluded by other object, it won't be rendered if this property is manually set to false.
    Similarly, if the object is occluded by other objects and isOccluded() returns true, this manual visibility state can stay true, since it is user's preference.
    By default this property is true.
    If set to false, the object's occluder state will be also forced to false.

    @param value The desired state: true to let the object be rendered, false to hide (skip rendering) it.
*/
void PureObject3D::SetRenderingAllowed(TPureBool state)
{
    pImpl->SetRenderingAllowed(state);
} // SetRenderingAllowed()


/**
    Enables rendering of this object.
    Equivalent to SetRenderingAllowed(true). See the details there.
*/
void PureObject3D::Show()
{
    pImpl->Show();
} // Show()


/**
    Disables rendering of this object.
    Equivalent to SetRenderingAllowed(false). See the details there.
*/
void PureObject3D::Hide()
{
    pImpl->Hide();
} // Hide()


/**
    Gets whether colliding is enabled.
    Legacy function unrelated to the purpose of this class, will need to be removed!
    By default this property is false.
    @return True, if colliding is enabled, false otherwise.
*/
TPureBool PureObject3D::isColliding_TO_BE_REMOVED() const
{
    return pImpl->isColliding_TO_BE_REMOVED();
}


/**
    Sets whether colliding is enabled.
    Legacy function unrelated to the purpose of this class, will need to be removed!
    By default this property is false.

    @param value The desired state.
*/
void PureObject3D::SetColliding_TO_BE_REMOVED(TPureBool value)
{
    pImpl->SetColliding_TO_BE_REMOVED(value);
}


/**
    Gets the rotation order.
    Rotation order is currently ignored for level-2 objects, they are rotated the same way as their level-1 parent object.
    By default this property is PURE_YXZ.
    @return Rotation order.
*/
TPURE_ROTATION_ORDER PureObject3D::getRotationOrder() const
{
    return pImpl->getRotationOrder();
} // getRotationOrder()


/**
    Sets the rotation order.
    Rotation order is currently ignored for level-2 objects, they are rotated the same way as their level-1 parent object.
    By default this property is PURE_YXZ.

    @param value The desired state.
*/
void PureObject3D::SetRotationOrder(TPURE_ROTATION_ORDER value)
{
    pImpl->SetRotationOrder(value);
} // SetRotationOrder()


/**
    Gets the lit state.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is true.
    @return True if lit, false otherwise.
*/
TPureBool PureObject3D::isLit() const
{
    return pImpl->isLit();
} // isLit()


/**
    Sets the lit state.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is true.

    @param value The desired state.
*/
void PureObject3D::SetLit(TPureBool value)
{
    pImpl->SetLit(value);
} // SetLit()


/**
    Gets the double sided state.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is false.
    @return True if double sided, false otherwise.
*/
TPureBool PureObject3D::isDoubleSided() const
{
    return pImpl->isDoubleSided();
} // isDoubleSided()


/**
    Sets the double sided state.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is false.

    @param value The desired state.
*/
void PureObject3D::SetDoubleSided(TPureBool value)
{
    pImpl->SetDoubleSided(value);
} // SetDoubleSided()


/**
    Gets the wireframed state.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is false.
    @return True if wireframed, false otherwise.
*/
TPureBool PureObject3D::isWireframed() const
{
    return pImpl->isWireframed();
} // isWireframed()


/**
    Sets the wireframed state.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is false.
    If set to true, the object's occluder state will be forced to false.

    @param value The desired state.
*/
void PureObject3D::SetWireframed(TPureBool value)
{
    pImpl->SetWireframed(value);
} // SetWireframed()


/**
    Gets the wireframed culling state.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is false.

    @return True if culling is enabled in wireframed state, false otherwise.
*/
TPureBool PureObject3D::isWireframedCulled() const
{
    return pImpl->isWireframedCulled();
} // isWireframedCulled()


/**
    Sets the wireframed culling state.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is false.

    @param value The desired state.
*/
void PureObject3D::SetWireframedCulled(TPureBool value)
{
    pImpl->SetWireframedCulled(value);
} // SetWireframedCulled()


/**
    Gets whether we write to the Z-Buffer while rendering.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is true.
    @return True if we write to Z-Buffer while rendering, false otherwise.
*/
TPureBool PureObject3D::isAffectingZBuffer() const
{
    return pImpl->isAffectingZBuffer();
} // isAffectingZBuffer()


/**
    Sets whether we write to the Z-Buffer while rendering.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is true.
    If set to false, the object's occluder state will be also forced to false.

    @param value The desired state.
*/
void PureObject3D::SetAffectingZBuffer(TPureBool value)
{
    pImpl->SetAffectingZBuffer(value);
} // SetAffectingZBuffer()


/**
    Gets whether we test against the Z-Buffer while rendering.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is true.
    @return True if we test against the Z-Buffer while rendering, false otherwise.
*/
TPureBool PureObject3D::isTestingAgainstZBuffer() const
{
    return pImpl->isTestingAgainstZBuffer();
} // isTestingAgainstZBuffer()


/**
    Sets whether we test against the Z-Buffer while rendering.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is true.

    @param value The desired state.
*/
void PureObject3D::SetTestingAgainstZBuffer(TPureBool value)
{
    pImpl->SetTestingAgainstZBuffer(value);
} // SetTestingAgainstZBuffer()


/**
    Gets the sticked-to-screen state.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is false.
    @return True if sticked to screen, false otherwise.
*/
TPureBool PureObject3D::isStickedToScreen() const
{
    return pImpl->isStickedToScreen();
} // isStickedToScreen()


/**
    Sets the sticked-to-screen state.
    Sticked to screen means the object is rendered with orthogonal projection and not overlapped by non-sticked objects.
    Suitable for 2D object rendering such as GUI elements.
    Note that this call not only changes sticked state of the object but also calls the following methods:
     - SetDoubleSided(true);
     - SetLit(false);
     - SetTestingAgainstZBuffer(false).
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is false.
    If set to true, the object's occluder state will be forced to false.

    @param value The desired state.
*/
void PureObject3D::SetStickedToScreen(TPureBool value)
{
    pImpl->SetStickedToScreen(value);
} // SetStickedToScreen()


/**
    Gets whether this object should be considered as an occluder during rendering.
    Occluders are rendered before non-occluder objects.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is decided based on the size of the object compared to other objects.
    More info about this can be read at the description of SetOccluder().

    @return True if considered as occluder, false otherwise (potential occludee).
*/
TPureBool PureObject3D::isOccluder() const
{
    return pImpl->isOccluder();
} // isOccluder()


/**
    Sets whether this object should be considered as an occluder during rendering.
    Occluders are rendered before non-occluder objects.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    By default this property is decided based on the size of the object compared to other objects.
    The following properties are incompatible with occluder state:
     - hidden by user;
     - wireframed;
     - having blended material;
     - not affecting z-buffer;
     - sticked-to-screen.
    Setting occluder state to true will be ignored when an Object3D already has any of the above properties.
    
    Note that renderer might invoke PureObject3DManager::UpdateOccluderStates() that can override your manual setting.
    This function is mostly for the engine to switch occluder state, not for the user. In the future this might change though.

    @param value The desired state.
*/
void PureObject3D::SetOccluder(TPureBool value)
{
    pImpl->SetOccluder(value);
} // SetOccluder()


/**
    Gets whether this object was occluded or not based on the last finished occlusion test.
    This last state might be a few rendered frames behind the current occlusion status, as occlusion tests might not be finished in
    the same frame they are started.
    By default this property is false after creating the object, but might be changed after the first few rendered frames.

    For more details on occlusion testing, read description of SetOcclusionTested().

    @return True if last occlusion test said it was occluded, false otherwise.
            Result is always false if the object is not tested for occlusion, see isOcclusionTested() member function for more info.
*/
TPureBool PureObject3D::isOccluded() const
{
    return pImpl->isOccluded();
} // isOccluded()


/**
    Gets whether this object is being tested if it is occluded or not.
    For more details on occlusion testing, read description of SetOcclusionTested().

    @return True if occlusion test is executed for this object, false otherwise.
*/
TPureBool PureObject3D::isOcclusionTested() const
{
    return pImpl->isOcclusionTested();
} // isOcclusionTested()


/**
    Sets whether this object should be tested for occlusion or not.
    Creates or deletes an occlusion query object and bounding box for the given object.
    If it failes to generate occlusion query id, the object won't be tested for occlusion later. Check state with isOcclusionTested() member function.
    This property is currently ignored for level-2 objects, they inherit this property from their level-1 parent object.
    
    Important: if you turn occlusion testing off for an object referred by other cloned objects, the occlusion testing is also implicitly turned off
    for those referring cloned objects. However, later you can still turn occlusion testing on for the specific cloned objects if you want to, without
    affecting the occlusion testing state of the original object referred by the cloned object.

    Occlusion status of objects having occlusion test enabled are continuously checked by renderer, so rendering the scene is needed to have these test done.
    The result can be obtained with isOccluded() member function after rendering at least a few frames.
    The renderer can optimize scene rendering by not skipping occluded objects, however occlusion testing is needed to decide if an
    object is occluded or not. Too many occlusion tests might also impact performance negatively, hence it is not recommended to turn occlusion testing on for every
    single objects in the scene.
    
    By default this property is decided based on the geometric complexity of the object.
    
    Occlusion tests are curently based on hardware occlusion queries, so this is not available on all hardware.
    Usually it is available since 2003 on desktop computers.
    Mobile is not yet supported by PURE.
    Occlusion tests are not supported when software rendering is active.

    @exception std::runtime_error - In case of inability of creating bounding box object or if this object instance doesn't have manager associated.
*/
void PureObject3D::SetOcclusionTested(TPureBool state)
{
    return pImpl->SetOcclusionTested(state);
} // SetOcclusionTested()


/**
    Gets the bounding box object used for occlusion tests.
    Level-2 objects never have bounding box, only their level-1 parent object might have one.

    @return The bounding box object used for occlusion tests. It might be PGENULL, if occlusion tests are not done for this object.
*/
const PureObject3D* PureObject3D::getBoundingBoxObject() const
{
    return pImpl->getBoundingBoxObject();
} // getBoundingBoxObject()


/**
    Waits for any pending occlusion test to be finished and reset occluded state.
    This is actually a helper function for a renderer in rare cases when rendering path change requires all pending queries to be finished.
    This is called forced, so it waits for finish even when rendering path implements async occlusion query handling.
    Ignored for level-2 objects, as only level-1 objects can have occlusion test.
*/
void PureObject3D::ForceFinishOcclusionTest()
{
    pImpl->ForceFinishOcclusionTest();
    pImpl->bOccluded = false;
} // ForceFinishOcclusionTest()


/**
    Gets the amount of allocated system memory.
    It includes the allocated Material size as well (getMaterial(false)).
    Level-1 (parent) objects summarize the memory usage of their level-2 subobjects and include it in the returned value.

    @return Amount of allocated system memory in Bytes.
*/
TPureUInt PureObject3D::getUsedSystemMemory() const
{
    TPureUInt sumSubObjectMemoryUsage = 0;
    for (TPureInt i = 0; i < getSize(); i++)
    {
        if ( getAttachedAt(i) == PGENULL )
            continue;
        sumSubObjectMemoryUsage += getAttachedAt(i)->getUsedSystemMemory();
    }

    return sumSubObjectMemoryUsage +
        PureVertexTransfer::getUsedSystemMemory() - sizeof(PureVertexTransfer) + 
        sizeof(*this) + pImpl->getUsedSystemMemory();
} // getUsedSystemMemory()


/**
    The details of this function are described in VertexTransfer class.
    If this is a cloned object, the returned value is usually 0 since it just refers to the mesh- and bounding box geometries of the original object.
    Small amount of extra video memory might be also used for the bounding box object utilized by this object.

    @return Amount of allocated video memory in Bytes for storing geometry of the underlying mesh, including all subobjects.
*/
TPureUInt PureObject3D::getUsedVideoMemory() const
{
    TPureUInt vidMemory = 0;

    if ( getBoundingBoxObject() != PGENULL )
        vidMemory += getBoundingBoxObject()->getUsedVideoMemory();

    // cloned object doesnt have underlying mesh geometry
    if ( !getReferredObject() )
        vidMemory += PureVertexTransfer::getUsedVideoMemory();

    return vidMemory;
} // getUsedVideoMemory()


/**
    Draws the object.
    This is only valid from outside for level-1 objects.
    The call is ignored by level-2 subobjects. Only their level-1 parent object can call on its subobjects.

    @param renderPass Which pass/mode of rendering we are doing now for this object.
                      Some render passes depend on the effect of other passes, so order is important.
    @param bASyncQuery If true, it just checks if query is complete, but won't wait for it.
                       If false, it will wait until query is finished.
    @param bRenderIfQueryPending If true, and we do not have query result yet, the function will respond as if the object is NOT occluded.
                                 If false, and we do not have query result yet, the function will respond based on the last occlusion state of this object,
                                 meaning that: if the last finished query said the object was occluded, the function will respond as the object was occluded,
                                 otherwise it will respond as the object was not occluded.
                                 This parameter is used only with async queries.

    @return @return The number of vertices sent to the graphics pipeline.
*/
TPureUInt PureObject3D::draw(const TPURE_RENDER_PASS& renderPass, TPureBool bASyncQuery, TPureBool bRenderIfQueryPending)
{
    return pImpl->draw(renderPass, bASyncQuery, bRenderIfQueryPending);
} // Draw()


// ############################## PROTECTED ##############################


/**
    Only PureObject3DManager creates it.

    @param matMgr                A MaterialManager instance to be used for constructing of ancestor class.
    @param vmod                  What vertex modifying habit to be set for the new Object3D instance.
    @param vref                  What vertex referencing mode to be set for the new Object3D instance.
    @param bForceUseClientMemory Force-select a vertex transfer mode storing geometry in client memory instead of server memory.
                                 Please note that this is considered only if dynamic modifying habit is specified.
                                 Specifying static modifying habit will always select a mode which places geometry data into server memory.

    @exception std::bad_alloc - This class or its ancestor dynamically allocates memory with operator new, in case of failure the exception is not handled but propagated to caller.                             
*/
PureObject3D::PureObject3D(
    PureMaterialManager& matMgr,
    const TPURE_VERTEX_MODIFYING_HABIT& vmod,
    const TPURE_VERTEX_REFERENCING_MODE& vref,
    TPureBool bForceUseClientMemory ) : PureVertexTransfer(matMgr, vmod, vref, bForceUseClientMemory)
{
    getManagedConsole().OLnOI("PureObject3D() ...");
    pImpl = new PureObject3DImpl(this, vmod, vref, bForceUseClientMemory);
    getManagedConsole().SOLnOO("Done!");
} // PureObject3D()


/**
    @exception std::bad_alloc - This class or its ancestor dynamically allocates memory with operator new, in case of failure the exception is not handled but propagated to caller.
*/
PureObject3D::PureObject3D(const PureObject3D& other)
    : PureVertexTransfer(other)
{
}


PureObject3D& PureObject3D::operator=(const PureObject3D&)
{
    return *this;
}


/**
    Same functionality as defined originally in PureVertexTransfer::ResetLastTransferredCounts() but extended with awareness of:
     - optional bounding box object;
     - referred object in case of cloned object.
    If there is a bounding box object, its counters should be reset.
    If this is a cloned object, this function resets the last transferred counts of the referred object since the transfer actually happens
    by the referred object, so counting also happens there, so we should reset those values.
*/
void PureObject3D::ResetLastTransferredCounts()
{
    if ( pImpl->pBoundingBox )
    {
        pImpl->pBoundingBox->ResetLastTransferredCounts();
    }
    if ( getReferredObject() )
    {
        getReferredObject()->ResetLastTransferredCounts();
    }
    PureVertexTransfer::ResetLastTransferredCounts();
}


// ############################### PRIVATE ###############################


