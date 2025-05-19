//
//  PlaygroundRenderer.cpp
//  SKIP
//
//  Created by Kuangming Qin on 4/24/25.
//

#include "PlaygroundRenderer.h"

// Define the static priorities map
const std::map<int, int> PlaygroundRenderer::CELL_PRIORITIES = {
    {0, -100},  // Regular tiles (lowest layer)
    {4, 30},    // Bear finish (above characters and breakable blocks)
    {5, 30},    // Penguin finish (above characters and breakable blocks)
    {2, 20},    // Bear blocks (above characters)
    {3, 20},    // Penguin blocks (above characters)
    {6, 20},    // Breakable blocks
    {8, -100}   // Empty passable (same as regular tiles)
};

void PlaygroundRenderer::init(const std::shared_ptr<cugl::scene2::Scene2>& scene,
                             const std::shared_ptr<cugl::AssetManager>& assets) {
    _scene = scene;
    _assets = assets;
    _tileHeightRatio = 0.6f;
    
    // Default values, will be updated in drawGrid
    _tileSize = 0.0f;
    _offsetX = 0.0f;
    _offsetY = 0.0f;
    
    // Initialize animation variables
    _animTime = 0.0f;
    
    _polarBear = nullptr;
    _penguin = nullptr;
}

void PlaygroundRenderer::clear() {
    // First, find and remove all flag nodes directly from the scene
    std::vector<std::shared_ptr<cugl::scene2::SceneNode>> flagsToRemove;
    
    // Search for all children in the scene that might be flag nodes
    for (int i = 0; i < _scene->getChildCount(); i++) {
        auto child = _scene->getChild(i);
        
        // Check if this looks like a flag (based on name, priority, or position)
        if (child && (child->getName().find("Flag") != std::string::npos || 
                    (child->getPriority() > 30 && child->getPriority() < 100))) {
            
            // This is likely a flag node, add it to removal list
            flagsToRemove.push_back(child);
        }
    }
    
    // Remove all identified flag nodes
    for (auto& flag : flagsToRemove) {
        _scene->removeChild(flag);
    }
    
    // Remove block nodes
    for (auto it = _blockNodes.begin(); it != _blockNodes.end(); ) {
        if (!isUIElement(*it)) {  // Only remove non-UI elements
            _scene->removeChild(*it);
            it = _blockNodes.erase(it);
        } else {
            ++it;
        }
    }
    
    // Remove character nodes
    if (_polarBear && !isUIElement(_polarBear)) {
        _scene->removeChild(_polarBear);
        _polarBear = nullptr;
    }
    
    if (_penguin && !isUIElement(_penguin)) {
        _scene->removeChild(_penguin);
        _penguin = nullptr;
    }
    
    // Clear all animations - first set opacity to 0 to make sure they're invisible
    for (auto& anim : _breakingAnimations) {
        if (anim.sprite) {
            // Set opacity to 0 before removal
            anim.sprite->setColor(cugl::Color4(255, 255, 255, 0));
            
            if (anim.sprite->getParent()) {
                _scene->removeChild(anim.sprite);
            }
        }
    }
    _breakingAnimations.clear();
    
    // Properly clean up finish block animations
    for (auto& anim : _finishBlockAnimations) {
        // Remove flag if it exists
        if (anim.flag && anim.flag->getParent()) {
            _scene->removeChild(anim.flag);
            anim.flag = nullptr;
        }
        
        // Reset block scale if it exists
        if (anim.block) {
            float originalScale = 1.13f * _tileSize / std::max(anim.block->getTexture()->getWidth(), anim.block->getTexture()->getHeight());
            anim.block->setScale(originalScale);
        }
    }
    _finishBlockAnimations.clear();
}

