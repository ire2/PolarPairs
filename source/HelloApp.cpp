#include "HelloApp.h"
#include "LevelManager.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <thread>
#include <iostream>
#include <cugl/audio/CUAudioEngine.h>
#include <cugl/audio/CUSoundLoader.h>
//#include <cugl/audio/CUAudioEngine.h>
//include <cugl/audio/CUSoundLoader.h>

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene2;

#define GAME_WIDTH 576
#define GAME_HEIGHT 1024

/**
 * Helper function to play button press sound
 */
void playButtonSound(const std::shared_ptr<cugl::AssetManager>& assets) {
    auto buttonSound = assets->get<cugl::audio::Sound>("buttonSound");
    if (buttonSound) {
        cugl::audio::AudioEngine::get()->play("buttonPress", buttonSound, false, 0.8f);
    }
}

/**
 * Calculate tile size based on display height and grid dimensions
 */
float HelloApp::calculateTileSize() const {
    const float tileHeightRatio = 0.6f;
    const float gridHeight = 11.0f;
    float targetGridHeight = getDisplaySize().height * tileHeightRatio;
    return targetGridHeight / gridHeight;
}

/**
 * The method called after OpenGL is initialized, but before running the application.
 */
void HelloApp::onStartup() {
    _batch = SpriteBatch::alloc();
    setClearColor(Color4::CLEAR);
    
    // Create and initialize asset manager
    _assets = AssetManager::alloc();
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());
    _assets->attach<JsonValue>(JsonLoader::alloc()->getHook());
    
    // Initialize Audio Engine and add Sound loader
    cugl::audio::AudioEngine::start();
    _assets->attach<cugl::audio::Sound>(cugl::audio::SoundLoader::alloc()->getHook());
    
    _assets->loadDirectory("json/assets.json");
    
    // Play background music in a loop
    auto backgroundMusic = _assets->get<cugl::audio::Sound>("backgroundMusic");
    if (backgroundMusic) {
        cugl::audio::AudioEngine::get()->play("backgroundMusic", backgroundMusic, true, 0.5f);
    }
    
    // Initialize LevelManager
    if (!LevelManager::getInstance()->init(_assets)) {
        CULog("Failed to initialize LevelManager");
        return;
    }
    
    // Create UI fade overlay
    auto node = SceneNode::allocWithBounds(getDisplaySize());
    _uiFadeOverlay = std::dynamic_pointer_cast<PolygonNode>(node);
    if (_uiFadeOverlay) {
        _uiFadeOverlay->setColor(Color4(0, 0, 0, 0)); // Start fully transparent
        _uiFadeOverlay->setPriority(1000); // Highest priority to be on top
    }
    
    // Initialize selected level
    _selectedLevel = 1;
    
    // Activate input devices with debug logging
    Input::activate<Keyboard>();
    Input::activate<Mouse>();
    #if defined(CU_PLATFORM_IOS) || defined(CU_PLATFORM_ANDROID)
    Input::activate<Touchscreen>();
    #endif
    
    // Initialize animation variables
    _animTime = 0.0f;
    _sealBaseY = 0.0f;
    _bearBaseY = 0.0f;
    _sealImage = nullptr;
    _bearImage = nullptr;
    
    // Create the menu scene
    _menuScene = Scene2::allocWithHint(getDisplaySize());
    _menuScene->setSpriteBatch(_batch);
    
    // Create the level selector scene
    _levelScene = Scene2::allocWithHint(getDisplaySize());
    _levelScene->setSpriteBatch(_batch);
    
    // Create the finish scene
    _finishScene = Scene2::allocWithHint(getDisplaySize());
    _finishScene->setSpriteBatch(_batch);
    
    // Create the backgrounds
    createSharedBackground();
    
    // Build the scenes
    buildMenuScene();
    buildLevelScene();
    buildFinishScene();
    
    // The game controller will be created when needed during transition
    _PolarPairsController = nullptr;
    
    // Start in menu scene
    _inMenuScene = true;
    _inLevelScene = false;
    _inFinishScene = false;
    _buttonPressed = false;
    
    Application::onStartup();
}

/**
 * The method called when the application is ready to quit.
 */
void HelloApp::onShutdown() {
    // Stop the audio engine
    cugl::audio::AudioEngine::stop();
    
    // Clean up menu elements
    if (_logo && _logo->getParent()) {
        _logo->getParent()->removeChild(_logo);
    }
    if (_startButton && _startButton->getParent()) {
        _startButton->getParent()->removeChild(_startButton);
    }
    if (_sealImage && _sealImage->getParent()) {
        _sealImage->getParent()->removeChild(_sealImage);
    }
    if (_bearImage && _bearImage->getParent()) {
        _bearImage->getParent()->removeChild(_bearImage);
    }
    if (_level1Button && _level1Button->getParent()) {
        _level1Button->getParent()->removeChild(_level1Button);
    }
    if (_finishExitButton && _finishExitButton->getParent()) {
        _finishExitButton->getParent()->removeChild(_finishExitButton);
    }
    
    if (_finishRestartButton && _finishRestartButton->getParent()) {
        _finishRestartButton->getParent()->removeChild(_finishRestartButton);
    }
    
    if (_finishNextButton && _finishNextButton->getParent()) {
        _finishNextButton->getParent()->removeChild(_finishNextButton);
    }
    
    if (_finishStarRating && _finishStarRating->getParent()) {
        _finishStarRating->getParent()->removeChild(_finishStarRating);
    }
    
    if (_levelFinishedText && _levelFinishedText->getParent()) {
        _levelFinishedText->getParent()->removeChild(_levelFinishedText);
    }
    
    if (_highestText && _highestText->getParent()) {
        _highestText->getParent()->removeChild(_highestText);
    }
    
    _logo = nullptr;
    _startButton = nullptr;
    _level1Button = nullptr;
    _sealImage = nullptr;
    _bearImage = nullptr;
    _menuBackground = nullptr;
    _levelBackground = nullptr;
    _gameBackground = nullptr;
    _finishBackground = nullptr;
    _finishExitButton = nullptr;
    _finishRestartButton = nullptr;
    _finishNextButton = nullptr;
    _finishStarRating = nullptr;
    _levelFinishedText = nullptr;
    _highestText = nullptr;
    _PolarPairsController = nullptr;
    _menuScene = nullptr;
    _levelScene = nullptr;
    _finishScene = nullptr;
    _batch = nullptr;
    _assets = nullptr;
    
    Input::deactivate<Keyboard>();
    Input::deactivate<Mouse>();
    #if defined(CU_PLATFORM_IOS) || defined(CU_PLATFORM_ANDROID)
    Input::deactivate<Touchscreen>();
    #endif
    
    Application::onShutdown();
}

/**
 * Updates the application data.
 */
