// HelloApp.h
#pragma once
#include <cugl/cugl.h>
#include "PolarPairsController.h"

/**
 * Main application class for the PolarPairs game
 */
class HelloApp : public cugl::Application {
private:
    /** The asset manager for this game */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    /** The sprite batch for this game */
    std::shared_ptr<cugl::graphics::SpriteBatch> _batch;
    
    /** The menu scene */
    std::shared_ptr<cugl::scene2::Scene2> _menuScene;
    
    /** The level selector scene */
    std::shared_ptr<cugl::scene2::Scene2> _levelScene;
    
    /** The finish scene */
    std::shared_ptr<cugl::scene2::Scene2> _finishScene;
    
    /** The logo image */
    std::shared_ptr<cugl::scene2::PolygonNode> _logo;
    
    /** The start button */
    std::shared_ptr<cugl::scene2::PolygonNode> _startButton;
    
    /** The level 1 button */
    std::shared_ptr<cugl::scene2::Button> _level1Button;
    
    /** The level 2 button */
    std::shared_ptr<cugl::scene2::Button> _level2Button;
    
    /** The level 3 button */
    std::shared_ptr<cugl::scene2::Button> _level3Button;
    
    /** The level 4 button */
    std::shared_ptr<cugl::scene2::Button> _level4Button;
    
    /** Vector to store all level buttons */
    std::vector<std::shared_ptr<cugl::scene2::Button>> _levelButtons;
    
    /** Vector to store star rating images for each level */
    std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _levelStars;
    
    /** The level 1 button down state texture */
    std::shared_ptr<cugl::graphics::Texture> _level1DownTexture;
    
    /** The seal image */
    std::shared_ptr<cugl::scene2::PolygonNode> _sealImage;
    
    /** The bear image */
    std::shared_ptr<cugl::scene2::PolygonNode> _bearImage;
    
    /** The menu background */
    std::shared_ptr<cugl::scene2::PolygonNode> _menuBackground;
    
    /** The level selector background */
    std::shared_ptr<cugl::scene2::PolygonNode> _levelBackground;
    
    /** The game background */
    std::shared_ptr<cugl::scene2::PolygonNode> _gameBackground;
    
    /** The finish scene background */
    std::shared_ptr<cugl::scene2::PolygonNode> _finishBackground;
    
    /** The exit button for finish scene */
    std::shared_ptr<cugl::scene2::Button> _finishExitButton;
    
    /** The restart button for finish scene */
    std::shared_ptr<cugl::scene2::Button> _finishRestartButton;
    
    /** The next level button for finish scene */
    std::shared_ptr<cugl::scene2::Button> _finishNextButton;
    
    /** The star rating display for finish scene */
    std::shared_ptr<cugl::scene2::PolygonNode> _finishStarRating;
    
    /** The "Level Finished" text for finish scene */
    std::shared_ptr<cugl::scene2::PolygonNode> _levelFinishedText;
    
    /** The "Highest" text for finish scene */
    std::shared_ptr<cugl::scene2::PolygonNode> _highestText;
    
    /** The fade overlay for transitions */
    std::shared_ptr<cugl::scene2::PolygonNode> _fadeOverlay;
    
    /** The UI fade overlay for transitions */
    std::shared_ptr<cugl::scene2::PolygonNode> _uiFadeOverlay;
    
    /** The selected level for game transition */
    int _selectedLevel;
    
    /** The game controller */
    std::shared_ptr<PolarPairsController> _PolarPairsController;
    
    /** Whether we are in the menu scene */
    bool _inMenuScene;
    
    /** Whether we are in the level selector scene */
    bool _inLevelScene;
    
    /** Whether we are in the finish scene */
    bool _inFinishScene;
    
    /** Whether we are transitioning between scenes */
    bool _isTransitioning;
    
    /** Whether we should go to the next level from finish scene */
    bool _goToNextLevel;
    