void PlaygroundRenderer::drawGrid(const std::vector<std::vector<int>>& grid, float gridRatio) {
    clear();
    
    // Calculate grid layout
    float sceneWidth = _scene->getSize().width;
    float sceneHeight = _scene->getSize().height;
    float targetGridHeight = sceneHeight * _tileHeightRatio;
    _tileSize = targetGridHeight / grid[0].size();
    _offsetX = (sceneWidth - grid.size() * _tileSize) / 2.0f;
    _offsetY = (sceneHeight - grid[0].size() * _tileSize) / 2.0f;
    
    // First pass: Render regular tiles, bear blocks, and penguin blocks
    for (size_t x = 0; x < grid.size(); ++x) {
        for (int y = grid[0].size() - 1; y >= 0; --y) {
            int cellType = grid[x][y];
            
            // Skip rendering for obstacles, invisible blocks, and empty passable
            if (cellType == 1 || cellType == 7 || cellType == 8) continue;
            
            // For regular tiles, bear blocks, penguin blocks - render directly
            if (cellType == 0 || cellType == 2 || cellType == 3) {
                auto baseNode = addCellNode(x, y, cellType);
                if (baseNode) {
                    auto it = CELL_PRIORITIES.find(cellType);
                    if (it != CELL_PRIORITIES.end()) {
                        baseNode->setPriority(it->second);
                    }
                }
                continue;
            }
            
            // For finish blocks and breakable blocks, add base layer first
            auto regularTile = addCellNode(x, y, 0);
            if (regularTile) regularTile->setPriority(-100);
        }
    }
    
    // Create and add character sprites
    createCharacters();
    if (_polarBear) _polarBear->setPriority(10);
    if (_penguin) _penguin->setPriority(10);
    
    // Second pass: Only add finish blocks and breakable blocks on top
    for (size_t x = 0; x < grid.size(); ++x) {
        for (int y = grid[0].size() - 1; y >= 0; --y) {
            int cellType = grid[x][y];
            
            // Only process finish blocks and breakable blocks
            if (cellType != 4 && cellType != 5 && cellType != 6) continue;
            
            // Add special block on top
            auto specialNode = addCellNode(x, y, cellType);
            if (specialNode) {
                auto it = CELL_PRIORITIES.find(cellType);
                if (it != CELL_PRIORITIES.end()) {
                    specialNode->setPriority(it->second);
                }
            }
        }
    }
}

std::shared_ptr<cugl::scene2::PolygonNode> PlaygroundRenderer::addCellNode(int x, int y, int cellType) {
    std::shared_ptr<cugl::graphics::Texture> tex = nullptr;
    
    switch (cellType) {
        case 0: // Regular floor tile
            tex = _assets->get<cugl::graphics::Texture>("regularblock");
            break;
        case 1: // Obstacle - should never be called for this
            return nullptr;
        case 2: // Bear-only block
            tex = _assets->get<cugl::graphics::Texture>("bearblock");
            break;
        case 3: // Penguin-only block
            tex = _assets->get<cugl::graphics::Texture>("penguinblock");
            break;
        case 4: // Bear finish block
            tex = _assets->get<cugl::graphics::Texture>("bearfinish");
            break;
        case 5: // Penguin finish block
            tex = _assets->get<cugl::graphics::Texture>("penguinfinish");
            break;
        case 6: // Breakable block
            tex = _assets->get<cugl::graphics::Texture>("breakableblock");
            break;
        case 7: // Invisible block - don't render anything
            return nullptr;
        case 8: // Empty passable - don't render anything
            return nullptr;
        default:
            return nullptr; // Unknown cell type
    }
    
    if (tex) {
        auto node = cugl::scene2::PolygonNode::allocWithTexture(tex);
        float blockScale;
        
        if (cellType == 0) {
            // Use a larger scale for regular tiles
            blockScale = 1.13f * _tileSize / std::max(tex->getWidth(), tex->getHeight());
        } else {
            // Keep the original scale for other tile types
            blockScale = 1.13f * _tileSize / std::max(tex->getWidth(), tex->getHeight());
        }
        
        node->setScale(blockScale);
        node->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        
        // Calculate base position
        float posX = _offsetX + x * _tileSize + _tileSize / 2.0f;
        float posY = _offsetY + y * _tileSize + _tileSize / 2.0f;
        
        // Add vertical offset for special blocks
        if (cellType == 4 || cellType == 5 || cellType == 6) {
            posY += 0.15f * _tileSize;
        }
        
        node->setPosition(posX, posY);
        
        _scene->addChild(node);
        _blockNodes.push_back(node);
        return node;
    }
    return nullptr;
}