void HelloApp::update(float timestep) {
    // Update animations using the same timer
    _animTime += timestep;
    
    // Update transition if active
    if (_isTransitioning) {
        updateTransition(timestep);
        return; // Don't process input during transition
    }
    
    // Update background animations regardless of scene
    float amplitude = getDisplaySize().height * 0.02f; // 2% of screen height
    float offsetY = amplitude * std::cos(_animTime * M_PI / 2.0f);
    float offsetX = amplitude * std::sin(_animTime * M_PI / 2.0f);
    
    // Always update menu background position if it exists
    if (_menuBackground) {
        _menuBackground->setPosition(_backgroundBaseX + offsetX, _backgroundBaseY + offsetY);
    }
    
    // Always update level background position if it exists
    if (_levelBackground) {
        _levelBackground->setPosition(_backgroundBaseX + offsetX, _backgroundBaseY + offsetY);
    }
    
    // Always update game background position if it exists
    if (_gameBackground) {
        _gameBackground->setPosition(_backgroundBaseX + offsetX, _backgroundBaseY + offsetY);
    }
    
    // Always update finish background position if it exists
    if (_finishBackground) {
        _finishBackground->setPosition(_backgroundBaseX + offsetX, _backgroundBaseY + offsetY);
    }
    
    // Update character animations for menu scene
    if (_sealImage) {
        float offset = getDisplaySize().height * 0.01f * cosf(_animTime * 1.5f);
        _sealImage->setPosition(_sealImage->getPositionX(), _sealBaseY + offset);
    }
    
    if (_bearImage) {
        float offset = getDisplaySize().height * 0.01f * cosf((_animTime - 0.5f) * 1.5f);
        _bearImage->setPosition(_bearImage->getPositionX(), _bearBaseY + offset);
    }
    
    // Update level scene characters
    if (_inLevelScene) {
        for (auto& child : _levelScene->getChildren()) {
            if (child->getName() == "bearseal") {
                float offset = getDisplaySize().height * 0.01f * cosf(_animTime * 1.5f);
                child->setPosition(child->getPositionX(), _levelBearBaseY + offset);
            }
        }
        
        // Process button interactions through scene update
        _levelScene->update(timestep);
        
        // Handle touch input for level title reset feature
        if (_levelTitle) {
            bool isTouching = false;
            Vec2 inputPos;
            
            // Check for touch input
            auto touch = Input::get<Touchscreen>();
            if (touch && touch->touchCount() > 0) {
                auto& touchSet = touch->touchSet();
                if (!touchSet.empty()) {
                    TouchID tid = *(touchSet.begin());
                    inputPos = touch->touchPosition(tid);
                    isTouching = touch->touchDown(tid);
                }
            }
            
            if (isTouching) {
                Vec2 scenePos = _levelScene->screenToWorldCoords(inputPos);
                
                // Check if touch is on level title
                cugl::Rect titleBounds = _levelTitle->getBoundingBox();
                
                // Touch began on level title
                if (titleBounds.contains(scenePos) && !_levelTitleTouched) {
                    _levelTitleTouched = true;
                    _levelTitleTouchTime = _animTime; // Store current time
                    _levelTitleTouchPos = scenePos;
                    CULog("Level title touch started");
                }
                // Touch continuing on level title
                else if (_levelTitleTouched) {
                    // Check if finger didn't move too much (30 pixels max drift)
                    float drift = (scenePos - _levelTitleTouchPos).length();
                    if (drift > 30.0f) {
                        // Cancel the reset if drifted too far
                        _levelTitleTouched = false;
                        CULog("Level title touch canceled (moved too far)");
                    } 
                    else {
                        // Check if we've been touching for 3 seconds
                        float touchDuration = _animTime - _levelTitleTouchTime;
                        if (touchDuration >= 3.0f) {
                            // Reset all level progress
                            LevelManager::getInstance()->resetAllProgress();
                            CULog("All level progress reset!");
                            
                            // Reset touch state
                            _levelTitleTouched = false;
                            
                            // Rebuild level scene to reflect changes
                            buildLevelScene();
                        }
                    }
                }
            } 
            else if (_levelTitleTouched) {
                // Touch ended or cancelled
                _levelTitleTouched = false;
            }
        }
        
        // Debug touch input for level buttons
        auto touch = Input::get<Touchscreen>();
        if (touch && touch->touchCount() > 0) {
            auto& touchSet = touch->touchSet();
            if (!touchSet.empty()) {
                TouchID tid = *(touchSet.begin());
                Vec2 touchPos = touch->touchPosition(tid);
                Vec2 scenePos = _levelScene->screenToWorldCoords(touchPos);
                
                // Update all level buttons
                for (auto& button : _levelButtons) {
                    if (button) {
                        Rect bounds = button->getBoundingBox();
                        button->setDown(bounds.contains(scenePos) && touch->touchDown(tid));
                    }
                }
            }
        } else {
            // Reset all buttons when no touch
            for (auto& button : _levelButtons) {
                if (button) {
                    button->setDown(false);
                }
            }
        }
        
        // Handle user pressing back to menu (escape key)
        auto keyboard = Input::get<Keyboard>();
        if (keyboard && keyboard->keyPressed(KeyCode::ESCAPE)) {
            _inLevelScene = false;
            _inMenuScene = true;
            buildMenuScene();
        }
    } else if (_inMenuScene) {
        // Update start page animations
        if (_startButton) {
            float alpha = 0.5f + 0.5f * (sinf(_animTime * 2.0f) + 1.0f) * 0.5f;
            _startButton->setColor(Color4(255, 255, 255, (int)(alpha * 255)));
        }
        
        // Full screen tap detection for start page
        bool tapped = false;
        
        // Check keyboard
        auto keyboard = Input::get<Keyboard>();
        if (keyboard && (keyboard->keyPressed(KeyCode::SPACE) || 
                         keyboard->keyPressed(KeyCode::RETURN))) {
            tapped = true;
        }
        
        // Check mouse
        auto mouse = Input::get<Mouse>();
        if (mouse && mouse->buttonPressed().hasLeft()) {
            tapped = true;
        }
        
        // Check touch
        auto touch = Input::get<Touchscreen>();
        if (touch && touch->touchCount() > 0) {
            tapped = true;
        }
        
        if (tapped && !_buttonPressed) {
            _buttonPressed = true;
            transitionToLevelSelector();
        }
    } else if (_inFinishScene) {
        // Update finish scene animations
        
        // Handle the finish scene entrance animation
        if (_isFinishSceneAnimating) {
            _finishAnimTime += timestep;
            float animDuration = 0.5f; // Animation duration in seconds
            
            // Calculate animation progress
            float progress = std::min(1.0f, _finishAnimTime / animDuration);
            
            // Apply enhanced cubic ease-out function for more pronounced effect
            float easedProgress = 1.0f - (1.0f - progress) * (1.0f - progress) * (1.0f - progress);
            
            // Animate "Level Finished" text
            if (_levelFinishedText) {
                float startY = -getDisplaySize().height * 0.1f; // Starting position (below screen)
                float targetY = _levelFinishedOrigPos.y;
                float currentY = startY + (targetY - startY) * easedProgress;
                _levelFinishedText->setPosition(_levelFinishedOrigPos.x, currentY);
            }
            
            // Animate star rating (no delay)
            if (_finishStarRating) {
                float startY = -getDisplaySize().height * 0.1f;
                float targetY = _starRatingOrigPos.y;
                float currentY = startY + (targetY - startY) * easedProgress;
                _finishStarRating->setPosition(_starRatingOrigPos.x, currentY);
            }
            
            // Animate "Highest" text (no delay)
            if (_highestText) {
                float startY = -getDisplaySize().height * 0.1f;
                float targetY = _highestTextOrigPos.y;
                float currentY = startY + (targetY - startY) * easedProgress;
                _highestText->setPosition(_highestTextOrigPos.x, currentY);
            }
            
            // Animate restart button (no delay)
            if (_finishRestartButton) {
                float startY = -getDisplaySize().height * 0.1f;
                float targetY = _restartButtonOrigPos.y;
                float currentY = startY + (targetY - startY) * easedProgress;
                _finishRestartButton->setPosition(_restartButtonOrigPos.x, currentY);
            }
            
            // Only show and animate the next button for levels 1-11, completely skip for level 12
            if (_finishNextButton && _selectedLevel < 12) {
                float startY = -getDisplaySize().height * 0.1f;
                float targetY = _nextButtonOrigPos.y;
                float currentY = startY + (targetY - startY) * easedProgress;
                _finishNextButton->setPosition(_nextButtonOrigPos.x, currentY);
            }
            
            // End animation when complete
            if (_finishAnimTime >= animDuration) { // No need for extra time since there are no delays
                _isFinishSceneAnimating = false;
                
                // Set everything to its final position
                if (_levelFinishedText) _levelFinishedText->setPosition(_levelFinishedOrigPos);
                if (_finishStarRating) _finishStarRating->setPosition(_starRatingOrigPos);
                if (_highestText) _highestText->setPosition(_highestTextOrigPos);
                if (_finishRestartButton) _finishRestartButton->setPosition(_restartButtonOrigPos);
                
                // Only position the next button for levels 1-11
                if (_finishNextButton && _selectedLevel < 12) {
                    _finishNextButton->setPosition(_nextButtonOrigPos);
                }
            }
        }
        
        // Process button interactions through scene update
        _finishScene->update(timestep);
        
        // Handle touch input for finish scene exit button
        auto touch = Input::get<Touchscreen>();
        if (touch && touch->touchCount() > 0) {
            auto& touchSet = touch->touchSet();
            if (!touchSet.empty()) {
                TouchID tid = *(touchSet.begin());
                Vec2 touchPos = touch->touchPosition(tid);
                Vec2 scenePos = _finishScene->screenToWorldCoords(touchPos);
                
                // Update exit button state
                if (_finishExitButton) {
                    Rect bounds = _finishExitButton->getBoundingBox();
                    _finishExitButton->setDown(bounds.contains(scenePos) && touch->touchDown(tid));
                }
                
                // Update restart button state
                if (_finishRestartButton) {
                    Rect bounds = _finishRestartButton->getBoundingBox();
                    _finishRestartButton->setDown(bounds.contains(scenePos) && touch->touchDown(tid));
                }
                
                // Update next button state
                if (_finishNextButton) {
                    Rect bounds = _finishNextButton->getBoundingBox();
                    _finishNextButton->setDown(bounds.contains(scenePos) && touch->touchDown(tid));
                }
            }
        } else {
            // Reset buttons when no touch
            if (_finishExitButton) {
                _finishExitButton->setDown(false);
            }
            if (_finishRestartButton) {
                _finishRestartButton->setDown(false);
            }
            if (_finishNextButton) {
                _finishNextButton->setDown(false);
            }
        }
        
        // Handle user pressing back to level selection (escape key)
        auto keyboard = Input::get<Keyboard>();
        if (keyboard && keyboard->keyPressed(KeyCode::ESCAPE)) {
            transitionToLevelSelector();
        }
    } else if (_PolarPairsController) {
        // Update game
        _PolarPairsController->update(timestep);
        
        // Handle transitions
        if (_PolarPairsController->hasWon() || _PolarPairsController->shouldExitToMenu()) {
            // Reset exit flag if it was set
            if (_PolarPairsController->shouldExitToMenu()) {
                _PolarPairsController->resetExitFlag();
            }
            
            // If player has won, go to finish scene
            if (_PolarPairsController->hasWon()) {
                transitionToFinishScene();
            } else {
                // Just exit to menu without going through finish scene
                transitionToLevelSelector();
            }
        }
        
        // Return to menu on escape key
        auto keyboard = Input::get<Keyboard>();
        if (keyboard && keyboard->keyPressed(KeyCode::ESCAPE)) {
            transitionToLevelSelector();
        }
    }
}

