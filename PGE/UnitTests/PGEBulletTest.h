#pragma once

/*
    ###################################################################################
    PGEBulletTest.h
    Unit test for PGEBullet.
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "UnitTest.h"  // PCH
#include "../Weapons/WeaponManager.h"

class PGEBulletTest :
    public UnitTest
{
public:

    PGEBulletTest() :
        UnitTest(__FILE__)
    {
        engine = NULL;
    }

protected:

    virtual void Initialize()
    {
        CConsole::getConsoleInstance().SetLoggingState(Bullet::getLoggerModuleName(), true);

        engine = &PR00FsReducedRenderingEngine::createAndGet();
        engine->initialize(PRRE_RENDERER_HW_FP, 800, 600, PRRE_WINDOWED, 0, 32, 24, 0, 0);  // pretty standard display mode, should work on most systems

        AddSubTest("test_bullet_ctor", (PFNUNITSUBTEST)&PGEBulletTest::test_bullet_ctor_good);
        AddSubTest("test_bullet_ctor_max_bullet_speed_incompatible_with_non_zero_bullet_drag",
            (PFNUNITSUBTEST)&PGEBulletTest::test_bullet_ctor_max_bullet_speed_incompatible_with_non_zero_bullet_drag);
        AddSubTest("test_bullet_update_updates_position", (PFNUNITSUBTEST)&PGEBulletTest::test_bullet_update_updates_position);
    }

    virtual bool setUp()
    {
        return assertTrue(engine && engine->isInitialized());
    }

    virtual void TearDown()
    {
    }

    virtual void Finalize()
    {
        if (engine)
        {
            engine->shutdown();
            engine = NULL;
        }

        CConsole::getConsoleInstance().SetLoggingState(Bullet::getLoggerModuleName(), false);
    }

private:

    PR00FsReducedRenderingEngine* engine;

    // ---------------------------------------------------------------------------

    PGEBulletTest(const PGEBulletTest&)
    {};

    PGEBulletTest& operator=(const PGEBulletTest&)
    {
        return *this;
    };

    bool test_bullet_ctor_good()
    {
        const PRREVector posVec(1.f, 2.f, 3.f);
        const PRREVector angleVec(20.f, 40.f, 60.f);
        const PRREVector sizeVec(4.f, 5.f, 0.f /* size-Z will be 0.f anyway */);
        const float fSpeed = 60.f;
        const float fGravity = 15.f;
        const float fDrag = 25.f;
        const bool bFragile = true;
        Bullet bullet(
            *engine,
            posVec.getX(), posVec.getY(), posVec.getZ(),
            angleVec.getX(), angleVec.getY(), angleVec.getZ(),
            sizeVec.getX(), sizeVec.getY(), sizeVec.getZ(),
            fSpeed, fGravity, fDrag, bFragile);
        
        bool b = assertEquals(posVec, bullet.getObject3D().getPosVec(), "pos");
        b &= assertEquals(angleVec, bullet.getObject3D().getAngleVec(), "angle");
        b &= assertEquals(sizeVec, bullet.getObject3D().getSizeVec(), "size");
        b &= assertEquals(fSpeed, bullet.getSpeed(), "speed");
        b &= assertEquals(fGravity, bullet.getGravity(), "gravity");
        b &= assertEquals(fDrag, bullet.getDrag(), "drag");
        b &= assertEquals(bFragile, bullet.isFragile(), "fragile");

        return b;
    }

    bool test_bullet_ctor_max_bullet_speed_incompatible_with_non_zero_bullet_drag()
    {
        bool b = false;
        try
        {
            Bullet bullet(
                *engine,
                1.f, 2.f, 3.f,
                20.f, 40.f, 60.f,
                4.f, 5.f, 0.f,
                1000.f, 15.f, 25.f, true);
        }
        catch (const std::exception)
        {
            b = true;
        }

        return b;
    }

    bool test_bullet_update_updates_position()
    {
        const PRREVector angleVec(0.f, 90.f, 45.f);
        const float speed = 60.f;

        PRREPosUpTarget put;
        put.SetRotation(angleVec.getX(), angleVec.getY(), angleVec.getZ());
        put.Move(speed);

        Bullet bullet(*engine, 0.f, 0.f, 0.f, angleVec.getX(), angleVec.getY(), angleVec.getZ(), 1.f, 1.f, 1.f, speed, 15.f, 25.f, true);
        bullet.Update();

        bool b = assertEquals(put.getPosVec(), bullet.getObject3D().getPosVec(), "pos");

        return b;
    }
};