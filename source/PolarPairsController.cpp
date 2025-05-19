//
//  PolarPairsController.cpp
//  SKIP
//

#include "PolarPairsController.h"
#include <chrono>
#include <thread>
#include <sstream>
#include "LevelManager.h"
#include <cugl/audio/CUAudioEngine.h>

// Special constant for "no target"
const cugl::Vec2 NO_TARGET(-999, -999);

/**
 * Helper function to play button press sound
 */
void playButtonPressSound(const std::shared_ptr<cugl::AssetManager>& assets) {
    auto buttonSound = assets->get<cugl::audio::Sound>("buttonSound");
    if (buttonSound) {
        cugl::audio::AudioEngine::get()->play("buttonPress", buttonSound, false, 0.8f);
    }
}

/**
 * Helper method to calculate tile size based on screen height
 */
float PolarPairsController::calculateTileSize(const cugl::Size& size) const {
    const float tileHeightRatio = 0.6f;
    const float gridHeight = 11.0f;
    float targetGridHeight = size.height * tileHeightRatio;
    return targetGridHeight / gridHeight;
}

void PolarPairsController::dispose() {
    _restartButton = nullptr;
    _pauseButton = nullptr;
    _exitButton = nullptr;
    _resumeButton = nullptr;
    _pauseOverlay = nullptr;
    _pausedText = nullptr;
    _questionButton = nullptr;
    _returnButton = nullptr;
    _instructionOverlay = nullptr;
    _instructionImage = nullptr;
    _scene = nullptr;
    _assets = nullptr;
}

bool PolarPairsController::init(const std::shared_ptr<cugl::AssetManager>& assets, const cugl::Size& size) {
    _assets = assets;
    if (!assets) return false;
    
    // Activate touch input
#if defined (CU_TOUCH_SCREEN)
    cugl::Input::activate<cugl::Touchscreen>();
#else
    cugl::Input::activate<cugl::Mouse>();
#endif
    
    // Create scene and initialize renderer
    _scene = cugl::scene2::Scene2::allocWithHint(size);
    if (!_scene) return false;
    
    // Create a fresh sprite batch for this scene
    auto batch = cugl::graphics::SpriteBatch::alloc();
    _scene->setSpriteBatch(batch);
    
    // Initialize the action timeline for animations
    _timeline = cugl::ActionTimeline::alloc();
    
    _renderer.init(_scene, _assets);
    _renderer.setFrameTime(0.05f);  // Make animation faster (default is 0.12f)
    
    // Initialize grid
    _grid.clear();
    for (int x = 0; x < GRID_WIDTH; x++) {
        _grid.push_back(std::vector<int>(GRID_HEIGHT, 0));
    }
    
    // Initialize state variables
    _currentLevel = 1;
    _isMoving = false;
    _moveProgress = 0.0f;
    _hasWon = false;
    _touchActive = false;
    _moveDirection = cugl::Vec2::ZERO;
    _bearTravelDistance = 0;
    _penguinTravelDistance = 0;
    _totalTravelDistance = 0;
    _shouldExitToMenu = false;
    _isPaused = false;
    _isShowingInstructions = false;
    
    // Initialize buttons to nullptr
    _restartButton = nullptr;
    _pauseButton = nullptr;
    _questionButton = nullptr;
    
    // Initialize button colors
    _restartButtonPressed = false;
    _pauseButtonPressed = false;
    _restartButtonOrigColor = cugl::Color4::WHITE;
    _pauseButtonOrigColor = cugl::Color4::WHITE;
    
    // Load first level - this will create the buttons too
    loadLevelData(_currentLevel);
    
    return true;
}

void PolarPairsController::loadLevelData(int levelNum) {
    // Load level data
    LevelData levelData = LevelData::loadLevel(_assets, levelNum);
    _currentLevel = levelNum;
    
    // First, completely clear the renderer to remove any old nodes
    _renderer.clear();
    
    // Reset game state
    _isMoving = false;
    _moveProgress = 0.0f;
    _hasWon = false;
    _bearMoves = 0;
    _penguinMoves = 0;
    _bearFinished = false;
    _penguinFinished = false;
    _moveDirection = cugl::Vec2::ZERO;
    _blocksToBreak.clear();
    
    // Set character positions
    _polarBearGridPos = levelData.polarBearPos;
    _penguinGridPos = levelData.penguinPos;
    
    // Set blocks
    _bearBlocks = levelData.bearBlocks;
    _penguinBlocks = levelData.penguinBlocks;
    _breakableBlocks = levelData.breakableBlocks;
    _bearFinishBlocks = levelData.bearFinishBlocks;
    _penguinFinishBlocks = levelData.penguinFinishBlocks;
    
    // Reset grid
    for (auto& column : _grid) std::fill(column.begin(), column.end(), 0);
    
    // Update grid with block positions - all in one loop
    auto updateGrid = [&](const std::vector<cugl::Vec2>& blocks, int type) {
        for (const auto& pos : blocks) {
            if (pos.x >= 0 && pos.x < GRID_WIDTH && pos.y >= 0 && pos.y < GRID_HEIGHT) {
                _grid[pos.x][pos.y] = type;
            }
        }
    };
    
    // Set all blocks in grid with their types
    updateGrid(levelData.blocks, 1);  // Regular obstacles (X)
    updateGrid(levelData.invisibleBlocks, 7);
    updateGrid(_bearBlocks, 2);
    updateGrid(_penguinBlocks, 3);
    updateGrid(_bearFinishBlocks, 4);
    updateGrid(_penguinFinishBlocks, 5);
    updateGrid(_breakableBlocks, 6);
    
    // Update the renderer
    _renderer.drawGrid(_grid);
    _renderer.updateCharacterPositions(_polarBearGridPos, _penguinGridPos);
    
    // Get the tile size from the renderer for button scaling
    float tileSize = _renderer.getTileSize();
    
    // First remove any existing buttons
    if (_restartButton && _restartButton->getParent()) {
        _restartButton->getParent()->removeChild(_restartButton);
        _restartButton = nullptr;
    }
    if (_pauseButton && _pauseButton->getParent()) {
        _pauseButton->getParent()->removeChild(_pauseButton);
        _pauseButton = nullptr;
    }
    
    // Create UI buttons
    cugl::Size size = _scene->getSize();
    
    // Create restart button with normal and pressed textures
    std::shared_ptr<cugl::graphics::Texture> restartUp = _assets->get<cugl::graphics::Texture>("Restart");
    std::shared_ptr<cugl::graphics::Texture> restartDown = _assets->get<cugl::graphics::Texture>("Restart_Pressed");
    
    if (restartUp && restartDown) {
        _restartButton = cugl::scene2::Button::alloc(
            cugl::scene2::PolygonNode::allocWithTexture(restartUp),
            cugl::scene2::PolygonNode::allocWithTexture(restartDown)
        );
        
        // Get tile size from renderer for scaling
        float tileSize = _renderer.getTileSize();
        
        // Calculate button scale based on tile size
        float buttonScale;
        if (tileSize > 0) {
            // Use tile size and the ratio constant for scaling
            buttonScale = tileSize * BUTTON_TO_TILE_RATIO / restartUp->getWidth();
        } else {
            // Fallback to original calculation if tile size not available
            buttonScale = size.width * BUTTON_SCALE_FACTOR / restartUp->getWidth();
        }
        
        _restartButton->setScale(buttonScale);
        _restartButton->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        _restartButton->setPosition(size.width * RESTART_BUTTON_X, size.height * RESTART_BUTTON_Y);
        _restartButton->setPriority(1000);
        
        // Add button listener
        _restartButton->setName("restart");
        _restartButton->addListener([=] (const std::string& name, bool down) {
            if (down) {  // On press
                playButtonPressSound(_assets);
            } else {  // On release
                restartLevel();
            }
        });
        
        _scene->addChild(_restartButton);
        _restartButton->activate();
    }
    
    // Create pause button with normal and pressed textures
    std::shared_ptr<cugl::graphics::Texture> pauseUp = _assets->get<cugl::graphics::Texture>("Pause");
    std::shared_ptr<cugl::graphics::Texture> pauseDown = _assets->get<cugl::graphics::Texture>("Pause_Pressed");
    
    if (pauseUp && pauseDown) {
        _pauseButton = cugl::scene2::Button::alloc(
            cugl::scene2::PolygonNode::allocWithTexture(pauseUp),
            cugl::scene2::PolygonNode::allocWithTexture(pauseDown)
        );
        
        // Calculate button scale based on tile size or fallback to screen-based calculation
        float buttonScale;
        if (tileSize > 0) {
            // Use tile size and the ratio for scaling
            buttonScale = tileSize * BUTTON_TO_TILE_RATIO / pauseUp->getWidth();
        } else {
            // Fallback to the original calculation
            buttonScale = size.width * BUTTON_SCALE_FACTOR / pauseUp->getWidth();
        }
        _pauseButton->setScale(buttonScale);
        _pauseButton->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        _pauseButton->setPosition(size.width * PAUSE_BUTTON_X, size.height * PAUSE_BUTTON_Y);
        _pauseButton->setPriority(1000);
        
        // Add button listener
        _pauseButton->setName("pause");
        _pauseButton->addListener([=] (const std::string& name, bool down) {
            if (down) {  // On press
                playButtonPressSound(_assets);
            } else {  // On release
                togglePause();
            }
        });
        
        _scene->addChild(_pauseButton);
        _pauseButton->activate();
    }
    
    // Create question button (for instructions)
    std::shared_ptr<cugl::graphics::Texture> questionUp = _assets->get<cugl::graphics::Texture>("Question_Up");
    std::shared_ptr<cugl::graphics::Texture> questionDown = _assets->get<cugl::graphics::Texture>("Question_Down");
    
    if (questionUp && questionDown) {
        _questionButton = cugl::scene2::Button::alloc(
            cugl::scene2::PolygonNode::allocWithTexture(questionUp),
            cugl::scene2::PolygonNode::allocWithTexture(questionDown)
        );
        
        // Calculate button scale based on tile size or fallback to screen-based calculation
        float buttonScale;
        if (tileSize > 0) {
            // Use tile size and the ratio for scaling
            buttonScale = tileSize * BUTTON_TO_TILE_RATIO / questionUp->getWidth();
        } else {
            // Fallback to the original calculation
            buttonScale = size.width * BUTTON_SCALE_FACTOR / questionUp->getWidth();
        }
        _questionButton->setScale(buttonScale);
        _questionButton->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        _questionButton->setPosition(size.width * 0.85f, size.height * 0.9f);  // Top right corner
        _questionButton->setPriority(1000);
        
        // Add button listener
        _questionButton->setName("question");
        _questionButton->addListener([=] (const std::string& name, bool down) {
            if (down) {  // On press
                playButtonPressSound(_assets);
            } else {  // On release
                showInstructions();
            }
        });
        
        _scene->addChild(_questionButton);
        _questionButton->activate();
    }
}

