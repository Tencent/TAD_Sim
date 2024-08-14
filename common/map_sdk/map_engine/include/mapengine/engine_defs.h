// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

namespace hadmap {
// Enumerate the types of map data
enum MAP_DATA_TYPE {
  // No data type specified
  NONE = 0,

  // SQLite database
  SQLITE = 1,

  // MySQL database
  MYSQL = 2,

  // Remote database
  REMOTE = 3,

  // In-memory database
  MEMORY = 4,

  // OpenDRIVE format
  OPENDRIVE = 5,

};

}  // namespace hadmap
