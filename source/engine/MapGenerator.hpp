/*
 * MIT License
 *
 * Copyright (c) 2020 Nathan Ollerenshaw
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <deque>

#include "TileMap.hpp"

namespace ge
{

// MapConnectors represent a specific location on the map that connects two regions.
class MapConnector
{
  public:
    MapConnector(sf::Vector2i, int, int);
    sf::Vector2i location;
    int first_region_id;
    int second_region_id;
};

// Represents the current configuration for a MapGenerator.
class MapGeneratorConfig
{
  public:
    int width        = 120;  // width in MapTiles
    int height       = 120;  // height in MapTiles
    int seed         = 1;    // seed used to generate the map
    int room_max     = 9;    // maximum number of tiles a room can be in either dimension
    int room_min     = 3;    // minimum number of tiles a room can be in either dimension
    float room_ratio = 0.6F; // a number from 0 to 1 that expresses the minimum ratio of w to h for a room. For
                             // exampple, 3:5 would be 0.6.
    int room_attempts          = 1000;  // number of times the generator will attempt to place rooms
    int extra_connector_chance = 100;   // extra door chance
    int dead_ends              = 0;     // number of dead ends you want to leave
    int sleep_msec             = 0;     // how long you want to sleep during generation
    bool stairs                = false; // generate stairs
    bool edge_egress           = false; // generate egress on the edge
};

enum class Direction {
    NORTH,
    SOUTH,
    WEST,
    EAST,
};

class MapGenerator
{
  public:
    MapGenerator();

  public:
    void generate(MapGeneratorConfig);
    float generationProgress();
    std::shared_ptr<TileMap> tile_map;
    MapGeneratorConfig config;

    // used by map generation to deterministically generate the same numbers from the same seed
    std::shared_ptr<std::default_random_engine> rand_engine;

  private:
    void generateRooms();
    void generateHallways();
    void generateConnectors();
    void connectRegions();
    void removeDeadends();
    void renderSleep();

    // Hallway generation
    void startWalking(sf::Vector2i);
    bool mazeWalk(sf::Vector2i *);
    bool mazeHunt(sf::Vector2i *);
    bool scanForWalls();

  private:
    //////////////////////////////////////////////////
    // internal state for the map generation algorithm.
    // this is where we keep track of the root region.
    int root_region_id;
    // when we generate new regions, we keep track of the current one here.
    int current_region_id;
    // map of region_id to a deque of Connectors for that region. This allows
    // us to quicky move all the connectors from on region to another
    // when a region is merged.
    std::map<int, std::deque<MapConnector>> connectors;

    //////////////////////////////////////////////////
    // methods used in generation
    bool roomExists(sf::Vector2i, int, int);
    void addConnectorForRegion(int, MapConnector);
    std::pair<bool, MapConnector> newConnectorAt(sf::Vector2i);
    Tile getAtDirection(sf::Vector2i, Direction, int);
    void carveToDirection(sf::Vector2i *, Direction, int, Tile::Type);
    int getRandomInt(int, int);
    std::array<Direction, 4> shuffleDirections();
    bool isConnector(sf::Vector2i);
    bool isNextToDoor(sf::Vector2i);
    bool isDeadEnd(sf::Vector2i);
    std::vector<sf::Vector2i> findDeadEnds(Tile::Type);

    float progress;
};

} // namespace ge
