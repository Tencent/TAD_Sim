/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "Sample_SoloMesh.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <cstdio>
#include <exception>
#include <iostream>
#include <set>

#include "nlohmann/json.hpp"
#include "recastnavigation/Recast.h"

namespace recast_detour {

struct EventSettings {
  struct Event {
    int id;
    int cost;
    std::map<int, std::set<int>> tile_poly_idx;
  };
  std::vector<Event> fds;
};

constexpr const char kNavMeshFileExt[] = "bin";
constexpr const char kGeomSetFileExt[] = "gset";

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BuildSettings, cellSize, cellHeight, agentHeight, agentRadius, agentMaxClimb,
                                   agentMaxSlope);

void ResetCommonSettings(BuildSettings& settings) {
  Sample sample;
  sample.resetCommonSettings();
  sample.collectSettings(settings);
}

void UpdateCommonSettings(const std::string& settings_str, BuildSettings& settings) {
  ResetCommonSettings(settings);

  nlohmann::json settings_json;
  to_json(settings_json, settings);
  settings_json.update(nlohmann::json::parse(settings_str));
  from_json(settings_json, settings);
}

void ParseEventSettings(const std::string& events_str, EventSettings& settings) {
  if (events_str.empty()) {
    return;
  }

  nlohmann::json events_json;
  try {
    events_json = nlohmann::json::parse(events_str);
  } catch (const nlohmann::json::parse_error& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
    return;
  }
  if (events_json.empty()) {
    // std::cerr << "Invalid json: " << events_str << std::endl;
    return;
  }

  for (auto& record : events_json) {
    auto& id = record["id"];
    auto& fds = record["fds"];
    auto& cost = record["cost"];
    if (!(id.is_string() || id.is_number()) || !fds.is_array() || !cost.is_number()) {
      continue;
    }

    EventSettings::Event event;
    for (auto& tile_poly_idx : fds) {
      if (tile_poly_idx.type() != nlohmann::json::value_t::string) {
        continue;
      }
      // format: "tile-poly-?"
      int tile = 0, poly = 0, unused = 0;
      int count = sscanf(tile_poly_idx.get<std::string>().c_str(), "%d-%d-%d", &tile, &poly, &unused);
      if (count != 3) {
        std::cerr << "Invalid event.fds: " << tile_poly_idx << std::endl;
        continue;
      }
      event.tile_poly_idx[tile].insert(poly);
    }

    if (event.tile_poly_idx.empty()) {
      continue;
    }
    event.id = id.is_number() ? id.get<int>() : std::atoi(id.get<std::string>().c_str());
    event.cost = cost.get<int>();
    settings.fds.emplace_back(event);
  }
}

bool BuildNavMesh(const char* obj, const char* settings_str = "{}", const char* events_str = "[]") {
  if (!obj || !settings_str) {
    return false;
  }

  boost::filesystem::path obj_file(obj);
  if (!boost::filesystem::exists(obj_file)) {
    return false;
  }

  BuildContext ctx;

  // init settings
  BuildSettings settings;
  try {
    UpdateCommonSettings(settings_str, settings);
  } catch (const std::exception& ex) {
    ctx.log(RC_LOG_ERROR, "Update common settings error: %s", ex.what());
  }

  // init geom && sample
  InputGeom geom;
  geom.load(&ctx, obj_file.c_str());

  std::shared_ptr<Sample> sample = std::make_shared<Sample_SoloMesh>();
  sample->setContext(&ctx);
  sample->handleMeshChanged(&geom);
  // set settings
  sample->handleCommonSettings(settings);
  if (!sample->handleBuild()) {
    return false;
  }

  // update navmesh by events
  EventSettings events_settings;
  ParseEventSettings(events_str, events_settings);
  const dtNavMesh* mesh = static_cast<const dtNavMesh*>(sample->getNavMesh());
  for (auto& event : events_settings.fds) {
    for (auto& kv : event.tile_poly_idx) {
      dtMeshTile* tile = const_cast<dtMeshTile*>(mesh->getTile(kv.first));
      if (!tile) {
        continue;
      }
      for (auto& poly_idx : kv.second) {
        if (poly_idx < 0 || poly_idx >= tile->header->polyCount) {
          continue;
        }
        tile->polys[poly_idx].setArea(event.id & (DT_MAX_AREAS - 1));
        +poly_idx;
      }
    }
  }

  // save navmesh
  sample->saveAll(obj_file.replace_extension(kNavMeshFileExt).c_str(), sample->getNavMesh());
  // for (int i = 0; i < sample->getNavMesh()->getMaxTiles(); ++i) {
  //   const dtMeshTile* tile = sample->getNavMesh()->getTile(i);
  //   for (int j = 0; j < tile->header->polyCount; j++) {
  //     tile->polys[j].setArea((DT_MAX_AREAS - 1));
  //     ctx.log(RC_LOG_PROGRESS, "tile idx: %d, poly idx: %d, poly vert cnt: %d, poly area id: %d", i, j,
  //     tile->polys[j].vertCount, tile->polys[j].getArea());
  //   }
  // }

  // save settings
  rcVcopy(settings.navMeshBMin, geom.getNavMeshBoundsMin());
  rcVcopy(settings.navMeshBMax, geom.getNavMeshBoundsMax());
  geom.saveGeomSet(&settings);
  FILE* file = fopen(obj_file.replace_extension(kGeomSetFileExt).c_str(), "a");
  if (file) {
    fprintf(file, "e %s\n", events_str);
    fclose(file);
  }

  ctx.dumpLog(">> Build log %s", geom.getMesh()->getFileName().c_str());

  return true;
}

}  // namespace recast_detour