void HelloApp::updateTransition(float timestep) {
    _transitionTime += timestep;
    float duration = 0.5f; // Half second transition
    
    if (_isFadingOut) {
        // Fade out phase
        float progress = _transitionTime / duration;
        if (progress >= 1.0f) {
            // Fade out complete, switch scenes
            _isFadingOut = false;
            _transitionTime = 0;
            
            if (_inMenuScene) {
                // Switching from menu to level
                _inMenuScene = false;
                _inLevelScene = true;
                
                // Remove menu elements
                if (_logo && _logo->getParent()) {
                    _logo->getParent()->removeChild(_logo);
                    _logo = nullptr;
                }
                if (_startButton && _startButton->getParent()) {
                    _startButton->getParent()->removeChild(_startButton);
                    _startButton = nullptr;
                }
                if (_sealImage && _sealImage->getParent()) {
                    _sealImage->getParent()->removeChild(_sealImage);
                    _sealImage = nullptr;
                }
                if (_bearImage && _bearImage->getParent()) {
                    _bearImage->getParent()->removeChild(_bearImage);
                    _bearImage = nullptr;
                }
                
                // Reset level scene UI opacity
                if (_levelScene) {
                    for (auto& child : _levelScene->getChildren()) {
                        if (child != _levelBackground) {
                            child->setColor(Color4(255, 255, 255, 0));
                        }
                    }
                }
            } else if (_inLevelScene) {
                // Switching from level to game
                _inLevelScene = false;
                
                // Create a new controller
                _PolarPairsController = std::make_shared<PolarPairsController>();
                
                if (_PolarPairsController->init(_assets, getDisplaySize())) {
                    auto gameScene = _PolarPairsController->getScene();
                    if (gameScene) {
                        gameScene->setSpriteBatch(_batch);
                        
                        // Add the game background
                        if (_gameBackground) {
                            if (_gameBackground->getParent()) {
                                _gameBackground->getParent()->removeChild(_gameBackground);
                            }
                            _gameBackground->setPosition(_backgroundBaseX, _backgroundBaseY);
                            _gameBackground->setPriority(-200);
                            gameScene->addChild(_gameBackground);
                        }
                        
                        // Set initial opacity for game scene UI elements
                        for (auto& child : gameScene->getChildren()) {
                            if (child != _gameBackground) {
                                child->setColor(Color4(255, 255, 255, 0));
                            }
                        }
                        
                        _PolarPairsController->switchLevel(_selectedLevel);
                    } else {
                        _inMenuScene = true;
                        _inLevelScene = false;
                    }
                } else {
                    _inMenuScene = true;
                    _inLevelScene = false;
                }
            } else if (_PolarPairsController && !_inFinishScene) {
                // Check if transition should go to finish scene or level selector
                if (_PolarPairsController->hasWon()) {
                    // Switching from game to finish scene
                    _inFinishScene = true;
                    
                    // Clean up game scene
                    if (_gameBackground && _gameBackground->getParent()) {
                        _gameBackground->getParent()->removeChild(_gameBackground);
                    }
                    
                    // Store the level we just completed
                    _selectedLevel = _PolarPairsController->getCurrentLevel();
                    
                    // Clean up controller
                    _PolarPairsController = nullptr;
                    
                    // Update finish scene for the completed level - don't rebuild
                    // Instead, just handle the next button visibility
                    if (_finishNextButton) {
                        if (_selectedLevel >= 12) {
                            // Hide the next button for level 12
                            _finishNextButton->setVisible(false);
                            _finishNextButton->deactivate(); // Disable interaction
                        } else {
                            // For other levels, ensure the button is visible
                            _finishNextButton->setVisible(true);
                            _finishNextButton->activate();
                        }
                    }
                    
                    // Reset finish scene UI opacity
                    if (_finishScene) {
                        for (auto& child : _finishScene->getChildren()) {
                            if (child != _finishBackground && child != _finishNextButton) {
                                child->setColor(Color4(255, 255, 255, 0));
                            }
                        }
                    }
                } else {
                    // Switching from game to level selector
                    _inLevelScene = true;
                    
                    // Clean up game scene
                    if (_gameBackground && _gameBackground->getParent()) {
                        _gameBackground->getParent()->removeChild(_gameBackground);
                    }
                    _PolarPairsController = nullptr;
                    
                    // Reset level scene UI opacity
                    if (_levelScene) {
                        for (auto& child : _levelScene->getChildren()) {
                            if (child != _levelBackground) {
                                child->setColor(Color4(255, 255, 255, 0));
                            }
                        }
                    }
                }
            } else if (_inFinishScene) {
                // Check if we should go to next level directly
                if (_goToNextLevel) {
                    // Create a new controller for the next level
                    _inFinishScene = false;
                    _isFinishSceneAnimating = false; // Reset animation state
                    
                    // Create a new controller
                    _PolarPairsController = std::make_shared<PolarPairsController>();
                    
                    if (_PolarPairsController->init(_assets, getDisplaySize())) {
                        auto gameScene = _PolarPairsController->getScene();
                        if (gameScene) {
                            gameScene->setSpriteBatch(_batch);
                            
                            // Add the game background
                            if (_gameBackground) {
                                if (_gameBackground->getParent()) {
                                    _gameBackground->getParent()->removeChild(_gameBackground);
                                }
                                _gameBackground->setPosition(_backgroundBaseX, _backgroundBaseY);
                                _gameBackground->setPriority(-200);
                                gameScene->addChild(_gameBackground);
                            }
                            
                            // Set initial opacity for game scene UI elements
                            for (auto& child : gameScene->getChildren()) {
                                if (child != _gameBackground) {
                                    child->setColor(Color4(255, 255, 255, 0));
                                }
                            }
                            
                            // Switch to the next level
                            _PolarPairsController->switchLevel(_selectedLevel);
                            
                            // Reset the flag
                            _goToNextLevel = false;
                        } else {
                            // Fallback to level selector on error
                            _inLevelScene = true;
                            _goToNextLevel = false;
                        }
                    } else {
                        // Fallback to level selector on error
                        _inLevelScene = true;
                        _goToNextLevel = false;
                    }
                } else {
                    // Switching from finish scene to level scene
                    _inFinishScene = false;
                    _isFinishSceneAnimating = false; // Reset animation state
                    _inLevelScene = true;
                    
                    // Reset level scene UI opacity
                    if (_levelScene) {
                        for (auto& child : _levelScene->getChildren()) {
                            if (child != _levelBackground) {
                                child->setColor(Color4(255, 255, 255, 0));
                            }
                        }
                    }
                }
            }
        } else {
            // Update UI elements opacity
            int alpha = static_cast<int>((1.0f - progress) * 255);
            if (_inMenuScene && _menuScene) {
                for (auto& child : _menuScene->getChildren()) {
                    if (child != _menuBackground) {
                        child->setColor(Color4(255, 255, 255, alpha));
                    }
                }
            } else if (_inLevelScene && _levelScene) {
                for (auto& child : _levelScene->getChildren()) {
                    if (child != _levelBackground) {
                        child->setColor(Color4(255, 255, 255, alpha));
                    }
                }
                
                // Also update star rating images
                for (auto& star : _levelStars) {
                    star->setColor(Color4(255, 255, 255, alpha));
                }
            } else if (_inFinishScene && _finishScene) {
                for (auto& child : _finishScene->getChildren()) {
                    if (child != _finishBackground) {
                        child->setColor(Color4(255, 255, 255, alpha));
                    }
                }
            } else if (_PolarPairsController && _PolarPairsController->getScene()) {
                // Fade out game scene UI elements
                auto gameScene = _PolarPairsController->getScene();
                for (auto& child : gameScene->getChildren()) {
                    if (child != _gameBackground) {
                        child->setColor(Color4(255, 255, 255, alpha));
                    }
                }
            }
        }
    } else {
        // Fade in phase
        float progress = _transitionTime / duration;
        if (progress >= 1.0f) {
            // Fade in complete
            _isTransitioning = false;
            _transitionTime = 0;
            if (_inMenuScene && _menuScene) {
                for (auto& child : _menuScene->getChildren()) {
                    if (child != _menuBackground) {
                        child->setColor(Color4::WHITE);
                    }
                }
            } else if (_inLevelScene && _levelScene) {
                for (auto& child : _levelScene->getChildren()) {
                    if (child != _levelBackground) {
                        child->setColor(Color4::WHITE);
                    }
                }
                
                // Also update star rating images
                for (auto& star : _levelStars) {
                    star->setColor(Color4::WHITE);
                }
            } else if (_inFinishScene && _finishScene) {
                for (auto& child : _finishScene->getChildren()) {
                    if (child != _finishBackground) {
                        child->setColor(Color4::WHITE);
                    }
                }
            } else if (_PolarPairsController && _PolarPairsController->getScene()) {
                // Fade in game scene UI elements
                auto gameScene = _PolarPairsController->getScene();
                for (auto& child : gameScene->getChildren()) {
                    if (child != _gameBackground) {
                        child->setColor(Color4::WHITE);
                    }
                }
            }
        } else {
            // Update UI elements opacity
            int alpha = static_cast<int>(progress * 255);
            if (_inMenuScene && _menuScene) {
                for (auto& child : _menuScene->getChildren()) {
                    if (child != _menuBackground) {
                        child->setColor(Color4(255, 255, 255, alpha));
                    }
                }
            } else if (_inLevelScene && _levelScene) {
                for (auto& child : _levelScene->getChildren()) {
                    if (child != _levelBackground) {
                        child->setColor(Color4(255, 255, 255, alpha));
                    }
                }
                
                // Also update star rating images
                for (auto& star : _levelStars) {
                    star->setColor(Color4(255, 255, 255, alpha));
                }
            } else if (_inFinishScene && _finishScene) {
                for (auto& child : _finishScene->getChildren()) {
                    if (child != _finishBackground) {
                        child->setColor(Color4(255, 255, 255, alpha));
                    }
                }
            } else if (_PolarPairsController && _PolarPairsController->getScene()) {
                // Fade in game scene UI elements
                auto gameScene = _PolarPairsController->getScene();
                for (auto& child : gameScene->getChildren()) {
                    if (child != _gameBackground) {
                        child->setColor(Color4(255, 255, 255, alpha));
                    }
                }
            }
        }
    }
}