void PolarPairsController::switchLevel(int levelNum) {

    //load the actual requested level
    loadLevelData(levelNum);
}

void PolarPairsController::update(float timestep) {
    // Update timeline for animations
    if (_timeline) {
        _timeline->update(timestep);
    }
    
    // Check if win condition was met and we're waiting for animation to finish
    if (_winConditionMet) {
        _winDelay -= timestep;
        if (_winDelay <= 0) {
            // Delay is over, set the win flag now
            _hasWon = true;
            _winConditionMet = false;
        }
    }
    
    // Process delayed block removals
    for (auto it = _blocksToRemove.begin(); it != _blocksToRemove.end(); ) {
        it->timeRemaining -= timestep;
        
        if (it->timeRemaining <= 0) {
            // Time to update the visual appearance of the cell
            int x = it->x;
            int y = it->y;
            
            // Only update the visual - the grid was already updated when the block was broken
            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                // Log before updating cell
                CULog("Updating cell %d,%d to empty after block break animation", x, y);
                // Update visual to show empty space
                _renderer.updateCell(x, y, 8);
            }
            
            // Remove from list
            it = _blocksToRemove.erase(it);
        } else {
            ++it;
        }
    }
    
    // Update renderer animations
    _renderer.update(timestep);
    
    // Update scene for button interactions
    _scene->update(timestep);
    
    // Log pause state at the beginning of each update
    static bool lastPauseState = false;
    if (_isPaused != lastPauseState) {
        lastPauseState = _isPaused;
    }
    
    // Handle fade animations for pause menu and instructions
    if (_isFading) {
        _fadeTime += timestep;
        float progress = _fadeTime / _fadeDuration;
        
        if (progress >= 1.0f) {
            // Fade animation complete
            _isFading = false;
            _fadeTime = 0;
            
            if (!_isFadingIn) {
                // Fade out complete - actually remove UI elements
                if (!_isPaused) {
                    // Remove pause menu elements
                    if (_pauseOverlay) {
                        _scene->removeChild(_pauseOverlay);
                        _pauseOverlay = nullptr;
                    }
                    
                    if (_resumeButton) {
                        _resumeButton->deactivate();
                        _scene->removeChild(_resumeButton);
                        _resumeButton = nullptr;
                    }
                    
                    if (_exitButton) {
                        _exitButton->deactivate();
                        _scene->removeChild(_exitButton);
                        _exitButton = nullptr;
                    }
                    
                    if (_pausedText) {
                        _scene->removeChild(_pausedText);
                        _pausedText = nullptr;
                    }
                }
                
                if (!_isShowingInstructions) {
                    // Remove instruction elements
                    if (_instructionOverlay) {
                        _scene->removeChild(_instructionOverlay);
                        _instructionOverlay = nullptr;
                    }
                    
                    if (_instructionImage) {
                        _scene->removeChild(_instructionImage);
                        _instructionImage = nullptr;
                    }
                    
                    if (_returnButton) {
                        _returnButton->deactivate();
                        _scene->removeChild(_returnButton);
                        _returnButton = nullptr;
                    }
                }
                
                // Re-enable gameplay buttons if we're not in either menu
                if (!_isPaused && !_isShowingInstructions) {
                    if (_restartButton) _restartButton->activate();
                    if (_pauseButton) _pauseButton->activate();
                    if (_questionButton) _questionButton->activate();
                }
            } else {
                // Fade in complete - fully opaque
                if (_isPaused) {
                    if (_pauseOverlay) _pauseOverlay->setColor(cugl::Color4(0, 0, 0, 192));
                    if (_pausedText) _pausedText->setColor(cugl::Color4(255, 255, 255, 255));
                    if (_resumeButton) _resumeButton->setColor(cugl::Color4(255, 255, 255, 255));
                    if (_exitButton) _exitButton->setColor(cugl::Color4(255, 255, 255, 255));
                } else if (_isShowingInstructions) {
                    if (_instructionOverlay) _instructionOverlay->setColor(cugl::Color4(0, 0, 0, 192));
                    if (_instructionImage) _instructionImage->setColor(cugl::Color4(255, 255, 255, 255));
                    if (_returnButton) _returnButton->setColor(cugl::Color4(255, 255, 255, 255));
                }
            }
        } else {
            // Update transparency based on animation progress
            if (_isFadingIn) {
                // Fading in - increase alpha
                int alpha = (int)(192 * progress);
                int textAlpha = (int)(255 * progress);
                
                if (_isPaused) {
                    if (_pauseOverlay) _pauseOverlay->setColor(cugl::Color4(0, 0, 0, alpha));
                    if (_pausedText) _pausedText->setColor(cugl::Color4(255, 255, 255, textAlpha));
                    if (_resumeButton) _resumeButton->setColor(cugl::Color4(255, 255, 255, textAlpha));
                    if (_exitButton) _exitButton->setColor(cugl::Color4(255, 255, 255, textAlpha));
                } else if (_isShowingInstructions) {
                    if (_instructionOverlay) _instructionOverlay->setColor(cugl::Color4(0, 0, 0, alpha));
                    if (_instructionImage) _instructionImage->setColor(cugl::Color4(255, 255, 255, textAlpha));
                    if (_returnButton) _returnButton->setColor(cugl::Color4(255, 255, 255, textAlpha));
                }
            } else {
                // Fading out - decrease alpha
                int alpha = (int)(192 * (1.0f - progress));
                int textAlpha = (int)(255 * (1.0f - progress));
                
                if (!_isPaused) {
                    if (_pauseOverlay) _pauseOverlay->setColor(cugl::Color4(0, 0, 0, alpha));
                    if (_pausedText) _pausedText->setColor(cugl::Color4(255, 255, 255, textAlpha));
                    if (_resumeButton) _resumeButton->setColor(cugl::Color4(255, 255, 255, textAlpha));
                    if (_exitButton) _exitButton->setColor(cugl::Color4(255, 255, 255, textAlpha));
                }
                
                if (!_isShowingInstructions) {
                    if (_instructionOverlay) _instructionOverlay->setColor(cugl::Color4(0, 0, 0, alpha));
                    if (_instructionImage) _instructionImage->setColor(cugl::Color4(255, 255, 255, textAlpha));
                    if (_returnButton) _returnButton->setColor(cugl::Color4(255, 255, 255, textAlpha));
                }
            }
        }
    }
    
    // Process the appropriate input based on current state
    if (_isPaused && !_isFading) {
        processPauseMenuInput();
    } else if (_isShowingInstructions && !_isFading) {
        processInstructionsInput();
    } else if (!_isPaused && !_isShowingInstructions && !_isFading) {
        // Only process gameplay when not in a menu
        if (!_isMoving) {
            processInput();
        } else {
            // Update character movement
            updateMovement(timestep);
        }
        
        // Update breaking blocks regardless of movement state
        updateBlockBreaking(timestep);
    }
}

