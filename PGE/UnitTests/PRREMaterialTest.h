#pragma once

/*
    ###################################################################################
    PRREMaterialTest.h
    Unit test for PRREMaterial.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "UnitTest.h"  // PCH
#include "../PRRE/include/external/Material/PRREMaterialManager.h"
#include "../PRRE/include/external/PR00FsReducedRenderingEngine.h"

#ifndef E
#define E 0.0001f
#endif // EPSILON

#ifndef BMP128x128x24
#define BMP128x128x24 "_res/proba128x128x24.bmp"
#endif 

class PRRESampleUtiliserManaged :
    public PRREManaged
{
public:
    PRRESampleUtiliserManaged() {}; /* TODO: mark this as noexcept(false) when using newer compiler! */
    virtual ~PRRESampleUtiliserManaged() {};

protected:

    // ---------------------------------------------------------------------------

    PRRESampleUtiliserManaged(const PRRESampleUtiliserManaged&) {};
    PRRESampleUtiliserManaged& operator=(const PRRESampleUtiliserManaged& ) { return *this; };

private:

    // ---------------------------------------------------------------------------

}; // class PRRESampleUtiliserManaged

class PRRESampleManagerForUtiliserManaged :
    public PRREManager
{

public:
    PRRESampleManagerForUtiliserManaged() : bHandleManagedPropertyChangedInvoked(false) {}; /* TODO: mark this as noexcept(false) when using newer compiler! */
    virtual ~PRRESampleManagerForUtiliserManaged() {};

    PRRESampleUtiliserManaged* createManaged()
    {
        PRRESampleUtiliserManaged* pNewManaged = PGENULL;
        try
        {
            pNewManaged = new PRRESampleUtiliserManaged();
            Attach( *pNewManaged );
        }
        catch (const std::bad_alloc&)
        {
            getConsole().EOLn("ERROR: failed to instantiate PRRESampleUtiliserManaged!");
            getConsole().OLn("");
        }

        return pNewManaged;
    } // createManaged()

    virtual void HandleManagedPropertyChanged(PRREManaged&)
    {
        bHandleManagedPropertyChangedInvoked = true;
    } // HandleManagedPropertyChanged()

    bool isHandleManagedPropertyChangedInvoked() const
    {
        return bHandleManagedPropertyChangedInvoked;
    }

    void ResetHandleManagedPropertyChangedInvoked()
    {
        bHandleManagedPropertyChangedInvoked = false;
    }

protected:

private:
    bool bHandleManagedPropertyChangedInvoked;

}; // class PRRESampleManagerForUtiliserManaged