/**
 * Draw the application to the screen
 */
void HelloApp::draw() {
    if (_inMenuScene) {
        _menuScene->render();
    } else if (_inLevelScene) {
        _levelScene->render();
    } else if (_inFinishScene) {
        _finishScene->render();
    } else if (_PolarPairsController && _PolarPairsController->getScene()) {
        auto gameScene = _PolarPairsController->getScene();
        if (!gameScene->getSpriteBatch()) {
            gameScene->setSpriteBatch(_batch);
        }
        gameScene->render();
    } else {
        _menuScene->render();
    }
    
    // Always render UI fade overlay if transitioning
    if (_isTransitioning && _uiFadeOverlay) {
        _uiFadeOverlay->render(_batch);
    }
}

std::shared_ptr<cugl::scene2::PolygonNode> HelloApp::createCharacter(const std::string& textureName, 
                                                                    float posX, float posY, 
                                                                    float scale, 
                                                                    const std::string& name,
                                                                    float& baseY) {
    if (auto texture = _assets->get<Texture>(textureName)) {
        auto character = PolygonNode::allocWithTexture(texture);
        character->setScale(scale);
        character->setAnchor(Vec2::ANCHOR_CENTER);
        character->setPosition(posX, posY);
        character->setPriority(100); // Set priority to be above background
        character->setName(name);
        baseY = posY; // Store base position for animation
        return character;
    }
    return nullptr;
}