void PolarPairsController::updateMovement(float timestep) {
    // Movement speed in grid cells per second
    float cellsPerSecond = 15.0f;
    
    // Progress movement based on total distance
    _moveProgress += (cellsPerSecond * timestep) / _totalTravelDistance;
    
    if (_moveProgress >= 1.0f) {
        // Movement complete
        if (!_bearFinished) {
            _polarBearGridPos = _polarBearTarget;
        }
        if (!_penguinFinished) {
            _penguinGridPos = _penguinTarget;
        }
        _renderer.updateCharacterPositions(_polarBearGridPos, _penguinGridPos);
        
        // Check if characters actually moved
        bool bearMoved = _polarBearGridPos != _polarBearPrevPos;
        bool penguinMoved = _penguinGridPos != _penguinPrevPos;
        
        // Check if both characters reached their destinations at the same time
        if (bearMoved && penguinMoved) {
            float bearDistance = (_polarBearTarget - _polarBearPrevPos).length();
            float penguinDistance = (_penguinTarget - _penguinPrevPos).length();
            
            // Both characters moved a significant distance in the same move
            if (bearDistance > 0.01f && penguinDistance > 0.01f) {
                _simultaneousDestinationReached = true;
                CULog("Both characters moved to destinations simultaneously!");
            }
        }
        
        // Increment move counters if characters actually moved
        if (bearMoved) _bearMoves++;
        if (penguinMoved) _penguinMoves++;
        
        // Reset movement state
        _isMoving = false;
        _moveDirection = cugl::Vec2::ZERO;
        _moveProgress = 0.0f;
        
        // Check win and finish conditions
        updateFinishState();
        checkWinCondition();
    } else {
        // Calculate individual progress values for each character
        float bearProgress = _bearFinished ? 1.0f : 
            (_bearTravelDistance > 0 ? std::min(1.0f, (_moveProgress * _totalTravelDistance) / _bearTravelDistance) : 1.0f);
        float penguinProgress = _penguinFinished ? 1.0f :
            (_penguinTravelDistance > 0 ? std::min(1.0f, (_moveProgress * _totalTravelDistance) / _penguinTravelDistance) : 1.0f);
        
        // Interpolate positions based on individual progress
        cugl::Vec2 bearPos = _polarBearPrevPos + (_polarBearTarget - _polarBearPrevPos) * bearProgress;
        cugl::Vec2 penguinPos = _penguinPrevPos + (_penguinTarget - _penguinPrevPos) * penguinProgress;
        
        // Update renderer with interpolated positions
        _renderer.updateCharacterPositions(bearPos, penguinPos);
    }
}

void PolarPairsController::processInput() {
    // Only process input if we're not already moving
    if (_isMoving) return;
    
    // TOUCH HANDLING for gameplay swipes
    auto touch = cugl::Input::get<cugl::Touchscreen>();
    if (touch) {
        // Check current touch state
        bool hasTouchNow = (touch->touchCount() > 0);
        
        // Get touch position if available
        if (hasTouchNow) {
            auto& touchSet = touch->touchSet();
            if (!touchSet.empty()) {
                cugl::TouchID tid = *(touchSet.begin());
                cugl::Vec2 currentPos = touch->touchPosition(tid);
                
                // Convert touch coordinates to scene space
                cugl::Vec2 scenePos = _scene->screenToWorldCoords(currentPos);
                
                // Debug touch coordinates when in pause menu
                if (_isPaused) {
                    // Track touch state like we do for regular gameplay
                    if (touch->touchDown(tid) && !_touchActive) {
                        _touchActive = true;
                        _touchStart = scenePos;
                        CULog("Touch started in pause menu");
                    }
                    
                    // Save current position
                    _lastTouchPos = scenePos;
                    
                    // Handle resume button
                    if (_resumeButton) {
                        cugl::Rect resumeBounds = _resumeButton->getBoundingBox();
                        CULog("Resume button bounds: (%f, %f) to (%f, %f)", 
                            resumeBounds.origin.x, resumeBounds.origin.y,
                            resumeBounds.origin.x + resumeBounds.size.width, 
                            resumeBounds.origin.y + resumeBounds.size.height);
                        
                        // Check if touch is over resume button and update button state
                        if (resumeBounds.contains(scenePos)) {
                            CULog("Touch is over resume button");
                            _resumeButton->setDown(touch->touchDown(tid));
                        } else {
                            _resumeButton->setDown(false);
                        }
                    }
                    
                    // Handle exit button
                    if (_exitButton) {
                        cugl::Rect exitBounds = _exitButton->getBoundingBox();
                        CULog("Exit button bounds: (%f, %f) to (%f, %f)", 
                            exitBounds.origin.x, exitBounds.origin.y,
                            exitBounds.origin.x + exitBounds.size.width, 
                            exitBounds.origin.y + exitBounds.size.height);
                        
                        // Check if touch is over exit button and update button state
                        if (exitBounds.contains(scenePos)) {
                            CULog("Touch is over exit button");
                            _exitButton->setDown(touch->touchDown(tid));
                        } else {
                            _exitButton->setDown(false);
                        }
                    }
                    
                    // When paused, don't process any other input
                    return;
                }
                
                // Similarly for instruction menu
                if (_isShowingInstructions) {
                    // Track touch state
                    if (touch->touchDown(tid) && !_touchActive) {
                        _touchActive = true;
                        _touchStart = scenePos;
                        CULog("Touch started in instruction menu");
                    }
                    
                    // Save current position
                    _lastTouchPos = scenePos;
                    
                    // Handle return button
                    if (_returnButton) {
                        cugl::Rect returnBounds = _returnButton->getBoundingBox();
                        
                        // Check if touch is over return button and update button state
                        if (returnBounds.contains(scenePos)) {
                            CULog("Touch is over return button");
                            _returnButton->setDown(touch->touchDown(tid));
                        } else {
                            _returnButton->setDown(false);
                        }
                    }
                    
                    // When showing instructions, don't process any other input
                    return;
                }
                
                // Check if touch is over any gameplay button (only when not paused)
                bool overButton = false;
                if (_restartButton) {
                    cugl::Rect bounds = _restartButton->getBoundingBox();
                    if (bounds.contains(scenePos)) {
                        overButton = true;
                        // Let the button handle its own state
                        _restartButton->setDown(touch->touchDown(tid));
                    } else {
                        _restartButton->setDown(false);
                    }
                }
                if (_pauseButton) {
                    cugl::Rect bounds = _pauseButton->getBoundingBox();
                    if (bounds.contains(scenePos)) {
                        overButton = true;
                        // Let the button handle its own state
                        _pauseButton->setDown(touch->touchDown(tid));
                    } else {
                        _pauseButton->setDown(false);
                    }
                }
                if (_questionButton) {
                    cugl::Rect bounds = _questionButton->getBoundingBox();
                    if (bounds.contains(scenePos)) {
                        overButton = true;
                        // Let the button handle its own state
                        _questionButton->setDown(touch->touchDown(tid));
                        CULog("Touch is over question button: %d", touch->touchDown(tid));
                    } else {
                        _questionButton->setDown(false);
                    }
                }
                
                // Only process game input if not over a button
                if (!overButton) {
                    // If this is a new touch (wasn't active last frame)
                    if (touch->touchDown(tid) && !_touchActive) {
                        _touchActive = true;
                        _touchStart = scenePos;
                }
                
                // Save current position
                    _lastTouchPos = scenePos;
            }
        }
        } else {
            // No touch - reset button states
            if (_restartButton) _restartButton->setDown(false);
            if (_pauseButton) _pauseButton->setDown(false);
            if (_questionButton) _questionButton->setDown(false);
            
            // Reset pause menu button states too
            if (_isPaused) {
                if (_resumeButton) _resumeButton->setDown(false);
                if (_exitButton) _exitButton->setDown(false);
                
                // Handle touch release in pause menu
                if (_touchActive) {
                    CULog("Touch ended in pause menu");
                    cugl::Vec2 releasePos = _lastTouchPos;
                    
                    // Check if released over resume button
                    if (_resumeButton) {
                        cugl::Rect resumeBounds = _resumeButton->getBoundingBox();
                        if (resumeBounds.contains(releasePos)) {
                            CULog("Resume button action triggered");
                            togglePause();
                        }
                    }
                    
                    // Check if released over exit button
                    if (_exitButton) {
                        cugl::Rect exitBounds = _exitButton->getBoundingBox();
                        if (exitBounds.contains(releasePos)) {
                            CULog("Exit button action triggered");
                            _hasWon = false;
                            _shouldExitToMenu = true;
                            hidePauseMenu();
                        }
                    }
                    
                    // Reset touch state
                    _touchActive = false;
                }
            }
            
            // Handle instructions button release
            if (_isShowingInstructions) {
                if (_returnButton) _returnButton->setDown(false);
                
                // Handle touch release in instruction menu
                if (_touchActive) {
                    CULog("Touch ended in instruction menu");
                    cugl::Vec2 releasePos = _lastTouchPos;
                    
                    // Check if released over return button
                    if (_returnButton) {
                        cugl::Rect returnBounds = _returnButton->getBoundingBox();
                        if (returnBounds.contains(releasePos)) {
                            CULog("Return button action triggered");
                            hideInstructions();
                        }
                    }
                    
                    // Reset touch state
                    _touchActive = false;
                }
            }
            
        // If touch ended, check for swipe
            if (_touchActive) {
            cugl::Vec2 delta = _lastTouchPos - _touchStart;
            float length = delta.length();
            
            // Minimum swipe threshold
            if (length >= 15.0f) {
                    // Normalize delta to get direction
                    delta.normalize();
                    
                    // Determine dominant direction
                    if (std::abs(delta.x) > std::abs(delta.y)) {
                    // Horizontal swipe
                        if (delta.x > 0) moveCharacters(cugl::Vec2(1, 0));
                        else moveCharacters(cugl::Vec2(-1, 0));
                } else {
                        // Vertical swipe - reversed direction
                        if (delta.y > 0) moveCharacters(cugl::Vec2(0, 1)); // Down swipe
                        else moveCharacters(cugl::Vec2(0, -1)); // Up swipe
                    }
            }
            
            // Reset touch state
            _touchActive = false;
        }
    }
    }
}

