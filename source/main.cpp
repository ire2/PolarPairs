//
//  main.cpp
//  PolarPairs
//

#include "HelloApp.h"

// This keeps us from having to write cugl:: all the time
using namespace cugl;

// Application dimensions
#define GAME_WIDTH 576
#define GAME_HEIGHT 1024  // Using the dimension from HelloApp.h

/**
 * The main entry point of the application.
 */
int main(int argc, char * argv[]) {
    // Create application instance
    HelloApp app;
    
    // Configure application properties
    app.setName("PolarPairs");
    app.setOrganization("GDIAC");
    app.setDisplaySize(GAME_WIDTH, GAME_HEIGHT);
    
    // REMOVED: app.setScaleMode(ScaleMode::LETTERBOX);
    
    app.setFPS(60.0f);
    app.setHighDPI(true);
    
    // Initialize and run the application
    if (!app.init()) {
        return 1;
    }
    
    app.onStartup();
    while (app.step());
    app.onShutdown();
    
    exit(0);    // Necessary to quit on mobile devices
    return 0;   // This line is never reached
}
