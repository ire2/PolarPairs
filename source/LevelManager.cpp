#include "LevelManager.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>

// Initialize static instance
LevelManager* LevelManager::_instance = nullptr;

LevelManager::LevelManager() {
    // Use the application's save directory instead of assets
    _saveFilePath = cugl::Application::get()->getSaveDirectory() + "level_progress.txt";
    CULog("LevelManager constructor: Save file path set to: %s", _saveFilePath.c_str());
    
    // Create save directory if it doesn't exist
    std::string saveDir = cugl::Application::get()->getSaveDirectory();
    CULog("LevelManager constructor: Save directory path: %s", saveDir.c_str());
    
    struct stat info;
    if (stat(saveDir.c_str(), &info) != 0) {
        CULog("LevelManager constructor: Save directory does not exist, attempting to create it");
        // Directory doesn't exist, create it
        #if defined(__WINDOWS__)
         /*   if (mkdir(saveDir.c_str()) != 0) {
                CULog("LevelManager constructor: Failed to create save directory");
            } else {
                CULog("LevelManager constructor: Successfully created save directory");
            }*/
        #else
            if (mkdir(saveDir.c_str(), 0777) != 0) {
                CULog("LevelManager constructor: Failed to create save directory");
            } else {
                CULog("LevelManager constructor: Successfully created save directory");
            }
        #endif
    } else {
        CULog("LevelManager constructor: Save directory already exists");
    }
}

LevelManager* LevelManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new LevelManager();
    }
    return _instance;
}

bool LevelManager::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    CULog("LevelManager::init() called");
    
    // Initialize with default values
    _levels.clear();
    
    // First level is always unlocked
    _levels.push_back({1, true, 0});
    
    // Add other levels (locked by default)
    for (int i = 2; i <= 12; i++) {  // 12 levels total
        _levels.push_back({i, false, 0});
    }
    
    // Try to load saved progress
    CULog("Attempting to load level data...");
    if (!loadLevelData()) {
        CULog("Failed to load level data, saving default state...");
        // If loading fails, save default state
        if (!saveLevelData()) {
            CULog("Failed to save default level data!");
        }
    }
    
    return true;
}

bool LevelManager::loadLevelData() {
    try {
        CULog("loadLevelData() called, trying to open: %s", _saveFilePath.c_str());
        std::ifstream file(_saveFilePath);
        if (!file.is_open()) {
            CULog("Could not open level progress file at: %s", _saveFilePath.c_str());
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            CULog("Read line: %s", line.c_str());
            std::istringstream iss(line);
            int level, score;
            bool unlocked;
            char colon;
            
            if (iss >> level >> colon >> unlocked >> colon >> score) {
                if (level > 0 && level <= _levels.size()) {
                    _levels[level - 1].isUnlocked = unlocked;
                    _levels[level - 1].score = score;
                }
            }
        }
        
        CULog("Successfully loaded level data");
        return true;
    } catch (const std::exception& e) {
        CULog("Error loading level data: %s", e.what());
        return false;
    }
}

bool LevelManager::saveLevelData() {
    try {
        CULog("saveLevelData() called");
        // Log the full path we're trying to write to
        CULog("Attempting to save level data to: %s", _saveFilePath.c_str());
        
        // Check if directory exists and is writable
        std::string saveDir = cugl::Application::get()->getSaveDirectory();
        CULog("Save directory path: %s", saveDir.c_str());
        
        struct stat info;
        if (stat(saveDir.c_str(), &info) != 0) {
            CULog("Save directory does not exist: %s", saveDir.c_str());
            return false;
        }
       /* if (!(info.st_mode & S_IWUSR)) {
            CULog("Save directory is not writable: %s", saveDir.c_str());
            return false;
        }*/
        
        std::ofstream file(_saveFilePath);
        if (!file.is_open()) {
            CULog("Could not open level progress file for writing at: %s", _saveFilePath.c_str());
            return false;
        }
        
        // Write each level in format: level:unlocked:score
        for (const auto& level : _levels) {
            file << level.levelNumber << ":" << level.isUnlocked << ":" << level.score << "\n";
            CULog("Writing level data: %d:%d:%d", level.levelNumber, level.isUnlocked, level.score);
        }
        
        CULog("Successfully saved level data");
        return true;
    } catch (const std::exception& e) {
        CULog("Error saving level data: %s", e.what());
        return false;
    }
}

bool LevelManager::isLevelUnlocked(int levelNumber) {
    if (levelNumber <= 0 || levelNumber > _levels.size()) {
        return false;
    }
    return _levels[levelNumber - 1].isUnlocked;
}

int LevelManager::getLevelScore(int levelNumber) {
    if (levelNumber <= 0 || levelNumber > _levels.size()) {
        return 0;
    }
    return _levels[levelNumber - 1].score;
}

void LevelManager::setLevelScore(int levelNumber, int score) {
    if (levelNumber <= 0 || levelNumber > _levels.size()) {
        return;
    }
    
    // Ensure score is between 0 and 3
    score = std::max(0, std::min(3, score));
    
    // Only update the score if the new score is higher than the existing one
    if (score > _levels[levelNumber - 1].score) {
        CULog("Updating level %d score from %d to %d", levelNumber, _levels[levelNumber - 1].score, score);
        _levels[levelNumber - 1].score = score;
        
        // If this level has a score > 0, unlock the next level
        if (score > 0 && levelNumber < _levels.size()) {
            _levels[levelNumber].isUnlocked = true;
        }
        
        saveLevelData();
    } else {
        CULog("Ignoring new score %d for level %d as it's not higher than existing score %d", 
              score, levelNumber, _levels[levelNumber - 1].score);
    }
}

void LevelManager::unlockLevel(int levelNumber) {
    if (levelNumber > 0 && levelNumber <= _levels.size()) {
        _levels[levelNumber - 1].isUnlocked = true;
        saveLevelData();
    }
}

void LevelManager::resetAllProgress() {
    // Reset all level data
    _levels.clear();
    
    // First level is always unlocked
    _levels.push_back({1, true, 0});
    
    // Add other levels (locked by default)
    for (int i = 2; i <= 12; i++) {  // 12 levels total
        _levels.push_back({i, false, 0});
    }
    
    // Save the reset state
    saveLevelData();
    
    CULog("All level progress has been reset");
} 