void PolarPairsController::checkWinCondition() {
    if (_bearFinished && _penguinFinished && !_winConditionMet) {
        // Play level complete sound
        auto levelCompleteSound = _assets->get<cugl::audio::Sound>("levelCompleteSound");
        if (levelCompleteSound) {
            cugl::audio::AudioEngine::get()->play("levelComplete", levelCompleteSound, false, 0.8f);
        }
        
        // Initialize score to 0
        int score = 0;
        
        // Get the target bear and seal step counts from the level file
        std::string assetDir = cugl::Application::get()->getAssetDirectory();
        std::string levelPath = assetDir + "levels/level" + std::to_string(_currentLevel) + ".txt";
        
        int targetBearSteps = 0;
        int targetSealSteps = 0;
        
        std::ifstream file(levelPath);
        if (file.is_open()) {
            std::string line;
            int lineNumber = 0;
            
            while (std::getline(file, line) && lineNumber <= 14) {
                lineNumber++;
                
                // Line 12 contains target bear steps
                if (lineNumber == 14) {
                    std::istringstream iss(line);
                    iss >> targetBearSteps;
                }
                
                // Line 13 contains target seal steps
                if (lineNumber == 15) {
                    std::istringstream iss(line);
                    iss >> targetSealSteps;
                }
            }
            file.close();
        }
        
        CULog("Bear moves: %d (target: %d), Seal moves: %d (target: %d)", 
              _bearMoves, targetBearSteps, _penguinMoves, targetSealSteps);
        
        // Score +1 if bear takes less steps than target
        if (_bearMoves <= targetBearSteps) {
            score += 1;
        }
        
        // Score +1 if seal takes less steps than target
        if (_penguinMoves <= targetSealSteps) {
            score += 1;
        }
        
        // Add third point if both characters reached destination at the same time
        // This is now handled during movement in updateMovement()
        if (_simultaneousDestinationReached) {
            score += 1;
            CULog("Both characters reached destinations simultaneously! +1 point");
        }
        
        // Ensure score is between 0 and 3
        score = std::max(0, std::min(score, 3));
        
        // Save the score and unlock next level
        LevelManager::getInstance()->setLevelScore(_currentLevel, score);
        LevelManager::getInstance()->unlockLevel(_currentLevel + 1);
        
        // Set win condition flag and delay
        _winConditionMet = true;
        _winDelay = 0.7f; // Wait 0.7 seconds before transitioning to win scene
        
        // Don't set _hasWon yet - it will be set after the delay
    }
}

// --- Main movement and squeeze logic ---
void PolarPairsController::moveCharacters(const cugl::Vec2& direction) {
    if (!_isMoving && direction != cugl::Vec2::ZERO) {
        _moveDirection = direction;
        _isMoving = true;
        
        // Calculate targets first to determine if a squeeze will occur
        calculateMovementTargets();
        
        // If a squeeze occurred, trigger bounce animation for the character being pushed (moving opposite to input direction)
        if (_squeezeJustOccurred) {
            _renderer.startCharacterBounceAnimation(_bearIsBeingPushed);
        }
        
        // Check if movement was blocked (targets equal current positions)
        if (_polarBearTarget == _polarBearGridPos && _penguinTarget == _penguinGridPos) {
            // Play the blocked sound
            auto blockedSound = _assets->get<cugl::audio::Sound>("blockedSound");
            if (blockedSound) {
                cugl::audio::AudioEngine::get()->play("blocked", blockedSound, false, 0.8f);
            }
            
            // Trigger the blocked animation
            _renderer.startBlockedAnimation(_moveDirection);
        } else if (!_squeezeJustOccurred) {
            // Play move sound ONLY if not a squeeze (squeeze sound is played in calculateMovementTargets)
            auto moveSound = _assets->get<cugl::audio::Sound>("moveSound");
            if (moveSound) {
                cugl::audio::AudioEngine::get()->play("move", moveSound, false, 0.8f);
            }
        }
        // Note: The squeeze sound is already played in calculateMovementTargets when _squeezeJustOccurred becomes true
    }
}

