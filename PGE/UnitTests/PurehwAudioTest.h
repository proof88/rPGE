#pragma once

/*
    ###################################################################################
    PurehwAudioTest.h
    Unit test for PurehwAudio.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "UnitTest.h"  // PCH
#include "../Pure/include/external/PR00FsReducedRenderingEngine.h"
#include "../Pure/include/external/Hardware/PureHwInfo.h"

#ifndef E
#define E 0.0001f
#endif // EPSILON

class PurehwAudioTest :
    public UnitTest
{
public:

    PurehwAudioTest() :
        UnitTest( __FILE__ ),
        hw( PureHwInfo::get() ), 
        audio( PurehwAudio::get() )
    {
        engine = NULL;
    }

    virtual ~PurehwAudioTest()
    {
        Finalize();   
    }

protected:

    virtual void Initialize()
    {
        //CConsole::getConsoleInstance().SetLoggingState("4LLM0DUL3S", true);
        engine = NULL;
        AddSubTest("testCtor", (PFNUNITSUBTEST) &PurehwAudioTest::testCtor);
        AddSubTest("testInitialize", (PFNUNITSUBTEST) &PurehwAudioTest::testInitialize);
        AddSubTest("testDeinitialize", (PFNUNITSUBTEST) &PurehwAudioTest::testDeinitialize);
        AddSubTest("testWriteStats", (PFNUNITSUBTEST) &PurehwAudioTest::testWriteStats);
    }

    virtual bool setUp()
    {
        bool ret = true;
        if ( engine == NULL )
        {
            engine = &PR00FsReducedRenderingEngine::createAndGet();
            ret = (0 == engine->initialize(PURE_RENDERER_HW_FP, 800, 600, PURE_WINDOWED, 0, 32, 24, 0, 0));  // pretty standard display mode, should work on most systems
        }
        return ret;
    }

    virtual void TearDown()
    {
        
    }

    virtual void Finalize()
    {
        if ( engine )
        {
            engine->shutdown();
            engine = NULL;
        }
        CConsole::getConsoleInstance().SetLoggingState("4LLM0DUL3S", false);
    }

private:

    PR00FsReducedRenderingEngine* engine;
    PureHwInfo& hw;
    PurehwAudio& audio;

    // ---------------------------------------------------------------------------

    PurehwAudioTest(const PurehwAudioTest&) :
        hw( PureHwInfo::get() ),
        audio( PurehwAudio::get() )
    {};         

    PurehwAudioTest& operator=(const PurehwAudioTest&)
    {
        return *this;
    };

    bool testCtor()
    {
        return assertTrue(audio.isInitialized(), "isInitialized()");
    }

    bool testInitialize()
    {
        return assertTrue(audio.isInitialized());
    }

    bool testDeinitialize()
    {
        audio.Deinitialize();
        const bool l = assertFalse(audio.isInitialized(), "isInitialized()");

        if ( engine )
        {
            engine->shutdown();
            engine = NULL;
        }
        return l;
    }

    bool testWriteStats()
    {
        audio.WriteStats();
        return true;
    }
    
}; // class PurehwAudioTest
