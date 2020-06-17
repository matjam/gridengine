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

#include <set>

#include "MapGenerator.hpp"
#include "Logging.hpp"

namespace ge
{
MapGenerator::MapGenerator()
{
    progress = 1.f;
}
//////////////// Generation

void MapGenerator::generate(MapGeneratorConfig config)
{
    this->config = config;

    // reset the random number engine to use the current seed.
    rand_engine = std::make_shared<std::default_random_engine>(config.seed);

    // TileMap needs to be configured to be the current size, so we need a new one.
    auto tm = std::make_shared<TileMap>(config.width, config.height);

    tile_map = tm;

    SPDLOG_INFO("Starting map generation. Seed {} instance {:p}", config.seed, (void *)this);

    progress = 0.f;
    generateRooms();
    progress = 0.2f;
    generateHallways();
    progress = 0.4f;
    generateConnectors();
    progress = 0.6f;
    connectRegions();
    progress = 0.8f;
    removeDeadends();
    progress = 1.f;
}

float MapGenerator::generationProgress()
{
    return progress;
}

//////////////// Room Generation

void MapGenerator::generateRooms()
{
    SPDLOG_INFO("Generating rooms", config.seed);

    int room_count = 0;

    for (auto attempts = 0; attempts < config.room_attempts; attempts++) {
        auto room_width  = getRandomInt(config.room_min / 2, config.room_max / 2) * 2 + 1;
        auto room_height = getRandomInt(config.room_min / 2, config.room_max / 2) * 2 + 1;
        auto ratio       = (float)room_width / (float)room_height;

        if (ratio > 1.7f || ratio < 0.3f)
            continue;

        auto room_x = getRandomInt(0, (tile_map->w - room_width) / 2) * 2;
        auto room_y = getRandomInt(0, (tile_map->h - room_height) / 2) * 2;

        if (!roomExists(sf::Vector2i{room_x, room_y}, room_width, room_height)) {
            auto region = tile_map->createRegion(fmt::format("room#{}", room_count));

            for (auto y = room_y; y < room_y + room_height; y++) {
                for (auto x = room_x; x < room_x + room_width; x++) {
                    tile_map->setTile(sf::Vector2i{x, y}, Tile::Type::ROOM, region);
                }
            }
            renderSleep();
            room_count++;
        }
    }

    SPDLOG_INFO("{} rooms generated", room_count);
}

// check to see if a room exists at the given location, respecting a border of 1
// tile all the way around the room
bool MapGenerator::roomExists(sf::Vector2i location, int width, int height)
{
    for (auto x = location.x - 1; x < location.x + width + 1; x += 1) {
        for (auto y = location.y - 1; y < location.y + height + 1; y += 1) {
            auto tile = tile_map->getTile(sf::Vector2i{x, y});
            if (tile.type != Tile::Type::WALL && tile.type != Tile::Type::INVALID) {
                return true;
            }
        }
    }
    return false;
}

//////////////// Hallway Generation

void MapGenerator::generateHallways()
{
    sf::Vector2i location{0, 0};

    bool room_exists = true;
    while (room_exists) {
        location.x = getRandomInt(0, config.width / 2) * 2;
        location.y = getRandomInt(0, config.height / 2) * 2;
        if (tile_map->getTile(location).type != Tile::Type::ROOM)
            room_exists = false;
    }
    startWalking(location);

    auto found_walls = scanForWalls();
    while (found_walls) {
        found_walls = scanForWalls();
    }
}

void MapGenerator::startWalking(sf::Vector2i location)
{
    current_region_id = tile_map->createRegion("hallway");
    tile_map->setTile(location, Tile::Type::HALLWAY, current_region_id);

    bool ok = true;
    while (ok) {
        ok = mazeWalk(&location);
        if (!ok)
            ok = mazeHunt(&location);
    }
}

// This function randomly scans the map for any walls that we missed; if we find a wall that
// was not replaced then we can assume that we need to generate a maze at this point because
// we've already filled as much as we can with other mazes.
bool MapGenerator::scanForWalls()
{
    bool found_walls = false;

    // generate a list of rows that we'll loop over and randomize them.
    std::vector<int> incomplete_rows;
    for (int y = 0; y < config.height; y += 2) {
        incomplete_rows.push_back(y);
    }
    shuffle(incomplete_rows.begin(), incomplete_rows.end(), *rand_engine);

    // Loop over the randomized y co-ordinates and look for tiles.
    for (auto const &y : incomplete_rows) {
        sf::Vector2i location{0, y};
        for (location.x = 0; location.x < config.width; location.x += 2) {
            if (tile_map->getTile(location).type == Tile::Type::WALL) {
                found_walls = true;
                startWalking(location);
            }
        }
    }

    return found_walls;
}

// Given a particular location, start generating a maze from that location.
// This will try each direction, randomly.
//
// TODO: make it so that when we're walking we also maintain direction, so we
// can tune how twisty things are. Right now we only make straight hallways out
// of pure luck.
bool MapGenerator::mazeWalk(sf::Vector2i *location)
{
    auto directions = shuffleDirections();

    for (auto d = 0; d < 4; d++) {
        auto face = directions[d];
        auto tile = getAtDirection(*location, face, 2);
        if (tile.type == Tile::Type::WALL) {
            carveToDirection(location, face, 2, Tile::Type::HALLWAY);
            renderSleep();
            return true;
        }
    }
    // could not find a path to carve out to.
    return false;
}

// This function iterates through every row in the map finds any part of the map where
// is a HALLWAY with a WALL next to it to carve to. If there is then we carve that hallway
// out in random directions to grow that part of the maze.
//
// If this function cannot find any hallways it makes a new hallway region with no tiles
// added to it yet, and returns false. This allows a new random location to be selected to
// start generating a new hallway region. This happens because sometimes rooms  will block
// the ability of the maze to grow to an empty part of the map.
bool MapGenerator::mazeHunt(sf::Vector2i *location)
{
    std::vector<int> incomplete_rows;

    // construct a vector of pointers to each row that will indicate
    // whether this row should be scanned again. We shuffle them and
    // use them in random order.
    for (int y = 0; y < config.height; y += 2) {
        incomplete_rows.push_back(y);
    }
    shuffle(incomplete_rows.begin(), incomplete_rows.end(), *rand_engine);

    while (incomplete_rows.size() > 0) {
        // grab the first row and process it
        auto y = incomplete_rows.back();
        int x;

        bool found_hall = false;

        for (x = 0; x < config.width; x += 2) {
            auto t = tile_map->getTile(sf::Vector2i{x, y});
            if (t.type == Tile::Type::HALLWAY) {
                if (tile_map->getTile(sf::Vector2i{x - 2, y}).type == Tile::Type::WALL) {
                    location->x = x;
                    location->y = y;
                    carveToDirection(location, Direction::WEST, 2, Tile::Type::HALLWAY);
                    return true; // go back to the walk
                }

                if (tile_map->getTile(sf::Vector2i{x + 2, y}).type == Tile::Type::WALL) {
                    location->x = x;
                    location->y = y;
                    carveToDirection(location, Direction::EAST, 2, Tile::Type::HALLWAY);
                    return true; // go back to the walk
                }

                if (tile_map->getTile(sf::Vector2i{x, y - 2}).type == Tile::Type::WALL) {
                    location->x = x;
                    location->y = y;
                    carveToDirection(location, Direction::SOUTH, 2, Tile::Type::HALLWAY);
                    return true; // go back to the walk
                }

                if (tile_map->getTile(sf::Vector2i{x, y + 2}).type == Tile::Type::WALL) {
                    location->x = x;
                    location->y = y;
                    carveToDirection(location, Direction::NORTH, 2, Tile::Type::HALLWAY);
                    return true; // go back to the walk
                }
            }
        }
        // we found nothing usable on this row, remove it from the incomplete_rows vector.
        if (x >= config.width) {
            incomplete_rows.pop_back();
        }
    }

    current_region_id = tile_map->createRegion("hallway");
    return false;
}

//////////////// Connectors methods

// Builds a list of all the possible connectors.
void MapGenerator::generateConnectors()
{
    auto connector_count = 0;
    // select a root region.
    root_region_id = -1;
    while (root_region_id == -1) {
        auto x = getRandomInt(0, config.width / 2) * 2;
        auto y = getRandomInt(0, config.height / 2) * 2;

        auto t = tile_map->getTile(sf::Vector2i{x, y});
        if (t.type == Tile::Type::ROOM) {
            root_region_id = t.region_id;
            SPDLOG_INFO("tile at {},{} in room with region name {} id {} selected as root_region",
                        x,
                        y,
                        tile_map->getRegionName(root_region_id),
                        root_region_id);
        }
    }

    // Generate all the connectors
    for (int y = 0; y < config.height; y++) {
        for (int x = 0; x < config.width; x++) {
            auto tile = tile_map->getTile(sf::Vector2i{x, y});

            // only walls can become connectors
            if (tile.type != Tile::Type::WALL)
                continue;

            // consider the tiles on either side of this tile, and see if they
            // are valid regions.

            auto [success, regions] = newConnectorAt(sf::Vector2i{x, y});
            if (success) {
                connector_count++;

                // This is a connector, set it to be part of one of the regions.
                // It doesn't really matter which; but it's not longer a wall and it
                // shouldn't be part of root as that will mess up the spanning tree.
                tile_map->setTile(sf::Vector2i{x, y}, Tile::Type::CONNECTOR, regions.first_region_id);

                // Now add the connectors to the connectors map so we can find them again quickly.
                addConnectorForRegion(regions.first_region_id, regions);
                addConnectorForRegion(regions.second_region_id, regions);
            }
        }
    }

    SPDLOG_INFO("{} connectors found", connector_count);
}

// We need to keep track of when we merge a region; before that region is deleted we need to
// - find all connectors that connected those two regions and remove them - they are now the same region
// - before we remove them, apply the extra_connector_chance to them - extra connections between regions is ok randomly

void MapGenerator::addConnectorForRegion(int region_id, ge::MapConnector connector)
{
    if (connectors.find(region_id) == connectors.end()) {
        connectors[region_id] = std::deque<MapConnector>(); // make an empty vector first
    }
    connectors[region_id].push_back(connector);
}

// gets the two regions connected by a given point, probably a connector
// or maybe a wall if we are in that phase. We only consider ROOMs and HALLs.
//
// Returns a new MapConnector that you can copy around; it contains weak references
// to the actual regions.
std::pair<bool, ge::MapConnector> MapGenerator::newConnectorAt(sf::Vector2i location)
{
    auto tile = tile_map->getTile(location);

    // only walls can become connectors
    if (tile.type != Tile::Type::WALL)
        return std::make_pair(false, MapConnector{location, -1, -1});

    // check W and E
    auto e = getAtDirection(location, Direction::EAST, 1);
    auto w = getAtDirection(location, Direction::WEST, 1);

    if ((e.type == Tile::Type::HALLWAY && w.type == Tile::Type::ROOM) ||
        (e.type == Tile::Type::ROOM && w.type == Tile::Type::HALLWAY) ||
        (e.type == Tile::Type::ROOM && w.type == Tile::Type::ROOM)) {
        return std::make_pair(true, MapConnector(location, e.region_id, w.region_id));
    }

    // check N and S
    auto n = getAtDirection(location, Direction::NORTH, 1);
    auto s = getAtDirection(location, Direction::SOUTH, 1);

    if ((n.type == Tile::Type::HALLWAY && s.type == Tile::Type::ROOM) ||
        (n.type == Tile::Type::ROOM && s.type == Tile::Type::HALLWAY) ||
        (n.type == Tile::Type::ROOM && s.type == Tile::Type::ROOM)) {
        return std::make_pair(true, MapConnector(location, n.region_id, s.region_id));
    }

    // failed to find a connector here
    return std::make_pair(false, MapConnector{location, -1, -1});
}

// This function scans through all of the regions that we currently have and while we have
// more than one region, it starts building connectors and joining the regions.
//
// We favor connecting things to the root region first, and only start connecting other
// regions when we cannot connect anything to root. This is so that we don't unnecessarily
// churn through regions connecting them uselessly when they could have just been merged
// with root.
void MapGenerator::connectRegions()
{
    std::set<int> merged_regions;
    std::set<int> extra_connections;

    for (auto &[k, v] : connectors) {
        shuffle(v.begin(), v.end(), *rand_engine);
    }

    // as we connect to the root_region, the connectors list for that region will grow.
    // We start off with grabbing one connector from the front, and pop it off so that
    // we don't try to use it again.
    auto current_connector = connectors[root_region_id].front();
    connectors[root_region_id].pop_front();

    // we maintain a couple of shared_ptr to Regions that we're operating on.
    int kept_region_id    = -1; // this is what we will keep when we merge
    int removed_region_id = -1; // this is we will remove when we merge

    // see if the current_connector can connect some unconnected regions
    while (tile_map->regions.size() > 1) {
        bool did_connect = false;

        bool has_already_merged = ((merged_regions.count(current_connector.first_region_id) != 0 ||
                                    merged_regions.count(current_connector.second_region_id) != 0));
        bool has_extra_door     = (extra_connections.count(current_connector.first_region_id) != 0 ||
                               extra_connections.count(current_connector.second_region_id) != 0);
        bool is_root_merge =
            current_connector.first_region_id == root_region_id || current_connector.second_region_id == root_region_id;
        bool will_allow_extra_door = getRandomInt(0, 1000) < 10;
        bool isNotNextToDoor       = !isNextToDoor(current_connector.location);

        // we will merge, if it has not already merged OR it has merged but doesn't have an extra door, and will
        // randomly allow one AND is a root merge AND is not next to a door.
        bool will_merge = (!has_already_merged || (has_already_merged && !has_extra_door && will_allow_extra_door)) &&
                          is_root_merge && isNotNextToDoor;

        if (will_merge && current_connector.first_region_id == root_region_id) {
            // The first region is the root region, so we connect the other region to
            // it. In the clean up phase we will figure out how to move the data.

            tile_map->setTile(current_connector.location, Tile::Type::DOOR, root_region_id);
            did_connect       = true;
            kept_region_id    = root_region_id;
            removed_region_id = current_connector.second_region_id;

        } else if (will_merge && current_connector.second_region_id == root_region_id) {
            // The second region is the root region, so we connect the other region to
            // it. In the clean up phase we will figure out how to move the data.

            tile_map->setTile(current_connector.location, Tile::Type::DOOR, root_region_id);
            did_connect       = true;
            kept_region_id    = root_region_id;
            removed_region_id = current_connector.first_region_id;
        }

        // we connected some regions, so we need to do the post merge clean up
        if (did_connect) {
            renderSleep();

            if (has_already_merged && !has_extra_door && will_allow_extra_door) {
                extra_connections.insert(removed_region_id);
            }

            merged_regions.insert(removed_region_id);

            // Move all the tiles from the removed region to the kept region
            tile_map->updateRegions(removed_region_id, kept_region_id);

            // all of the connectors that belonged to the removed region now belong
            // to the region we kept, so we move them all to that list.
            for (auto &connector : connectors[removed_region_id]) {
                // we need to replace the removed_region_id from the connector and replace
                // it with the kept_region_id. But we only do this for connectors that don't duplicate what we just did
                if (!(connector.first_region_id == removed_region_id && connector.second_region_id == kept_region_id) &&
                    !(connector.first_region_id == kept_region_id && connector.second_region_id == removed_region_id)) {
                    if (connector.first_region_id == removed_region_id) {
                        connector.first_region_id = kept_region_id;
                    } else {
                        connector.second_region_id = kept_region_id;
                    }
                    connectors[kept_region_id].push_back(connector);
                }
            }
            connectors.erase(removed_region_id);
            tile_map->regions.erase(removed_region_id);
        } else if (tile_map->getTile(current_connector.location).type == Tile::Type::CONNECTOR) {
            // if this connector wasn't used to connect anything, lets remove it from the map
            tile_map->setTile(current_connector.location, Tile::Type::WALL, tile_map->wall_region.id);
        }

        // Get another connector, preferentially from the root_connectors list.
        if (connectors[root_region_id].size() > 0) {
            // We have connectors in the root_region still, grab one and remove it from the deque.
            current_connector = connectors[root_region_id].front();
            connectors[root_region_id].erase(connectors[root_region_id].begin());
        } else {
            SPDLOG_INFO("out of root connectors and other connectors.");
            break;
        }
    }
}

//////////////// Remove Dead Ends

bool MapGenerator::isDeadEnd(sf::Vector2i location)
{
    Tile n = getAtDirection(location, Direction::NORTH, 1);
    Tile s = getAtDirection(location, Direction::SOUTH, 1);
    Tile w = getAtDirection(location, Direction::WEST, 1);
    Tile e = getAtDirection(location, Direction::EAST, 1);

    auto total_walls = (n.type == Tile::Type::WALL) + (n.type == Tile::Type::INVALID) + (s.type == Tile::Type::WALL) +
                       (s.type == Tile::Type::INVALID) + (w.type == Tile::Type::WALL) +
                       (w.type == Tile::Type::INVALID) + (e.type == Tile::Type::WALL) + (e.type == Tile::Type::INVALID);

    if (total_walls > 2) {
        return true;
    }

    return false;
}

std::vector<sf::Vector2i> MapGenerator::findDeadEnds(Tile::Type type)
{
    std::vector<sf::Vector2i> dead_ends;

    for (int y = 0; y < config.height; y += 1) {
        for (int x = 0; x < config.width; x += 1) {
            auto location = sf::Vector2i{x, y};
            auto tile     = tile_map->getTile(location);
            if (tile.type == type && isDeadEnd(location)) {
                dead_ends.push_back(location);
            }
        }
    }
    shuffle(dead_ends.begin(), dead_ends.end(), *rand_engine);

    return dead_ends;
}

// scan for any tiles that are dead ends

void MapGenerator::removeDeadends()
{
    SPDLOG_INFO("removing dead ends until there are {}", config.dead_ends);

    int removed    = 0;
    auto dead_ends = findDeadEnds(Tile::Type::HALLWAY);
    int diff       = 0;

    while (dead_ends.size() > 0 && (diff = dead_ends.size() - config.dead_ends) > 0) {
        for (int i = 0; i < diff; i++) {
            auto location = dead_ends.back();
            dead_ends.pop_back();

            tile_map->setTile(location, Tile::Type::WALL, tile_map->wall_region.id);
            renderSleep();
            removed++;
        }

        dead_ends = findDeadEnds(Tile::Type::HALLWAY);
    }

    SPDLOG_INFO("removed {} walls, cleaning up any hanging doors", removed);
    removed = 0;

    dead_ends = findDeadEnds(Tile::Type::DOOR);
    SPDLOG_INFO("found {} dead end doors", dead_ends.size());
    for (auto dead_end : dead_ends) {
        tile_map->setTile(dead_end, Tile::Type::WALL, tile_map->wall_region.id);
        renderSleep();
        removed++;
    }
    SPDLOG_INFO("removed {} doors", removed);
}

//////////////// Utility methods

void MapGenerator::renderSleep()
{
    if (config.sleep_msec > 0)
        sf::sleep(sf::milliseconds(config.sleep_msec));
}

int MapGenerator::getRandomInt(int start, int end)
{
    std::uniform_int_distribution<int> r(start, end);
    return r(*rand_engine);
}

std::array<Direction, 4> MapGenerator::shuffleDirections()
{
    std::array<Direction, 4> directions{
        Direction::NORTH,
        Direction::SOUTH,
        Direction::WEST,
        Direction::EAST,
    };

    shuffle(directions.begin(), directions.end(), *rand_engine);
    return directions;
}

Tile MapGenerator::getAtDirection(sf::Vector2i location, Direction face, int distance)
{
    switch (face) {
    case Direction::NORTH: location.y += distance; break;
    case Direction::EAST: location.x += distance; break;
    case Direction::SOUTH: location.y -= distance; break;
    case Direction::WEST: location.x -= distance; break;
    }

    return tile_map->getTile(location);
}

// carves a path into a given direction by replacing each tile with the new
// tile type and the current_region. It modifies the values passed into it
// for location, with the final values being the final tile that you land on.
void MapGenerator::carveToDirection(sf::Vector2i *location, Direction face, int distance, Tile::Type type)
{
    auto tx = location->x;
    auto ty = location->y;

    switch (face) {
    case Direction::NORTH:
        for (ty = location->y; ty < location->y + distance; ty++) {
            tile_map->setTile(sf::Vector2i{location->x, ty}, type, current_region_id);
        }
        break;
    case Direction::EAST:
        for (tx = location->x; tx < location->x + distance; tx++) {
            tile_map->setTile(sf::Vector2i{tx, location->y}, type, current_region_id);
        }
        break;
    case Direction::SOUTH:
        for (ty = location->y; ty > location->y - distance; ty--) {
            tile_map->setTile(sf::Vector2i{location->x, ty}, type, current_region_id);
        }
        break;
    case Direction::WEST:
        for (tx = location->x; tx > location->x - distance; tx--) {
            tile_map->setTile(sf::Vector2i{tx, location->y}, type, current_region_id);
        }
        break;
    }

    location->x = tx;
    location->y = ty;

    tile_map->setTile(*location, type, current_region_id);
}

// Checks to see whether a given location has a door within a few tiles of it,
// so we can avoid making doors right next to each other. It does this in
// a pretty simplistic way; we just count all the doors in a 2 tile radius and
// if we find any doors return true.
bool MapGenerator::isNextToDoor(sf::Vector2i location)
{
    auto n  = getAtDirection(location, Direction::NORTH, 1);
    auto s  = getAtDirection(location, Direction::SOUTH, 1);
    auto w  = getAtDirection(location, Direction::WEST, 1);
    auto e  = getAtDirection(location, Direction::EAST, 1);
    auto n2 = getAtDirection(location, Direction::NORTH, 2);
    auto s2 = getAtDirection(location, Direction::SOUTH, 2);
    auto w2 = getAtDirection(location, Direction::WEST, 2);
    auto e2 = getAtDirection(location, Direction::EAST, 2);

    auto total_doors = (n.type == Tile::Type::DOOR) + (s.type == Tile::Type::DOOR) + (w.type == Tile::Type::DOOR) +
                       (e.type == Tile::Type::DOOR) + (n2.type == Tile::Type::DOOR) + (s2.type == Tile::Type::DOOR) +
                       (w2.type == Tile::Type::DOOR) + (e2.type == Tile::Type::DOOR);

    if (total_doors > 0) {
        return true;
    }

    return false;
}

/////////////////////////////////////

MapConnector::MapConnector(sf::Vector2i location, int first_region_id, int second_region_id)
{
    this->location         = location;
    this->first_region_id  = first_region_id;
    this->second_region_id = second_region_id;
}

} // namespace ge