void PolarPairsController::calculateMovementTargets() {
    // Store previous positions before calculating targets
    _polarBearPrevPos = _polarBearGridPos;
    _penguinPrevPos = _penguinGridPos;
    
    // Initialize targets to current positions
    _polarBearTarget = _polarBearGridPos;
    _penguinTarget = _penguinGridPos;
    
    // If both characters are on their finish blocks, don't calculate new targets
    if (_bearFinished && _penguinFinished) {
        _isMoving = false;
        _moveDirection = cugl::Vec2::ZERO;
        _moveProgress = 0.0f;
        return;
    }
    
    // Check for squeeze first - handles a special movement case
    _squeezeJustOccurred = checkForSqueeze();
    
    if (_squeezeJustOccurred) {
        // Play squeeze sound
        auto squeezeSound = _assets->get<cugl::audio::Sound>("squeezeSound");
        if (squeezeSound) {
            cugl::audio::AudioEngine::get()->play("squeeze", squeezeSound, false, 0.8f);
        }
        
        // Squeeze mechanics: rear character pushes front character in opposite direction
        bool bearFront = !_polarBearIsRear;
        
        // The bear is the one moving in the opposite direction if it's the front character
        _bearIsBeingPushed = bearFront;
        
        // Calculate movements for front and rear characters
        cugl::Vec2 frontStart = bearFront ? _polarBearGridPos : _penguinGridPos;
        cugl::Vec2 rearStart = bearFront ? _penguinGridPos : _polarBearGridPos;
        bool frontIsPenguin = !bearFront;
        
        // Calculate slide targets
        cugl::Vec2 frontTarget = slide(frontStart, frontIsPenguin, true, -_moveDirection);
        cugl::Vec2 rearTarget = slide(rearStart, !frontIsPenguin, true, _moveDirection);
        
        // Assign targets based on finish state
        if (bearFront) {
            if (!_bearFinished) _polarBearTarget = frontTarget;
            if (!_penguinFinished) _penguinTarget = rearTarget;
        } else {
            if (!_penguinFinished) _penguinTarget = frontTarget;
            if (!_bearFinished) _polarBearTarget = rearTarget;
        }
    } else {
        // Standard movement: determine which character is in front
        bool bearInFront = false;
        if (_moveDirection.x != 0) {
            bearInFront = (_moveDirection.x > 0) ? 
                          (_polarBearGridPos.x > _penguinGridPos.x) : 
                          (_polarBearGridPos.x < _penguinGridPos.x);
        } else {
            bearInFront = (_moveDirection.y > 0) ? 
                          (_polarBearGridPos.y > _penguinGridPos.y) : 
                          (_polarBearGridPos.y < _penguinGridPos.y);
        }
        
        // Calculate targets for front and rear characters
        cugl::Vec2 frontPos = bearInFront ? _polarBearGridPos : _penguinGridPos;
        cugl::Vec2 rearPos = bearInFront ? _penguinGridPos : _polarBearGridPos;
        bool frontIsPenguin = !bearInFront;
        
        // Calculate front character target first
        cugl::Vec2 frontTarget = frontPos;
        if (!(_bearFinished && bearInFront) && !(_penguinFinished && !bearInFront)) {
            frontTarget = computeTarget(frontPos, frontIsPenguin, false, _moveDirection, rearPos, NO_TARGET);
            if (bearInFront) _polarBearTarget = frontTarget;
            else _penguinTarget = frontTarget;
        }
        
        // Now calculate rear character target using the UPDATED front target position
        cugl::Vec2 frontCharacterNewPos = frontTarget;
        if (!(_bearFinished && !bearInFront) && !(_penguinFinished && bearInFront)) {
            cugl::Vec2 rearTarget = computeTarget(rearPos, !frontIsPenguin, false, _moveDirection, 
                                                  frontCharacterNewPos, frontTarget);
            if (bearInFront) _penguinTarget = rearTarget;
            else _polarBearTarget = rearTarget;
        }
        
        // Prevent character overlap
        if (_polarBearTarget == _penguinTarget) {
            if (bearInFront) _penguinTarget = _polarBearTarget - _moveDirection;
            else _polarBearTarget = _penguinTarget - _moveDirection;
        }
    }
    
    // Calculate travel distances for smooth movement
    _bearTravelDistance = (_polarBearTarget - _polarBearGridPos).length();
    _penguinTravelDistance = (_penguinTarget - _penguinGridPos).length();
    _totalTravelDistance = std::max(_bearTravelDistance, _penguinTravelDistance);
    
    // Reset movement progress
    _moveProgress = 0;
}

cugl::Vec2 PolarPairsController::calculateSingleCharacterMove(const cugl::Vec2& start, bool isPenguin, const cugl::Vec2& direction) {
    cugl::Vec2 curr = start;
    cugl::Vec2 next = curr + direction;
    
    // Check boundary
    if (next.x < 0 || next.x >= GRID_WIDTH || next.y < 0 || next.y >= GRID_HEIGHT) {
        return curr;
    }
    
    // Check cell type
    int cell = _grid[next.x][next.y];
    
    // Check for obstacles
    if (cell == 1 || cell == 7) { // Regular or invisible block
        return curr;
    }
    
    // Check for character-specific blocks
    if (isPenguin) {
        if (cell == 2 || cell == 4) { // Bear block or finish
            return curr;
        }
    } else {
        if (cell == 3 || cell == 5) { // Penguin block or finish
            return curr;
        }
    }
    
    // Handle breakable blocks
    if (cell == 6) {
        scheduleBlockBreaking(next.x, next.y, 0.2f);
    }
    
    // Check for other character
    if (next == (isPenguin ? _polarBearGridPos : _penguinGridPos)) {
        return curr;
    }
    
    return next;
}

// --- Squeeze logic ---
bool PolarPairsController::checkForSqueeze() {
    cugl::Vec2 diff = _polarBearGridPos - _penguinGridPos;
    
    // Quick check: must be adjacent and aligned with movement direction
    if (diff.length() != 1.0f) return false;
    
    // Check alignment with movement in one step
    bool isHorizontal = _moveDirection.x != 0;
    if ((isHorizontal && diff.y != 0) || (!isHorizontal && diff.x != 0)) return false;
    
    // Determine which character is in front (simplified logic)
    bool bearInFront = (isHorizontal ? 
                       (_moveDirection.x * diff.x > 0) : 
                       (_moveDirection.y * diff.y > 0));
    
    // Get front character position and next position in one step
    cugl::Vec2 frontPos = bearInFront ? _polarBearGridPos : _penguinGridPos;
    cugl::Vec2 frontNext = frontPos + _moveDirection;
    
    // Check boundary first (fastest check)
    if (frontNext.x < 0 || frontNext.x >= GRID_WIDTH || 
        frontNext.y < 0 || frontNext.y >= GRID_HEIGHT) {
        // Only check special blocks if we would squeeze
        if (checkNoSqueezeBlocks()) return false;
        _polarBearIsRear = !bearInFront;
        return true;
    }
    
    // Check cell type
    int cell = _grid[frontNext.x][frontNext.y];
    bool blocked = (cell == 1 || cell == 7) || // Regular or invisible block
                  (bearInFront && (cell == 3 || cell == 5)) || // Bear blocked by penguin tiles
                  (!bearInFront && (cell == 2 || cell == 4)); // Penguin blocked by bear tiles
    
    if (cell == 6) { // Breakable block
        scheduleBlockBreaking(frontNext.x, frontNext.y, 0.2f);
        blocked = true;
    }
    
    if (blocked) {
        // Only check special blocks if we would squeeze
        if (checkNoSqueezeBlocks()) return false;
        _polarBearIsRear = !bearInFront;
        return true;
    }
    
    return false;
}

// Helper function to check if either character is on their special blocks (passable or finish)
bool PolarPairsController::checkNoSqueezeBlocks() {
    int bearCell = _grid[_polarBearGridPos.x][_polarBearGridPos.y];
    int penguinCell = _grid[_penguinGridPos.x][_penguinGridPos.y];
    // Check both passable blocks (2,3) and finish blocks (4,5)
    return (bearCell == 2 || bearCell == 4 || penguinCell == 3 || penguinCell == 5);
}

cugl::Vec2 PolarPairsController::slide(const cugl::Vec2& start, bool isPenguin, bool canBreak, const cugl::Vec2& direction) {
    cugl::Vec2 curr = start;
    
    while (true) {
        cugl::Vec2 next = curr + direction;
        
        // Check boundary
        if (next.x < 0 || next.x >= GRID_WIDTH || next.y < 0 || next.y >= GRID_HEIGHT) break;
        
        // Check cell type
        int cell = _grid[next.x][next.y];
        if (cell == 1 || cell == 7) break; // Regular or invisible block
        
        // Block characters from entering wrong finish blocks or other character's blocks
        if (isPenguin) {
            if (cell == 2 || cell == 4) break; // Block penguin from bear blocks and bear finish
        } else {
            if (cell == 3 || cell == 5) break; // Block bear from penguin blocks and penguin finish
        }
        
        // Check if the next cell is a finish block for this character
        bool isFinishBlock = false;
        if (isPenguin) {
            // Check if next is a penguin finish block
            for (const auto& pos : _penguinFinishBlocks) {
                if (next.x == pos.x && next.y == pos.y) {
                    isFinishBlock = true;
                    break;
                }
            }
        } else {
            // Check if next is a bear finish block
            for (const auto& pos : _bearFinishBlocks) {
                if (next.x == pos.x && next.y == pos.y) {
                    isFinishBlock = true;
                    break;
                }
            }
        }
        
        // If moving to a finish block, move there and stop
        if (isFinishBlock) {
            curr = next;
            break;
        }
        
        if (cell == 6) { // Breakable block
            if (canBreak) {
                // Schedule block to break with a delay
                scheduleBlockBreaking(next.x, next.y, 0.2f);
                // Continue moving after breaking the block
                curr = next;
            } else {
                curr = next; // allow moving into breakable block for check
                break;
            }
        } else {
            // Move to next position for non-breakable blocks
            curr = next;
        }
    }
    
    return curr;
}

