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
        objFromFile = NULL;
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
        objFromFile = NULL;
        objPlane = om->createPlane(1.0f, 2.0f);
        objBox   = om->createBox(1.0f, 2.0f, 3.0f);
        objCube  = om->createCube(1.0f);

        AddSubTest("testCtor", (PFNUNITSUBTEST) &PRREObject3DTest::testCtor);
        AddSubTest("testDtor", (PFNUNITSUBTEST) &PRREObject3DTest::testDtor);
        AddSubTest("testGetReferredObject", (PFNUNITSUBTEST) &PRREObject3DTest::testGetReferredObject);
        AddSubTest("testGetVertexModifyingHabit", (PFNUNITSUBTEST) &PRREObject3DTest::testGetVertexModifyingHabit);
        AddSubTest("testSetVertexModifyingHabit", (PFNUNITSUBTEST) &PRREObject3DTest::testSetVertexModifyingHabit);
        AddSubTest("testGetVertexReferencingMode", (PFNUNITSUBTEST) &PRREObject3DTest::testGetVertexReferencingMode);
        AddSubTest("testSetVertexReferencingMode", (PFNUNITSUBTEST) &PRREObject3DTest::testSetVertexReferencingMode);
        AddSubTest("testGetVertexTransferMode", (PFNUNITSUBTEST) &PRREObject3DTest::testGetVertexTransferMode);
        AddSubTest("testSetVertexTransferMode", (PFNUNITSUBTEST) &PRREObject3DTest::testSetVertexTransferMode);
        AddSubTest("testGetTransformedVertices", (PFNUNITSUBTEST) &PRREObject3DTest::testGetTransformedVertices);
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
        objFromFile = om->createFromFile("_res/models/snail_proofps/snail.obj");
        return assertNotNull(obj, "obj null") &
            assertNotNull(objFromFile, "objFromFile null");
    }

    virtual void TearDown()
    {
        if ( obj )
        {
            delete obj;
            obj = NULL;
        }
        if ( objFromFile )
        {
            delete objFromFile;
            objFromFile = NULL;
        }
    }

    virtual void Finalize()
    {
        obj = NULL;
        objFromFile = NULL;
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
    PRREObject3D* objFromFile;
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

        delete objFromFile;
        objFromFile = NULL;

        return assertEquals(3, om->getCount());
    }

    bool testGetReferredObject()
    {
        
        PRREObject3D* const objCloned = om->createCloned(*obj);
        PRREObject3D* const objFromFileCloned = om->createCloned(*objFromFile);

        return assertNull(obj->getReferredObject(), "obj") &
            assertNull(objFromFile->getReferredObject(), "obj") &
            assertNull(objPlane->getReferredObject(), "plane") &
            assertNull(objBox->getReferredObject(), "box") &
            assertNull(objCube->getReferredObject(), "cube") &
            assertNotNull(objCloned->getReferredObject(), "refobj") &
            assertNotNull(objFromFileCloned->getReferredObject(), "refobjFromFile") &
            assertEquals(obj, objCloned->getReferredObject(), "refobjEquals") &
            assertEquals(objFromFile, objFromFileCloned->getReferredObject(), "refobjFromFileEquals");
    }

    bool testGetVertexModifyingHabit()
    {
        return assertEquals(PRRE_VMOD_STATIC, obj->getVertexModifyingHabit(), "obj") &
            assertEquals(PRRE_VMOD_STATIC, objFromFile->getVertexModifyingHabit(), "objFromFile") &
            assertEquals(PRRE_VMOD_STATIC, objPlane->getVertexModifyingHabit(), "plane") &
            assertEquals(PRRE_VMOD_STATIC, objBox->getVertexModifyingHabit(), "box") &
            assertEquals(PRRE_VMOD_STATIC, objCube->getVertexModifyingHabit(), "cube");
    }

    bool testSetVertexModifyingHabit()
    {
        objFromFile->SetVertexModifyingHabit(PRRE_VMOD_DYNAMIC);
        obj->SetVertexModifyingHabit(PRRE_VMOD_DYNAMIC);

        // subobjects must reject this when called by user
        PRREObject3D* const subobj1Obj = (PRREObject3D*)(obj->getAttachedAt(0));
        subobj1Obj->SetVertexModifyingHabit( PRRE_VMOD_STATIC );

        PRREObject3D* const subobj1ObjFromFile = (PRREObject3D*)(objFromFile->getAttachedAt(0));
        subobj1ObjFromFile->SetVertexModifyingHabit( PRRE_VMOD_STATIC );

        // cloned object must reject this in any case
        PRREObject3D* const objCloned = om->createCloned( *obj );
        objCloned->SetVertexModifyingHabit( PRRE_VMOD_STATIC );

        PRREObject3D* const objFromFileCloned = om->createCloned( *objFromFile );
        objFromFileCloned->SetVertexModifyingHabit( PRRE_VMOD_STATIC );

        return assertTrue( PRREObject3DManager::isVertexModifyingDynamic(obj->getVertexTransferMode()), "obj" ) &
            assertTrue( PRREObject3DManager::isVertexModifyingDynamic(objFromFile->getVertexTransferMode()), "objFromFile" ) &
            assertTrue( PRREObject3DManager::isVertexModifyingDynamic(subobj1Obj->getVertexTransferMode()), "subobj1Obj" ) &
            assertTrue( PRREObject3DManager::isVertexModifyingDynamic(subobj1ObjFromFile->getVertexTransferMode()), "subobj1ObjFromFile" ) &
            assertTrue( PRREObject3DManager::isVertexModifyingDynamic(objCloned->getVertexTransferMode()), "objCloned" ) &
            assertTrue( PRREObject3DManager::isVertexModifyingDynamic(objFromFileCloned->getVertexTransferMode()), "objFromFileCloned" );
    }

    bool testGetVertexReferencingMode()
    {
        return assertEquals(PRRE_VREF_DIRECT, obj->getVertexReferencingMode(), "obj") &
            assertEquals(PRRE_VREF_INDEXED, objFromFile->getVertexReferencingMode(), "objFromFile") &
            assertEquals(PRRE_VREF_DIRECT, objPlane->getVertexReferencingMode(), "plane") &
            assertEquals(PRRE_VREF_DIRECT, objBox->getVertexReferencingMode(), "box") &
            assertEquals(PRRE_VREF_DIRECT, objCube->getVertexReferencingMode(), "cube");
    }

    bool testSetVertexReferencingMode()
    {
        obj->SetVertexReferencingMode(PRRE_VREF_INDEXED);
        objFromFile->SetVertexReferencingMode(PRRE_VREF_DIRECT);

        // subobjects must reject this when called by user
        PRREObject3D* const subobj1Obj = (PRREObject3D*)(obj->getAttachedAt(0));
        subobj1Obj->SetVertexReferencingMode( PRRE_VREF_DIRECT );

        PRREObject3D* const subobj1ObjFromFile = (PRREObject3D*)(objFromFile->getAttachedAt(0));
        subobj1ObjFromFile->SetVertexReferencingMode( PRRE_VREF_INDEXED );

        // cloned object must reject this in any case
        PRREObject3D* const objCloned = om->createCloned( *obj );
        objCloned->SetVertexReferencingMode( PRRE_VREF_DIRECT );

        PRREObject3D* const objFromFileCloned = om->createCloned( *objFromFile );
        objFromFileCloned->SetVertexReferencingMode( PRRE_VREF_DIRECT );

        return assertTrue( PRREObject3DManager::isVertexReferencingIndexed(obj->getVertexTransferMode()), "obj" ) &
            assertFalse( PRREObject3DManager::isVertexReferencingIndexed(objFromFile->getVertexTransferMode()), "objFromFile" ) &
            assertTrue( PRREObject3DManager::isVertexReferencingIndexed(subobj1Obj->getVertexTransferMode()), "subobj1Obj" ) &
            assertFalse( PRREObject3DManager::isVertexReferencingIndexed(subobj1ObjFromFile->getVertexTransferMode()), "subobj1ObjFromFile" ) &
            assertTrue( PRREObject3DManager::isVertexReferencingIndexed(objCloned->getVertexTransferMode()), "objCloned" ) &
            assertFalse( PRREObject3DManager::isVertexReferencingIndexed(objFromFileCloned->getVertexTransferMode()), "objFromFileCloned" );
    }

    bool testGetVertexTransferMode()
    {
        // Generic server-side vertex array should be selected, main test machine supports it
        const TPRRE_VERTEX_TRANSFER_MODE vtExpected = PRRE_VMOD_STATIC | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);

        return assertEquals(vtExpected, obj->getVertexTransferMode() & vtExpected, "obj 1") &
            assertEquals(0u, BITF_READ(obj->getVertexTransferMode(), PRRE_VT_VENDOR_BITS, 3), "obj 2") &
            assertEquals(vtExpected, objFromFile->getVertexTransferMode() & vtExpected, "objFromFile 1") &
            assertEquals(0u, BITF_READ(objFromFile->getVertexTransferMode(), PRRE_VT_VENDOR_BITS, 3), "objFromFile 2") &
            assertEquals(vtExpected, objPlane->getVertexTransferMode() & vtExpected, "plane 1") &
            assertEquals(0u, BITF_READ(objPlane->getVertexTransferMode(), PRRE_VT_VENDOR_BITS, 3), "plane 2") &
            assertEquals(vtExpected, objBox->getVertexTransferMode() & vtExpected, "box 1") &
            assertEquals(0u, BITF_READ(objBox->getVertexTransferMode(), PRRE_VT_VENDOR_BITS, 3), "box 2") &
            assertEquals(vtExpected, objCube->getVertexTransferMode() & vtExpected, "cube 1") &
            assertEquals(0u, BITF_READ(objCube->getVertexTransferMode(), PRRE_VT_VENDOR_BITS, 3), "cube 2");
    }

    bool testSetVertexTransferMode()
    {
        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedObj = obj->getVertexTransferMode();
        obj->SetVertexTransferMode( obj->getVertexTransferMode() ); // intentionally testing setting to the same
        bool l = assertEquals(vtExpectedObj, obj->getVertexTransferMode(), "sva obj 1");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedObjFromFile = objFromFile->getVertexTransferMode();
        objFromFile->SetVertexTransferMode( objFromFile->getVertexTransferMode() ); // intentionally testing setting to the same
        l &= assertEquals(vtExpectedObjFromFile, objFromFile->getVertexTransferMode(), "sva objFromFile 1");

        // make sure the mode of the 2 objects is not just simple (PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT) (1by1 immediate mode)
        l &= assertNotEquals(PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT, obj->getVertexTransferMode(), "obj not dir 1by1");
        l &= assertNotEquals(PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT, objFromFile->getVertexTransferMode(), "objFromFile not dir 1by1");

        // subobjects must reject this when called by user
        // by default the selected transfer mode is NOT PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT hence we try set that but expect no change!
        PRREObject3D* const subobj1Obj = (PRREObject3D*)(obj->getAttachedAt(0));
        subobj1Obj->SetVertexTransferMode( PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT );
        l = assertEquals(vtExpectedObj, subobj1Obj->getVertexTransferMode(), "dir obj subobject");

        PRREObject3D* const subobj1ObjFromFile = (PRREObject3D*)(objFromFile->getAttachedAt(0));
        subobj1ObjFromFile->SetVertexTransferMode( PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT );
        l = assertEquals(vtExpectedObjFromFile, subobj1ObjFromFile->getVertexTransferMode(), "dir objFromFile subobject");

        // cloned object must reject this in any case
        // by default the selected transfer mode is NOT PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT hence we try set that but expect no change!
        PRREObject3D* const objCloned = om->createCloned( *obj );
        objCloned->SetVertexTransferMode( PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT );
        l = assertEquals(vtExpectedObj, objCloned->getVertexTransferMode(), "dir obj cloned");

        PRREObject3D* const objFromFileCloned = om->createCloned( *objFromFile );
        objFromFileCloned->SetVertexTransferMode( PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT );
        l = assertEquals(vtExpectedObjFromFile, objFromFileCloned->getVertexTransferMode(), "dir objFromFile cloned");

        // Generic server-side vertex arrays are supported by main test machine

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedStaIndSVAobj = PRRE_VMOD_STATIC  | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);
        obj->SetVertexTransferMode( vtExpectedStaIndSVAobj );
        l &= assertEquals(vtExpectedStaIndSVAobj, obj->getVertexTransferMode(), "sva obj 2");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedStaIndSVAobjFromFile = PRRE_VMOD_STATIC  | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);
        objFromFile->SetVertexTransferMode( vtExpectedStaIndSVAobjFromFile );
        l &= assertEquals(vtExpectedStaIndSVAobjFromFile, objFromFile->getVertexTransferMode(), "sva objFromFile 2");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynIndSVAobj = PRRE_VMOD_DYNAMIC  | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);
        obj->SetVertexTransferMode( vtExpectedDynIndSVAobj );
        l &= assertEquals(vtExpectedDynIndSVAobj, obj->getVertexTransferMode(), "sva obj 3");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynIndSVAobjFromFile = PRRE_VMOD_DYNAMIC  | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);
        objFromFile->SetVertexTransferMode( vtExpectedDynIndSVAobjFromFile );
        l &= assertEquals(vtExpectedDynIndSVAobjFromFile, objFromFile->getVertexTransferMode(), "sva objFromFile 3");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynDirSVAobj = PRRE_VMOD_DYNAMIC  | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);
        obj->SetVertexTransferMode( vtExpectedDynDirSVAobj );
        l &= assertEquals(vtExpectedDynDirSVAobj, obj->getVertexTransferMode(), "sva obj 4");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynDirSVAobjFromFile = PRRE_VMOD_DYNAMIC  | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT);
        objFromFile->SetVertexTransferMode( vtExpectedDynDirSVAobjFromFile );
        l &= assertEquals(vtExpectedDynDirSVAobjFromFile, objFromFile->getVertexTransferMode(), "sva objFromFile 4");

        // following modes must be supported on every machine

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynDir1by1obj = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT;
        obj->SetVertexTransferMode( vtExpectedDynDir1by1obj );
        l &= assertEquals(vtExpectedDynDir1by1obj, obj->getVertexTransferMode(), "dir obj 1b1");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynDir1by1objFromFile = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT;
        objFromFile->SetVertexTransferMode( vtExpectedDynDir1by1objFromFile );
        l &= assertEquals(vtExpectedDynDir1by1objFromFile, objFromFile->getVertexTransferMode(), "dir objFromFile 1b1");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynDirVAobj = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT);
        obj->SetVertexTransferMode( vtExpectedDynDirVAobj );
        l &= assertEquals(vtExpectedDynDirVAobj, obj->getVertexTransferMode(), "dir obj rva");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynDirVAobjFromFile = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT);
        objFromFile->SetVertexTransferMode( vtExpectedDynDirVAobjFromFile );
        l &= assertEquals(vtExpectedDynDirVAobjFromFile, objFromFile->getVertexTransferMode(), "dir objFromFile rva");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynInd1by1obj = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED;
        obj->SetVertexTransferMode( vtExpectedDynInd1by1obj );
        l &= assertEquals(vtExpectedDynInd1by1obj, obj->getVertexTransferMode(), "ind obj 1b1");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynInd1by1objFromFile = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED;
        objFromFile->SetVertexTransferMode( vtExpectedDynInd1by1objFromFile );
        l &= assertEquals(vtExpectedDynInd1by1objFromFile, objFromFile->getVertexTransferMode(), "ind objFromFile 1b1");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynIndVAobj = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT);
        obj->SetVertexTransferMode( vtExpectedDynIndVAobj );
        l &= assertEquals(vtExpectedDynIndVAobj, obj->getVertexTransferMode(), "ind obj rva");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedDynIndVAobjFromFile = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT);
        objFromFile->SetVertexTransferMode( vtExpectedDynIndVAobjFromFile );
        l &= assertEquals(vtExpectedDynIndVAobjFromFile, objFromFile->getVertexTransferMode(), "ind objFromFile rva");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedStaDirDLobj = PRRE_VMOD_STATIC | PRRE_VREF_DIRECT;
        obj->SetVertexTransferMode( vtExpectedStaDirDLobj );
        l &= assertEquals(vtExpectedStaDirDLobj, obj->getVertexTransferMode(), "dir obj DL");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedStaDirDLobjFromFile = PRRE_VMOD_STATIC | PRRE_VREF_DIRECT;
        objFromFile->SetVertexTransferMode( vtExpectedStaDirDLobjFromFile );
        l &= assertEquals(vtExpectedStaDirDLobjFromFile, objFromFile->getVertexTransferMode(), "dir objFromFile DL");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedStaIndDLobj = PRRE_VMOD_STATIC | PRRE_VREF_INDEXED;
        obj->SetVertexTransferMode( vtExpectedStaIndDLobj );
        l &= assertEquals(vtExpectedStaIndDLobj, obj->getVertexTransferMode(), "ind obj DL");

        const TPRRE_VERTEX_TRANSFER_MODE vtExpectedStaIndDLobjFromFile = PRRE_VMOD_STATIC | PRRE_VREF_INDEXED;
        objFromFile->SetVertexTransferMode( vtExpectedStaIndDLobjFromFile );
        l &= assertEquals(vtExpectedStaIndDLobjFromFile, objFromFile->getVertexTransferMode(), "ind objFromFile DL");

        return l;
    }

    bool testGetTransformedVertices()
    {
        return assertNotNull(objPlane->getTransformedVertices(), "plane") &
            assertNotNull(objBox->getTransformedVertices(), "box") &
            assertNotNull(objCube->getTransformedVertices(), "cube") &
            assertNull(objFromFile->getTransformedVertices(), "objFromFile") &
            assertEquals(((PRREObject3D*)objPlane->getAttachedAt(0))->getTransformedVertices(), objPlane->getTransformedVertices(), "plane 2") &
            assertEquals(((PRREObject3D*)objBox->getAttachedAt(0))->getTransformedVertices(), objBox->getTransformedVertices(), "box 2") &
            assertEquals(((PRREObject3D*)objCube->getAttachedAt(0))->getTransformedVertices(), objCube->getTransformedVertices(), "cube 2") &
            assertNotEquals(((PRREObject3D*)objFromFile->getAttachedAt(0))->getTransformedVertices(), objFromFile->getTransformedVertices(), "objFromFile 2") &
            assertNull(objPlane->getTransformedVertices(false), "plane noimplicit") &
            assertNull(objBox->getTransformedVertices(false), "box noimplicit") &
            assertNull(objCube->getTransformedVertices(false), "cube noimplicit") &
            assertNull(objFromFile->getTransformedVertices(false), "objFromFile noimplicit");
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
            assertEquals(0.0f, objCube->getAngleVec().getZ(), E, "cube z") &
            assertEquals(0.0f, objFromFile->getAngleVec().getX(), E, "objFromFile x") &        
            assertEquals(0.0f, objFromFile->getAngleVec().getY(), E, "objFromFile y") &
            assertEquals(0.0f, objFromFile->getAngleVec().getZ(), E, "objFromFile z");
    }

    bool testGetScaledSizeVec()
    {
        obj->SetScaling(2.0f);
        objFromFile->SetScaling(3.0f);

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
            assertEquals(6.0f, obj->getScaledSizeVec().getZ(), E, "obj z") &
            assertEquals(objFromFile->getSizeVec().getX() * objFromFile->getScaling().getX(), objFromFile->getScaledSizeVec().getX(), E, "objFromFile x") &        
            assertEquals(objFromFile->getSizeVec().getY() * objFromFile->getScaling().getY(), objFromFile->getScaledSizeVec().getY(), E, "objFromFile y") &
            assertEquals(objFromFile->getSizeVec().getZ() * objFromFile->getScaling().getZ(), objFromFile->getScaledSizeVec().getZ(), E, "objFromFile z");
    }

    bool testGetScaling()
    {
        const PRREVector vUnit(1.0f, 1.0f, 1.0f);

        const bool b1 = ( vUnit == objPlane->getScaling() );
        const bool b2 = ( vUnit == objBox->getScaling() );
        const bool b3 = ( vUnit == objCube->getScaling() );
        const bool b4 = ( vUnit == objFromFile->getScaling() );

        return assertTrue(b1, "plane") &
            assertTrue(b2, "box") &
            assertTrue(b3, "cube") &
            assertTrue(b4, "objFromFile");
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
            assertTrue(objFromFile->isVisible(), "objFromFile") &
            assertTrue(objBox->isVisible(), "box") &
            assertTrue(objCube->isVisible(), "cube");
    }

    bool testSetVisible()
    {
        obj->SetVisible(false);
        objFromFile->SetVisible(false);

        return assertFalse(obj->isVisible(), "obj") & assertFalse(objFromFile->isVisible(), "objFromFile");
    }

    bool testShow()
    {
        obj->SetVisible(false);
        obj->Show();

        objFromFile->SetVisible(false);
        objFromFile->Show();

        return assertTrue(obj->isVisible(), "obj") & assertTrue(objFromFile->isVisible(), "objFromFile");
    }

    bool testHide()
    {
        obj->Hide();
        objFromFile->Hide();

        return assertFalse(obj->isVisible(), "obj") & assertFalse(objFromFile->isVisible(), "objFromFile");
    }

    bool testIsColliding_TO_BE_REMOVED()
    {
        return assertFalse(obj->isColliding_TO_BE_REMOVED(), "obj") &
            assertFalse(objFromFile->isColliding_TO_BE_REMOVED(), "objFromFile") &
            assertFalse(objPlane->isColliding_TO_BE_REMOVED(), "plane") &
            assertFalse(objBox->isColliding_TO_BE_REMOVED(), "box") &
            assertFalse(objCube->isColliding_TO_BE_REMOVED(), "cube");
    }

    bool testSetColliding_TO_BE_REMOVED()
    {
        obj->SetColliding_TO_BE_REMOVED(true);
        objFromFile->SetColliding_TO_BE_REMOVED(true);

        return assertTrue(obj->isColliding_TO_BE_REMOVED(), "obj") & assertTrue(objFromFile->isColliding_TO_BE_REMOVED(), "objFromFile");
    }

    bool testGetRotationOrder()
    {
        return assertEquals(PRRE_YXZ, obj->getRotationOrder(), "obj") &
            assertEquals(PRRE_YXZ, objFromFile->getRotationOrder(), "objFromFile") &
            assertEquals(PRRE_YXZ, objPlane->getRotationOrder(), "plane") &
            assertEquals(PRRE_YXZ, objBox->getRotationOrder(), "box") &
            assertEquals(PRRE_YXZ, objCube->getRotationOrder(), "cube");
    }

    bool testSetRotationOrder()
    {
        obj->SetRotationOrder(PRRE_XYZ);
        objFromFile->SetRotationOrder(PRRE_XYZ);

        return assertEquals(PRRE_XYZ, obj->getRotationOrder(), "obj") & assertEquals(PRRE_XYZ, objFromFile->getRotationOrder(), "objFromFile");
    }

    bool testIsLit()
    {
        return assertTrue(obj->isLit(), "obj") &
            assertTrue(objFromFile->isLit(), "objFromFile") &
            assertTrue(objPlane->isLit(), "plane") &
            assertTrue(objBox->isLit(), "box") &
            assertTrue(objCube->isLit(), "cube");       
    }

    bool testSetLit()
    {
        obj->SetLit(false);
        objFromFile->SetLit(false);

        return assertFalse(obj->isLit(), "obj") & assertFalse(objFromFile->isLit(), "objFromFile");
    }

    bool testIsDoubleSided()
    {
        return assertFalse(obj->isDoubleSided(), "obj") &
            assertFalse(objFromFile->isDoubleSided(), "objFromFile") &
            assertFalse(objPlane->isDoubleSided(), "plane") &
            assertFalse(objBox->isDoubleSided(), "box") &
            assertFalse(objCube->isDoubleSided(), "cube");
    }

    bool testSetDoubleSided()
    {
        obj->SetDoubleSided(true);
        objFromFile->SetDoubleSided(true);

        return assertTrue(obj->isDoubleSided(), "obj") & assertTrue(objFromFile->isDoubleSided(), "objFromFile");
    }

    bool testIsWireframed()
    {
        return assertFalse(obj->isWireframed(), "obj") &
            assertFalse(objFromFile->isWireframed(), "objFromFile") &
            assertFalse(objPlane->isWireframed(), "plane") &
            assertFalse(objBox->isWireframed(), "box") &
            assertFalse(objCube->isWireframed(), "cube");
    }

    bool testSetWireframed()
    {
        obj->SetWireframed(true);
        objFromFile->SetWireframed(true);

        return assertTrue(obj->isWireframed(), "obj") & assertTrue(objFromFile->isWireframed(), "objFromFile");
    }

    bool testIsWireframedCulled()
    {
        return assertFalse(obj->isWireframedCulled(), "obj") &
            assertFalse(objFromFile->isWireframedCulled(), "objFromFile") &
            assertFalse(objPlane->isWireframedCulled(), "plane") &
            assertFalse(objBox->isWireframedCulled(), "box") &
            assertFalse(objCube->isWireframedCulled(), "cube");
    }

    bool testSetWireframedCulled()
    {
        obj->SetWireframedCulled(true);
        objFromFile->SetWireframedCulled(true);

        return assertTrue(obj->isWireframedCulled(), "obj") & assertTrue(objFromFile->isWireframedCulled(), "objFromFile");
    }

    bool testIsAffectingZBuffer()
    {
        return assertTrue(obj->isAffectingZBuffer(), "obj") &
            assertTrue(objFromFile->isAffectingZBuffer(), "objFromFile") &
            assertTrue(objPlane->isAffectingZBuffer(), "plane") &
            assertTrue(objBox->isAffectingZBuffer(), "box") &
            assertTrue(objCube->isAffectingZBuffer(), "cube"); 
    }

    bool testSetAffectingZBuffer()
    {
        obj->SetAffectingZBuffer(false);
        objFromFile->SetAffectingZBuffer(false);

        return assertFalse(obj->isAffectingZBuffer(), "obj") & assertFalse(objFromFile->isAffectingZBuffer(), "objFromFile");
    }

    bool testIsTestingAgainstZBuffer()
    {
        return assertTrue(obj->isTestingAgainstZBuffer(), "obj") &
            assertTrue(objFromFile->isTestingAgainstZBuffer(), "objFromFile") &
            assertTrue(objPlane->isTestingAgainstZBuffer(), "plane") &
            assertTrue(objBox->isTestingAgainstZBuffer(), "box") &
            assertTrue(objCube->isTestingAgainstZBuffer(), "cube");
    }

    bool testSetTestingAgainstZBuffer()
    {
        obj->SetTestingAgainstZBuffer(false);
        objFromFile->SetTestingAgainstZBuffer(false);

        return assertFalse(obj->isTestingAgainstZBuffer(), "obj") & assertFalse(objFromFile->isTestingAgainstZBuffer(), "objFromFile");
    }

    bool testIsStickedToScreen()
    {
        return assertFalse(obj->isStickedToScreen(), "obj") &
            assertFalse(objFromFile->isStickedToScreen(), "objFromFile") &
            assertFalse(objPlane->isStickedToScreen(), "plane") &
            assertFalse(objBox->isStickedToScreen(), "box") &
            assertFalse(objCube->isStickedToScreen(), "cube");
    }

    bool testSetStickedToScreen()
    {
        obj->SetStickedToScreen(true);
        objFromFile->SetStickedToScreen(true);

        return assertTrue(obj->isStickedToScreen(), "obj") & assertTrue(objFromFile->isStickedToScreen(), "objFromFile");
    }

    bool testGetUsedSystemMemory()
    {
        return assertGreater(obj->getUsedSystemMemory(),          sizeof(PRREObject3D), "obj") &
               assertGreater(objFromFile->getUsedSystemMemory(),  sizeof(PRREObject3D), "objFromFile") &
               assertGreater(objPlane->getUsedSystemMemory(),     sizeof(PRREObject3D), "plane") &
               assertGreater(objBox->getUsedSystemMemory(),       sizeof(PRREObject3D), "box") &
               assertGreater(objCube->getUsedSystemMemory(),      sizeof(PRREObject3D), "cube");
    }

    bool testGetCount()
    {
        return assertEquals(1, obj->getCount(), "obj") &
            assertEquals(9, objFromFile->getCount(), "objFromFile") &
            assertEquals(1, objPlane->getCount(), "plane") &
            assertEquals(1, objBox->getCount(), "box") &
            assertEquals(1, objCube->getCount(), "cube");
    }

    bool testIsEmpty()
    {
        return assertFalse(obj->isEmpty(), "obj") &
            assertFalse(objFromFile->isEmpty(), "objFromFile") &
            assertFalse(objPlane->isEmpty(), "plane") &
            assertFalse(objBox->isEmpty(), "box") &
            assertFalse(objCube->isEmpty(), "cube");
    }

    bool testGetSize()
    {
        return assertGequals(obj->getSize(),      obj->getCount(),         "obj") &
            assertGequals(objFromFile->getSize(), objFromFile->getCount(), "objFromFile") &
            assertGequals(objPlane->getSize(),    objPlane->getCount(),    "plane") &
            assertGequals(objBox->getSize(),      objBox->getCount(),      "box") &
            assertGequals(objCube->getSize(),     objCube->getCount(),     "cube");
    }

    bool testGetAttachedAt()
    {
        return assertNotNull(obj->getAttachedAt(0),   "obj") &
            assertNotNull(objFromFile->getAttachedAt(0),   "objFromFile") &
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