void PlaygroundRenderer::updateCell(int x, int y, int cellType) {
    // First remove all nodes at this position - regular tiles and special blocks
    for (auto it = _blockNodes.begin(); it != _blockNodes.end(); ) {
        // Account for the offset when calculating the grid position
        float baseX = ((*it)->getPosition().x - _offsetX - _tileSize/2.0f) / _tileSize;
        float baseY = ((*it)->getPosition().y - _offsetY - _tileSize/2.0f) / _tileSize;
        
        // Adjust y-coordinate for finish and breakable blocks which have offsets
        float nodeY = baseY;
        int priority = (*it)->getPriority();
        if (priority == CELL_PRIORITIES.at(4) || priority == CELL_PRIORITIES.at(5) || priority == CELL_PRIORITIES.at(6)) {
            // Special blocks have a vertical offset we need to account for
            nodeY = ((*it)->getPosition().y - _offsetY - _tileSize/2.0f - 0.15f * _tileSize) / _tileSize;
        }
        
        if (std::abs(baseX - x) < 0.1f && std::abs(nodeY - y) < 0.1f) {
            // For type 8 (empty passable), keep regular tiles
            if (cellType == 8 && (*it)->getPriority() == -100) {
                ++it;
            } else {
                _scene->removeChild(*it);
                it = _blockNodes.erase(it);
            }
        } else {
            ++it;
        }
    }
    
    // Also check for and hide any breaking animations at this position
    for (auto& anim : _breakingAnimations) {
        if (anim.x == x && anim.y == y && anim.sprite) {
            // Force hide the animation sprite
            anim.sprite->setColor(cugl::Color4(255, 255, 255, 0));
        }
    }
    
    // Skip rendering for obstacles, invisible blocks, and empty passable
    if (cellType == 1 || cellType == 7 || cellType == 8) return;
    
    // For regular tiles, bear blocks, penguin blocks - render directly
    if (cellType == 0 || cellType == 2 || cellType == 3) {
        auto node = addCellNode(x, y, cellType);
        if (node) {
            auto it = CELL_PRIORITIES.find(cellType);
            if (it != CELL_PRIORITIES.end()) {
                node->setPriority(it->second);
            }
        }
        return;
    }
    
    // For finish blocks and breakable blocks
    // 1. First add regular tile as base
    auto regularTile = addCellNode(x, y, 0);
    if (regularTile) {
        regularTile->setPriority(-100);  // Lowest layer
    }
    
    // 2. Add the special block with high priority
    auto specialBlock = addCellNode(x, y, cellType);
    if (specialBlock) {
        auto it = CELL_PRIORITIES.find(cellType);
        if (it != CELL_PRIORITIES.end()) {
            specialBlock->setPriority(it->second);  // Use priority from map
        }
    }
}

void PlaygroundRenderer::createCharacters() {
    // Create polar bear sprite
    std::shared_ptr<cugl::graphics::Texture> bearTexture = _assets->get<cugl::graphics::Texture>("polarbear");
    if (bearTexture) {
        _polarBear = cugl::scene2::PolygonNode::allocWithTexture(bearTexture);
        float scale = 1.0f * _tileSize / std::max(bearTexture->getWidth(), bearTexture->getHeight());
        _polarBear->setScale(scale);
        _polarBear->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        _polarBear->setPriority(10);  // Characters should be at priority 10
        _scene->addChild(_polarBear);
    }
    
    // Create penguin sprite
    std::shared_ptr<cugl::graphics::Texture> penguinTexture = _assets->get<cugl::graphics::Texture>("penguin");
    if (penguinTexture) {
        _penguin = cugl::scene2::PolygonNode::allocWithTexture(penguinTexture);
        float scale = 1.0f * _tileSize / std::max(penguinTexture->getWidth(), penguinTexture->getHeight());
        _penguin->setScale(scale);
        _penguin->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        _penguin->setPriority(10);  // Characters should be at priority 10
        _scene->addChild(_penguin);
    }
}

void PlaygroundRenderer::updateCharacterPositions(const cugl::Vec2& bearPos, const cugl::Vec2& penguinPos) {
    cugl::Vec2 bearScreenPos = gridToScreenPos(bearPos.x, bearPos.y);
    cugl::Vec2 penguinScreenPos = gridToScreenPos(penguinPos.x, penguinPos.y);
    
    // Add vertical offset for characters
    bearScreenPos.y += 0.15f * _tileSize;
    penguinScreenPos.y += 0.15f * _tileSize;
    
    if (_polarBear) {
        _polarBear->setPosition(bearScreenPos);
    }
    
    if (_penguin) {
        _penguin->setPosition(penguinScreenPos);
    }
}