cugl::Vec2 PolarPairsController::computeTarget(const cugl::Vec2& start, bool isPenguin, bool canBreak,
                                       const cugl::Vec2& direction,
                                       const cugl::Vec2& otherStart,
                                       const cugl::Vec2& otherTarget) {
    cugl::Vec2 curr = start;
    
    while (true) {
        cugl::Vec2 next = curr + direction;
        
        // Check boundary
        if (next.x < 0 || next.x >= GRID_WIDTH || next.y < 0 || next.y >= GRID_HEIGHT) break;
        
        // Check for other character's current or future position
        if (next == otherTarget || (otherTarget == NO_TARGET && next == otherStart)) break;
        
        // Check if characters are aligned on movement axis and would pass through each other
        bool wouldPassThroughOther = false;
        if (direction.x != 0) { // Horizontal movement
            if (next.y == otherStart.y) { // Same row
                // Check if we'd pass through the other character
                if ((direction.x > 0 && curr.x < otherStart.x && next.x > otherStart.x) || 
                    (direction.x < 0 && curr.x > otherStart.x && next.x < otherStart.x)) {
                    wouldPassThroughOther = true;
                }
            }
        } else if (direction.y != 0) { // Vertical movement
            if (next.x == otherStart.x) { // Same column
                // Check if we'd pass through the other character
                if ((direction.y > 0 && curr.y < otherStart.y && next.y > otherStart.y) || 
                    (direction.y < 0 && curr.y > otherStart.y && next.y < otherStart.y)) {
                    wouldPassThroughOther = true;
                }
            }
        }
        
        if (wouldPassThroughOther) break;
        
        // Check cell type
        int cell = _grid[next.x][next.y];
        if (cell == 1 || cell == 7) break; // Regular or invisible block
        if (isPenguin && (cell == 2 || cell == 4)) break; // Bear block or finish
        if (!isPenguin && (cell == 3 || cell == 5)) break; // Penguin block or finish
        
        // Check if the next cell is a finish block for this character
        bool isFinishBlock = false;
        if (isPenguin) {
            // Check if next is a penguin finish block
            for (const auto& pos : _penguinFinishBlocks) {
                if (next.x == pos.x && next.y == pos.y) {
                    isFinishBlock = true;
                    break;
                }
            }
        } else {
            // Check if next is a bear finish block
            for (const auto& pos : _bearFinishBlocks) {
                if (next.x == pos.x && next.y == pos.y) {
                    isFinishBlock = true;
                    break;
                }
            }
        }
        
        // If moving to a finish block, move there and stop
        if (isFinishBlock) {
            curr = next;
            break;
        }
        
        if (cell == 6) { // Breakable block
            if (canBreak) {
                // Schedule block to break with a delay
                scheduleBlockBreaking(next.x, next.y, 0.2f);
            } else break;
        }
        
        // Move to next position
        curr = next;
    }
    
    return curr;
}

// Block breaking methods
void PolarPairsController::scheduleBlockBreaking(int x, int y, float delay) {
    // Use find_if to avoid duplicates
    auto it = std::find_if(_blocksToBreak.begin(), _blocksToBreak.end(), 
        [x, y](const BreakableBlockInfo& info) {
            return info.position.x == x && info.position.y == y;
        });
    
    // Only add if not already scheduled
    if (it == _blocksToBreak.end()) {
        _blocksToBreak.emplace_back(cugl::Vec2(x, y), delay);
    }
}

void PolarPairsController::updateBlockBreaking(float timestep) {
    // Process blocks scheduled to break
    for (auto it = _blocksToBreak.begin(); it != _blocksToBreak.end(); ) {
        it->delay -= timestep;
        
        if (it->delay <= 0) {
            // Time to break the block
            int x = it->position.x;
            int y = it->position.y;
            
            // Only break if still a breakable block
            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT && _grid[x][y] == 6) {
                breakBlock(x, y);
            }
            
            // Remove from our list and continue with next
            it = _blocksToBreak.erase(it);
        } else {
            ++it;
        }
    }
}

void PolarPairsController::breakBlock(int x, int y) {
    // Remove from breakable blocks list - find and erase in one operation
    auto it = std::find_if(_breakableBlocks.begin(), _breakableBlocks.end(),
        [x, y](const cugl::Vec2& pos) {
            return pos.x == x && pos.y == y;
        });
    
    if (it != _breakableBlocks.end()) {
        _breakableBlocks.erase(it);
    }
    
    // Play ice break sound effect
    auto iceBreakSound = _assets->get<cugl::audio::Sound>("iceBreakSound");
    if (iceBreakSound) {
        // Use the AudioEngine to play the sound
        cugl::audio::AudioEngine::get()->play("iceBreak", iceBreakSound, false, 0.6f);
    }
    
    // IMMEDIATELY update the grid to make the tile passable (type 8)
    // This allows characters to move through it without waiting
    if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT && _grid[x][y] == 6) {
        _grid[x][y] = 8;
    }
    
    // Start animation for the visual breaking effect
    _renderer.startBreakAnimation(x, y);
    
    // Schedule VISUAL update after animation duration
    // This delay should match the full animation duration (frameTime * numFrames)
    float animationDuration = _renderer.getFrameTime() * 6 + 0.05f; // Add small buffer to ensure frame completes
    
    // Debug log
    CULog("Scheduled block removal at %d,%d after %f seconds", x, y, animationDuration);
    
    // Add to our delayed removal list to update the visuals after animation
    _blocksToRemove.emplace_back(x, y, animationDuration);
}

void PolarPairsController::createMoveCounters() {
    // Initialize move counts
    _bearMoves = 0;
    _penguinMoves = 0;
    _bearFinished = false;
    _penguinFinished = false;
    _simultaneousDestinationReached = false;
}

void PolarPairsController::updateMoveCounters() {
    // Log for debugging
}

void PolarPairsController::updateFinishState() {
    // Check if bear is on finish
    bool wasBearFinished = _bearFinished;
    _bearFinished = false;
    for (const auto& pos : _bearFinishBlocks) {
        if (pos.x == _polarBearGridPos.x && pos.y == _polarBearGridPos.y) {
            _bearFinished = true;
            // If the bear just reached the finish block, play the sound and start animation
            if (!wasBearFinished) {
                auto finishBlockSound = _assets->get<cugl::audio::Sound>("finishBlockSound");
                if (finishBlockSound) {
                    cugl::audio::AudioEngine::get()->play("finishBlock", finishBlockSound, false, 1.0f);
                }
                
                // Start the finish block animation
                _renderer.startFinishBlockAnimation(pos.x, pos.y, true);
            }
            break;
        }
    }
    
    // Check if penguin is on finish
    bool wasPenguinFinished = _penguinFinished;
    _penguinFinished = false;
    for (const auto& pos : _penguinFinishBlocks) {
        if (pos.x == _penguinGridPos.x && pos.y == _penguinGridPos.y) {
            _penguinFinished = true;
            // If the penguin just reached the finish block, play the sound and start animation
            if (!wasPenguinFinished) {
                auto finishBlockSound = _assets->get<cugl::audio::Sound>("finishBlockSound");
                if (finishBlockSound) {
                    cugl::audio::AudioEngine::get()->play("finishBlock", finishBlockSound, false, 1.0f);
                }
                
                // Start the finish block animation
                _renderer.startFinishBlockAnimation(pos.x, pos.y, false);
            }
            break;
        }
    }
    
    // Update the move counters to reflect finish state
    updateMoveCounters();
}

// Simple restart that preserves and reuses buttons
void PolarPairsController::restartLevel() {
    loadLevelData(_currentLevel);
}

void PolarPairsController::togglePause() {
    if (_isPaused) {
        // Currently paused - hide the menu
        hidePauseMenu();
    } else {
        // Currently unpaused - show the menu
        showPauseMenu();
    }
}