/**
 * Build the main menu scene
 */
void HelloApp::buildMenuScene() {
    Size displaySize = getDisplaySize();
    
    // Get safe area information for iOS devices
    float topSafeArea = 0.0f;
    float bottomSafeArea = 0.0f;
    
    #if defined(CU_PLATFORM_IOS)
    auto safeArea = Application::get()->getSafeArea();
    if (safeArea) {
        topSafeArea = safeArea->top;
        bottomSafeArea = safeArea->bottom;
    } else if (displaySize.height > displaySize.width) {
        topSafeArea = displaySize.height * 0.05f;
        bottomSafeArea = displaySize.height * 0.05f;
    }
    #endif
    
    float effectiveHeight = displaySize.height - topSafeArea - bottomSafeArea;
    
    // Add background to menu scene
    if (_menuBackground) {
        _menuScene->addChild(_menuBackground);
    }
    
    // Calculate tile size using our helper method
    float tileSize = calculateTileSize();
    
    // Add logo at the top center
    if (auto logoTexture = _assets->get<Texture>("logo")) {
        auto topLogo = PolygonNode::allocWithTexture(logoTexture);
        float logoScale = (tileSize * 1.2f) / logoTexture->getHeight();
        topLogo->setScale(logoScale);
        topLogo->setAnchor(Vec2::ANCHOR_CENTER);
        topLogo->setPosition(displaySize.width *0.2f, displaySize.height * 0.1f);
        topLogo->setPriority(200); // Set priority to be above characters
        _menuScene->addChild(topLogo);
    }
    
    // Create characters using tileSize as reference
    // SealTitleIMG and BearTitleIMG should be 7x of tileSize height
    float bearScale = (tileSize * 7.0f) / _assets->get<Texture>("BearTitleIMG")->getHeight();
    float sealScale = (tileSize * 7.0f) / _assets->get<Texture>("SealTitleIMG")->getHeight();
    
    _bearImage = createCharacter("BearTitleIMG", 
                                displaySize.width * 0.7f,
                                bottomSafeArea + (effectiveHeight * 0.12f),
                                bearScale,
                                "bear",
                                _bearBaseY);
    if (_bearImage) _menuScene->addChild(_bearImage);
    
    _sealImage = createCharacter("SealTitleIMG",
                                displaySize.width * 0.25f,
                                bottomSafeArea + (effectiveHeight * 0.75f),
                                sealScale,
                                "seal",
                                _sealBaseY);
    if (_sealImage) _menuScene->addChild(_sealImage);
    
    // Add title - PolarPairsTextTitle should be 1x of tileSize height
    if (auto titleTexture = _assets->get<Texture>("PolarPairsTextTitle")) {
        _logo = PolygonNode::allocWithTexture(titleTexture);
        float scale = (tileSize * 3.2f) / titleTexture->getHeight();
        _logo->setScale(scale);
        _logo->setAnchor(Vec2::ANCHOR_CENTER);
        _logo->setPosition(displaySize.width / 2.0f, displaySize.height / 1.9f);
        _logo->setPriority(200); // Set priority to be above characters
        _menuScene->addChild(_logo);
    }
    
    // Add start button - TapToStart should be 0.7x of tileSize height
    if (auto promptTexture = _assets->get<Texture>("TapToStart")) {
        _startButton = PolygonNode::allocWithTexture(promptTexture);
        float scale = (tileSize * 0.7f) / promptTexture->getHeight();
        _startButton->setScale(scale);
        _startButton->setAnchor(Vec2::ANCHOR_CENTER);
        _startButton->setPosition(displaySize.width / 2.0f, bottomSafeArea + effectiveHeight * 0.32f);
        _startButton->setPriority(300); // Set priority to be above everything
        _startButton->setColor(Color4(255, 255, 255, 128));
        _menuScene->addChild(_startButton);
    }
}

/**
 * Create backgrounds for menu and game scenes
 */
