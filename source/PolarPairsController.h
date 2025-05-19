//
//  PolarPairsController.h
//  PolarPairs
//

#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__

#include <cugl/cugl.h>
#include "LevelData.h"
#include "PlaygroundRenderer.h"

// Grid dimensions
#define GRID_WIDTH 7
#define GRID_HEIGHT 11

// Button position constants - relative to screen width/height
#define RESTART_BUTTON_X 0.15f
#define RESTART_BUTTON_Y 0.1f
#define PAUSE_BUTTON_X 0.85f
#define PAUSE_BUTTON_Y 0.1f
// Button scale relative to tile size - 1.2x the size of a regular tile
#define BUTTON_SCALE_FACTOR 0.20f
// Ratio of button size to tile size (1.2 = buttons are 20% larger than tiles)
#define BUTTON_TO_TILE_RATIO 1.4f
//#define PAUSED_TEXT_SCALE_FACTOR 0.33f
#define PAUSED_TEXT_X 0.5f
#define PAUSED_TEXT_Y 0.5f
#define INSTRUCTION_IMAGE_SCALE_FACTOR 0.8f
#define INSTRUCTION_IMAGE_X 0.5f
#define INSTRUCTION_IMAGE_Y 0.5f

/**
 * Class for controlling the game logic of PolarPairs.
 */
class PolarPairsController {
private:
    // Asset manager and scene graph
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<cugl::scene2::Scene2> _scene;
    PlaygroundRenderer _renderer;
    
    // Grid and level data
    std::vector<std::vector<int>> _grid;
    std::vector<cugl::Vec2> _bearBlocks;
    std::vector<cugl::Vec2> _penguinBlocks;
    std::vector<cugl::Vec2> _breakableBlocks;
    std::vector<cugl::Vec2> _bearFinishBlocks;
    std::vector<cugl::Vec2> _penguinFinishBlocks;
    
    // Character positions
    cugl::Vec2 _polarBearGridPos;
    cugl::Vec2 _penguinGridPos;
    
    // For tracking previous positions
    cugl::Vec2 _polarBearPrevPos;
    cugl::Vec2 _penguinPrevPos;
    
    // Movement state
    cugl::Vec2 _polarBearTarget;
    cugl::Vec2 _penguinTarget;
    cugl::Vec2 _moveDirection;
    bool _isMoving;
    float _moveProgress;
    
    // Move counters and finish states
    int _bearMoves;
    int _penguinMoves;
    bool _bearFinished;
    bool _penguinFinished;
    bool _simultaneousDestinationReached;
    
    // Constant speed movement variables
    float _bearTravelDistance;
    float _penguinTravelDistance;
    float _totalTravelDistance;
    
    // Win state
    bool _hasWon;
    int _currentLevel;
    float _winDelay;        // Delay before showing win screen
    bool _winConditionMet;  // Flag to indicate win condition is met but waiting for animation
    
    // Touch tracking
    bool _touchActive;
    cugl::Vec2 _touchStart;
    cugl::Vec2 _lastTouchPos;
    bool _touchingPauseButton;
    
    // Breakable blocks
    struct BreakableBlockInfo {
        cugl::Vec2 position;
        float delay;
        BreakableBlockInfo(cugl::Vec2 pos, float d) : position(pos), delay(d) {}
    };
    std::vector<BreakableBlockInfo> _blocksToBreak;
    
    // Structure to track delayed block removal after animation
    struct DelayedBlockRemoval {
        int x;
        int y;
        float timeRemaining;
        DelayedBlockRemoval(int posX, int posY, float time) : x(posX), y(posY), timeRemaining(time) {}
    };
    std::vector<DelayedBlockRemoval> _blocksToRemove;
    
    // Squeeze mechanic
    bool _squeezeJustOccurred;
    bool _polarBearIsRear;
    bool checkForSqueeze();
    
    // Track which character is being pushed during squeeze (bear = true, penguin = false)
    bool _bearIsBeingPushed;
    
    // Movement and collision
    cugl::Vec2 calculateSingleCharacterMove(const cugl::Vec2& start, bool isPenguin, const cugl::Vec2& direction);
    cugl::Vec2 computeTarget(const cugl::Vec2& start, bool isPenguin, bool canBreak,
                             const cugl::Vec2& direction,
                             const cugl::Vec2& otherPos,
                             const cugl::Vec2& otherTarget);
    cugl::Vec2 slide(const cugl::Vec2& start, bool isPenguin, bool canBreak, const cugl::Vec2& direction);
    void calculateMovementTargets();
    void updateCharacterPositions();
    void updateCharacterPositionsInterpolated(float progress);
    void updateCharacterPositionsWithSeparateProgress(float bearProgress, float penguinProgress);
    
    // Move counters
    void createMoveCounters();
    void updateMoveCounters();
    void updateFinishState();
    
    // Win condition
    void checkWinCondition();
    
    // Block breaking
    void scheduleBlockBreaking(int x, int y, float delay);
    void updateBlockBreaking(float timestep);
    void breakBlock(int x, int y);
    
    // UI Buttons
    /** The restart button */
    std::shared_ptr<cugl::scene2::Button> _restartButton;
    
    /** The pause button */
    std::shared_ptr<cugl::scene2::Button> _pauseButton;
    
    // Pause menu elements
    /** Flag indicating if the game is paused */
    bool _isPaused;
    
