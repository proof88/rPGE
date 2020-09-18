#pragma once

/*
    ###################################################################################
    PRREImageTest.h
    Unit test for PRREObject3D.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "UnitTest.h"  // PCH
#include "../PRRE/PR00FsReducedRenderingEngine.h"
#include "../PRRE/Object3D/PRREObject3DManager.h"

#ifndef E
#define E 0.0001f
#endif // EPSILON

class PRREObject3DTest :
    public UnitTest
{
public:

    PRREObject3DTest() :
        UnitTest( __FILE__ )
    {
        engine = NULL;
        obj = NULL;
        objPlane = NULL;
        objBox = NULL;
        objCube = NULL;
    }

    ~PRREObject3DTest()
    {
        Finalize();
    }

protected:

    virtual void Initialize()
    {
        engine = &PR00FsReducedRenderingEngine::createAndGet();
        engine->initialize(PRRE_RENDERER_HW_FP, 800, 600, PRRE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems
        om = &engine->getObject3DManager();
        obj = NULL;
        objPlane = om->createPlane(1.0f, 2.0f);
        objBox   = om->createBox(1.0f, 2.0f, 3.0f);
        objCube  = om->createCube(1.0f);

        AddSubTest("testCtor", (PFNUNITSUBTEST) &PRREObject3DTest::testCtor);
        AddSubTest("testDtor", (PFNUNITSUBTEST) &PRREObject3DTest::testDtor);
        AddSubTest("testGetReferredObject", (PFNUNITSUBTEST) &PRREObject3DTest::testGetReferredObject);
        AddSubTest("testGetPrimitiveFormat", (PFNUNITSUBTEST) &PRREObject3DTest::testGetPrimitiveFormat);
        AddSubTest("testGetVertexModifyingHabit", (PFNUNITSUBTEST) &PRREObject3DTest::testGetVertexModifyingHabit);
        AddSubTest("testSetVertexModifyingHabit", (PFNUNITSUBTEST) &PRREObject3DTest::testSetVertexModifyingHabit);
        AddSubTest("testGetVertexReferencingMode", (PFNUNITSUBTEST) &PRREObject3DTest::testGetVertexReferencingMode);
        AddSubTest("testSetVertexReferencingMode", (PFNUNITSUBTEST) &PRREObject3DTest::testSetVertexReferencingMode);
        AddSubTest("testGetVertexTransferMode", (PFNUNITSUBTEST) &PRREObject3DTest::testGetVertexTransferMode);
        AddSubTest("testSetVertexTransferMode", (PFNUNITSUBTEST) &PRREObject3DTest::testSetVertexTransferMode);
        AddSubTest("testGetVerticesCount", (PFNUNITSUBTEST) &PRREObject3DTest::testGetVerticesCount);
        AddSubTest("testGetVertices", (PFNUNITSUBTEST) &PRREObject3DTest::testGetVertices);
        AddSubTest("testGetNormalsCount", (PFNUNITSUBTEST) &PRREObject3DTest::testGetNormalsCount);
        AddSubTest("testGetNormals", (PFNUNITSUBTEST) &PRREObject3DTest::testGetNormals);
        AddSubTest("testGetAngleVec", (PFNUNITSUBTEST) &PRREObject3DTest::testGetAngleVec);
        AddSubTest("testGetScaledSizeVec", (PFNUNITSUBTEST) &PRREObject3DTest::testGetScaledSizeVec);
        AddSubTest("testGetScaling", (PFNUNITSUBTEST) &PRREObject3DTest::testGetScaling);
        AddSubTest("testSetScalingToScalar", (PFNUNITSUBTEST) &PRREObject3DTest::testSetScalingToScalar);
        AddSubTest("testSetScalingToVector", (PFNUNITSUBTEST) &PRREObject3DTest::testSetScalingToVector);
        AddSubTest("testScaleByScalar", (PFNUNITSUBTEST) &PRREObject3DTest::testScaleByScalar);
        AddSubTest("testScaleByVector", (PFNUNITSUBTEST) &PRREObject3DTest::testScaleByVector);
        AddSubTest("testIsVisible", (PFNUNITSUBTEST) &PRREObject3DTest::testIsVisible);
        AddSubTest("testSetVisible", (PFNUNITSUBTEST) &PRREObject3DTest::testSetVisible);
        AddSubTest("testShow", (PFNUNITSUBTEST) &PRREObject3DTest::testShow);
        AddSubTest("testHide", (PFNUNITSUBTEST) &PRREObject3DTest::testHide);
        AddSubTest("testIsColliding_TO_BE_REMOVED", (PFNUNITSUBTEST) &PRREObject3DTest::testIsColliding_TO_BE_REMOVED);
        AddSubTest("testSetColliding_TO_BE_REMOVED", (PFNUNITSUBTEST) &PRREObject3DTest::testSetColliding_TO_BE_REMOVED);
        AddSubTest("testGetRotationOrder", (PFNUNITSUBTEST) &PRREObject3DTest::testGetRotationOrder);
        AddSubTest("testSetRotationOrder", (PFNUNITSUBTEST) &PRREObject3DTest::testSetRotationOrder);
        AddSubTest("testIsLit", (PFNUNITSUBTEST) &PRREObject3DTest::testIsLit);
        AddSubTest("testSetLit", (PFNUNITSUBTEST) &PRREObject3DTest::testSetLit);
        AddSubTest("testIsDoubleSided", (PFNUNITSUBTEST) &PRREObject3DTest::testIsDoubleSided);
        AddSubTest("testSetDoubleSided", (PFNUNITSUBTEST) &PRREObject3DTest::testSetDoubleSided);
        AddSubTest("testIsWireframed", (PFNUNITSUBTEST) &PRREObject3DTest::testIsWireframed);
        AddSubTest("testSetWireframed", (PFNUNITSUBTEST) &PRREObject3DTest::testSetWireframed);
        AddSubTest("testIsWireframedCulled", (PFNUNITSUBTEST) &PRREObject3DTest::testIsWireframedCulled);
        AddSubTest("testSetWireframedCulled", (PFNUNITSUBTEST) &PRREObject3DTest::testSetWireframedCulled);
        AddSubTest("testIsAffectingZBuffer", (PFNUNITSUBTEST) &PRREObject3DTest::testIsAffectingZBuffer);
        AddSubTest("testSetAffectingZBuffer", (PFNUNITSUBTEST) &PRREObject3DTest::testSetAffectingZBuffer);
        AddSubTest("testIsTestingAgainstZBuffer", (PFNUNITSUBTEST) &PRREObject3DTest::testIsTestingAgainstZBuffer);
        AddSubTest("testSetTestingAgainstZBuffer", (PFNUNITSUBTEST) &PRREObject3DTest::testSetTestingAgainstZBuffer);
        AddSubTest("testIsStickedToScreen", (PFNUNITSUBTEST) &PRREObject3DTest::testIsStickedToScreen);
        AddSubTest("testSetStickedToScreen", (PFNUNITSUBTEST) &PRREObject3DTest::testSetStickedToScreen);
        AddSubTest("testGetUsedSystemMemory", (PFNUNITSUBTEST) &PRREObject3DTest::testGetUsedSystemMemory);

        // since Object3D became a Manager also, we should test these capabalities here as well for possible changed behavior
        AddSubTest("testGetCount", (PFNUNITSUBTEST) &PRREObject3DTest::testGetCount);
        AddSubTest("testIsEmpty", (PFNUNITSUBTEST) &PRREObject3DTest::testIsEmpty);
        AddSubTest("testGetSize", (PFNUNITSUBTEST) &PRREObject3DTest::testGetSize);
        AddSubTest("testGetAttachedAt", (PFNUNITSUBTEST) &PRREObject3DTest::testGetAttachedAt);
        AddSubTest("testPreAlloc", (PFNUNITSUBTEST) &PRREObject3DTest::testPreAlloc);
        AddSubTest("testHasAttached2", (PFNUNITSUBTEST) &PRREObject3DTest::testHasAttached2);
        AddSubTest("testAttach", (PFNUNITSUBTEST) &PRREObject3DTest::testAttach);
        AddSubTest("testDetach", (PFNUNITSUBTEST) &PRREObject3DTest::testDetach);
        AddSubTest("testDeleteAttachedInstance", (PFNUNITSUBTEST) &PRREObject3DTest::testDeleteAttachedInstance);
        AddSubTest("testDeleteAll", (PFNUNITSUBTEST) &PRREObject3DTest::testDeleteAll);
        AddSubTest("testWriteList", (PFNUNITSUBTEST) &PRREObject3DTest::testWriteList);
    }

    virtual bool setUp()
    {
        obj = om->createBox(1.0f, 2.0f, 3.0f);
        return assertNotNull(obj, "obj null");
    }

    virtual void TearDown()
    {
        if ( obj )
        {
            delete obj;
            obj = NULL;
        }
    }

    virtual void Finalize()
    {
        obj = NULL;
        om = NULL;

        if ( engine )
        {
            engine->shutdown();
            engine = NULL;
        }
    }

private:
    PR00FsReducedRenderingEngine* engine;
    PRREObject3DManager* om;
    PRREObject3D* obj;
    const PRREObject3D* objPlane,
                      * objBox,
                      * objCube;

    // ---------------------------------------------------------------------------

    PRREObject3DTest(const PRREObject3DTest&)
    {};         

    PRREObject3DTest& operator=(const PRREObject3DTest&)
    {
        return *this;
    };

    bool testCtor()
    {
        return true;
    }

    bool testDtor()
    {
        delete obj;
        obj = NULL;
        return assertEquals(3, om->getCount());
    }

    bool testGetReferredObject()
    {
        
        PRREObject3D* const objCloned = om->createCloned(*obj);

        return assertNull(obj->getReferredObject(), "obj") &
            assertNull(objPlane->getReferredObject(), "plane") &
            assertNull(objBox->getReferredObject(), "box") &
            assertNull(objCube->getReferredObject(), "cube") &
            assertNotNull(objCloned->getReferredObject(), "refobj") &
            assertEquals(obj, objCloned->getReferredObject(), "refequals");
    }

    bool testGetPrimitiveFormat()
    {
        return assertEquals(PRRE_PM_QUADS, obj->getPrimitiveFormat(), "obj") &
            assertEquals(PRRE_PM_QUADS, objPlane->getPrimitiveFormat(), "plane") &
            assertEquals(PRRE_PM_QUADS, objBox->getPrimitiveFormat(), "box") &
            assertEquals(PRRE_PM_QUADS, objCube->getPrimitiveFormat(), "cube");
    }

    bool testGetVertexModifyingHabit()
    {
        return assertEquals(PRRE_VMOD_STATIC, obj->getVertexModifyingHabit(), "obj") &
            assertEquals(PRRE_VMOD_STATIC, objPlane->getVertexModifyingHabit(), "plane") &
            assertEquals(PRRE_VMOD_STATIC, objBox->getVertexModifyingHabit(), "box") &
            assertEquals(PRRE_VMOD_STATIC, objCube->getVertexModifyingHabit(), "cube");
    }

    bool testSetVertexModifyingHabit()
    {
        obj->SetVertexModifyingHabit(PRRE_VMOD_DYNAMIC);

        return assertTrue( PRREObject3DManager::isVertexModifyingDynamic(obj->getVertexTransferMode()) );
    }

    bool testGetVertexReferencingMode()
    {
        return assertEquals(PRRE_VREF_DIRECT, obj->getVertexReferencingMode(), "obj") &
            assertEquals(PRRE_VREF_DIRECT, objPlane->getVertexReferencingMode(), "plane") &
            assertEquals(PRRE_VREF_DIRECT, objBox->getVertexReferencingMode(), "box") &
            assertEquals(PRRE_VREF_DIRECT, objCube->getVertexReferencingMode(), "cube");
    }

    bool testSetVertexReferencingMode()
    {
        obj->SetVertexReferencingMode(PRRE_VREF_INDEXED);

        return assertTrue( PRREObject3DManager::isVertexReferencingIndexed(obj->getVertexTransferMode()) );
    }

    bool testGetVertexTransferMode()
    {
        // Generic server-side vertex array should be selected, main test machine supports it
        const TPRRE_VERTEX_TRANSFER_MODE vtExpected = PRRE_VMOD_STATIC | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);

        return assertEquals(vtExpected, obj->getVertexTransferMode() & vtExpected, "obj 1") &
            assertEquals(0u, BITF_READ(obj->getVertexTransferMode(), PRRE_VT_VENDOR_BITS, 3), "obj 2") &
            assertEquals(vtExpected, objPlane->getVertexTransferMode() & vtExpected, "plane 1") &
            assertEquals(0u, BITF_READ(objPlane->getVertexTransferMode(), PRRE_VT_VENDOR_BITS, 3), "plane 2") &
            assertEquals(vtExpected, objBox->getVertexTransferMode() & vtExpected, "box 1") &
            assertEquals(0u, BITF_READ(objBox->getVertexTransferMode(), PRRE_VT_VENDOR_BITS, 3), "box 2") &
            assertEquals(vtExpected, objCube->getVertexTransferMode() & vtExpected, "cube 1") &
            assertEquals(0u, BITF_READ(objCube->getVertexTransferMode(), PRRE_VT_VENDOR_BITS, 3), "cube 2");
    }

    bool testSetVertexTransferMode()
    {
        const TPRRE_VERTEX_TRANSFER_MODE vtExpected = obj->getVertexTransferMode();
        obj->SetVertexTransferMode( obj->getVertexTransferMode() ); // intentionally testing setting to the same
        bool l = assertEquals(vtExpected, obj->getVertexTransferMode(), "sva 1");

        // Generic server-side vertex arrays are supported by main test machine

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedStaIndSVA = PRRE_VMOD_STATIC  | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);
        obj->SetVertexTransferMode( vtExpectedStaIndSVA );
        l &= assertEquals(vtExpectedStaIndSVA, obj->getVertexTransferMode(), "sva 2");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynIndSVA = PRRE_VMOD_DYNAMIC  | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);
        obj->SetVertexTransferMode( vtExpectedDynIndSVA );
        l &= assertEquals(vtExpectedDynIndSVA, obj->getVertexTransferMode(), "sva 3");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynDirSVA = PRRE_VMOD_DYNAMIC  | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);
        obj->SetVertexTransferMode( vtExpectedDynDirSVA );
        l &= assertEquals(vtExpectedDynDirSVA, obj->getVertexTransferMode(), "sva 4");

        // following modes must be supported on every machine

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynDir1by1 = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT;
        obj->SetVertexTransferMode( vtExpectedDynDir1by1 );
        l &= assertEquals(vtExpectedDynDir1by1, obj->getVertexTransferMode(), "dir 1b1");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynDirVA = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT);
        obj->SetVertexTransferMode( vtExpectedDynDirVA );
        l &= assertEquals(vtExpectedDynDirVA, obj->getVertexTransferMode(), "dir rva");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynInd1by1 = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED;
        obj->SetVertexTransferMode( vtExpectedDynInd1by1 );
        l &= assertEquals(vtExpectedDynInd1by1, obj->getVertexTransferMode(), "ind 1b1");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynIndVA = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT);
        obj->SetVertexTransferMode( vtExpectedDynIndVA );
        l &= assertEquals(vtExpectedDynIndVA, obj->getVertexTransferMode(), "ind rva");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedStaDirDL = PRRE_VMOD_STATIC | PRRE_VREF_DIRECT;
        obj->SetVertexTransferMode( vtExpectedStaDirDL );
        l &= assertEquals(vtExpectedStaDirDL, obj->getVertexTransferMode(), "dir DL");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedStaIndDL = PRRE_VMOD_STATIC | PRRE_VREF_INDEXED;
        obj->SetVertexTransferMode( vtExpectedStaIndDL );
        l &= assertEquals(vtExpectedStaIndDL, obj->getVertexTransferMode(), "ind DL");

        return l;
    }

    bool testGetVerticesCount()
    {
        // comes from Mesh3D but make sure behavior of this functionality stayed the same
        return assertEquals((TPRREuint) 4, objPlane->getVerticesCount(), "plane") &
            assertEquals((TPRREuint) 24, objBox->getVerticesCount(), "box") &
            assertEquals((TPRREuint) 24, objCube->getVerticesCount(), "cube") &
            assertEquals(((PRREObject3D*)objPlane->getAttachedAt(0))->getVerticesCount(), objPlane->getVerticesCount(), "plane 2") &
            assertEquals(((PRREObject3D*)objBox->getAttachedAt(0))->getVerticesCount(), objBox->getVerticesCount(), "box 2") &
            assertEquals(((PRREObject3D*)objCube->getAttachedAt(0))->getVerticesCount(), objCube->getVerticesCount(), "cube 2") &
            assertEquals((TPRREuint) 0, objPlane->getVerticesCount(false), "plane noimplicit") &
            assertEquals((TPRREuint) 0, objBox->getVerticesCount(false), "box noimplicit") &
            assertEquals((TPRREuint) 0, objCube->getVerticesCount(false), "cube noimplicit");
    }

    bool testGetVertices()
    {
        // comes from Mesh3D but make sure behavior of this functionality stayed the same
        return assertNotNull(objPlane->getVertices(), "plane") &
            assertNotNull(objBox->getVertices(), "box") &
            assertNotNull(objCube->getVertices(), "cube") &
            assertEquals(((PRREObject3D*)objPlane->getAttachedAt(0))->getVertices(), objPlane->getVertices(), "plane 2") &
            assertEquals(((PRREObject3D*)objBox->getAttachedAt(0))->getVertices(), objBox->getVertices(), "box 2") &
            assertEquals(((PRREObject3D*)objCube->getAttachedAt(0))->getVertices(), objCube->getVertices(), "cube 2") &
            assertNull(objPlane->getVertices(false), "plane noimplicit") &
            assertNull(objBox->getVertices(false), "box noimplicit") &
            assertNull(objCube->getVertices(false), "cube noimplicit");
    }

    bool testGetNormalsCount()
    {
        // comes from Mesh3D but make sure behavior of this functionality stayed the same
        return assertEquals((TPRREuint) 4, objPlane->getNormalsCount(), "plane") &
            assertEquals((TPRREuint) 24, objBox->getNormalsCount(), "box") &
            assertEquals((TPRREuint) 24, objCube->getNormalsCount(), "cube") &
            assertEquals(((PRREObject3D*)objPlane->getAttachedAt(0))->getNormalsCount(), objPlane->getNormalsCount(), "plane 2") &
            assertEquals(((PRREObject3D*)objBox->getAttachedAt(0))->getNormalsCount(), objBox->getNormalsCount(), "box 2") &
            assertEquals(((PRREObject3D*)objCube->getAttachedAt(0))->getNormalsCount(), objCube->getNormalsCount(), "cube 2") &
            assertEquals((TPRREuint) 0, objPlane->getNormalsCount(false), "plane noimplicit") &
            assertEquals((TPRREuint) 0, objBox->getNormalsCount(false), "box noimplicit") &
            assertEquals((TPRREuint) 0, objCube->getNormalsCount(false), "cube noimplicit");
    }

    bool testGetNormals()
    {
        // comes from Mesh3D but make sure behavior of this functionality stayed the same
        return assertNotNull(objPlane->getNormals(), "plane") &
            assertNotNull(objBox->getNormals(), "box") &
            assertNotNull(objCube->getNormals(), "cube") &
            assertEquals(((PRREObject3D*)objPlane->getAttachedAt(0))->getNormals(), objPlane->getNormals(), "plane 2") &
            assertEquals(((PRREObject3D*)objBox->getAttachedAt(0))->getNormals(), objBox->getNormals(), "box 2") &
            assertEquals(((PRREObject3D*)objCube->getAttachedAt(0))->getNormals(), objCube->getNormals(), "cube 2") &
            assertNull(objPlane->getNormals(false), "plane noimplicit") &
            assertNull(objBox->getNormals(false), "box noimplicit") &
            assertNull(objCube->getNormals(false), "cube noimplicit");
    }

    bool testGetAngleVec()
    {
        return assertEquals(0.0f, objPlane->getAngleVec().getX(), E, "plane x") &        
            assertEquals(0.0f, objPlane->getAngleVec().getY(), E, "plane y") &
            assertEquals(0.0f, objPlane->getAngleVec().getZ(), E, "plane z") &
            assertEquals(0.0f, objBox->getAngleVec().getX(), E, "box x") &        
            assertEquals(0.0f, objBox->getAngleVec().getY(), E, "box y") &
            assertEquals(0.0f, objBox->getAngleVec().getZ(), E, "box z") &
            assertEquals(0.0f, objCube->getAngleVec().getX(), E, "cube x") &        
            assertEquals(0.0f, objCube->getAngleVec().getY(), E, "cube y") &
            assertEquals(0.0f, objCube->getAngleVec().getZ(), E, "cube z");
    }

    bool testGetScaledSizeVec()
    {
        obj->SetScaling(2.0f);

        return assertEquals(1.0f, objPlane->getScaledSizeVec().getX(), E, "plane x") &        
            assertEquals(2.0f, objPlane->getScaledSizeVec().getY(), E, "plane y") &
            assertEquals(0.0f, objPlane->getScaledSizeVec().getZ(), E, "plane z") &
            assertEquals(1.0f, objBox->getScaledSizeVec().getX(), E, "box x") &        
            assertEquals(2.0f, objBox->getScaledSizeVec().getY(), E, "box y") &
            assertEquals(3.0f, objBox->getScaledSizeVec().getZ(), E, "box z") &
            assertEquals(1.0f, objCube->getScaledSizeVec().getX(), E, "cube x") &        
            assertEquals(1.0f, objCube->getScaledSizeVec().getY(), E, "cube y") &
            assertEquals(1.0f, objCube->getScaledSizeVec().getZ(), E, "cube z") &
            assertEquals(2.0f, obj->getScaledSizeVec().getX(), E, "obj x") &        
            assertEquals(4.0f, obj->getScaledSizeVec().getY(), E, "obj y") &
            assertEquals(6.0f, obj->getScaledSizeVec().getZ(), E, "obj z");
    }

    bool testGetScaling()
    {
        const PRREVector vUnit(1.0f, 1.0f, 1.0f);

        const bool b1 = ( vUnit == objPlane->getScaling() );
        const bool b2 = ( vUnit == objBox->getScaling() );
        const bool b3 = ( vUnit == objCube->getScaling() );

        return assertTrue(b1, "plane") &
            assertTrue(b2, "box") &
            assertTrue(b3, "cube");
    }

    bool testSetScalingToScalar()
    {
        obj->SetScaling(3.0f);
        return assertEquals(3.0f, obj->getScaling().getX(), E, "X") &
            assertEquals(3.0f, obj->getScaling().getY(), E, "Y") &
            assertEquals(3.0f, obj->getScaling().getZ(), E, "Z");
    }

    bool testSetScalingToVector()
    {
        const PRREVector vNewScaling(2.0f, 3.0f, 4.0f);
        obj->SetScaling(vNewScaling);

        return assertEquals(2.0f, obj->getScaling().getX(), E, "X") &
            assertEquals(3.0f, obj->getScaling().getY(), E, "Y") &
            assertEquals(4.0f, obj->getScaling().getZ(), E, "Z");
    }

    bool testScaleByScalar()
    {
        obj->Scale(3.0f);
        bool b = assertEquals(3.0f, obj->getScaling().getX(), E, "b X") &
            assertEquals(3.0f, obj->getScaling().getY(), E, "b Y") &
            assertEquals(3.0f, obj->getScaling().getZ(), E, "b Z");

        obj->Scale(2.5f);
        b &= assertEquals(3.0f*2.5f, obj->getScaling().getX(), E, "b2 X") &
            assertEquals(3.0f*2.5f, obj->getScaling().getY(), E, "b2 Y") &
            assertEquals(3.0f*2.5f, obj->getScaling().getZ(), E, "b2 Z");
        
        return b;
    }

    bool testScaleByVector()
    {
        const PRREVector vScaleBy(2.0f, 3.0f, 4.0f);

        obj->Scale(vScaleBy);
        bool b = assertEquals(2.0f, obj->getScaling().getX(), E, "b X") &
            assertEquals(3.0f, obj->getScaling().getY(), E, "b Y") &
            assertEquals(4.0f, obj->getScaling().getZ(), E, "b Z");

        obj->Scale(vScaleBy);
        b &= assertEquals(2.0f*2.0f, obj->getScaling().getX(), E, "b2 X") &
            assertEquals(3.0f*3.0f, obj->getScaling().getY(), E, "b2 Y") &
            assertEquals(4.0f*4.0f, obj->getScaling().getZ(), E, "b2 Z");

        return b;
    }

    bool testIsVisible()
    {
        return assertTrue(objPlane->isVisible(), "plane") &
            assertTrue(objBox->isVisible(), "box") &
            assertTrue(objCube->isVisible(), "cube");
    }

    bool testSetVisible()
    {
        obj->SetVisible(false);

        return assertFalse(obj->isVisible());
    }

    bool testShow()
    {
        obj->SetVisible(false);
        obj->Show();

        return assertTrue(obj->isVisible());       
    }

    bool testHide()
    {
        obj->Hide();

        return assertFalse(obj->isVisible());
    }

    bool testIsColliding_TO_BE_REMOVED()
    {
        return assertFalse(obj->isColliding_TO_BE_REMOVED(), "obj") &
            assertFalse(objPlane->isColliding_TO_BE_REMOVED(), "plane") &
            assertFalse(objBox->isColliding_TO_BE_REMOVED(), "box") &
            assertFalse(objCube->isColliding_TO_BE_REMOVED(), "cube");
    }

    bool testSetColliding_TO_BE_REMOVED()
    {
        obj->SetColliding_TO_BE_REMOVED(true);

        return assertTrue(obj->isColliding_TO_BE_REMOVED());
    }

    bool testGetRotationOrder()
    {
        return assertEquals(PRRE_YXZ, obj->getRotationOrder(), "obj") &
            assertEquals(PRRE_YXZ, objPlane->getRotationOrder(), "plane") &
            assertEquals(PRRE_YXZ, objBox->getRotationOrder(), "box") &
            assertEquals(PRRE_YXZ, objCube->getRotationOrder(), "cube");
    }

    bool testSetRotationOrder()
    {
        obj->SetRotationOrder(PRRE_XYZ);

        return assertEquals(PRRE_XYZ, obj->getRotationOrder());
    }

    bool testIsLit()
    {
        return assertTrue(obj->isLit(), "obj") &
            assertTrue(objPlane->isLit(), "plane") &
            assertTrue(objBox->isLit(), "box") &
            assertTrue(objCube->isLit(), "cube");       
    }

    bool testSetLit()
    {
        obj->SetLit(false);

        return assertFalse(obj->isLit());
    }

    bool testIsDoubleSided()
    {
        return assertFalse(obj->isDoubleSided(), "obj") &
            assertFalse(objPlane->isDoubleSided(), "plane") &
            assertFalse(objBox->isDoubleSided(), "box") &
            assertFalse(objCube->isDoubleSided(), "cube");
    }

    bool testSetDoubleSided()
    {
        obj->SetDoubleSided(true);

        return assertTrue(obj->isDoubleSided());
    }

    bool testIsWireframed()
    {
        return assertFalse(obj->isWireframed(), "obj") &
            assertFalse(objPlane->isWireframed(), "plane") &
            assertFalse(objBox->isWireframed(), "box") &
            assertFalse(objCube->isWireframed(), "cube");
    }

    bool testSetWireframed()
    {
        obj->SetWireframed(true);

        return assertTrue(obj->isWireframed());
    }

    bool testIsWireframedCulled()
    {
        return assertFalse(obj->isWireframedCulled(), "obj") &
            assertFalse(objPlane->isWireframedCulled(), "plane") &
            assertFalse(objBox->isWireframedCulled(), "box") &
            assertFalse(objCube->isWireframedCulled(), "cube");
    }

    bool testSetWireframedCulled()
    {
        obj->SetWireframedCulled(true);

        return assertTrue(obj->isWireframedCulled());
    }

    bool testIsAffectingZBuffer()
    {
        return assertTrue(obj->isAffectingZBuffer(), "obj") &
            assertTrue(objPlane->isAffectingZBuffer(), "plane") &
            assertTrue(objBox->isAffectingZBuffer(), "box") &
            assertTrue(objCube->isAffectingZBuffer(), "cube"); 
    }

    bool testSetAffectingZBuffer()
    {
        obj->SetAffectingZBuffer(false);

        return assertFalse(obj->isAffectingZBuffer());
    }

    bool testIsTestingAgainstZBuffer()
    {
        return assertTrue(obj->isTestingAgainstZBuffer(), "obj") &
            assertTrue(objPlane->isTestingAgainstZBuffer(), "plane") &
            assertTrue(objBox->isTestingAgainstZBuffer(), "box") &
            assertTrue(objCube->isTestingAgainstZBuffer(), "cube");
    }

    bool testSetTestingAgainstZBuffer()
    {
        obj->SetTestingAgainstZBuffer(false);

        return assertFalse(obj->isTestingAgainstZBuffer());
    }

    bool testIsStickedToScreen()
    {
        return assertFalse(obj->isStickedToScreen(), "obj") &
            assertFalse(objPlane->isStickedToScreen(), "plane") &
            assertFalse(objBox->isStickedToScreen(), "box") &
            assertFalse(objCube->isStickedToScreen(), "cube");
    }

    bool testSetStickedToScreen()
    {
        obj->SetStickedToScreen(true);

        return assertTrue(obj->isStickedToScreen());
    }

    bool testGetUsedSystemMemory()
    {
        return assertGreater(obj->getUsedSystemMemory(),      sizeof(PRREObject3D), "obj") &
               assertGreater(objPlane->getUsedSystemMemory(), sizeof(PRREObject3D), "plane") &
               assertGreater(objBox->getUsedSystemMemory(),   sizeof(PRREObject3D), "box") &
               assertGreater(objCube->getUsedSystemMemory(),  sizeof(PRREObject3D), "cube");
    }

    bool testGetCount()
    {
        return assertEquals(1, obj->getCount(), "obj") &
            assertEquals(1, objPlane->getCount(), "plane") &
            assertEquals(1, objBox->getCount(), "box") &
            assertEquals(1, objCube->getCount(), "cube");
    }

    bool testIsEmpty()
    {
        return assertFalse(obj->isEmpty(), "obj") &
            assertFalse(objPlane->isEmpty(), "plane") &
            assertFalse(objBox->isEmpty(), "box") &
            assertFalse(objCube->isEmpty(), "cube");
    }

    bool testGetSize()
    {
        return assertGequals(obj->getSize(),   obj->getCount(),      "obj") &
            assertGequals(objPlane->getSize(), objPlane->getCount(), "plane") &
            assertGequals(objBox->getSize(),   objBox->getCount(),   "box") &
            assertGequals(objCube->getSize(),  objCube->getCount(),  "cube");
    }

    bool testGetAttachedAt()
    {
        return assertNotNull(obj->getAttachedAt(0),   "obj") &
            assertNotNull(objPlane->getAttachedAt(0), "plane") &
            assertNotNull(objBox->getAttachedAt(0),   "box") &
            assertNotNull(objCube->getAttachedAt(0),  "cube");
    }

    bool testPreAlloc()
    {
        const TPRREint prevSize = obj->getSize();
        obj->PreAlloc(10);

        return assertEquals(prevSize + 10, obj->getSize());
    }

    bool testHasAttached2()
    {
        PRREObject3D* const mgd1 = om->createCube(1);
        mgd1->DetachFrom();
        obj->Attach(*mgd1);

        return assertFalse(obj->hasAttached(*objPlane), "objPlane") &
               assertTrue(obj->hasAttached(*mgd1), "mgd1");
    }

    bool testAttach()
    {
        PRREObject3D& mgr1 = *(om->createCube(1));
        PRREObject3D& mgr2 = *(om->createCube(1));
        obj->DetachFrom();
        mgr1.Attach(*obj);
        mgr2.Attach(*obj);

        return assertTrue(mgr1.hasAttached(*obj), "mgr1.hasAttached()") &
               assertEquals(2, mgr1.getCount(), "mgr1.getCount()") &
               assertFalse(mgr2.hasAttached(*obj), "mgr2.hasAttached()") &
               assertEquals(1, mgr2.getCount(), "mgr2.getCount()");
    }

    bool testDetach()
    {
        PRREObject3D& mgd = *((PRREObject3D*) (obj->getAttachedAt(0)));
        obj->Detach(mgd);
        bool l = assertEquals(0, obj->getCount());
        obj->Attach(mgd);

        return l;
    }

    bool testDeleteAttachedInstance()
    {
        PRREObject3D& mgd = *((PRREObject3D*) (obj->getAttachedAt(0)));
        obj->DeleteAttachedInstance(mgd);

        return assertEquals(0, obj->getCount());
    }

    bool testDeleteAll()
    {
        PRREObject3D* const mgd1 = om->createCube(1), * const mgd2 = om->createCube(1);
        obj->DeleteAll();
        bool l = assertEquals(0, obj->getCount(), "getCount 1");

        obj->Attach(*mgd1);
        obj->Attach(*mgd2);
        obj->DeleteAll();
        l &= assertEquals(0, obj->getCount(), "getCount 2");

        return l;
    }

    bool testWriteList()
    {
        obj->WriteList();

        return true;
    }

}; // class PRREObject3DTest