void HelloApp::createSharedBackground() {
    std::shared_ptr<cugl::graphics::Texture> bgTexture = _assets->get<cugl::graphics::Texture>("SeaBackground");
    if (!bgTexture) {
        return;
    }
    
    // Get scene dimensions
    float sceneWidth = getDisplaySize().width;
    float sceneHeight = getDisplaySize().height;
    
    // Make background slightly larger than the screen (110%)
    float scaleX = sceneWidth * 1.1f / bgTexture->getWidth();
    float scaleY = sceneHeight * 1.1f / bgTexture->getHeight();
    cugl::Vec2 scale(scaleX, scaleY);
    
    // Save the base position for animation
    _backgroundBaseX = sceneWidth/2;
    _backgroundBaseY = sceneHeight/2;
    
    // Create menu background
    _menuBackground = cugl::scene2::PolygonNode::allocWithTexture(bgTexture);
    _menuBackground->setScale(scale);
    _menuBackground->setAnchor(cugl::Vec2::ANCHOR_CENTER);
    _menuBackground->setPosition(_backgroundBaseX, _backgroundBaseY);
    _menuBackground->setPriority(-100);
    
    // Create level selector background
    _levelBackground = cugl::scene2::PolygonNode::allocWithTexture(bgTexture);
    _levelBackground->setScale(scale);
    _levelBackground->setAnchor(cugl::Vec2::ANCHOR_CENTER);
    _levelBackground->setPosition(_backgroundBaseX, _backgroundBaseY);
    _levelBackground->setPriority(-100);
    
    // Create game background but DO NOT add to any scene yet
    // It will be added when the game scene is created during transition
    _gameBackground = cugl::scene2::PolygonNode::allocWithTexture(bgTexture);
    _gameBackground->setScale(scale);
    _gameBackground->setAnchor(cugl::Vec2::ANCHOR_CENTER);
    _gameBackground->setPosition(_backgroundBaseX, _backgroundBaseY);
    _gameBackground->setPriority(-100);
    
    // Create finish scene background but DO NOT add to finish scene yet
    // It will be added in buildFinishScene
    _finishBackground = cugl::scene2::PolygonNode::allocWithTexture(bgTexture);
    _finishBackground->setScale(scale);
    _finishBackground->setAnchor(cugl::Vec2::ANCHOR_CENTER);
    _finishBackground->setPosition(_backgroundBaseX, _backgroundBaseY);
    _finishBackground->setPriority(-100);
}

std::shared_ptr<cugl::scene2::Button> HelloApp::createLevelButton(int level, const cugl::Vec2& position, float buttonSize) {
    // Check if level is unlocked using LevelManager
    bool isUnlocked = LevelManager::getInstance()->isLevelUnlocked(level);
    CULog("Level %d unlocked status: %d", level, isUnlocked);
    
    // Get level score (0-3)
    int score = LevelManager::getInstance()->getLevelScore(level);
    CULog("Level %d score: %d", level, score);
    
    // For locked levels, use the Down texture for both states
    std::string upTextureName = isUnlocked ? "Level" + std::to_string(level) + "_Up" : "Level" + std::to_string(level) + "_Down";
    std::string downTextureName = "Level" + std::to_string(level) + "_Down";
    
    std::shared_ptr<Texture> upTexture = _assets->get<Texture>(upTextureName);
    std::shared_ptr<Texture> downTexture = _assets->get<Texture>(downTextureName);
    
    if (!upTexture || !downTexture) {
        return nullptr;
    }
    
    auto upNode = PolygonNode::allocWithTexture(upTexture);
    auto downNode = PolygonNode::allocWithTexture(downTexture);
    
    float buttonScale = buttonSize / upTexture->getWidth();
    upNode->setScale(buttonScale);
    downNode->setScale(buttonScale);
    
    auto button = Button::alloc(upNode, downNode);
    button->setAnchor(Vec2::ANCHOR_CENTER);
    button->setPosition(position);
    button->setPriority(1000);
    
    button->setName("level" + std::to_string(level));
    button->addListener([=] (const std::string& name, bool down) {
        if (down) {
            // Play button sound on press
            playButtonSound(_assets);
        } else if (!down && isUnlocked) {
            _selectedLevel = level;
            transitionToGame(level);
        }
    });
    
    return button;
}

void HelloApp::buildLevelScene() {
    Size displaySize = getDisplaySize();
    
    // Add background to level scene
    if (_levelBackground) {
        _levelScene->addChild(_levelBackground);
    }
    
    // Calculate tile size using our helper method
    float tileSize = calculateTileSize();
    
    // Add level menu title
    if (auto titleTexture = _assets->get<Texture>("LevelMenuTitle")) {
        auto levelTitle = PolygonNode::allocWithTexture(titleTexture);
        float titleScale = (tileSize * 0.8f) / titleTexture->getHeight();  // 1.0x tile height
        levelTitle->setScale(titleScale);
        levelTitle->setAnchor(Vec2::ANCHOR_CENTER);
        levelTitle->setPosition(displaySize.width * 0.5f, displaySize.height * 0.75f);
        levelTitle->setPriority(200);  // Set priority to be above characters
        _levelScene->addChild(levelTitle);
        
        // Store the level title for reset feature
        _levelTitle = levelTitle;
        _levelTitleTouched = false;
        _levelTitleTouchTime = 0;
    }
    
    // Create combined character image for level scene
    float bearSealScale = (displaySize.height * 0.27f) / _assets->get<Texture>("BearSealIMG")->getHeight();
    
    auto combinedCharacters = createCharacter("BearSealIMG",
                                 displaySize.width * 0.5f,  // Center horizontally
                                 displaySize.height * 0.06f, // Position at lower 10% of screen
                                 bearSealScale,
                                 "bearseal",
                                 _levelBearBaseY);  // Reuse existing animation base Y
    if (combinedCharacters) _levelScene->addChild(combinedCharacters);
    
    // Grid dimensions (7x11 is the actual grid size in the game)
    const int gridWidth = 7;
    const int gridHeight = 11;
    
    // Calculate button size as 1.7x the regular tile size
    float buttonSize = tileSize * 1.7f;
    
    // Fixed 3x4 layout for level buttons
    const int COLS = 3;
    const int ROWS = 4;
    const int TOTAL_LEVELS = 12;  // Total number of levels to display
    
    // Calculate adaptive spacing
    float spacing = buttonSize * 0.3f;  // 25% of button size
    const float BUTTON_SPACING_X = spacing;
    const float BUTTON_SPACING_Y = spacing;
    
    // Calculate total width and height of the button grid
    float totalGridWidth = (COLS * buttonSize) + ((COLS - 1) * BUTTON_SPACING_X);
    float totalGridHeight = (ROWS * buttonSize) + ((ROWS - 1) * BUTTON_SPACING_Y);
    
    // Calculate starting position to center the grid
    float startX = (displaySize.width - totalGridWidth) / 2.0f + (buttonSize / 2.0f);
    float startY = displaySize.height * 0.67f - (buttonSize / 2.0f);  // Add the buttonSize/2 offset
    
    // Clear existing buttons and stars
    _levelButtons.clear();
    _levelStars.clear();
    
    // Create level buttons in a 3x4 grid
    int level = 1;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (level <= TOTAL_LEVELS) {  // Create all 12 buttons
                // Calculate position in grid
                Vec2 position(
                    startX + col * (buttonSize + BUTTON_SPACING_X),
                    startY - row * (buttonSize + BUTTON_SPACING_Y)
                );
                
                auto button = createLevelButton(level, position, buttonSize);
                if (button) {
                    _levelScene->addChild(button);
                    button->activate();
                    _levelButtons.push_back(button);
                    
                    // Get level score (0-3)
                    int score = LevelManager::getInstance()->getLevelScore(level);
                    
                    // Add star rating based on score (0-3)
                    std::string starTextureName;
                    switch (score) {
                        case 1: starTextureName = "OneStar"; break;
                        case 2: starTextureName = "TwoStars"; break;
                        case 3: starTextureName = "ThreeStars"; break;
                        default: starTextureName = "NoStar"; break;
                    }
                    
                    auto starTexture = _assets->get<Texture>(starTextureName);
                    if (starTexture) {
                        auto starNode = PolygonNode::allocWithTexture(starTexture);
                        
                        // Scale star to be 1.2x the button width
                        float starScale = (buttonSize * 1.1f) / starTexture->getWidth();
                        starNode->setScale(starScale);
                        
                        // Position star so half of it overlays the top of the button
                        // Button anchor is CENTER, so the button's top is at position.y + buttonSize/2
                        float starHeight = starTexture->getHeight() * starScale;
                        float starY = position.y + (buttonSize / 2) - (starHeight / 10);
                        
                        starNode->setAnchor(Vec2::ANCHOR_CENTER);
                        starNode->setPosition(position.x, starY);
                        starNode->setPriority(1001); // Higher priority than buttons (1000)
                        
                        _levelScene->addChild(starNode);
                        _levelStars.push_back(starNode);
                    }
                    
                    // Store first 4 buttons in member variables for backward compatibility
                    switch(level) {
                        case 1: _level1Button = button; break;
                        case 2: _level2Button = button; break;
                        case 3: _level3Button = button; break;
                        case 4: _level4Button = button; break;
                        default: break;
                    }
                }
                level++;
            }
        }
    }
}

