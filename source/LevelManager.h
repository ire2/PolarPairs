#pragma once
#include <cugl/cugl.h>
#include <string>
#include <vector>
#include <fstream>

class LevelManager {
private:
    // Singleton instance
    static LevelManager* _instance;
    
    // Level data
    struct LevelInfo {
        int levelNumber;
        bool isUnlocked;
        int score;  // 0-3 stars
    };
    std::vector<LevelInfo> _levels;
    
    // File paths
    std::string _saveFilePath;
    
    // Private constructor for singleton
    LevelManager();
    
    // Load level data from file
    bool loadLevelData();
    
    // Save level data to file
    bool saveLevelData();
    
public:
    // Get singleton instance
    static LevelManager* getInstance();
    
    // Initialize the level manager
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
    
    // Check if a level is unlocked
    bool isLevelUnlocked(int levelNumber);
    
    // Get level score
    int getLevelScore(int levelNumber);
    
    // Set level score (0-3)
    void setLevelScore(int levelNumber, int score);
    
    // Unlock a level
    void unlockLevel(int levelNumber);
    
    // Get total number of levels
    int getTotalLevels() const { return _levels.size(); }
    
    // Reset all level progress (only keep level 1 unlocked)
    void resetAllProgress();
}; 