void PolarPairsController::showPauseMenu() {
    if (_isPaused) {
        return; // Already paused
    }
    
    // If instructions are showing, hide them first
    if (_isShowingInstructions) {
        hideInstructions();
    }
    
    _isPaused = true;
    
    // Clean up any existing elements first (just in case)
    if (_pauseOverlay && _pauseOverlay->getParent()) {
        _scene->removeChild(_pauseOverlay);
        _pauseOverlay = nullptr;
    }
    if (_pausedText && _pausedText->getParent()) {
        _scene->removeChild(_pausedText);
        _pausedText = nullptr;
    }
    if (_resumeButton && _resumeButton->getParent()) {
        _resumeButton->deactivate();
        _scene->removeChild(_resumeButton);
        _resumeButton = nullptr;
    }
    if (_exitButton && _exitButton->getParent()) {
        _exitButton->deactivate();
        _scene->removeChild(_exitButton);
        _exitButton = nullptr;
    }
    
    // Create a semi-transparent overlay
    cugl::Size size = _scene->getSize();
    
    _pauseOverlay = cugl::scene2::PolygonNode::alloc();
    _pauseOverlay->setColor(cugl::Color4(0, 0, 0, 0)); // Start fully transparent
    _pauseOverlay->setContentSize(size);
    _pauseOverlay->setAnchor(cugl::Vec2::ANCHOR_CENTER);
    _pauseOverlay->setPosition(size.width/2, size.height/2);
    _pauseOverlay->setPriority(900); // Below buttons but above game
    _scene->addChild(_pauseOverlay);
    
    // Create paused text
    std::shared_ptr<cugl::graphics::Texture> pausedTexture = _assets->get<cugl::graphics::Texture>("Paused");
    if (pausedTexture) {
        _pausedText = cugl::scene2::PolygonNode::allocWithTexture(pausedTexture);
        
        // Use our helper method to calculate tile size
        float tileSize = calculateTileSize(size);
        
        // Scale to be 1.0x the tileSize
        float textScale = (tileSize * 0.8f) / pausedTexture->getHeight();
        _pausedText->setScale(textScale);
        _pausedText->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        _pausedText->setPosition(size.width * PAUSED_TEXT_X, size.height * PAUSED_TEXT_Y);
        _pausedText->setPriority(1001);
        _pausedText->setColor(cugl::Color4(255, 255, 255, 0)); // Start transparent
    }
    if (_pausedText) {
        _scene->addChild(_pausedText);
    }
    
    // Create resume button in the same position as the pause button
    if (!_resumeButton) {
        std::shared_ptr<cugl::graphics::Texture> resumeUp = _assets->get<cugl::graphics::Texture>("Resume_Up");
        std::shared_ptr<cugl::graphics::Texture> resumeDown = _assets->get<cugl::graphics::Texture>("Resume_Down");
        
        if (resumeUp && resumeDown) {
            _resumeButton = cugl::scene2::Button::alloc(
                cugl::scene2::PolygonNode::allocWithTexture(resumeUp),
                cugl::scene2::PolygonNode::allocWithTexture(resumeDown)
            );
            
            // Get tile size from renderer for scaling
            float tileSize = _renderer.getTileSize();
            
            // Calculate button scale based on tile size
            float buttonScale;
            if (tileSize > 0) {
                // Use tile size and the ratio constant for scaling
                buttonScale = tileSize * BUTTON_TO_TILE_RATIO / resumeUp->getWidth();
            } else {
                // Use the same scale as the restart button if available, otherwise use screen-based calculation
                if (_restartButton) {
                    buttonScale = _restartButton->getScale().x;
                } else {
                    buttonScale = size.width * BUTTON_SCALE_FACTOR / resumeUp->getWidth();
                }
            }
            
            _resumeButton->setScale(buttonScale);
            _resumeButton->setAnchor(cugl::Vec2::ANCHOR_CENTER);
            
            // Set position based on _pauseButton if available, otherwise use constants
            if (_pauseButton) {
                _resumeButton->setPosition(_pauseButton->getPosition());
            } else {
                _resumeButton->setPosition(size.width * PAUSE_BUTTON_X, size.height * PAUSE_BUTTON_Y);
            }
            
            _resumeButton->setPriority(1001);
            _resumeButton->setColor(cugl::Color4(255, 255, 255, 0)); // Start transparent
            
            // Add button listener using direct callback function
            _resumeButton->setName("resume");
            _resumeButton->clearListeners();  // Clear any existing listeners
            _resumeButton->addListener([=](const std::string& name, bool down) {
                CULog("Resume button listener, down: %d", down);
                
                // Set visual state immediately
                _resumeButton->setDown(down);
                
                if (down) {  // On press
                    playButtonPressSound(_assets);
                } else {  // On release
                    // Deactivate the button immediately to prevent multiple clicks
                    _resumeButton->deactivate();
                    
                    // Call hidePauseMenu directly (not togglePause)
                    hidePauseMenu();
                }
            });
        }
    }
    _scene->addChild(_resumeButton);
    _resumeButton->activate();
    
    // Create exit button in the same position as the restart button
    if (!_exitButton) {
        std::shared_ptr<cugl::graphics::Texture> exitUp = _assets->get<cugl::graphics::Texture>("Exit_Up");
        std::shared_ptr<cugl::graphics::Texture> exitDown = _assets->get<cugl::graphics::Texture>("Exit_Down");
        
        if (exitUp && exitDown) {
            _exitButton = cugl::scene2::Button::alloc(
                cugl::scene2::PolygonNode::allocWithTexture(exitUp),
                cugl::scene2::PolygonNode::allocWithTexture(exitDown)
            );
            
            // Use the same scale and position as the restart button
            if (_restartButton) {
                float buttonScale = _restartButton->getScale().x;
                _exitButton->setScale(buttonScale);
                _exitButton->setAnchor(cugl::Vec2::ANCHOR_CENTER);
                _exitButton->setPosition(_restartButton->getPosition());
                _exitButton->setPriority(1001);
            } else {
                // Get tile size from renderer for scaling
                float tileSize = _renderer.getTileSize();
                
                // Calculate button scale based on tile size
                float buttonScale;
                if (tileSize > 0) {
                    // Use tile size and the ratio constant for scaling
                    buttonScale = tileSize * BUTTON_TO_TILE_RATIO / exitUp->getWidth();
                } else {
                    // Fallback to original calculation if tile size not available
                    buttonScale = size.width * BUTTON_SCALE_FACTOR / exitUp->getWidth();
                }
                
                _exitButton->setScale(buttonScale);
                _exitButton->setAnchor(cugl::Vec2::ANCHOR_CENTER);
                _exitButton->setPosition(size.width * RESTART_BUTTON_X, size.height * RESTART_BUTTON_Y);
                _exitButton->setPriority(1001);
            }
            
            _exitButton->setColor(cugl::Color4(255, 255, 255, 0)); // Start transparent
            
            // Add button listener using direct callback function
            _exitButton->setName("exit");
            _exitButton->clearListeners();  // Clear any existing listeners
            _exitButton->addListener([=](const std::string& name, bool down) {
                CULog("Exit button listener, down: %d", down);
                
                // Set visual state immediately
                _exitButton->setDown(down);
                
                if (down) {  // On press
                    playButtonPressSound(_assets);
                } else {  // On release
                    // Deactivate the button immediately
                    _exitButton->deactivate();
                    
                    _hasWon = false;
                    _shouldExitToMenu = true;
                    hidePauseMenu();
                }
            });
        }
    }
    _scene->addChild(_exitButton);
    _exitButton->activate();
    
    // Disable gameplay buttons during pause
    if (_restartButton) _restartButton->deactivate();
    if (_pauseButton) _pauseButton->deactivate();
    if (_questionButton) _questionButton->deactivate();
    
    // Start fade-in animation
    _isFading = true;
    _fadeTime = 0;
    _fadeDuration = 0.3f; // 0.3 seconds for the animation
    _isFadingIn = true;
}

void PolarPairsController::hidePauseMenu() {
    if (!_isPaused) {
        return; // Not paused
    }
    
    // First update the pause state flag
    _isPaused = false;
    
    // Start fade-out animation
    _isFading = true;
    _fadeTime = 0;
    _fadeDuration = 0.3f; // 0.3 seconds for the animation
    _isFadingIn = false;
    
    // The elements will be actually removed once the fade completes
    // This happens in the update method
}