class PRREMaterialTest :
    public UnitTest
{
public:

    PRREMaterialTest() :
        UnitTest(__FILE__)
    {
        engine = NULL;
        tm = NULL;
        mat = NULL;
    }

    virtual ~PRREMaterialTest()
    {
        Finalize();
    } // ~PRREMaterialTest()

protected:

    virtual void Initialize()
    {
        /*
        CConsole::getConsoleInstance().SetLoggingState(PRREMaterial::getLoggerModuleName(), true);
        CConsole::getConsoleInstance().SetLoggingState(PRREMaterialManager::getLoggerModuleName(), true);
        CConsole::getConsoleInstance().SetLoggingState(PRRETexture::getLoggerModuleName(), true);
        CConsole::getConsoleInstance().SetLoggingState(PRRETextureManager::getLoggerModuleName(), true);
        CConsole::getConsoleInstance().SetLoggingState(PRREImage::getLoggerModuleName(), true);
        CConsole::getConsoleInstance().SetLoggingState(PRREImageManager::getLoggerModuleName(), true);
        */
        engine = &PR00FsReducedRenderingEngine::createAndGet();
        engine->initialize(PRRE_RENDERER_HW_FP, 800, 600, PRRE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems
        tm = &engine->getTextureManager();
        mm = &engine->getMaterialManager();

        AddSubTest("testIsBlendFuncBlends", (PFNUNITSUBTEST) &PRREMaterialTest::testIsBlendFuncBlends);
        AddSubTest("testCtor", (PFNUNITSUBTEST) &PRREMaterialTest::testCtor);
        AddSubTest("testDtor", (PFNUNITSUBTEST) &PRREMaterialTest::testDtor);
        AddSubTest("testAllocateArrays", (PFNUNITSUBTEST) &PRREMaterialTest::testAllocateArrays);
        AddSubTest("testSetTexture", (PFNUNITSUBTEST) &PRREMaterialTest::testSetTexture);
        AddSubTest("testGetTextureCount", (PFNUNITSUBTEST) &PRREMaterialTest::testGetTextureCount);
        AddSubTest("testIsTextured", (PFNUNITSUBTEST) &PRREMaterialTest::testIsTextured);
        AddSubTest("testIsSingleTextured", (PFNUNITSUBTEST) &PRREMaterialTest::testIsSingleTextured);
        AddSubTest("testIsMultiTextured", (PFNUNITSUBTEST) &PRREMaterialTest::testIsMultiTextured);
        AddSubTest("testSetSourceBlendFunc", (PFNUNITSUBTEST) &PRREMaterialTest::testSetSourceBlendFunc);
        AddSubTest("testSetDestinationBlendFunc", (PFNUNITSUBTEST) &PRREMaterialTest::testSetDestinationBlendFunc);
        AddSubTest("testSetBlendFuncs", (PFNUNITSUBTEST) &PRREMaterialTest::testSetBlendFuncs);
        AddSubTest("testSetBlendMode", (PFNUNITSUBTEST) &PRREMaterialTest::testSetBlendMode);
        AddSubTest("testCopyFromMaterial", (PFNUNITSUBTEST) &PRREMaterialTest::testCopyFromMaterial);
        AddSubTest("testGetUsedSystemMemory", (PFNUNITSUBTEST) &PRREMaterialTest::testGetUsedSystemMemory);

    }

    virtual bool setUp()
    {
        mat = mm->createMaterial();
        return assertNotNull(mat, "mat null");
    }

    virtual void TearDown()
    {
        mm->DeleteAll();
        mat = NULL;
    }

    virtual void Finalize()
    {
        tm = NULL;
        mm = NULL;
        mat = NULL;

        if ( engine )
        {
            engine->shutdown();
            engine = NULL;
        }
        CConsole::getConsoleInstance().SetLoggingState(PRREMaterial::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PRREMaterialManager::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PRRETexture::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PRRETextureManager::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PRREImage::getLoggerModuleName(), false);
        CConsole::getConsoleInstance().SetLoggingState(PRREImageManager::getLoggerModuleName(), false);
    }

private:
    PR00FsReducedRenderingEngine* engine;
    PRRETextureManager* tm;
    PRREMaterialManager* mm;
    PRREMaterial* mat;

    // ---------------------------------------------------------------------------

    PRREMaterialTest(const PRREMaterialTest&)
    {};         

    PRREMaterialTest& operator=(const PRREMaterialTest&)
    {
        return *this;
    };

    bool testIsBlendFuncBlends()
    {
        return assertFalse(PRREMaterial::isBlendFuncReallyBlending(PRRE_ONE, PRRE_ZERO), "(ONE, ZERO)") &
            assertTrue(PRREMaterial::isBlendFuncReallyBlending(PRRE_ONE, PRRE_ONE), "(ONE, ONE)") &
            assertTrue(PRREMaterial::isBlendFuncReallyBlending(PRRE_SRC_COLOR, PRRE_ZERO), "(SRC_COLOR, ZERO)") &
            assertTrue(PRREMaterial::isBlendFuncReallyBlending(PRRE_DST_COLOR, PRRE_ONE), "(DST_COLOR, ONE)") &
            assertTrue(PRREMaterial::isBlendFuncReallyBlending(PRRE_ZERO, PRRE_DST_ALPHA), "(ZERO, DST_ALPHA)") &
            assertTrue(PRREMaterial::isBlendFuncReallyBlending(PRRE_ONE_MINUS_CONSTANT_COLOR, PRRE_ONE), "(ONE_MINUS_CONSTANT_COLOR, ONE)") &
            assertTrue(PRREMaterial::isBlendFuncReallyBlending(PRRE_CONSTANT_COLOR, PRRE_ONE), "(CONSTANT_COLOR, ONE)") &
            assertTrue(PRREMaterial::isBlendFuncReallyBlending(PRRE_ZERO, PRRE_ONE_MINUS_DST_ALPHA), "(ZERO, ONE_MINUS_DST_ALPHA)") &
            assertTrue(PRREMaterial::isBlendFuncReallyBlending(PRRE_ZERO, PRRE_CONSTANT_ALPHA), "(ZERO, CONSTANT_ALPHA)") &
            assertTrue(PRREMaterial::isBlendFuncReallyBlending(PRRE_SRC_ALPHA_SATURATE, PRRE_ONE), "(SRC_ALPHA_SATURATE, ONE)");
        // not checking every combination, no problem
    }

    bool testCtor()
    {
        const PRREColor clrWhite(255,255,255,255);

        const TPRREuint nMaxLayers = mm->getMaximumLayerCount();
        bool b = true;
        TPRREuint i = 0;

        while ( i < nMaxLayers )
        {
            // by default no array is allocated
            b &= assertEquals((TPRREuint) 0, mat->getColorsCount(i), "getColorsCount") &
                assertNull(mat->getColors(i), "getColors") &
                assertEquals((TPRREuint)0, mat->getIndicesCount(), "getIndicesCount") &
                assertEquals((TPRREuint)0, mat->getTexcoordsCount(i), "getTexcoordsCount") &
                assertNull(mat->getTexcoords(i), "getTexcoords") &
                assertNull(mat->getTexture(i), "getTexture") &
                assertTrue(clrWhite == mat->getTextureEnvColor(i), "getTextureEnvColor") &
                assertEquals(TPRRE_BLENDFACTOR::PRRE_ONE, mat->getSourceBlendFunc(i), "getSourceBlendFunc") &
                assertEquals(TPRRE_BLENDFACTOR::PRRE_ZERO, mat->getDestinationBlendFunc(i), "getDestinationBlendFunc") &
                assertEquals(TPRRE_BLENDMODE::PRRE_BM_NONE, mat->getBlendMode(i), "getBlendMode");
            i++;
        }
        std::stringstream ss;
        ss << "Failed in " << i << ". iteration" << std::endl;
        assertTrue(b, ss.str().c_str());

        return b &
            assertNotEquals(std::string::npos, mat->getName().find("Material "), "img128x128x24 name") &
            assertFalse(mat->isTextured(), "isTextured") &
            assertFalse(mat->isSingleTextured(), "isSingleTextured") &
            assertFalse(mat->isMultiTextured(), "isMultiTextured");
    }

    bool testDtor()
    {
        delete mat;
        mat = NULL;

        return assertEquals(0, mm->getCount());
    }

    bool testAllocateArrays()
    {
        const TPRREuint nColors    = 5;
        const TPRREuint nTexCoords = 6;
        const TPRREuint nIndices   = 7;
        const TPRREuint nIndexSize = 4;

        const bool bAlloc = assertTrue(mat->allocateArrays(nColors, nTexCoords, nIndices, nIndexSize), "allocate");

        bool b = true;
        TPRREuint  i = 0;
        while ( i < mm->getMaximumLayerCount() )
        {
            b &= assertEquals(nColors, mat->getColorsCount(i), "getColorsCount") &
                assertNotNull(mat->getColors(i), "getColors") &
                assertEquals(nIndices, mat->getIndicesCount(), "getIndicesCount") &
                assertEquals(nTexCoords, mat->getTexcoordsCount(i), "getTexcoordsCount") &
                assertNotNull(mat->getTexcoords(i), "getTexcoords");
            i++;
        }

        std::stringstream ss;
        ss << "Failed in " << i << ". iteration" << std::endl;

        return bAlloc & assertTrue(b, ss.str().c_str());
    }

    bool testSetTexture()
    {
        PRRETexture* tex128x128x24 = PR00FsReducedRenderingEngine::createAndGet().getTextureManager().createFromFile(BMP128x128x24);
        if ( !tex128x128x24 )
            return assertNotNull(tex128x128x24, "tex");

        bool b = assertTrue(mat->setTexture(tex128x128x24), "setTexture text128x128x24");
        b &= assertNotNull(mat->getTexture(), "1st tex 0") &
            assertNull(mat->getTexture(1), "1st tex 1");

        b &= assertTrue(mat->setTexture(NULL), "setTexture NULL");
        b &= assertTrue(mat->setTexture(tex128x128x24, 1), "setTexture text128x128x24, 1");
        b &= assertNotNull(mat->getTexture(1), "2nd tex 1") &
            assertNull(mat->getTexture(), "2nd tex 0");

        return b;
    }

    bool testGetTextureCount()
    {
        PRRETexture* tex128x128x24 = PR00FsReducedRenderingEngine::createAndGet().getTextureManager().createFromFile(BMP128x128x24);
        if ( !tex128x128x24 )
            return assertNotNull(tex128x128x24, "tex");

        bool b = assertEquals((TPRREuint)0, mat->getTextureCount(), "1st");

        b &= assertTrue(mat->setTexture(tex128x128x24), "setTexture text128x128x24");
        b &= assertEquals((TPRREuint)1, mat->getTextureCount(), "2nd");

        b &= assertTrue(mat->setTexture(NULL), "setTexture NULL");
        b &= assertTrue(mat->setTexture(tex128x128x24, 1), "setTexture text128x128x24, 1");
        b &= assertEquals((TPRREuint)1, mat->getTextureCount(), "3rd");

        b &= assertTrue(mat->setTexture(tex128x128x24), "setTexture text128x128x24");
        b &= assertEquals((TPRREuint)2, mat->getTextureCount(), "4th");

        return b;
    }

    bool testIsTextured()
    {
        PRRETexture* tex128x128x24 = PR00FsReducedRenderingEngine::createAndGet().getTextureManager().createFromFile(BMP128x128x24);
        if ( !tex128x128x24 )
            return assertNotNull(tex128x128x24, "tex");

        bool b = assertFalse(mat->isTextured(), "1st istex");

        b &= assertTrue(mat->setTexture(tex128x128x24), "setTexture text128x128x24");
        b &= assertTrue(mat->isTextured(), "2nd istex");

        b &= assertTrue(mat->setTexture(NULL), "setTexture NULL");
        b &= assertTrue(mat->setTexture(tex128x128x24, 1), "setTexture text128x128x24, 1");
        b &= assertTrue(mat->isTextured(), "3rd istex");

        return b;
    }

    bool testIsSingleTextured()
    {
        PRRETexture* tex128x128x24 = PR00FsReducedRenderingEngine::createAndGet().getTextureManager().createFromFile(BMP128x128x24);
        if ( !tex128x128x24 )
            return assertNotNull(tex128x128x24, "tex");

        bool b = assertFalse(mat->isSingleTextured(), "1st");

        b &= assertTrue(mat->setTexture(tex128x128x24), "setTexture text128x128x24");
        b &= assertTrue(mat->isSingleTextured(), "2nd");

        b &= assertTrue(mat->setTexture(NULL), "setTexture NULL");
        b &= assertTrue(mat->setTexture(tex128x128x24, 1), "setTexture text128x128x24, 1");
        b &= assertTrue(mat->isSingleTextured(), "3rd");

        b &= assertTrue(mat->setTexture(tex128x128x24), "setTexture text128x128x24");
        b &= assertFalse(mat->isSingleTextured(), "4th");

        return b;
    }

    bool testIsMultiTextured()
    {
        PRRETexture* tex128x128x24 = PR00FsReducedRenderingEngine::createAndGet().getTextureManager().createFromFile(BMP128x128x24);
        if ( !tex128x128x24 )
            return assertNotNull(tex128x128x24, "tex");

        bool b = assertFalse(mat->isMultiTextured(), "1st");

        b &= assertTrue(mat->setTexture(tex128x128x24), "setTexture text128x128x24");
        b &= assertFalse(mat->isMultiTextured(), "2nd");

        b &= assertTrue(mat->setTexture(NULL), "setTexture NULL");
        b &= assertTrue(mat->setTexture(tex128x128x24, 1), "setTexture text128x128x24, 1");
        b &= assertFalse(mat->isMultiTextured(), "3rd");

        b &= assertTrue(mat->setTexture(tex128x128x24), "setTexture text128x128x24");
        b &= assertTrue(mat->isMultiTextured(), "4th");

        return b;
    }

    bool testSetSourceBlendFunc()
    {
        PRRESampleManagerForUtiliserManaged sampleManager;
        PRRESampleUtiliserManaged* sampleUtiliser = sampleManager.createManaged();
        if ( !assertNotNull(sampleUtiliser, "sampleUtiliser not null"))
        {
            return assertNotNull(sampleUtiliser, "sampleUtiliser is NULL");
        }

        mat->SetUtiliser(sampleUtiliser);
        bool b = assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 1");

        b &= assertTrue(mat->setSourceBlendFunc(TPRRE_BLENDFACTOR::PRRE_CONSTANT_ALPHA), "PRRE_CONSTANT_ALPHA");
        b &= assertEquals( TPRRE_BLENDFACTOR::PRRE_CONSTANT_ALPHA, mat->getSourceBlendFunc(), "1st") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 2");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 3");

        b &= assertTrue(mat->setSourceBlendFunc(TPRRE_BLENDFACTOR::PRRE_CONSTANT_COLOR, 1), "PRRE_CONSTANT_COLOR");
        b &= assertEquals( TPRRE_BLENDFACTOR::PRRE_CONSTANT_COLOR, mat->getSourceBlendFunc(1), "2nd") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 4");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 5");

        b &= assertFalse(mat->setSourceBlendFunc(TPRRE_BLENDFACTOR::PRRE_SRC_COLOR), "PRRE_SRC_COLOR"); // invalid src param
        b &= assertEquals( TPRRE_BLENDFACTOR::PRRE_CONSTANT_COLOR, mat->getSourceBlendFunc(1), "3rd") &
            assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 6");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 7");

        b &= assertFalse(mat->setSourceBlendFunc(TPRRE_BLENDFACTOR::PRRE_ONE_MINUS_SRC_COLOR), "PRRE_ONE_MINUS_SRC_COLOR"); // invalid src param
        b &= assertEquals( TPRRE_BLENDFACTOR::PRRE_CONSTANT_COLOR, mat->getSourceBlendFunc(1), "4th") &
            assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 8");

        return b;
    }

    bool testSetDestinationBlendFunc()
    {
        PRRESampleManagerForUtiliserManaged sampleManager;
        PRRESampleUtiliserManaged* sampleUtiliser = sampleManager.createManaged();
        if ( !assertNotNull(sampleUtiliser, "sampleUtiliser not null"))
        {
            return assertNotNull(sampleUtiliser, "sampleUtiliser is NULL");
        }

        mat->SetUtiliser(sampleUtiliser);
        bool b = assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 1");

        b &= assertTrue(mat->setDestinationBlendFunc(TPRRE_BLENDFACTOR::PRRE_CONSTANT_ALPHA), "PRRE_CONSTANT_ALPHA");
        b &= assertEquals( TPRRE_BLENDFACTOR::PRRE_CONSTANT_ALPHA, mat->getDestinationBlendFunc(), "1st") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 2");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 3");

        b &= assertTrue(mat->setDestinationBlendFunc(TPRRE_BLENDFACTOR::PRRE_CONSTANT_COLOR, 1), "PRRE_CONSTANT_COLOR");
        b &= assertEquals( TPRRE_BLENDFACTOR::PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "2nd") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 4");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 5");

        b &= assertFalse(mat->setDestinationBlendFunc(TPRRE_BLENDFACTOR::PRRE_DST_COLOR, 1), "PRRE_DST_COLOR"); // invalid dst param
        b &= assertEquals( TPRRE_BLENDFACTOR::PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "3rd") &
            assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 6");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 7");

        b &= assertFalse(mat->setDestinationBlendFunc(TPRRE_BLENDFACTOR::PRRE_ONE_MINUS_DST_COLOR, 1), "PRRE_ONE_MINUS_DST_COLOR"); // invalid dst param
        b &= assertEquals( TPRRE_BLENDFACTOR::PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "4th") &
            assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 8");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 9");

        b &= assertFalse(mat->setDestinationBlendFunc(TPRRE_BLENDFACTOR::PRRE_SRC_ALPHA_SATURATE, 1), "PRRE_SRC_ALPHA_SATURATE"); // invalid dst param
        b &= assertEquals( TPRRE_BLENDFACTOR::PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "5th") &
            assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 10");

        return b;
    }

    bool testSetBlendFuncs()
    {
        PRRESampleManagerForUtiliserManaged sampleManager;
        PRRESampleUtiliserManaged* sampleUtiliser = sampleManager.createManaged();
        if ( !assertNotNull(sampleUtiliser, "sampleUtiliser not null"))
        {
            return assertNotNull(sampleUtiliser, "sampleUtiliser is NULL");
        }

        mat->SetUtiliser(sampleUtiliser);
        bool b = assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 1");

        b &= assertTrue(mat->setBlendFuncs(PRRE_CONSTANT_ALPHA, PRRE_CONSTANT_COLOR), "PRRE_CONSTANT_ALPHA, PRRE_CONSTANT_COLOR");
        b &= assertEquals( PRRE_CONSTANT_ALPHA, mat->getSourceBlendFunc(), "1st src") &
            assertEquals( PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(), "1st dst") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 1");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 2");

        b &= assertTrue(mat->setBlendFuncs(PRRE_CONSTANT_ALPHA, PRRE_CONSTANT_COLOR, 1), "RRE_CONSTANT_ALPHA, PRRE_CONSTANT_COLOR, 1");
        b &= assertEquals( PRRE_CONSTANT_ALPHA, mat->getSourceBlendFunc(1), "2nd src") &
            assertEquals( PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "2nd dst") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 3");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 4");

        b &= assertFalse(mat->setBlendFuncs(PRRE_SRC_COLOR, PRRE_CONSTANT_COLOR, 1), "PRRE_SRC_COLOR, PRRE_CONSTANT_COLOR, 1"); // invalid src param
        b &= assertEquals( PRRE_CONSTANT_ALPHA, mat->getSourceBlendFunc(1), "3rd src") &
            assertEquals( PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "3rd dst") &
            assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 5"); // srcBlendFunc is invalid, so dstBlendFunc will not be invoked at all thus no call to change handler!

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 6");

        b &= assertFalse(mat->setBlendFuncs(PRRE_ONE_MINUS_SRC_COLOR, PRRE_CONSTANT_COLOR, 1), "PRRE_ONE_MINUS_SRC_COLOR, PRRE_CONSTANT_COLOR, 1"); // invalid src param
        b &= assertEquals( PRRE_CONSTANT_ALPHA, mat->getSourceBlendFunc(1), "4th src") &
            assertEquals( PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "4th dst") &
            assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 7"); // srcBlendFunc is invalid, so dstBlendFunc will not be invoked at all thus no call to change handler!

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 8");

        b &= assertFalse(mat->setBlendFuncs(PRRE_CONSTANT_ALPHA, PRRE_DST_COLOR, 1), "PRRE_CONSTANT_ALPHA, PRRE_DST_COLOR, 1"); // invalid dst param
        b &= assertEquals( PRRE_CONSTANT_ALPHA, mat->getSourceBlendFunc(1), "5th src") &
            assertEquals( PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "5th dst") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 9"); // srcBlendFunc is valid, so that will be set and will invoke change handler!

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 10");

        b &= assertFalse(mat->setBlendFuncs(PRRE_CONSTANT_ALPHA, PRRE_ONE_MINUS_DST_COLOR, 1), "PRRE_CONSTANT_ALPHA, PRRE_ONE_MINUS_DST_COLOR, 1"); // invalid dst param
        b &= assertEquals( PRRE_CONSTANT_ALPHA, mat->getSourceBlendFunc(1), "6th src") &
            assertEquals( PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "6th dst") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 11"); // srcBlendFunc is valid, so that will be set and will invoke change handler!

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 12");

        b &= assertFalse(mat->setBlendFuncs(PRRE_CONSTANT_ALPHA, PRRE_SRC_ALPHA_SATURATE, 1), "PRRE_CONSTANT_ALPHA, PRRE_SRC_ALPHA_SATURATE, 1"); // invalid dst param
        b &= assertEquals( PRRE_CONSTANT_ALPHA, mat->getSourceBlendFunc(1), "7th src") &
            assertEquals( PRRE_CONSTANT_COLOR, mat->getDestinationBlendFunc(1), "7th dst") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 13"); // srcBlendFunc is valid, so that will be set and will invoke change handler!

        return b;
    }

    bool testSetBlendMode()
    {
        PRRESampleManagerForUtiliserManaged sampleManager;
        PRRESampleUtiliserManaged* sampleUtiliser = sampleManager.createManaged();
        if ( !assertNotNull(sampleUtiliser, "sampleUtiliser"))
        {
            return assertNotNull(sampleUtiliser, "sampleUtiliser");
        }

        mat->SetUtiliser(sampleUtiliser);
        bool b = assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 1");

        b &= assertTrue(mat->setBlendMode(TPRRE_BLENDMODE::PRRE_BM_STANDARD_TRANSPARENCY), "PRRE_BM_STANDARD_TRANSPARENCY");
        b &= assertEquals( PRRE_BM_STANDARD_TRANSPARENCY, mat->getBlendMode(), "1st mode") & 
            assertEquals( PRRE_SRC_ALPHA, mat->getSourceBlendFunc(), "1st src") &
            assertEquals( PRRE_ONE_MINUS_SRC_ALPHA, mat->getDestinationBlendFunc(), "1st dst") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 2");

        sampleManager.ResetHandleManagedPropertyChangedInvoked();
        b &= assertFalse(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 3");

        b &= assertTrue(mat->setBlendMode(TPRRE_BLENDMODE::PRRE_BM_NONE, 1), "PRRE_BM_NONE, 1");
        b &= assertEquals( PRRE_BM_NONE, mat->getBlendMode(1), "2nd mode") & 
            assertEquals( PRRE_ONE, mat->getSourceBlendFunc(1), "2nd src") &
            assertEquals( PRRE_ZERO, mat->getDestinationBlendFunc(1), "2nd dst") &
            assertTrue(sampleManager.isHandleManagedPropertyChangedInvoked(), "propChangedInvoked 4");

        return b;
    }

    bool testCopyFromMaterial()
    {
        PRREMaterial* mat2 = mm->createMaterial();
        if ( !mat2 )
            return assertNotNull(mat2, "mat2");
        
        const bool bAllocMat2 = assertTrue(mat2->allocateArrays(2, 2, 2, 2), "allocate mat2");
        mat2->getColors()[0].green = 1.0f;
        mat2->getTexcoords()[1].u = 1.0f;
 
        bool b = assertFalse(mat->copyFromMaterial(*mat2, 0, 0), "1st");  // should fail due to non-matching array layout
       
        // make sure nothing has been changed in mat
        TPRREuint i = 0;
        while ( i < mm->getMaximumLayerCount() )
        {
            b &= assertEquals((TPRREuint) 0, mat->getColorsCount(i), "getColorsCount") &
                assertNull(mat->getColors(i), "getColors") &
                assertEquals((TPRREuint)0, mat->getIndicesCount(), "getIndicesCount") &
                assertEquals((TPRREuint)0, mat->getTexcoordsCount(i), "getTexcoordsCount") &
                assertNull(mat->getTexcoords(i), "getTexcoords");
            i++;
        }
        std::stringstream ss;
        ss << "Failed in " << i << ". iteration" << std::endl;
        assertTrue(b, ss.str().c_str());

        const bool bAllocMat = assertTrue(mat->allocateArrays(2, 2, 2, 2), "allocate mat");

        b &= assertTrue(mat->copyFromMaterial(*mat2, 0, 0), "2nd");  // should succeed now

        // sanity check for the layout before checking actual values in arrays
        i = 0;
        while ( i < mm->getMaximumLayerCount() )
        {
            b &= assertEquals((TPRREuint) 2, mat->getColorsCount(i), "getColorsCount") &
                assertNotNull(mat->getColors(i), "getColors") &
                assertEquals((TPRREuint)2, mat->getIndicesCount(), "getIndicesCount") &
                assertEquals((TPRREuint)2, mat->getTexcoordsCount(i), "getTexcoordsCount") &
                assertNotNull(mat->getTexcoords(i), "getTexcoords");
            i++;
        }
        ss.clear();
        ss << "Failed in " << i << ". iteration" << std::endl;
        assertTrue(b, ss.str().c_str());

        // we should have the same values in the 1st material too 
        b &= assertEquals(1.0f, mat->getColors()[0].green, "clr lvl0 green") &
            assertEquals(1.0f, mat->getTexcoords()[1].u, "texcoord lvl1 U");

        return b & bAllocMat & bAllocMat2;
    }

    bool testGetUsedSystemMemory()
    {
        return assertGreater(mat->getUsedSystemMemory(), sizeof(PRREMaterial));
    }

}; // class PRREMaterialTest