/**
 * Handles the transition from menu to level selector
 */
void HelloApp::transitionToLevelSelector() {
    if (!_isTransitioning) {
        _isTransitioning = true;
        _isFadingOut = true;
        _transitionTime = 0;
        
        // Set initial UI opacity
        if (_inMenuScene && _menuScene) {
            for (auto& child : _menuScene->getChildren()) {
                if (child != _menuBackground) {
                    child->setColor(Color4::WHITE);
                }
            }
        } else if (_inLevelScene && _levelScene) {
            for (auto& child : _levelScene->getChildren()) {
                if (child != _levelBackground) {
                    child->setColor(Color4::WHITE);
                }
            }
        } else if (_PolarPairsController && _PolarPairsController->getScene()) {
            auto gameScene = _PolarPairsController->getScene();
            for (auto& child : gameScene->getChildren()) {
                if (child != _gameBackground) {
                    child->setColor(Color4::WHITE);
                }
            }
        } else if (_inFinishScene && _finishScene) {
            for (auto& child : _finishScene->getChildren()) {
                if (child != _finishBackground) {
                    child->setColor(Color4::WHITE);
                }
            }
        }
        
        // If we're transitioning from game or finish scene to level selector, reinitialize the level scene
        if (_PolarPairsController || _inFinishScene) {
            // Clean up the old level scene
            _levelScene = nullptr;
            _levelButtons.clear();
            
            // Create a new level scene
            _levelScene = Scene2::allocWithHint(getDisplaySize());
            _levelScene->setSpriteBatch(_batch);
            
            // Rebuild the level scene
            buildLevelScene();
        }
    }
}

/**
 * Handles the transition from level selector to game scene
 * Creates the controller immediately to prevent rendering issues
 */
void HelloApp::transitionToGame(int level) {
    if (!_isTransitioning) {
        _isTransitioning = true;
        _isFadingOut = true;
        _transitionTime = 0;
        _selectedLevel = level;
        
        // Set initial UI opacity
        if (_levelScene) {
            for (auto& child : _levelScene->getChildren()) {
                if (child != _levelBackground) {
                    child->setColor(Color4::WHITE);
                }
            }
        }
    }
}

/**
 * Build the finish scene with an exit button in the center
 */