// New method to handle pause menu input separately
void PolarPairsController::processPauseMenuInput() {
    auto touch = cugl::Input::get<cugl::Touchscreen>();
    if (!touch) return;
    
    // Only process if pause menu is active and not fading
    if (!_isPaused || _isFading) return;
    
    // Check for touches
    if (touch->touchCount() > 0) {
        auto& touchSet = touch->touchSet();
        if (!touchSet.empty()) {
            cugl::TouchID tid = *(touchSet.begin());
            cugl::Vec2 currentPos = touch->touchPosition(tid);
            cugl::Vec2 scenePos = _scene->screenToWorldCoords(currentPos);
            
            // Only update visual states - leave action to button listeners
            if (_resumeButton) {
                cugl::Rect resumeBounds = _resumeButton->getBoundingBox();
                if (resumeBounds.contains(scenePos)) {
                    _resumeButton->setDown(true);
                } else {
                    _resumeButton->setDown(false);
                }
            }
            
            if (_exitButton) {
                cugl::Rect exitBounds = _exitButton->getBoundingBox();
                if (exitBounds.contains(scenePos)) {
                    _exitButton->setDown(true);
                } else {
                    _exitButton->setDown(false);
                }
            }
        }
    } else {
        // No touches - reset all button states to up
        if (_resumeButton) _resumeButton->setDown(false);
        if (_exitButton) _exitButton->setDown(false);
    }
}

void PolarPairsController::showInstructions() {
    if (_isShowingInstructions) {
        return; // Already showing instructions
    }
    
    // If paused, hide pause menu first
    if (_isPaused) {
        hidePauseMenu();
    }
    
    _isShowingInstructions = true;
    
    // Clean up any existing elements first (just in case)
    if (_instructionOverlay && _instructionOverlay->getParent()) {
        _scene->removeChild(_instructionOverlay);
        _instructionOverlay = nullptr;
    }
    if (_instructionImage && _instructionImage->getParent()) {
        _scene->removeChild(_instructionImage);
        _instructionImage = nullptr;
    }
    if (_returnButton && _returnButton->getParent()) {
        _returnButton->deactivate();
        _scene->removeChild(_returnButton);
        _returnButton = nullptr;
    }
    
    // Create a semi-transparent overlay for instructions
    cugl::Size size = _scene->getSize();
    
    _instructionOverlay = cugl::scene2::PolygonNode::alloc();
    _instructionOverlay->setColor(cugl::Color4(0, 0, 0, 0)); // Start fully transparent
    _instructionOverlay->setContentSize(size);
    _instructionOverlay->setAnchor(cugl::Vec2::ANCHOR_CENTER);
    _instructionOverlay->setPosition(size.width/2, size.height/2);
    _instructionOverlay->setPriority(900); // Below buttons but above game
    _scene->addChild(_instructionOverlay);
    
    // Load instruction image
    std::shared_ptr<cugl::graphics::Texture> instructionTexture = _assets->get<cugl::graphics::Texture>("Instruction");
    if (instructionTexture) {
        _instructionImage = cugl::scene2::PolygonNode::allocWithTexture(instructionTexture);
        
        // Use our helper method to calculate tile size
        float tileSize = calculateTileSize(size);
        
        // Scale to a multiple of the tile size while preserving aspect ratio
        float aspectRatio = instructionTexture->getHeight() / instructionTexture->getWidth();
        float imageScale = (tileSize * 8.0f) / instructionTexture->getWidth();
        _instructionImage->setScale(imageScale);
        _instructionImage->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        _instructionImage->setPosition(size.width * INSTRUCTION_IMAGE_X, size.height * INSTRUCTION_IMAGE_Y);
        _instructionImage->setPriority(901);
        _instructionImage->setColor(cugl::Color4(255, 255, 255, 0)); // Start transparent
    }
    if (_instructionImage) {
        _scene->addChild(_instructionImage);
    }
    
    // Create return button in the same position as the question button
    std::shared_ptr<cugl::graphics::Texture> returnUp = _assets->get<cugl::graphics::Texture>("Return_Up");
    std::shared_ptr<cugl::graphics::Texture> returnDown = _assets->get<cugl::graphics::Texture>("Return_Down");
    
    if (returnUp && returnDown && _questionButton) {
        _returnButton = cugl::scene2::Button::alloc(
            cugl::scene2::PolygonNode::allocWithTexture(returnUp),
            cugl::scene2::PolygonNode::allocWithTexture(returnDown)
        );
        
        // Use the position of the question button
        float buttonScale = _questionButton->getScale().x;
        _returnButton->setScale(buttonScale);
        _returnButton->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        _returnButton->setPosition(_questionButton->getPosition());
        _returnButton->setPriority(1001);
        
        _returnButton->setColor(cugl::Color4(255, 255, 255, 0)); // Start transparent
        
        // Add button listener with simplified approach
        _returnButton->setName("return");
        _returnButton->clearListeners();  // Clear any existing listeners
        _returnButton->addListener([=] (const std::string& name, bool down) {
            // Handle both press and release events
            if (down) {  // On press
                playButtonPressSound(_assets);
            } else {  // On release
                hideInstructions();
            }
        });
    }
    if (_returnButton) {
        _scene->addChild(_returnButton);
        _returnButton->activate();
    }
    
    // Deactivate gameplay buttons during instructions
    if (_restartButton) _restartButton->deactivate();
    if (_pauseButton) _pauseButton->deactivate();
    if (_questionButton) _questionButton->deactivate();
    
    // Start fade-in animation
    _isFading = true;
    _fadeTime = 0;
    _fadeDuration = 0.3f; // 0.3 seconds for the animation
    _isFadingIn = true;
}

void PolarPairsController::hideInstructions() {
    if (!_isShowingInstructions) {
        return; // Not showing instructions
    }
    
    // First update the instruction state flag
    _isShowingInstructions = false;
    
    // Deactivate button immediately to prevent multiple clicks
    if (_returnButton) _returnButton->deactivate();
    
    // Start fade-out animation
    _isFading = true;
    _fadeTime = 0;
    _fadeDuration = 0.3f; // 0.3 seconds for the animation
    _isFadingIn = false;
    
    // The elements will be actually removed once the fade completes
    // This happens in the update method
}

void PolarPairsController::processInstructionsInput() {
    auto touch = cugl::Input::get<cugl::Touchscreen>();
    if (!touch) return;
    
    // Only process if instructions are showing and not fading
    if (!_isShowingInstructions || _isFading) return;
    
    // Handle direct button clicks for the return button
    if (touch->touchCount() > 0) {
        auto& touchSet = touch->touchSet();
        if (!touchSet.empty()) {
            cugl::TouchID tid = *(touchSet.begin());
            cugl::Vec2 currentPos = touch->touchPosition(tid);
            cugl::Vec2 scenePos = _scene->screenToWorldCoords(currentPos);
            
            // Update return button state
            if (_returnButton) {
                cugl::Rect returnBounds = _returnButton->getBoundingBox();
                _returnButton->setDown(returnBounds.contains(scenePos) && touch->touchDown(tid));
            }
        }
    } else {
        // No touches - reset button state
        if (_returnButton) _returnButton->setDown(false);
    }
}

// Add this new method to force cleanup of all UI elements
void PolarPairsController::forceCleanupAllUIElements() {
    // Immediately remove all overlay and menu elements
    if (_pauseOverlay && _pauseOverlay->getParent()) {
        _scene->removeChild(_pauseOverlay);
        _pauseOverlay = nullptr;
    }
    
    if (_pausedText && _pausedText->getParent()) {
        _scene->removeChild(_pausedText);
        _pausedText = nullptr;
    }
    
    if (_resumeButton && _resumeButton->getParent()) {
        _resumeButton->deactivate();
        _scene->removeChild(_resumeButton);
        _resumeButton = nullptr;
    }
    
    if (_exitButton && _exitButton->getParent()) {
        _exitButton->deactivate();
        _scene->removeChild(_exitButton);
        _exitButton = nullptr;
    }
    
    if (_instructionOverlay && _instructionOverlay->getParent()) {
        _scene->removeChild(_instructionOverlay);
        _instructionOverlay = nullptr;
    }
    
    if (_instructionImage && _instructionImage->getParent()) {
        _scene->removeChild(_instructionImage);
        _instructionImage = nullptr;
    }
    
    if (_returnButton && _returnButton->getParent()) {
        _returnButton->deactivate();
        _scene->removeChild(_returnButton);
        _returnButton = nullptr;
    }
    
    // Reset state flags
    _isPaused = false;
    _isShowingInstructions = false;
    _isFading = false;
    
    // Reactivate gameplay buttons
    if (_restartButton) _restartButton->activate();
    if (_pauseButton) _pauseButton->activate();
    if (_questionButton) _questionButton->activate();
}