    /** Semi-transparent overlay for pause menu */
    std::shared_ptr<cugl::scene2::PolygonNode> _pauseOverlay;
    
    /** The exit button (shown when paused) */
    std::shared_ptr<cugl::scene2::Button> _exitButton;
    
    /** The resume button (shown when paused) */
    std::shared_ptr<cugl::scene2::Button> _resumeButton;
    
    /** The paused text image */
    std::shared_ptr<cugl::scene2::PolygonNode> _pausedText;
    
    // Instruction menu elements
    /** Flag indicating if the instruction screen is showing */
    bool _isShowingInstructions;
    
    /** Semi-transparent overlay for instruction menu */
    std::shared_ptr<cugl::scene2::PolygonNode> _instructionOverlay;
    
    /** The question button (to show instructions) */
    std::shared_ptr<cugl::scene2::Button> _questionButton;
    
    /** The return button (to exit instructions) */
    std::shared_ptr<cugl::scene2::Button> _returnButton;
    
    /** The instruction image */
    std::shared_ptr<cugl::scene2::PolygonNode> _instructionImage;
    
    /** Fade animation variables for pause menu */
    bool _isFading;
    float _fadeTime;
    float _fadeDuration;
    bool _isFadingIn;
    
    /** Tracking which button is currently pressed for visual feedback */
    bool _restartButtonPressed;
    bool _pauseButtonPressed;
    
    /** Original colors of buttons for restoring after pressing */
    cugl::Color4 _restartButtonOrigColor;
    cugl::Color4 _pauseButtonOrigColor;
    
    // Menu exit flag
    bool _shouldExitToMenu;
    
    /** Helper function to check if either character is on their special blocks (passable or finish) */
    bool checkNoSqueezeBlocks();
    
    // Movement update
    void updateMovement(float timestep);
    
    // Safe restart that preserves UI elements
    void restartLevel();
    
    /** Handle character movement with the given direction */
    void moveCharacters(const cugl::Vec2& direction);
    
    /** Process player input from touch/keyboard */
    void processInput();
    
    /** Toggle pause state */
    void togglePause();
    
    /** Show pause menu */
    void showPauseMenu();
    
    /** Hide pause menu */
    void hidePauseMenu();
    
    /** Show instruction screen */
    void showInstructions();
    
    /** Hide instruction screen */
    void hideInstructions();
    
    /* Process input specifically for the pause menu */
    void processPauseMenuInput();
    
    /* Process input specifically for the instruction screen */
    void processInstructionsInput();
    
    /** Forces immediate cleanup of all UI elements */
    void forceCleanupAllUIElements();
    
    /** Helper method to calculate tile size based on screen height */
    float calculateTileSize(const cugl::Size& size) const;
    
    /** Timeline for animations and delayed actions */
    std::shared_ptr<cugl::ActionTimeline> _timeline;
    
    // Finish block animation tracking
    struct FinishBlockAnimation {
        int x;
        int y;
        bool isBear;
        float timeRemaining;
        bool showFlag;
        
        FinishBlockAnimation(int posX, int posY, bool bear) 
            : x(posX), y(posY), isBear(bear), timeRemaining(0.4f), showFlag(false) {}
    };
    std::vector<FinishBlockAnimation> _finishAnimations;
    
public:
    // Constructors/Destructors
    PolarPairsController() : _isMoving(false), _moveProgress(0.0f), _hasWon(false),
                           _currentLevel(1), _touchActive(false), _shouldExitToMenu(false),
                           _bearMoves(0), _penguinMoves(0), _bearFinished(false), _penguinFinished(false),
                           _simultaneousDestinationReached(false), _bearTravelDistance(0), _penguinTravelDistance(0), _totalTravelDistance(0),
                           _touchingPauseButton(false), _restartButtonPressed(false), _pauseButtonPressed(false),
                           _restartButtonOrigColor(cugl::Color4::WHITE), _pauseButtonOrigColor(cugl::Color4::WHITE),
                           _isPaused(false), _exitButton(nullptr), _resumeButton(nullptr), _pauseOverlay(nullptr),
                           _isShowingInstructions(false), _questionButton(nullptr), _returnButton(nullptr), 
                           _instructionOverlay(nullptr), _instructionImage(nullptr),
                           _isFading(false), _fadeTime(0), _fadeDuration(0), _isFadingIn(true),
                           _winDelay(0.0f), _winConditionMet(false), _bearIsBeingPushed(false) {}
    ~PolarPairsController() { dispose(); }
    
    // Core methods
    void dispose();
    bool init(const std::shared_ptr<cugl::AssetManager>& assets, const cugl::Size& size);
    void update(float timestep);
    
    // Game logic methods
    void loadLevelData(int levelNum);
    void switchLevel(int levelNum);
    
    // Accessors
    std::shared_ptr<cugl::scene2::Scene2> getScene() const { return _scene; }
    bool hasWon() const { return _hasWon; }
    int getCurrentLevel() const { return _currentLevel; }
    bool shouldExitToMenu() const { return _shouldExitToMenu; }
    void resetExitFlag() { _shouldExitToMenu = false; }
    
    /**
     * Updates finish block animations
     */
    void updateFinishBlockAnimations(float timestep);
};

#endif /* __GAME_CONTROLLER_H__ */