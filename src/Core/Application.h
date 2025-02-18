#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppContext.h"

// Third party
#include <SDL.h>

// Forward Declarations
//------------------------------------------------------------------------------
struct AppConfig;

//------------------------------------------------------------------------------
class Application
{
public:
    explicit Application(const AppConfig& config);

    bool IsValid() const { return mContext.IsValid(); }
    void Run();

    const AppContext& GetContext() const { return mContext; }
    AppContext& GetContext() { return mContext; }

protected:
    // Lifecycle Methods (Meant for Subclassing)
    virtual void OnCreate() { }
    virtual void OnEvent(const SDL_Event& event, float timeslice) { (void)event; (void)timeslice; }
    virtual void OnUpdate(float timeslice) { (void)timeslice; }
    virtual void OnRender() { }

private:
    void ProcessEvents(float timeslice);

    AppContext mContext;
    bool mRunning;
};