void PlaygroundRenderer::moveCharacters(const cugl::Vec2& bearStart, const cugl::Vec2& bearTarget,
                                      const cugl::Vec2& penguinStart, const cugl::Vec2& penguinTarget,
                                      float progress) {
    cugl::Vec2 bearStartScreen = gridToScreenPos(bearStart.x, bearStart.y);
    cugl::Vec2 bearTargetScreen = gridToScreenPos(bearTarget.x, bearTarget.y);
    cugl::Vec2 penguinStartScreen = gridToScreenPos(penguinStart.x, penguinStart.y);
    cugl::Vec2 penguinTargetScreen = gridToScreenPos(penguinTarget.x, penguinTarget.y);
    
    // Add vertical offset for characters
    bearStartScreen.y += 0.15f * _tileSize;
    bearTargetScreen.y += 0.15f * _tileSize;
    penguinStartScreen.y += 0.15f * _tileSize;
    penguinTargetScreen.y += 0.15f * _tileSize;
    
    cugl::Vec2 bearPos = bearStartScreen.lerp(bearTargetScreen, progress);
    cugl::Vec2 penguinPos = penguinStartScreen.lerp(penguinTargetScreen, progress);
    
    if (_polarBear) {
        _polarBear->setPosition(bearPos);
    }
    
    if (_penguin) {
        _penguin->setPosition(penguinPos);
    }
}

cugl::Vec2 PlaygroundRenderer::gridToScreenPos(float x, float y) const {
    return cugl::Vec2(
        _offsetX + x * _tileSize + _tileSize / 2.0f,
        _offsetY + y * _tileSize + _tileSize / 2.0f
    );
}

cugl::Vec2 PlaygroundRenderer::getScreenPosition(int x, int y) const {
    return gridToScreenPos(x, y);
}

void PlaygroundRenderer::startBreakAnimation(int x, int y) {
    // Check if we already have an animation for this position
    for (auto& anim : _breakingAnimations) {
        if (anim.x == x && anim.y == y) {
            return; // Already animating this block
        }
    }
    
    // First, remove the static breakable block texture at this position
    for (auto it = _blockNodes.begin(); it != _blockNodes.end(); ) {
        // Calculate base position
        float baseX = ((*it)->getPosition().x - _offsetX - _tileSize/2.0f) / _tileSize;
        float baseY = ((*it)->getPosition().y - _offsetY - _tileSize/2.0f) / _tileSize;
        
        // For breakable blocks (priority 20), account for the vertical offset
        float nodeY = baseY;
        if ((*it)->getPriority() == CELL_PRIORITIES.at(6)) { // Breakable block priority
            nodeY = ((*it)->getPosition().y - _offsetY - _tileSize/2.0f - 0.15f * _tileSize) / _tileSize;
        }
        
        if (std::abs(baseX - x) < 0.1f && std::abs(nodeY - y) < 0.1f) {
            // Don't remove the regular tile (priority -100)
            if ((*it)->getPriority() == -100) {
                ++it;
            } else {
                // Remove the static breakable block
                _scene->removeChild(*it);
                it = _blockNodes.erase(it);
            }
        } else {
            ++it;
        }
    }
    
    // Get the texture for breaking animation
    std::shared_ptr<cugl::graphics::Texture> texture = _assets->get<cugl::graphics::Texture>("BreakIceSS");
    if (texture) {
        // Create a sprite node with the filmstrip texture
        // The filmstrip information comes from the assets.json
        // With 6 frames and 3 columns, we need 2 rows (6/3 = 2)
        std::shared_ptr<cugl::scene2::SpriteNode> spriteNode = 
            cugl::scene2::SpriteNode::allocWithSheet(texture, 2, 3, 6);
        
        // Position the animation at the correct spot
        spriteNode->setFrame(0); // Start at the first frame
        
        // Scale the node to match the block size
        float scale = 1.12f * _tileSize / std::max(texture->getWidth()/3, texture->getHeight()/2);
        spriteNode->setScale(scale);
        spriteNode->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        
        // Calculate position with offset
        float posX = _offsetX + x * _tileSize + _tileSize / 2.0f;
        float posY = _offsetY + y * _tileSize + _tileSize / 2.0f + (0.15f * _tileSize); // Add vertical offset
        spriteNode->setPosition(posX, posY);
        
        // Give the sprite a unique name for easier tracking
        std::string spriteName = "break_anim_" + std::to_string(x) + "_" + std::to_string(y);
        spriteNode->setName(spriteName);
        
        spriteNode->setPriority(25); // Higher than regular blocks
        
        _scene->addChild(spriteNode);
        
        // Add to active animations
        _breakingAnimations.emplace_back(x, y, spriteNode);
    }
}

