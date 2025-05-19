//
//  PlaygroundRenderer.h
//  SKIP
//
//  Created by Kuangming Qin on 4/24/25.
//

#pragma once
#include <cugl/cugl.h>

/**
 * Class responsible for rendering the game playground (map) and all visual elements.
 * Handles grid rendering, character animations, and visual feedback.
 */
class PlaygroundRenderer {
private:
    // Static map for cell rendering priorities
    static const std::map<int, int> CELL_PRIORITIES;
    
    std::shared_ptr<cugl::scene2::Scene2> _scene;
    std::shared_ptr<cugl::AssetManager> _assets;
    std::vector<std::shared_ptr<cugl::scene2::SceneNode>> _blockNodes;
    std::shared_ptr<cugl::scene2::SceneNode> _polarBear;
    std::shared_ptr<cugl::scene2::SceneNode> _penguin;
    
    // Store grid rendering parameters
    float _tileSize;
    float _offsetX;
    float _offsetY;
    float _tileHeightRatio;
    float _animTime;  // Animation time counter
    float _frameTime; // Animation frame time in seconds
    
    // Structure to track breaking block animations
    struct BreakingBlockAnimation {
        int x;
        int y;
        std::shared_ptr<cugl::scene2::SpriteNode> sprite;
        float totalTime;
        bool complete;
        
        BreakingBlockAnimation(int gridX, int gridY, std::shared_ptr<cugl::scene2::SpriteNode> fs)
            : x(gridX), y(gridY), sprite(fs), totalTime(0), complete(false) {}
    };
    
    // Structure to track finish block animations
    struct FinishBlockAnimation {
        int x;
        int y;
        bool isBear;
        float progress;  // 0 to 1
        std::shared_ptr<cugl::scene2::PolygonNode> block;
        std::shared_ptr<cugl::scene2::PolygonNode> flag;
        
        FinishBlockAnimation(int gridX, int gridY, bool bear, 
                             std::shared_ptr<cugl::scene2::PolygonNode> b)
            : x(gridX), y(gridY), isBear(bear), progress(0), block(b), flag(nullptr) {}
    };
    
    // Structure to track character bounce animations
    struct CharacterBounceAnimation {
        bool isBear;
        float progress;  // 0 to 1
        float originalScale;
        std::shared_ptr<cugl::scene2::SceneNode> character;
        
        CharacterBounceAnimation(bool bear, std::shared_ptr<cugl::scene2::SceneNode> c, float scale)
            : isBear(bear), progress(0), originalScale(scale), character(c) {}
    };
    
    // Structure to track blocked movement animations
    struct BlockedAnimation {
        float progress;  // 0 to 1
        cugl::Vec2 direction;  // Direction of the blocked movement
        float tileSize;  // Tile size for distance calculation
        std::shared_ptr<cugl::scene2::SceneNode> bearCharacter;
        std::shared_ptr<cugl::scene2::SceneNode> penguinCharacter;
        cugl::Vec2 bearOriginalPos;
        cugl::Vec2 penguinOriginalPos;
        
        BlockedAnimation(cugl::Vec2 dir, float size,
                         std::shared_ptr<cugl::scene2::SceneNode> bear, 
                         std::shared_ptr<cugl::scene2::SceneNode> penguin)
            : progress(0), direction(dir), tileSize(size),
              bearCharacter(bear), penguinCharacter(penguin),
              bearOriginalPos(bear ? bear->getPosition() : cugl::Vec2::ZERO),
              penguinOriginalPos(penguin ? penguin->getPosition() : cugl::Vec2::ZERO) {}
    };
    
    // List of active finish block animations
    std::vector<FinishBlockAnimation> _finishBlockAnimations;
    
    // List of active breaking block animations
    std::vector<BreakingBlockAnimation> _breakingAnimations;
    
    // List of active character bounce animations
    std::vector<CharacterBounceAnimation> _characterBounceAnimations;
    
    // List of active blocked animations
    std::vector<BlockedAnimation> _blockedAnimations;
    
    // Helper method to add a node for a single cell
    std::shared_ptr<cugl::scene2::PolygonNode> addCellNode(int x, int y, int cellType);
    
    // Convert grid coordinates to screen coordinates
    cugl::Vec2 gridToScreenPos(float x, float y) const;
    
    // Helper method to check if a node is a UI element
    bool isUIElement(const std::shared_ptr<cugl::scene2::SceneNode>& node) const;

public:
    PlaygroundRenderer() : _tileHeightRatio(0.75f), _animTime(0.0f), _frameTime(0.12f) {}
    
    /**
     * Initialize the renderer
     */
    void init(const std::shared_ptr<cugl::scene2::Scene2>& scene,
              const std::shared_ptr<cugl::AssetManager>& assets);

    /**
     * Draw the entire grid
     */
    void drawGrid(const std::vector<std::vector<int>>& grid, float tileHeightRatio = 0.75f);
    
    /**
     * Update a single cell in the grid
     */
    void updateCell(int x, int y, int cellType);
    
    /**
     * Create character sprites
     */
    void createCharacters();
    
    /**
     * Update character positions
     */
    void updateCharacterPositions(const cugl::Vec2& bearPos, const cugl::Vec2& penguinPos);
    
    /**
     * Animate character movement
     */
    void moveCharacters(const cugl::Vec2& bearStart, const cugl::Vec2& bearTarget,
                      const cugl::Vec2& penguinStart, const cugl::Vec2& penguinTarget,
                      float progress);
    
    /**
     * Start a breaking block animation at the specified grid position
     */
    void startBreakAnimation(int x, int y);
    
    /**
     * Start a finish block animation at the specified grid position
     */
    void startFinishBlockAnimation(int x, int y, bool isBear);
    
    /**
     * Start a bounce animation for a character (bear or seal)
     */
    void startCharacterBounceAnimation(bool isBear);
    
    /**
     * Start a blocked animation for both characters
     */
    void startBlockedAnimation(const cugl::Vec2& direction);
    
    /**
     * Update finish block animations
     */
    void updateFinishBlockAnimations(float progress);
    
    /**
     * Clear all visual elements
     */
    void clear();
    
    /**
     * Convert grid position to screen position
     */
    cugl::Vec2 getScreenPosition(int x, int y) const;
    
    /**
     * Get the tile size
     */
    float getTileSize() const { return _tileSize; }
    
    /**
     * Get the animation frame time in seconds
     */
    float getFrameTime() const { return _frameTime; }
    
    /**
     * Set the animation frame time in seconds
     * Lower values = faster animation, higher values = slower animation
     */
    void setFrameTime(float frameTime) { _frameTime = frameTime; }
    
    /**
     * Update animation and other time-based elements
     */
    void update(float dt);
};
