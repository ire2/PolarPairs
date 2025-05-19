#ifndef __LEVEL_DATA_H__
#define __LEVEL_DATA_H__

#include <cugl/cugl.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

/**
 * Class to store and load level data.
 */
class LevelData {
public:
    // Character positions
    cugl::Vec2 polarBearPos;
    cugl::Vec2 penguinPos;
    
    // Block positions
    std::vector<cugl::Vec2> blocks;           // Regular obstacles (neither can pass)
    std::vector<cugl::Vec2> bearBlocks;       // Only bear can pass
    std::vector<cugl::Vec2> penguinBlocks;    // Only penguin can pass
    std::vector<cugl::Vec2> breakableBlocks;  // Breakable blocks
    std::vector<cugl::Vec2> bearFinishBlocks; // Bear finish positions
    std::vector<cugl::Vec2> penguinFinishBlocks; // Penguin finish positions
    std::vector<cugl::Vec2> invisibleBlocks;  // Invisible blocks (type 7)
    
    std::string name;
    
    static LevelData loadLevel(const std::shared_ptr<cugl::AssetManager>& assets, int levelNum) {

        std::string assetDir = cugl::Application::get()->getAssetDirectory();
        std::string levelPath = assetDir + "levels/level" + std::to_string(levelNum) + ".txt";
        CULog("%s",levelPath.c_str());
        
        std::ifstream file(levelPath);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            
            LevelData data;
            if (parseFromString(data, content)) {
                CULog("Successfully loaded level %d", levelNum);
                return data;
            }
        }
        
        CULog("Could not load level %d, creating default level", levelNum);
        return createDefaultLevel(levelNum);
    }
    
private:
    /**
     * Parse level data from a string containing the level file content
     *
     * @param data The LevelData to populate
     * @param content The string containing the level file content
     * @return true if parsing was successful, false otherwise
     */
    static bool parseFromString(LevelData& data, const std::string& content) {
        if (content.empty()) return false;
        
        std::istringstream stream(content);
        std::string line;
        
        // Read the level name (first line)
        if (!std::getline(stream, line) || line.empty()) return false;
        data.name = line;
        
        // Read the level dimensions (second line)
        int width = 7;  // Default width
        int height = 11; // Default height
        if (std::getline(stream, line)) {
            std::istringstream iss(line);
            iss >> width >> height;
        }
        
        // Read the level data (remaining lines)
        std::vector<std::string> rows;
        while (std::getline(stream, line)) {
            rows.push_back(line);
        }
        
        // Parse the grid (convert from text file coordinates to game coordinates)
        int maxRows = std::min(static_cast<int>(rows.size()), height);
        for (int row = 0; row < maxRows; row++) {
            // Calculate y coordinate (flip y axis - in file, 0 is top, but in game, 0 is bottom)
            int y = height - row - 1;
            
            for (int x = 0; x < std::min(static_cast<int>(rows[row].length()), width); x++) {
                parseCell(data, x, y, rows[row][x]);
            }
        }
        
        return true;
    }
    
    /**
     * Parse a single cell from the level file
     *
     * @param data The LevelData to populate
     * @param x The x coordinate of the cell
     * @param y The y coordinate of the cell
     * @param c The character representing the cell type
     */
    static void parseCell(LevelData& data, int x, int y, char c) {
        switch (c) {
            case '.': // Empty space
                // Do nothing
                break;
                
            case 'X': // Regular obstacle
                data.blocks.push_back(cugl::Vec2(x, y));
                break;
                
            case '!': // Breakable block
                data.breakableBlocks.push_back(cugl::Vec2(x, y));
                break;
                
            case 'B': // Bear start point
                data.polarBearPos = cugl::Vec2(x, y);
                break;
                
            case 'S': // Seal (penguin) start point
                data.penguinPos = cugl::Vec2(x, y);
                break;
                
            case '$': // Seal-passable block
                data.penguinBlocks.push_back(cugl::Vec2(x, y));
                break;
                
            case '&': // Bear-passable block
                data.bearBlocks.push_back(cugl::Vec2(x, y));
                break;
                
            case '^': // Seal finish block
                data.penguinFinishBlocks.push_back(cugl::Vec2(x, y));
                break;
                
            case '*': // Bear finish block
                data.bearFinishBlocks.push_back(cugl::Vec2(x, y));
                break;
                
            case 'I': // Invisible block
                data.invisibleBlocks.push_back(cugl::Vec2(x, y));
                break;
        }
    }
    
    /**
     * Create a default level (simple empty level)
     */
    static LevelData createDefaultLevel(int levelNum) {
        LevelData data;
        data.name = "Default Level " + std::to_string(levelNum);
        
        // Default positions
        data.polarBearPos = cugl::Vec2(1, 1);
        data.penguinPos = cugl::Vec2(5, 1);
        
        // Add walls around the edge
        for (int x = 0; x < 7; x++) {
            data.blocks.push_back(cugl::Vec2(x, 0));
            data.blocks.push_back(cugl::Vec2(x, 10));
        }
        
        for (int y = 1; y < 10; y++) {
            data.blocks.push_back(cugl::Vec2(0, y));
            data.blocks.push_back(cugl::Vec2(6, y));
        }
        
        // Add finish blocks
        data.bearFinishBlocks.push_back(cugl::Vec2(5, 8));
        data.penguinFinishBlocks.push_back(cugl::Vec2(1, 8));
        
        return data;
    }
};

#endif /* __LEVEL_DATA_H__ */