void PlaygroundRenderer::update(float dt) {
    // Update animation time
    _animTime += dt;
    
    // Update breaking animations
    for (auto it = _breakingAnimations.begin(); it != _breakingAnimations.end(); ) {
        it->totalTime += dt;
        if (it->totalTime >= _frameTime * 6) {
            // Animation is complete, make the sprite invisible instead of removing it
            if (it->sprite) {
                // Set opacity to 0 to make it invisible
                it->sprite->setColor(cugl::Color4(255, 255, 255, 0));
            }
            
            // Mark as complete and remove from list
            it->complete = true;
            it = _breakingAnimations.erase(it);
        } else {
            // Animation still running - update the frame
            int frame = static_cast<int>(it->totalTime / _frameTime);
            if (frame < 6 && it->sprite) {
                it->sprite->setFrame(frame);
            }
            ++it;
        }
    }
    
    // Update blocked animations
    for (auto it = _blockedAnimations.begin(); it != _blockedAnimations.end(); ) {
        it->progress += dt / 0.15f;  // 0.15 second animation
        
        if (it->progress >= 1.0f) {
            // Animation complete, reset characters to original positions
            if (it->bearCharacter) {
                it->bearCharacter->setPosition(it->bearOriginalPos);
            }
            if (it->penguinCharacter) {
                it->penguinCharacter->setPosition(it->penguinOriginalPos);
            }
            
            it = _blockedAnimations.erase(it);
        } else {
            float offset = 0.0f;
            
            if (it->progress < 0.5f) {
                // Moving forward phase (0 to 0.5)
                offset = 0.2f * it->tileSize * (it->progress / 0.5f);
            } else {
                // Moving back phase (0.5 to 1.0)
                offset = 0.2f * it->tileSize * (1.0f - ((it->progress - 0.5f) / 0.5f));
            }
            
            // Calculate the position offset based on direction
            cugl::Vec2 posOffset = it->direction * offset;
            
            // Apply offset to character positions
            if (it->bearCharacter) {
                it->bearCharacter->setPosition(it->bearOriginalPos + posOffset);
            }
            if (it->penguinCharacter) {
                it->penguinCharacter->setPosition(it->penguinOriginalPos + posOffset);
            }
            
            ++it;
        }
    }
    
    // Update character bounce animations
    for (auto it = _characterBounceAnimations.begin(); it != _characterBounceAnimations.end(); ) {
        it->progress += dt / 0.15f;  // 0.15 second animation (twice as fast as original)
        
        if (it->progress >= 1.0f) {
            // Animation complete, reset character to normal scale
            if (it->character) {
                it->character->setScale(it->originalScale);
            }
            
            it = _characterBounceAnimations.erase(it);
        } else {
            // Calculate current animation state
            float scale = 1.0f;
            if (it->progress < 0.5f) {
                // Expanding phase (0 to 0.5)
                scale = 1.0f + 0.35f * (it->progress / 0.5f);  // 35% expansion
            } else {
                // Contracting phase (0.5 to 1.0)
                scale = 1.35f - 0.35f * ((it->progress - 0.5f) / 0.5f);
            }
            
            // Apply scale to character
            if (it->character) {
                it->character->setScale(it->originalScale * scale);
            }
            
            ++it;
        }
    }
    
    // Update finish block animations
    for (auto it = _finishBlockAnimations.begin(); it != _finishBlockAnimations.end(); ) {
        it->progress += dt / 0.4f;  // 0.4 second animation
        
        if (it->progress >= 1.0f) {
            // Animation complete, reset block to normal scale
            if (it->block) {
                float originalScale = 1.13f * _tileSize / std::max(it->block->getTexture()->getWidth(), it->block->getTexture()->getHeight());
                it->block->setScale(originalScale);
            }
            
            // Remove flag if it exists
            if (it->flag && it->flag->getParent()) {
                _scene->removeChild(it->flag);
                it->flag = nullptr;
            }
            
            it = _finishBlockAnimations.erase(it);
        } else {
            // Calculate current animation state
            float scale = 1.0f;
            if (it->progress < 0.5f) {
                // Expanding phase (0 to 0.5)
                scale = 1.0f + 0.3f * (it->progress / 0.5f);
            } else {
                // Contracting phase (0.5 to 1.0)
                scale = 1.3f - 0.3f * ((it->progress - 0.5f) / 0.5f);
            }
            
            // Apply scale to finish block
            if (it->block) {
                float baseScale = 1.13f * _tileSize / std::max(it->block->getTexture()->getWidth(), it->block->getTexture()->getHeight());
                it->block->setScale(baseScale * scale);
                
                // Create flag immediately if it doesn't exist yet
                if (!it->flag) {
                    std::string flagTexture = it->isBear ? "BearFlag" : "SealFlag";
                    auto tex = _assets->get<cugl::graphics::Texture>(flagTexture);
                    if (tex) {
                        it->flag = cugl::scene2::PolygonNode::allocWithTexture(tex);
                        float flagScale = 1.5f * _tileSize / tex->getHeight(); // 1.5x tile height
                        it->flag->setScale(flagScale);
                        it->flag->setAnchor(cugl::Vec2::ANCHOR_CENTER);
                        
                        // Add vertical offset of 0.2f * _tileSize to the flag position
                        cugl::Vec2 blockPos = it->block->getPosition();
                        cugl::Vec2 flagPos = blockPos + cugl::Vec2(0, 0.28f * _tileSize);
                        it->flag->setPosition(flagPos);
                        
                        it->flag->setPriority(it->block->getPriority() + 1);  // Higher than finish block
                        it->flag->setColor(cugl::Color4(255, 255, 255, 0));  // Start with 0 opacity
                        _scene->addChild(it->flag);
                        
                        // Name the flag for better tracking
                        std::string flagName = (it->isBear ? "BearFlag_" : "SealFlag_") + std::to_string(it->x) + "_" + std::to_string(it->y);
                        it->flag->setName(flagName);
                    }
                }
                
                // Fade in the flag over the first 0.3 seconds (75% of the animation)
                if (it->flag) {
                    float fadeProgress = std::min(1.0f, it->progress / 0.75f);
                    int alpha = static_cast<int>(255.0f * fadeProgress);
                    it->flag->setColor(cugl::Color4(255, 255, 255, alpha));
                }
            }
            
            ++it;
        }
    }
}