void HelloApp::buildFinishScene() {
    // Add the finish scene background that was created in createSharedBackground
    if (_finishBackground && !_finishBackground->getParent()) {
        _finishScene->addChild(_finishBackground);
    }
    
    // Get tile size for consistent UI scaling
    float tileSize = calculateTileSize();
    
    // Initialize "Level Finished" text
    if (!_levelFinishedText) {
        auto textTexture = _assets->get<Texture>("LevelFinished");
        if (textTexture) {
            _levelFinishedText = PolygonNode::allocWithTexture(textTexture);
            
            // Scale to be the same height as pausedTexture
            float textScale = (tileSize) / textTexture->getHeight();
            _levelFinishedText->setScale(textScale);
            
            // Position "Level Finished" text at the top of the scene
            _levelFinishedText->setAnchor(Vec2::ANCHOR_CENTER);
            _levelFinishedText->setPosition(getDisplaySize().width/2, getDisplaySize().height * 0.7f);
            _levelFinishedText->setPriority(900);
            
            // Store original position
            _levelFinishedOrigPos = _levelFinishedText->getPosition();
            
            if (!_levelFinishedText->getParent()) {
                _finishScene->addChild(_levelFinishedText);
            }
        }
    }
    
    // Initialize star rating node with NoStar texture
    if (!_finishStarRating) {
        auto starTexture = _assets->get<Texture>("NoStar");
        if (starTexture) {
            _finishStarRating = PolygonNode::allocWithTexture(starTexture);
            
            // Scale star to be 1.5x the tileSize
            float starScale = (tileSize * 1.5f) / starTexture->getHeight();
            _finishStarRating->setScale(starScale);
            
            // Position star above the buttons
            _finishStarRating->setAnchor(Vec2::ANCHOR_CENTER);
            _finishStarRating->setPosition(getDisplaySize().width/2, getDisplaySize().height * 0.5f);
            _finishStarRating->setPriority(900); // Below buttons (1000) but above background
            
            // Store original position
            _starRatingOrigPos = _finishStarRating->getPosition();
            
            if (!_finishStarRating->getParent()) {
                _finishScene->addChild(_finishStarRating);
            }
        }
    }
    
    // Initialize "Highest" text
    if (!_highestText) {
        auto highestTexture = _assets->get<Texture>("Highest");
        if (highestTexture) {
            _highestText = PolygonNode::allocWithTexture(highestTexture);
            
            // Scale to be 0.5x of tileSize
            float textScale = (tileSize * 0.5f) / highestTexture->getHeight();
            _highestText->setScale(textScale);
            
            // Position "Highest" text just above the star rating
            _highestText->setAnchor(Vec2::ANCHOR_CENTER);
            _highestText->setPosition(getDisplaySize().width/2, getDisplaySize().height * 0.57f);
            _highestText->setPriority(900);
            
            // Store original position
            _highestTextOrigPos = _highestText->getPosition();
            
            if (!_highestText->getParent()) {
                _finishScene->addChild(_highestText);
            }
        }
    }
    
    // Create exit button in the bottom left corner (same position as restart in game)
    if (!_finishExitButton) {
        std::shared_ptr<Texture> exitUp = _assets->get<Texture>("Exit_Up");
        std::shared_ptr<Texture> exitDown = _assets->get<Texture>("Exit_Down");
        
        if (exitUp && exitDown) {
            _finishExitButton = Button::alloc(
                PolygonNode::allocWithTexture(exitUp),
                PolygonNode::allocWithTexture(exitDown)
            );
            
            // Scale button using the same approach as PolarPairsController
            float buttonScale = (tileSize * 1.4f) / exitUp->getWidth();
            _finishExitButton->setScale(buttonScale);
            _finishExitButton->setAnchor(Vec2::ANCHOR_CENTER);
            
            // Position at the bottom left corner (same as restart button in PolarPairsController)
            _finishExitButton->setPosition(getDisplaySize().width * 0.15f, getDisplaySize().height * 0.1f);
            
            // Add button listener
            _finishExitButton->setName("finishExit");
            _finishExitButton->addListener([=](const std::string& name, bool down) {
                if (down) {  // On press
                    playButtonSound(_assets);
                } else {  // On release
                    _finishExitButton->deactivate();
                    _goToNextLevel = false;  // Make sure we're not going to next level
                    transitionToLevelSelector();
                }
            });
            
            if (!_finishExitButton->getParent()) {
                _finishScene->addChild(_finishExitButton);
            }
        }
    }
    
    // Create restart button (to replay the current level)
    if (!_finishRestartButton) {
        std::shared_ptr<Texture> restartUp = _assets->get<Texture>("Restart");
        std::shared_ptr<Texture> restartDown = _assets->get<Texture>("Restart_Pressed");
        
        if (restartUp && restartDown) {
            _finishRestartButton = Button::alloc(
                PolygonNode::allocWithTexture(restartUp),
                PolygonNode::allocWithTexture(restartDown)
            );
            
            // Scale button using the same approach as PolarPairsController
            float buttonScale = (tileSize * 1.4f) / restartUp->getWidth();
            _finishRestartButton->setScale(buttonScale);
            _finishRestartButton->setAnchor(Vec2::ANCHOR_CENTER);
            _finishRestartButton->setPosition(getDisplaySize().width/2 - getDisplaySize().width * 0.15f, getDisplaySize().height * 0.37f);
            
            // Store original position
            _restartButtonOrigPos = _finishRestartButton->getPosition();
            
            // Add button listener to restart the current level
            _finishRestartButton->setName("finishRestart");
            _finishRestartButton->addListener([=](const std::string& name, bool down) {
                if (down) {  // On press
                    playButtonSound(_assets);
                } else {  // On release
                    _finishRestartButton->deactivate();
                    // Set flag to restart the level directly (use the same path as "next" button)
                    _goToNextLevel = true;
                    // Transition back to the same level
                    transitionToGame(_selectedLevel);
                }
            });
            
            if (!_finishRestartButton->getParent()) {
                _finishScene->addChild(_finishRestartButton);
            }
        }
    }
    
    // Create next level button (only for levels 1-11)
    if (!_finishNextButton) {
        std::shared_ptr<Texture> nextUp = _assets->get<Texture>("Next_Up");
        std::shared_ptr<Texture> nextDown = _assets->get<Texture>("Next_Down");
        
        if (nextUp && nextDown) {
            _finishNextButton = Button::alloc(
                PolygonNode::allocWithTexture(nextUp),
                PolygonNode::allocWithTexture(nextDown)
            );
            
            // Scale button using the same approach as PolarPairsController
            float buttonScale = (tileSize * 1.4f) / nextUp->getWidth();
            _finishNextButton->setScale(buttonScale);
            _finishNextButton->setAnchor(Vec2::ANCHOR_CENTER);
            _finishNextButton->setPosition(getDisplaySize().width/2 + getDisplaySize().width * 0.15f, getDisplaySize().height * 0.37f);
            
            // Store original position
            _nextButtonOrigPos = _finishNextButton->getPosition();
            
            // Add button listener
            _finishNextButton->setName("finishNext");
            _finishNextButton->addListener([=](const std::string& name, bool down) {
                if (down) {  // On press
                    playButtonSound(_assets);
                } else {  // On release
                    _finishNextButton->deactivate();
                    
                    // Calculate next level number
                    int nextLevel = _selectedLevel + 1;
                    int totalLevels = LevelManager::getInstance()->getTotalLevels();
                    
                    // Check if next level exists and is unlocked
                    if (nextLevel <= totalLevels && LevelManager::getInstance()->isLevelUnlocked(nextLevel)) {
                        // Set flag to go to next level directly
                        _goToNextLevel = true;
                        // Transition to the next level
                        _selectedLevel = nextLevel;
                        transitionToGame(nextLevel);
                    }
                }
            });
            
            if (!_finishNextButton->getParent()) {
                _finishScene->addChild(_finishNextButton);
            }
        }
    }
    
    // Set visibility of next button based on level
    if (_finishNextButton) {
        if (_selectedLevel >= 12) {
            // Hide for level 12 (final level)
            _finishNextButton->setVisible(false);
            _finishNextButton->deactivate();
        } else {
            // Show for other levels
            _finishNextButton->setVisible(true);
        }
    }
}

/**
 * Updates the star rating display in the finish scene based on the current level score
 */
void HelloApp::updateFinishSceneStars() {
    if (!_finishStarRating) return;
    
    // Get current level score and determine which star image to use
    int score = LevelManager::getInstance()->getLevelScore(_selectedLevel);
    
    // Add star rating based on score (0-3)
    std::string starTextureName;
    switch (score) {
        case 1: starTextureName = "OneStar"; break;
        case 2: starTextureName = "TwoStars"; break;
        case 3: starTextureName = "ThreeStars"; break;
        default: starTextureName = "NoStar"; break;
    }
    
    // Update star texture if it exists
    auto starTexture = _assets->get<Texture>(starTextureName);
    if (starTexture) {
        _finishStarRating->setTexture(starTexture);
    }
}

/**
 * Handle the transition from game to finish scene
 */
void HelloApp::transitionToFinishScene() {
    if (_isTransitioning) return;
    _isTransitioning = true;
    _isFadingOut = true;
    _transitionTime = 0;
    
    // Update the star rating with the current level score
    updateFinishSceneStars();
    
    // Set up elements for animation
    if (_finishExitButton) {
        _finishExitButton->activate();
    }
    
    // Position elements off-screen for animation entrance
    float offscreenY = -getDisplaySize().height * 0.1f;
    
    // Restart button
    if (_finishRestartButton) {
        _finishRestartButton->setPosition(_restartButtonOrigPos.x, offscreenY);
        _finishRestartButton->activate();
    }
    
    // Next button - only for non-final levels
    if (_finishNextButton) {
        if (_selectedLevel < 12) {
            _finishNextButton->setVisible(true);
            _finishNextButton->setPosition(_nextButtonOrigPos.x, offscreenY);
            _finishNextButton->activate();
        } else {
            _finishNextButton->setVisible(false);
            _finishNextButton->deactivate();
        }
    }
    
    // Position other elements off-screen for animation
    if (_levelFinishedText) {
        _levelFinishedText->setPosition(_levelFinishedOrigPos.x, offscreenY);
    }
    
    if (_finishStarRating) {
        _finishStarRating->setPosition(_starRatingOrigPos.x, offscreenY);
    }
    
    if (_highestText) {
        _highestText->setPosition(_highestTextOrigPos.x, offscreenY);
    }
    
    // Reset animation timer
    _finishAnimTime = 0.0f;
    _isFinishSceneAnimating = true;
}

