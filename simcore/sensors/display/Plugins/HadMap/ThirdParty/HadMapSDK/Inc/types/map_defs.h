// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <stdint.h>

namespace hadmap {

typedef uint64_t pkid_t;
typedef uint64_t taskpkid;
typedef uint64_t roadpkid;
typedef uint64_t junctionpkid;
typedef uint64_t controllerkid;
typedef uint64_t sectionpkid;
typedef int64_t lanepkid;
typedef uint64_t lanelinkpkid;
typedef uint64_t lanedivisionpkid;
typedef uint64_t laneboundarypkid;
typedef uint64_t objectpkid;
typedef uint64_t objectdivisionpkid;
typedef uint64_t objectrelpkid;
typedef uint64_t objectgeompkid;
typedef uint64_t objectgeomrelpkid;

#define PKID_INVALID 0x0
#define TASK_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define ROAD_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define JUNC_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define SECT_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define LANE_PKID_INVALID 0x0
#define LANE_DIV_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define LINK_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define BOUN_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define OBJ_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define OBJ_DIV_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define OBJ_REL_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define OBJ_GEOM_PKID_INVALID 0xFFFFFFFFFFFFFFFF
#define OBJ_GEOM_REL_PKID_INVALID 0xFFFFFFFFFFFFFFFF

}  // namespace hadmap