void PlaygroundRenderer::startCharacterBounceAnimation(bool isBear) {
    // Get the appropriate character node
    std::shared_ptr<cugl::scene2::SceneNode> character = isBear ? _polarBear : _penguin;
    
    if (!character) {
        return; // Character doesn't exist
    }
    
    // Check if this character is already animating
    bool alreadyAnimating = false;
    for (const auto& anim : _characterBounceAnimations) {
        if (anim.isBear == isBear) {
            alreadyAnimating = true;
            break;
        }
    }
    
    if (!alreadyAnimating) {
        // Get original scale
        float originalScale = character->getScale().x;
        
        // Create and add the animation
        _characterBounceAnimations.emplace_back(isBear, character, originalScale);
    }
}

void PlaygroundRenderer::startBlockedAnimation(const cugl::Vec2& direction) {
    // Check if either character doesn't exist
    if (!_polarBear || !_penguin) {
        return;
    }
    
    // Check if there's already an active blocked animation
    if (!_blockedAnimations.empty()) {
        return;
    }
    
    // Create and add the animation
    _blockedAnimations.emplace_back(direction, _tileSize, _polarBear, _penguin);
}

void PlaygroundRenderer::startFinishBlockAnimation(int x, int y, bool isBear) {
    // Find the finish block node at this position
    for (auto& node : _blockNodes) {
        // Calculate grid position from node position, accounting for the vertical offset
        float nodeX = (node->getPosition().x - _offsetX - _tileSize/2.0f) / _tileSize;
        float nodeY = (node->getPosition().y - _offsetY - _tileSize/2.0f - 0.15f * _tileSize) / _tileSize;
        
        if (std::abs(nodeX - x) < 0.1f && std::abs(nodeY - y) < 0.1f) {
            // Check if it's a finish block by looking at its priority
            int priority = node->getPriority();
            if (priority == CELL_PRIORITIES.at(isBear ? 4 : 5)) {  // Bear finish = 4, Penguin finish = 5
                // Cast to PolygonNode
                auto blockNode = std::dynamic_pointer_cast<cugl::scene2::PolygonNode>(node);
                if (blockNode) {
                    // Check if this block is already animating
                    bool alreadyAnimating = false;
                    for (const auto& anim : _finishBlockAnimations) {
                        if (anim.x == x && anim.y == y) {
                            alreadyAnimating = true;
                            break;
                        }
                    }
                    
                    if (!alreadyAnimating) {
                        _finishBlockAnimations.emplace_back(x, y, isBear, blockNode);
                    }
                    return;
                }
            }
        }
    }
}

bool PlaygroundRenderer::isUIElement(const std::shared_ptr<cugl::scene2::SceneNode>& node) const {
    // Check if the node is a UI element (like buttons)
    // UI elements typically have high priority values (>= 1000)
    return node && node->getPriority() >= 1000;
} 