    /** Whether we are fading in or out */
    bool _isFadingOut;
    
    /** Whether a button is currently pressed */
    bool _buttonPressed;
    
    /** Whether finish scene elements are animating in */
    bool _isFinishSceneAnimating;
    
    /** Animation timer */
    float _animTime;
    
    /** Transition timer */
    float _transitionTime;
    
    /** Finish scene animation timer */
    float _finishAnimTime;
    
    /** Base Y position for seal floating animation */
    float _sealBaseY;
    
    /** Base Y position for bear floating animation */
    float _bearBaseY;
    
    /** Base Y position for level scene seal floating animation */
    float _levelSealBaseY;
    
    /** Base Y position for level scene bear floating animation */
    float _levelBearBaseY;
    
    /** Base X position for background animation */
    float _backgroundBaseX;
    
    /** Base Y position for background animation */
    float _backgroundBaseY;
    
    /** Original position of Level Finished text */
    cugl::Vec2 _levelFinishedOrigPos;
    
    /** Original position of star rating */
    cugl::Vec2 _starRatingOrigPos;
    
    /** Original position of highest text */
    cugl::Vec2 _highestTextOrigPos;
    
    /** Original position of restart button */
    cugl::Vec2 _restartButtonOrigPos;
    
    /** Original position of next button */
    cugl::Vec2 _nextButtonOrigPos;
    
    /** Level title texture node */
    std::shared_ptr<cugl::scene2::PolygonNode> _levelTitle;
    
    /** Whether level title is being pressed for reset feature */
    bool _levelTitleTouched;
    
    /** Time when level title touch started */
    float _levelTitleTouchTime;
    
    /** Starting position of level title touch */
    cugl::Vec2 _levelTitleTouchPos;
    
    /** Build the menu scene */
    void buildMenuScene();
    
    /** Build the level selector scene */
    void buildLevelScene();
    
    /** Build the finish scene */
    void buildFinishScene();
    
    /** Updates the star rating display in the finish scene */
    void updateFinishSceneStars();
    
    /** Create backgrounds for menu and game scenes */
    void createSharedBackground();
    
    /** Create a character (seal or bear) with animation */
    std::shared_ptr<cugl::scene2::PolygonNode> createCharacter(const std::string& textureName, 
                                                              float posX, float posY, 
                                                              float scale, 
                                                              const std::string& name,
                                                              float& baseY);
    
    /** Handle the transition from menu to level selector */
    void transitionToLevelSelector();
    
    /** Handle the transition from level selector to game */
    void transitionToGame(int level);
    
    /** Handle the transition from game to finish scene */
    void transitionToFinishScene();
    
    /** Update transition animations */
    void updateTransition(float timestep);
    
    /** Create a level button with given level number and position */
    std::shared_ptr<cugl::scene2::Button> createLevelButton(int level, const cugl::Vec2& position, float buttonSize);
    
    /** Calculate tile size based on display height and grid dimensions */
    float calculateTileSize() const;
    
public:
    /** Constructor */
    HelloApp() : _inMenuScene(true), _inLevelScene(false), _inFinishScene(false), _isTransitioning(false), 
                 _isFadingOut(false), _buttonPressed(false), _animTime(0), _transitionTime(0),
                 _sealBaseY(0), _bearBaseY(0), _levelSealBaseY(0), _levelBearBaseY(0), _backgroundBaseX(0), _backgroundBaseY(0),
                 _goToNextLevel(false), _isFinishSceneAnimating(false), _finishAnimTime(0),
                 _levelTitleTouched(false), _levelTitleTouchTime(0) {}
    
    /** Destructor */
    ~HelloApp() { dispose(); }
    
    /** Initialize the application */
    void onStartup() override;
    
    /** Clean up the application */
    void onShutdown() override;
    
    /** Update the application */
    void update(float timestep) override;
    
    /** Draw the application */
    void draw() override;
};
