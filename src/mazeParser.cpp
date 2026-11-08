#pragma once

#include"mazeParser.h"

using namespace std;


vector<string> tokenize(string s, string del = " ")
{
    int start = 0;
    int end = s.find(del);
    vector<string> ret;
    while (end != -1) {
        if(s.at(start)=='#') return ret;
        if(s.substr(start, end - start)!="")ret.push_back(s.substr(start, end - start));
        start = end + del.size();
        end = s.find(del, start);
    }
    ret.push_back(s.substr(start, end - start));
    return ret;
}

void parseMaze(GLint& pXSIZE, GLint& pYSIZE, string maze_path){
    maze_path = "./resources/" + maze_path;
    ifstream infile(maze_path);

    int textureNum=2, xSize=pXSIZE, ySize=pYSIZE;
    float cell = FULL_CUBE, ht = FULL_CUBE;
    vector<vector<int>> wallTextures(ySize*2, vector<int>(xSize,0));
    vector<string> texturePaths{ WALL_FILE, FLOOR_FILE };

    if (!infile) { 
        cout << "File does not exist!"; 
        WALL_TEXTURES = wallTextures;
        TEXTURE_PATHS = texturePaths;
        return ; 
    }

    
    string line1;
    while(getline(infile, line1)){
        if(line1[0]=='#') continue;
        else{
            vector<string> tokens = tokenize(line1);
                
            if (tokens[0] == "DIMENSIONS") {
                xSize = stoi(tokens[1]); ySize = stoi(tokens[2]);
            }
            else if (tokens[0] == "HEIGHT") {
                ht = stof(tokens[1]);
            }
            else if (tokens[0] == "CELL") {
                cell = stof(tokens[1]);
            }
            else if (tokens[0] == "TEXTURES") {
                textureNum = stoi(tokens[1]);
                texturePaths.resize(textureNum);
                string texture; 
                int t = 0;
                while (getline(infile, texture)&&t<textureNum) {
                    if (texture[0] == '#') continue;

                    vector<string> texts = tokenize(texture);
                    int idx = stoi(texts[0])-1; 
                    string bmp = texts[1].replace(texts[1].find("tif"), 4, "bmp");
                    texturePaths[idx] = TEXTURE_PATH_PREFIX + bmp;
                    t++;
                }
            }
            else if (tokens[0] == "FLOORPLAN") {
                wallTextures.resize(ySize * 2);
                string texture;
                    
                for (int i = 0; i < ySize*2;) {
                    getline(infile, texture);
                    if (texture[0] == '#') continue;
                    vector<string> texts = tokenize(texture);
                    wallTextures[i].resize(xSize);
                    for (int j = 0; j < xSize;j++) {
                        wallTextures[i][j] = stoi(texts[j]);
                    }
                    i++;
                }                    
            }                
        }
    }

    WALL_TEXTURES = wallTextures;
    TEXTURE_PATHS = texturePaths;

    pXSIZE = xSize;
    pYSIZE = ySize;
    
    FULL_CUBE = cell;
    HALF_CUBE = cell / 2;
    LEFTMOST_CUBE_CENTER = MAZE_EXTREME_LEFT + HALF_CUBE;
    WALL_HT = ht;
    X_INIT = LEFTMOST_CUBE_CENTER + (xSize / 2) * FULL_CUBE;
    Y_INIT = MAZE_EXTREME_TOP + (ySize / 2) * FULL_CUBE + HALF_CUBE;
}
