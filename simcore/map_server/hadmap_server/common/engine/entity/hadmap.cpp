/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/engine/entity/hadmap.h"

#include <fstream>
#include <iomanip>
#include <iostream>

#include <common/engine/math/utils.h>
#include <mapengine/hadmap_codes.h>
#include "common/log/system_logger.h"
#include "common/map_data/map_object.h"
#include "map_data/lane_boundary_data.h"
#include "map_data/lane_data.h"
#include "map_data/lane_link.h"
#include "map_data/road_data.h"
#include "map_data/section_data.h"

#ifndef __HADMAP_SERVER__

#  include "../config.h"
#  include "../engine.h"
#  include "../renderer/material.h"
#  include "../renderer/material_manager.h"
#  include "../renderer/vao/vao_manager.h"
#  include "../renderer/vao/vertex_array_object.h"

#endif

CHadmap::CHadmap() {
  m_vao_road = nullptr;
  m_vao_lane = nullptr;
  m_vao_lane_boundary = nullptr;

  m_strFileName = "";
  m_strFullPath = "";
  m_llLastModifiedTime = 0;
}

CHadmap::~CHadmap() { Clear(); }

void CHadmap::Reset() {
  std::vector<CRoadData*>::iterator itr = m_roads.begin();

  for (; itr != m_roads.end(); ++itr) {
    delete (*itr);
  }

  m_roads.clear();
}

#ifndef __HADMAP_SERVER__
void CHadmap::Draw() {
  if (!m_material) {
    return;
  }

  // m_material->Apply(true);

  if (CEngineConfig::Instance().DrawRoad()) {
    m_mat_road->Apply(true);
    m_vao_road->Bind();
    m_vao_road->Draw();
    m_vao_road->Unbind();
    m_mat_road->Apply(false);
  }

  if (CEngineConfig::Instance().DrawLane()) {
    m_mat_lane->Apply(true);
    m_vao_lane->Bind();
    m_vao_lane->Draw();
    m_vao_lane->Unbind();
    m_mat_lane->Apply(false);
  }

  if (CEngineConfig::Instance().DrawLaneBoundary()) {
    m_mat_boundary->Apply(true);
    m_vao_lane_boundary->Bind();
    m_vao_lane_boundary->Draw();
    m_vao_lane_boundary->Unbind();
    m_mat_boundary->Apply(false);
  }

  if (CEngineConfig::Instance().DrawLaneLink()) {
    m_mat_link->Apply(true);
    m_vao_lane_link->Bind();
    m_vao_lane_link->Draw();
    m_vao_lane_link->Unbind();
    m_mat_link->Apply(false);
  }

  if (CEngineConfig::Instance().DrawMapObject()) {
    m_mat_object->Apply(true);
    m_vao_map_object->Bind();
    m_vao_map_object->Draw();
    m_vao_map_object->Unbind();
    m_mat_object->Apply(false);
  }

  if (CEngineConfig::Instance().DrawRoadMesh()) {
    m_material->Apply(true);

    VaoVec::iterator vitr = m_vao_lane_meshes.begin();
    for (; vitr != m_vao_lane_meshes.end(); ++vitr) {
      (*vitr)->Bind();
      (*vitr)->Draw();
      (*vitr)->Unbind();
    }

    m_material->Apply(false);
  }

  // if (CEngineConfig::Instance().DrawAabb())
  {
    EntityVec::iterator eIter = m_entity_selected.begin();
    for (; eIter != m_entity_selected.end(); ++eIter) {
      if ((*eIter)->Type() == ENTITY_TYPE_Road) {
        CRoadData* pRoad = static_cast<CRoadData*>(*eIter);
        pRoad->AabbXyz().Draw();
      } else if ((*eIter)->Type() == ENTITY_TYPE_Lane) {
        CLaneData* pLane = static_cast<CLaneData*>(*eIter);
        pLane->AabbXyz().Draw();
      } else if ((*eIter)->Type() == ENTITY_TYPE_Lane_Boundary) {
        CLaneBoundaryData* pData = static_cast<CLaneBoundaryData*>(*eIter);
        pData->AabbXyz().Draw();
      } else if ((*eIter)->Type() == ENTITY_TYPE_Lane_Link) {
        CLaneLink* pData = static_cast<CLaneLink*>(*eIter);
        pData->AabbXyz().Draw();
      } else if ((*eIter)->Type() == ENTITY_TYPE_MapObject) {
        CMapObject* pData = static_cast<CMapObject*>(*eIter);
        pData->AabbXyz().Draw();
      }
    }
  }
}
#endif

void CHadmap::PushRoad(CRoadData* road) {
  if (!road) {
    return;
  }

  m_roads.push_back(road);
}

void CHadmap::PushRoad(RoadVec& roads) {
  RoadVec::iterator itr = roads.begin();
  for (; itr != roads.end(); ++itr) {
    m_roads.push_back((*itr));
  }

  roads.clear();
}

void CHadmap::PushLaneLink(CLaneLink* link) {
  if (!link) {
    return;
  }

  m_links.push_back(link);
}

void CHadmap::PushLaneLink(LaneLinkVec& links) {
  lanelinkpkid uNO = 0;
  LaneLinkVec::iterator itr = links.begin();
  for (; itr != links.end(); ++itr) {
    m_links.push_back((*itr));
    uNO++;
  }

  links.clear();
}

void CHadmap::PushObject(CMapObject* obj) {
  if (!obj) {
    return;
  }

  m_objects.push_back(obj);
}

void CHadmap::PushObject(MapObjectVec& objects) {
  MapObjectVec::iterator itr = objects.begin();
  for (; itr != objects.end(); ++itr) {
    m_objects.push_back((*itr));
  }

  objects.clear();
}

void CHadmap::SetHeaderVersion(std::string version) { m_version = version; }

void CHadmap::PushTrafficLights(LogicLightVec& lights) {
  for (auto itr : lights) {
    m_lights.push_back(itr);
  }
}

void CHadmap::PushJunctionTraffics(std::map<int, std::map<int, int>>& junctions) {
  this->m_junction2roadid = junctions;
}

#ifndef __HADMAP_SERVER__

void CHadmap::PrepareData() {
  PrepareGeomData();
  PrepareMatData();
}

void CHadmap::PrepareGeomData() {
  PrepareRoadData();
  PrepareLaneData();
  PrepareLaneBoundaryData();
  PrepareLaneLinkData();
  PrepareMapObjectData();
}

void CHadmap::PrepareMatData() {
  IMaterialManager* matMgr = Engine::Instance().GetRender()->GetMaterialManager();
  m_material = matMgr->GetDefaultMaterial();
  m_mat_road = matMgr->CreateMaterial();
  m_mat_lane = matMgr->CreateMaterial();
  m_mat_boundary = matMgr->CreateMaterial();
  m_mat_link = matMgr->CreateMaterial();
  m_mat_object = matMgr->CreateMaterial();

  (*m_mat_road) = (*m_material);
  m_mat_road->Diffuse() = Color(1.0f, 0.0f, 0.0f, 1.0f);
  (*m_mat_lane) = (*m_material);
  m_mat_lane->Diffuse() = Color(0.0f, 1.0f, 0.0f, 1.0f);
  (*m_mat_boundary) = (*m_material);
  m_mat_boundary->Diffuse() = Color(0.0f, 0.0f, 1.0f, 1.0f);
  (*m_mat_link) = (*m_material);
  m_mat_link->Diffuse() = Color(1.0f, 0.0f, 1.0f, 1.0f);
  (*m_mat_object) = (*m_material);
  m_mat_object->Diffuse() = Color(1.0f, 1.0f, 0.0f, 1.0f);
}

void CHadmap::PrepareRoadData() {
  IVAOManager* vaomgr = Engine::Instance().GetRender()->GetVaoManager();
  if (!vaomgr) {
    assert(false);
    SYSTEM_LOGGER_ERROR("vaomanager not created!");
    return;
  }

  if (m_vao_road) {
    vaomgr->DestoryVAO(m_vao_road);
    m_vao_road = nullptr;
  }
  // 分配vao
  m_vao_road = vaomgr->CreateVAO();

  // 计算所包含数据点数
  int nTotalPointNum = 0;
  RoadVec::iterator itr = m_roads.begin();
  for (; itr != m_roads.end(); ++itr) {
    int nRoadPointCount = (*itr)->Data().size();
    nTotalPointNum += nRoadPointCount;
  }

  // SYSTEM_LOGGER_DEBUG("===========================Road Data=====================");

  // 申请内存，拷贝数据
  if (nTotalPointNum > 0) {
    float* buffer = new float[3 * nTotalPointNum];

    itr = m_roads.begin();
    int i = 0;
    for (; itr != m_roads.end(); ++itr) {
      PointsVec pts = (*itr)->Data();
      PointsVec::iterator ptIter = pts.begin();
      for (; ptIter != pts.end(); ++ptIter) {
        buffer[i * 3 + 0] = ptIter->X();
        buffer[i * 3 + 1] = ptIter->Y();
        buffer[i * 3 + 2] = ptIter->Z();
        i++;
      }
      /*buffer[i * 6 + 0] = pts[0].X();
      buffer[i * 6 + 1] = pts[0].Y();
      buffer[i * 6 + 2] = pts[0].Z();
      buffer[i * 6 + 3] = pts[1].X();
      buffer[i * 6 + 4] = pts[1].Y();
      buffer[i * 6 + 5] = pts[1].Z();*/

      /*SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, pts[0].X()
              , pts[0].Y(), pts[0].Z(), pts[1].X(), pts[1].Y(), pts[1].Z());*/

      /*SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, buffer[i * 6 + 0]
              , buffer[i * 6 + 1], buffer[i * 6 + 2], buffer[i * 6 + 3]
              , buffer[i * 6 + 4], buffer[i * 6 + 5]);*/
    }

    m_vao_road->SetData(buffer);
    m_vao_road->SetDataSize(3 * nTotalPointNum * sizeof(float));
    m_vao_road->SetDataType(VET_float3);
    m_vao_road->SetDataSemantic(VES_position);
    m_vao_road->SetElementCount(nTotalPointNum);
    m_vao_road->PrepareData();
  }

  RoadVec::iterator rdIter = m_roads.begin();
  for (; rdIter != m_roads.end(); ++rdIter) {
    (*rdIter)->AabbXyz().PrepareDraw();
  }
}

void CHadmap::PrepareLaneData() {
  IVAOManager* vaomgr = Engine::Instance().GetRender()->GetVaoManager();
  if (!vaomgr) {
    assert(false);
    SYSTEM_LOGGER_ERROR("vaomanager not created!");
    return;
  }

  if (m_vao_lane) {
    vaomgr->DestoryVAO(m_vao_lane);
    m_vao_lane = nullptr;
  }
  // 分配vao
  m_vao_lane = vaomgr->CreateVAO();

  // 计算所包含数据点数
  int nTotalPointNum = 0;
  {
    RoadVec::iterator itr = m_roads.begin();
    // road
    for (; itr != m_roads.end(); ++itr) {
      CRoadData::Sections& sec = (*itr)->Secs();
      CRoadData::Sections::iterator secItr = sec.begin();
      // section
      for (; secItr != sec.end(); ++secItr) {
        Lanes& lanes = (*secItr)->Data();
        Lanes::iterator lanItr = lanes.begin();
        // lanes
        for (; lanItr != lanes.end(); ++lanItr) {
          int nLanePointCount = (*itr)->Data().size();
          nTotalPointNum += nLanePointCount;
        }
      }
    }
  }

  // SYSTEM_LOGGER_DEBUG("===========================Lane Data=====================");

  // 申请内存，拷贝数据
  if (nTotalPointNum > 0) {
    float* buffer = new float[3 * nTotalPointNum];

    int i = 0;
    RoadVec::iterator itr = m_roads.begin();
    // road
    for (; itr != m_roads.end(); ++itr) {
      CRoadData::Sections& sec = (*itr)->Secs();
      CRoadData::Sections::iterator secItr = sec.begin();
      // section
      for (; secItr != sec.end(); ++secItr) {
        Lanes& lanes = (*secItr)->Data();
        Lanes::iterator lanItr = lanes.begin();
        // lanes
        for (; lanItr != lanes.end(); ++lanItr) {
          PointsVec pts = (*lanItr)->Data();
          PointsVec::iterator ptIter = pts.begin();
          for (; ptIter != pts.end(); ++ptIter) {
            buffer[i * 3 + 0] = ptIter->X();
            buffer[i * 3 + 1] = ptIter->Y();
            buffer[i * 3 + 2] = ptIter->Z();
            i++;
            /*SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, pts[0].X()
            , pts[0].Y(), pts[0].Z(), pts[1].X(), pts[1].Y(), pts[1].Z());

            SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, buffer[i * 6 + 0]
            , buffer[i * 6 + 1], buffer[i * 6 + 2], buffer[i * 6 + 3]
            , buffer[i * 6 + 4], buffer[i * 6 + 5]);*/
          }
          /*if (pts.size() > 0)
          {
                  buffer[i * 6 + 0] = pts[0].X();
                  buffer[i * 6 + 1] = pts[0].Y();
                  buffer[i * 6 + 2] = pts[0].Z();
                  buffer[i * 6 + 3] = pts[1].X();
                  buffer[i * 6 + 4] = pts[1].Y();
                  buffer[i * 6 + 5] = pts[1].Z();
                  i++;
          }*/
        }
      }
    }

    m_vao_lane->SetData(buffer);
    m_vao_lane->SetDataSize(3 * nTotalPointNum * sizeof(float));
    m_vao_lane->SetDataType(VET_float3);
    m_vao_lane->SetDataSemantic(VES_position);
    m_vao_lane->SetElementCount(nTotalPointNum);
    m_vao_lane->SetDrawType(DET_line);
    m_vao_lane->PrepareData();
  }

  if (m_vao_lane_meshes.size() > 0) {
    VaoVec::iterator vitr = m_vao_lane_meshes.begin();
    for (; vitr != m_vao_lane_meshes.end(); ++vitr) {
      vaomgr->DestoryVAO(*vitr);
    }
    m_vao_lane_meshes.clear();
  }

  int i = 0;
  Lanes::iterator lIter = m_lanes.begin();
  for (; lIter != m_lanes.end(); ++lIter) {
    // SYSTEM_LOGGER_INFO("parpare lane data %d", i);
    // 生成包围盒
    (*lIter)->AabbXyz().PrepareDraw();

    // 生成路面
    (*lIter)->GenerateMesh();
    // 分配vao
    CVertexArrayObject* pVao = vaomgr->CreateVAO();

    pVao->SetData((*lIter)->Mesh().m_pData);
    pVao->SetDataSemantic(VES_position);
    pVao->SetDataType(VET_float3);
    pVao->SetElementCount((*lIter)->Mesh().m_uDataCount);
    pVao->SetDataSize((*lIter)->Mesh().m_uDataSize);
    pVao->SetDrawType(DET_triangle);
    pVao->PrepareData();
    m_vao_lane_meshes.push_back(pVao);

    i++;
  }
}

void CHadmap::PrepareLaneBoundaryData() {
  IVAOManager* vaomgr = Engine::Instance().GetRender()->GetVaoManager();
  if (!vaomgr) {
    assert(false);
    SYSTEM_LOGGER_ERROR("vaomanager not created!");
    return;
  }

  if (m_vao_lane_boundary) {
    vaomgr->DestoryVAO(m_vao_lane_boundary);
    m_vao_lane_boundary = nullptr;
  }
  // 分配vao
  m_vao_lane_boundary = vaomgr->CreateVAO();

  // 计算所包含数据点数
  int nTotalPointNum = 0;
  {
    RoadVec::iterator itr = m_roads.begin();
    // road
    for (; itr != m_roads.end(); ++itr) {
      CRoadData::Sections& sec = (*itr)->Secs();
      CRoadData::Sections::iterator secItr = sec.begin();
      // section
      for (; secItr != sec.end(); ++secItr) {
        Lanes& lanes = (*secItr)->Data();
        Lanes::iterator lanItr = lanes.begin();
        // lanes
        for (; lanItr != lanes.end(); ++lanItr) {
          CLaneBoundaryData** pBoundary = (*lanItr)->Boundarys();
          // lane boundary
          for (int i = 0; i < 2; ++i) {
            if (pBoundary + i) {
              int nLaneBoundaryPointCount = pBoundary[i]->Data().size();
              nTotalPointNum += nLaneBoundaryPointCount;
            }
          }
        }
      }
    }
  }

  // SYSTEM_LOGGER_DEBUG("===========================Lane Boundary Data=====================");
  // 申请内存，拷贝数据
  if (nTotalPointNum > 0) {
    float* buffer = new float[3 * nTotalPointNum];

    int i = 0;
    RoadVec::iterator itr = m_roads.begin();
    // road
    for (; itr != m_roads.end(); ++itr) {
      CRoadData::Sections& sec = (*itr)->Secs();
      CRoadData::Sections::iterator secItr = sec.begin();
      // section
      for (; secItr != sec.end(); ++secItr) {
        LaneVec& lanes = (*secItr)->Data();
        LaneVec::iterator lanItr = lanes.begin();
        // lanes
        for (; lanItr != lanes.end(); ++lanItr) {
          CLaneBoundaryData** pBoundary = (*lanItr)->Boundarys();
          // lane boundary
          for (int j = 0; j < 2; ++j) {
            if (pBoundary + j) {
              PointsVec pts = pBoundary[j]->Data();
              PointsVec::iterator ptIter = pts.begin();
              for (; ptIter != pts.end(); ++ptIter) {
                buffer[i * 3 + 0] = ptIter->X();
                buffer[i * 3 + 1] = ptIter->Y();
                buffer[i * 3 + 2] = ptIter->Z();

                /*SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, pts[0].X()
                , pts[0].Y(), pts[0].Z(), pts[1].X(), pts[1].Y(), pts[1].Z());*/

                /*SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, buffer[i * 6 + 0]
                , buffer[i * 6 + 1], buffer[i * 6 + 2], buffer[i * 6 + 3]
                , buffer[i * 6 + 4], buffer[i * 6 + 5]);*/

                i++;
              }

              /*if (pts.size() > 0)
              {
                      buffer[i * 6 + 0] = pts[0].X();
                      buffer[i * 6 + 1] = pts[0].Y();
                      buffer[i * 6 + 2] = pts[0].Z();
                      buffer[i * 6 + 3] = pts[1].X();
                      buffer[i * 6 + 4] = pts[1].Y();
                      buffer[i * 6 + 5] = pts[1].Z();
                      i++;
              }*/
            }
          }
        }
      }
    }

    m_vao_lane_boundary->SetData(buffer);
    m_vao_lane_boundary->SetDataSize(3 * nTotalPointNum * sizeof(float));
    m_vao_lane_boundary->SetDataType(VET_float3);
    m_vao_lane_boundary->SetDataSemantic(VES_position);
    m_vao_lane_boundary->SetElementCount(nTotalPointNum);
    m_vao_lane_boundary->PrepareData();
  }

  LaneBoundaryVec::iterator lbIter = m_laneboundaries.begin();
  for (; lbIter != m_laneboundaries.end(); ++lbIter) {
    (*lbIter)->AabbXyz().PrepareDraw();
  }
}

void CHadmap::PrepareLaneLinkData() {
  IVAOManager* vaomgr = Engine::Instance().GetRender()->GetVaoManager();
  if (!vaomgr) {
    assert(false);
    SYSTEM_LOGGER_ERROR("vaomanager not created!");
    return;
  }

  if (m_vao_lane_link) {
    vaomgr->DestoryVAO(m_vao_lane_link);
    m_vao_lane_link = nullptr;
  }
  // 分配vao
  m_vao_lane_link = vaomgr->CreateVAO();

  // 计算所包含数据点数
  int nTotalPointNum = 0;
  LaneLinkVec::iterator itr = m_links.begin();
  for (; itr != m_links.end(); ++itr) {
    int nRoadPointCount = (*itr)->Data().size();
    nTotalPointNum += nRoadPointCount;
  }

  // SYSTEM_LOGGER_DEBUG("===========================Road Data=====================");

  // 申请内存，拷贝数据
  if (nTotalPointNum > 0) {
    float* buffer = new float[3 * nTotalPointNum];

    itr = m_links.begin();
    int i = 0;
    for (; itr != m_links.end(); ++itr) {
      PointsVec pts = (*itr)->Data();
      PointsVec::iterator ptIter = pts.begin();
      for (; ptIter != pts.end(); ++ptIter) {
        buffer[i * 3 + 0] = ptIter->X();
        buffer[i * 3 + 1] = ptIter->Y();
        buffer[i * 3 + 2] = ptIter->Z();
        i++;
      }

      /*SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, pts[0].X()
      , pts[0].Y(), pts[0].Z(), pts[1].X(), pts[1].Y(), pts[1].Z());*/

      /*SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, buffer[i * 6 + 0]
      , buffer[i * 6 + 1], buffer[i * 6 + 2], buffer[i * 6 + 3]
      , buffer[i * 6 + 4], buffer[i * 6 + 5]);*/
    }

    m_vao_lane_link->SetData(buffer);
    m_vao_lane_link->SetDataSize(3 * nTotalPointNum * sizeof(float));
    m_vao_lane_link->SetDataType(VET_float3);
    m_vao_lane_link->SetDataSemantic(VES_position);
    m_vao_lane_link->SetElementCount(nTotalPointNum);
    m_vao_lane_link->PrepareData();
  }

  LaneLinkVec::iterator llIter = m_links.begin();
  for (; llIter != m_links.end(); ++llIter) {
    (*llIter)->AabbXyz().PrepareDraw();
  }
}

void CHadmap::PrepareMapObjectData() {
  IVAOManager* vaomgr = Engine::Instance().GetRender()->GetVaoManager();
  if (!vaomgr) {
    assert(false);
    SYSTEM_LOGGER_ERROR("vaomanager not created!");
    return;
  }

  if (m_vao_map_object) {
    vaomgr->DestoryVAO(m_vao_map_object);
    m_vao_map_object = nullptr;
  }
  // 分配vao
  m_vao_map_object = vaomgr->CreateVAO();

  // 计算所包含数据点数
  int nTotalPointNum = 0;
  MapObjectVec::iterator itr = m_objects.begin();
  for (; itr != m_objects.end(); ++itr) {
    int nRoadPointCount = (*itr)->Data().size();
    nTotalPointNum += nRoadPointCount;
  }

  // SYSTEM_LOGGER_DEBUG("===========================Road Data=====================");

  // 申请内存，拷贝数据
  if (nTotalPointNum > 0) {
    float* buffer = new float[3 * nTotalPointNum];

    itr = m_objects.begin();
    int i = 0;
    for (; itr != m_objects.end(); ++itr) {
      PointsVec pts = (*itr)->Data();
      PointsVec::iterator ptIter = pts.begin();
      for (; ptIter != pts.end(); ++ptIter) {
        buffer[i * 3 + 0] = ptIter->X();
        buffer[i * 3 + 1] = ptIter->Y();
        buffer[i * 3 + 2] = ptIter->Z();
        i++;
      }

      /*SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, pts[0].X()
      , pts[0].Y(), pts[0].Z(), pts[1].X(), pts[1].Y(), pts[1].Z());*/

      /*SYSTEM_LOGGER_DEBUG("%4d  (%f, %f, %f), (%f, %f, %f)", i, buffer[i * 6 + 0]
      , buffer[i * 6 + 1], buffer[i * 6 + 2], buffer[i * 6 + 3]
      , buffer[i * 6 + 4], buffer[i * 6 + 5]);*/
    }

    m_vao_map_object->SetData(buffer);
    m_vao_map_object->SetDataSize(3 * nTotalPointNum * sizeof(float));
    m_vao_map_object->SetDataType(VET_float3);
    m_vao_map_object->SetDataSemantic(VES_position);
    m_vao_map_object->SetElementCount(nTotalPointNum);
    m_vao_map_object->PrepareData();
  }

  MapObjectVec::iterator moIter = m_objects.begin();
  for (; moIter != m_objects.end(); ++moIter) {
    (*moIter)->AabbXyz().PrepareDraw();
  }
}
#endif

void CHadmap::RearrangeData() {
  // roads
  int nRoadTotalPointNum = 0;
  int nLaneTotalPointNum = 0;
  int nLaneBoundaryTotalPointNum = 0;
  RoadVec::iterator itr = m_roads.begin();
  for (; itr != m_roads.end(); ++itr) {
    // 统计road顶点数
    size_t nRoadPointCount = (*itr)->Data().size();
    nRoadTotalPointNum += nRoadPointCount;
    LaneID id;
    id.m_roadID = (*itr)->Id();
    m_roadmap.insert(std::make_pair(id, (*itr)));

    CRoadData::Sections& sec = (*itr)->Secs();
    CRoadData::Sections::iterator secItr = sec.begin();
    // section
    for (; secItr != sec.end(); ++secItr) {
      m_sections.push_back((*secItr));

      assert(id.m_roadID == (*secItr)->RoadId());
      id.m_sectionID = (*secItr)->Id();
      id.m_laneID = -1;
      m_sectionmap.insert(std::make_pair(id, (*secItr)));

      ::Lanes& lanes = (*secItr)->Data();
      ::Lanes::iterator lanItr = lanes.begin();
      // lanes
      for (; lanItr != lanes.end(); ++lanItr) {
        m_lanes.push_back(*lanItr);

        assert(id.m_roadID == (*lanItr)->RoadId());
        assert(id.m_sectionID == (*lanItr)->SectionId());
        id.m_laneID = (*lanItr)->LaneId();
        m_lanemap.insert(std::make_pair(id, (*lanItr)));

        // 统计lane顶点数
        size_t nLanePointCount = (*itr)->Data().size();
        nLaneTotalPointNum += nLanePointCount;

        CLaneBoundaryData** pBoundary = (*lanItr)->Boundarys();
        // lane boundary
        for (int i = 0; i < 2; ++i) {
          if (pBoundary + i) {
            laneboundarypkid boundaryId = pBoundary[i]->Id();
            LaneBoundaryMap::iterator boundaryItr = m_laneboundarymap.find(boundaryId);
            if (boundaryItr != m_laneboundarymap.end()) {
              delete pBoundary[i];
              pBoundary[i] = boundaryItr->second;
            } else {
              m_laneboundaries.push_back(pBoundary[i]);
              m_laneboundarymap.insert(std::make_pair(boundaryId, pBoundary[i]));

              // 统计 lane boundary 顶点数
              size_t nLaneBoundaryPointCount = pBoundary[i]->Data().size();
              nLaneBoundaryTotalPointNum += nLaneBoundaryPointCount;
            }
          }
        }
      }
    }
  }

  // lane links
  int nLaneLinkTotalPointNum = 0;
  LaneLinkVec::iterator linkItr = m_links.begin();
  for (; linkItr != m_links.end(); ++linkItr) {
    // id map
    m_lanelinkmap.insert(std::make_pair((*linkItr)->Id(), (*linkItr)));

    // from map
    LaneID fromId = (*linkItr)->FromId();
    LaneLinkMap::iterator linkMapItr = m_fromlanemap.find(fromId);
    if (linkMapItr == m_fromlanemap.end()) {
      std::set<CLaneLink*> laneLinkSet;
      laneLinkSet.insert(*linkItr);
      m_fromlanemap.insert(std::make_pair(fromId, laneLinkSet));
    } else {
      linkMapItr->second.insert((*linkItr));
    }

    // to map
    LaneID toId = (*linkItr)->ToId();
    linkMapItr = m_tolanemap.find(toId);
    if (linkMapItr == m_tolanemap.end()) {
      std::set<CLaneLink*> laneLinkSet;
      laneLinkSet.insert(*linkItr);
      m_tolanemap.insert(std::make_pair(toId, laneLinkSet));
    } else {
      linkMapItr->second.insert((*linkItr));
    }

    size_t nRoadPointCount = (*linkItr)->Data().size();
    nLaneLinkTotalPointNum += nRoadPointCount;
  }

  // map objects
  int nMapObjectTotalPointNum = 0;
  MapObjectVec::iterator objItr = m_objects.begin();
  for (; objItr != m_objects.end(); ++objItr) {
    m_objectmap.insert(std::make_pair((*objItr)->Id(), (*objItr)));

    size_t nRoadPointCount = (*objItr)->Data().size();
    nMapObjectTotalPointNum += nRoadPointCount;
  }
}

LogicLightVec& CHadmap::GetTrafficLights() { return m_lights; }

std::map<int, std::map<int, int>>& CHadmap::GetJunction2TrafficRoadMap() { return this->m_junction2roadid; }

void CHadmap::Intersect(Ray& ray) {
  // m_lanes_selected.clear();
  m_entity_selected.clear();

#ifndef __HADMAP_SERVER__

  if (CEngineConfig::Instance().DrawRoad()) {
    RoadVec::iterator rdIter = m_roads.begin();
    for (; rdIter != m_roads.end(); ++rdIter) {
      if (ray.Intersect((*rdIter)->AabbXyz())) {
        m_entity_selected.push_back((*rdIter));
      }
    }
  }

  if (CEngineConfig::Instance().DrawLane()) {
    LaneVec::iterator lIter = m_lanes.begin();
    for (; lIter != m_lanes.end(); ++lIter) {
      if (ray.Intersect((*lIter)->AabbXyz())) {
        m_entity_selected.push_back((*lIter));
      }
    }
  }

  if (CEngineConfig::Instance().DrawLaneBoundary()) {
    LaneBoundaryVec::iterator lbIter = m_laneboundaries.begin();
    for (; lbIter != m_laneboundaries.end(); ++lbIter) {
      if (ray.Intersect((*lbIter)->AabbXyz())) {
        m_entity_selected.push_back((*lbIter));
      }
    }
  }

  if (CEngineConfig::Instance().DrawLaneLink()) {
    LaneLinkVec::iterator llIter = m_links.begin();
    for (; llIter != m_links.end(); ++llIter) {
      if (ray.Intersect((*llIter)->AabbXyz())) {
        m_entity_selected.push_back((*llIter));
      }
    }
  }

  if (CEngineConfig::Instance().DrawMapObject()) {
    MapObjectVec::iterator moIter = m_objects.begin();
    for (; moIter != m_objects.end(); ++moIter) {
      if (ray.Intersect((*moIter)->AabbXyz())) {
        m_entity_selected.push_back((*moIter));
      }
    }
  }
#endif
}

void CHadmap::SetFullPath(std::string strName) {
  if (!m_strFullPath.empty()) {
    m_map_query.Deinit();
  }

  m_strFullPath = strName;

  m_map_query.SetMapPath(m_strFullPath.c_str());
}

void CHadmap::SetRefPoint(CPoint3D& point) {
  m_ref_point_ll = point;
  m_map_query.SetRefPoint(point.X(), point.Y(), point.Z());
}

int CHadmap::QueryForwardPoints(sTagGetForwardPointsParam& param, std::vector<std::vector<hadmap::txLaneId>>* ids,
                                std::vector<std::pair<double, hadmap::PointVec>>* points) {
  if (m_map_query.Initialized()) {
    int nRet = m_map_query.QueryForwardPoints(param, ids, points);
    return nRet;
  }

  return -1;
}
int CHadmap::GetRoadByPointLL(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID) {
  uRoadID = -1;
  uSecID = -1;
  lanepkid laneID = 100;

  if (m_map_query.Initialized()) {
    int nRet = m_map_query.QuerySection(dLon, dLat, uRoadID, uSecID, laneID);
    return nRet;
  }

  return -1;
}

int CHadmap::GetLaneByPointLL(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& ulaneID,
                              double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth,
                              double& dOffset, double& dYaw, double searchDistance) {
  uRoadID = -1;
  uSecID = -1;
  ulaneID = -1;

  if (m_map_query.Initialized()) {
    int nRet = m_map_query.QuerySection(dLon, dLat, uRoadID, uSecID, ulaneID, dLaneLon, dLaneLat, dLaneAlt, dDist,
                                        dWidth, dOffset, dYaw, searchDistance);

    return nRet;
  }

  return -1;
}

int CHadmap::GetLaneByPointLL(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& ulaneID,
                              double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth,
                              double& dOffset, std::string& rname) {
  double dYaw = 0;
  int ret = GetLaneByPointLL(dLon, dLat, uRoadID, uSecID, ulaneID, dLaneLon, dLaneLat, dLaneAlt, dDist, dWidth, dOffset,
                             dYaw);
  if (m_map_query.Initialized()) {
    hadmap::txRoadPtr roadPtr = nullptr;
    if (TX_HADMAP_DATA_OK == hadmap::getRoad(m_map_query.GettxMapHandle(), uRoadID, true, roadPtr) && roadPtr) {
      rname = roadPtr->getName();
    }
  }
  return ret;
}

int CHadmap::GetLanelinkByPointLL(double dLon, double dLat, lanelinkpkid& lanelinkID, double& dLanelinkLon,
                                  double& dLanelinkLat, double& dLanelinkAlt, double& dDist, double& dOffset,
                                  double& dYaw, double searchDistance) {
  if (m_map_query.Initialized()) {
    int nRet = m_map_query.QueryLaneLink(dLon, dLat, lanelinkID, dLanelinkLon, dLanelinkLat, dLanelinkAlt, dDist,
                                         dOffset, dYaw, searchDistance);
    return nRet;
  }

  return -1;
}

int CHadmap::LocalToLonlat(double& dLon, double& dLat, double& dAlt) {
  if (m_map_query.Initialized()) {
    m_map_query.LocalToLonlat(dLon, dLat, dAlt);
    return 0;
  }

  return -1;
}

int CHadmap::LonlatToLocal(double& dLon, double& dLat, double& dAlt) {
  if (m_map_query.Initialized()) {
    m_map_query.LonlatToLocal(dLon, dLat, dAlt);
    return 0;
  }

  return -1;
}

int CHadmap::QueryNearbyInfo(double dLon, double dLat, sTagQueryNearbyInfoResult& result) {
  result.m_uRoadID = -1;
  result.m_uSecID = -1;
  result.m_laneID = 100;
  result.m_lanelinkID = -1;

  if (m_map_query.Initialized()) {
    int nRet = m_map_query.QuerySection(dLon, dLat, result.m_uRoadID, result.m_uSecID, result.m_laneID);

    if (nRet == -1) {
      nRet = m_map_query.QueryLaneLink(dLon, dLat, result.m_lanelinkID);
      if (nRet == 0) {
        result.m_elemType = "lanelink";
        return 0;
      }

    } else {
      result.m_elemType = "lane";
      return 0;
    }
  }

  return -1;
}

int CHadmap::QueryNearbyLane(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID) {
  uRoadID = -1;
  uSecID = -1;
  laneID = 100;

  if (m_map_query.Initialized()) {
    int nRet = m_map_query.QuerySection(dLon, dLat, uRoadID, uSecID, laneID);
    return nRet;
  }

  return -1;
}

void CHadmap::ProcessMapData() {
  double dX = 0;
  double dY = 0;
  double dZ = 0;
  for (auto itr : m_objects) {
    if (itr->Type() == HADMAP_OBJECT_TYPE_TrafficSign || itr->Type() == HADMAP_OBJECT_TYPE_TrafficLights) {
      double dHeight = 0;
      PointsVec& pts = itr->Data();
      if (pts.size() > 0) {
        dX = pts[0].X();
        dY = pts[0].Y();
        dZ = pts[0].Z();
        m_map_query.LocalToLonlat(dX, dY, dZ);
        m_map_query.GetLaneCenterHeight(dX, dY, dZ, dHeight);
        m_map_query.LonlatToLocal(dX, dY, dHeight);
        itr->SetGroundHeight(dHeight);
        SYSTEM_LOGGER_INFO("Set obj type:%s Height:%f, mapname:%s", itr->StrType().c_str(), (float)dHeight,
                           this->Name().c_str());
      }
    }
  }
}

int CHadmap::QueryNextLane(roadpkid uRoadID, sectionpkid uSecID, lanepkid laneID, roadpkid& nextRoadID,
                           sectionpkid& nextSecID, lanepkid& nextLaneID) {
  nextRoadID = -1;
  nextRoadID = -1;
  nextLaneID = 100;

  if (m_map_query.Initialized()) {
    int nRet = m_map_query.QueryNextLane(uRoadID, uSecID, laneID, nextRoadID, nextSecID, nextLaneID);
    return nRet;
  }

  return -1;
}

int CHadmap::QueryNextLanes(const roadpkid& uRoadID, const sectionpkid& uSecID, const lanepkid& laneID,
                            LaneIDVec& outLaneVec) {
  if (m_map_query.Initialized()) {
    return m_map_query.QueryNextLanes(uRoadID, uSecID, laneID, outLaneVec);
  }
  return -1;
}

int CHadmap::QueryLonLat(roadpkid uRoadID, sectionpkid usecID, lanepkid laneID, double dShift, double dOffset,
                         double& dLon, double& dLat, double& dAlt, double& dWidth) {
  dLon = -999;
  dLat = -999;
  dAlt = -999;
  dWidth = -999;

  if (m_map_query.Initialized()) {
    double yaw = 0;
    int nRet = m_map_query.QueryLonLat(uRoadID, usecID, laneID, dShift, dOffset, dLon, dLat, dAlt, dWidth, yaw);
    return nRet;
  }

  return -1;
}

int CHadmap::QueryLonLatByPoint(double dLon, double dLat, lanepkid laneID, double dShift, double dOffset,
                                double& dFinalLon, double& dFinalLat, double& dFinalAlt, double& dWidth,
                                double& dFinalShift, bool isStart) {
  dFinalLon = -999;
  dFinalLat = -999;
  dFinalAlt = -999;
  dWidth = -999;

  if (m_map_query.Initialized()) {
    double dFinalYaw = 0;
    int nRet = m_map_query.QueryLonLatByPoint(dLon, dLat, laneID, dShift, dOffset, dFinalLon, dFinalLat, dFinalAlt,
                                              dWidth, dFinalShift, dFinalYaw, isStart);
    return nRet;
  }

  return -1;
}

int CHadmap::QueryLonLatByPointOnLanelink(double dLon, double dLat, lanelinkpkid lanelinkID, double dShift,
                                          double dOffset, double& dFinalLon, double& dFinalLat, double& dFinalAlt,
                                          double& dFinalShift) {
  dFinalLon = -999;
  dFinalLat = -999;
  dFinalAlt = -999;

  if (m_map_query.Initialized()) {
    int nRet = m_map_query.QueryLonLatByPointOnLanelink(dLon, dLat, lanelinkID, dShift, dOffset, dFinalLon, dFinalLat,
                                                        dFinalAlt, dFinalShift);
    return nRet;
  }

  return -1;
}

int CHadmap::getLaneLink(const double lon, const double lat, int& roadid, std::vector<linkData>& linkdatas) {
  if (m_map_query.Initialized()) {
    int nRet = m_map_query.QueryLaneLinkDataFromLonLat(lon, lat, roadid, linkdatas);
    return nRet;
  }
  return -1;
}

hadmap::txMapHandle* CHadmap::getHandle() { return m_map_query.GettxMapHandle(); }

std::string& CHadmap::Version() { return this->m_version; }

CRoadData* CHadmap::FindRoad(roadpkid id) {
  LaneID roadid;
  roadid.m_roadID = id;
  RoadMap::iterator itr = m_roadmap.find(roadid);

  if (itr != m_roadmap.end()) {
    return itr->second;
  }

  return NULL;
}

CSectionData* CHadmap::FindSection(roadpkid roadId, sectionpkid secId) {
  LaneID secid;
  secid.m_roadID = roadId;
  secid.m_sectionID = secId;
  SectionMap::iterator itr = m_sectionmap.find(secid);

  if (itr != m_sectionmap.end()) {
    return itr->second;
  }

  return NULL;
}

CLaneData* CHadmap::FindLane(roadpkid roadId, sectionpkid secId, lanepkid LaneId) {
  LaneID laneId;
  laneId.m_roadID = roadId;
  laneId.m_sectionID = secId;
  laneId.m_laneID = LaneId;

  LaneMap::iterator itr = m_lanemap.find(laneId);

  if (itr != m_lanemap.end()) {
    return itr->second;
  }

  return NULL;
}

CLaneBoundaryData* CHadmap::FindLaneBoundary(laneboundarypkid id) {
  LaneBoundaryMap::iterator itr = m_laneboundarymap.find(id);

  if (itr != m_laneboundarymap.end()) {
    return itr->second;
  }

  return NULL;
}

CLaneLink* CHadmap::FindLaneLink(lanelinkpkid id) {
  LaneLinkIDMap::iterator itr = m_lanelinkmap.find(id);

  if (itr != m_lanelinkmap.end()) {
    return itr->second;
  }

  return NULL;
}

CMapObject* CHadmap::FindMapObject(objectpkid id) {
  MapObjectMap::iterator itr = m_objectmap.find(id);

  if (itr != m_objectmap.end()) {
    return itr->second;
  }

  return NULL;
}

void CHadmap::SetSelected(EntityVec& entitys) {
  m_entity_selected.clear();
  m_entity_selected = entitys;
}
#ifndef __HADMAP_SERVER__
void CHadmap::Clear() {
  m_strFileName.clear();
  m_aabb_ll.Reset();
  m_aabb_xyz.Reset();
  m_center.Reset();

  if (m_map_query.Initialized()) {
    m_map_query.Deinit();
  }

  IMaterialManager* matMgr = Engine::Instance().GetRender()->GetMaterialManager();
  IVAOManager* vaomgr = Engine::Instance().GetRender()->GetVaoManager();

  RoadVec::iterator itr = m_roads.begin();
  for (; itr != m_roads.end(); ++itr) {
    delete (*itr);
  }
  m_roads.clear();
  m_roadmap.clear();
  if (m_mat_road) {
    matMgr->ReleaseMaterial(m_mat_road);
    m_mat_road = NULL;
  }
  if (m_vao_road) {
    vaomgr->DestoryVAO(m_vao_road);
    m_vao_road = NULL;
  }

  SectionVec::iterator sitr = m_sections.begin();
  for (; sitr != m_sections.end(); ++sitr) {
    delete (*sitr);
  }
  m_sections.clear();
  m_sectionmap.clear();
  /*if (m_mat_road)
  {
          matMgr->ReleaseMaterial(m_mat_road);
          m_mat_road = NULL;
  }*/

  LaneVec::iterator litr = m_lanes.begin();
  for (; litr != m_lanes.end(); ++litr) {
    delete (*litr);
  }
  m_lanes.clear();
  m_lanemap.clear();
  if (m_mat_lane) {
    matMgr->ReleaseMaterial(m_mat_lane);
    m_mat_lane = NULL;
  }
  if (m_vao_lane) {
    vaomgr->DestoryVAO(m_vao_lane);
    m_vao_lane = NULL;
  }

  LaneBoundaryVec::iterator lbitr = m_laneboundaries.begin();
  for (; lbitr != m_laneboundaries.end(); ++lbitr) {
    delete (*lbitr);
  }
  m_laneboundaries.clear();
  m_laneboundarymap.clear();
  if (m_mat_boundary) {
    matMgr->ReleaseMaterial(m_mat_boundary);
    m_mat_boundary = NULL;
  }
  if (m_vao_lane_boundary) {
    vaomgr->DestoryVAO(m_vao_lane_boundary);
    m_vao_lane_boundary = NULL;
  }

  LaneLinkVec::iterator llitr = m_links.begin();
  for (; llitr != m_links.end(); ++llitr) {
    delete (*llitr);
  }
  m_links.clear();
  m_fromlanemap.clear();
  m_tolanemap.clear();
  if (m_mat_link) {
    matMgr->ReleaseMaterial(m_mat_link);
    m_mat_link = NULL;
  }
  if (m_vao_lane_link) {
    vaomgr->DestoryVAO(m_vao_lane_link);
    m_vao_lane_link = NULL;
  }

  MapObjectVec::iterator moitr = m_objects.begin();
  for (; moitr != m_objects.end(); ++moitr) {
    delete (*moitr);
  }
  m_objects.clear();
  m_objectmap.clear();
  if (m_mat_object) {
    matMgr->ReleaseMaterial(m_mat_object);
    m_mat_object = NULL;
  }
  if (m_vao_map_object) {
    vaomgr->DestoryVAO(m_vao_map_object);
    m_vao_map_object = NULL;
  }

  if (m_vao_map_object) {
    vaomgr->DestoryVAO(m_vao_map_object);
    m_vao_map_object = NULL;
  }

  VaoVec::iterator vaoiter = m_vao_lane_meshes.begin();
  for (; vaoiter != m_vao_lane_meshes.end(); ++vaoiter) {
    delete (*vaoiter);
  }
  m_vao_lane_meshes.clear();
}
#else
void CHadmap::Clear() {
  m_strFileName.clear();
  m_aabb_ll.Reset();
  m_aabb_xyz.Reset();
  m_center.Reset();

  if (m_map_query.Initialized()) {
    m_map_query.Deinit();
  }

  RoadVec::iterator itr = m_roads.begin();
  for (; itr != m_roads.end(); ++itr) {
    delete (*itr);
  }
  m_roads.clear();
  m_roadmap.clear();
  if (m_mat_road) {
    m_mat_road = NULL;
  }
  if (m_vao_road) {
    m_vao_road = NULL;
  }

  SectionVec::iterator sitr = m_sections.begin();
  for (; sitr != m_sections.end(); ++sitr) {
    delete (*sitr);
  }
  m_sections.clear();
  m_sectionmap.clear();

  LaneVec::iterator litr = m_lanes.begin();
  for (; litr != m_lanes.end(); ++litr) {
    delete (*litr);
  }
  m_lanes.clear();
  m_lanemap.clear();
  if (m_mat_lane) {
    m_mat_lane = NULL;
  }
  if (m_vao_lane) {
    m_vao_lane = NULL;
  }

  LaneBoundaryVec::iterator lbitr = m_laneboundaries.begin();
  for (; lbitr != m_laneboundaries.end(); ++lbitr) {
    delete (*lbitr);
  }
  m_laneboundaries.clear();
  m_laneboundarymap.clear();
  if (m_mat_boundary) {
    m_mat_boundary = NULL;
  }
  if (m_vao_lane_boundary) {
    m_vao_lane_boundary = NULL;
  }

  LaneLinkVec::iterator llitr = m_links.begin();
  for (; llitr != m_links.end(); ++llitr) {
    delete (*llitr);
  }
  m_links.clear();
  m_fromlanemap.clear();
  m_tolanemap.clear();
  if (m_mat_link) {
    m_mat_link = NULL;
  }
  if (m_vao_lane_link) {
    m_vao_lane_link = NULL;
  }

  MapObjectVec::iterator moitr = m_objects.begin();
  for (; moitr != m_objects.end(); ++moitr) {
    delete (*moitr);
  }
  m_objects.clear();
  m_objectmap.clear();
  if (m_mat_object) {
    m_mat_object = NULL;
  }
  if (m_vao_map_object) {
    m_vao_map_object = NULL;
  }

  if (m_vao_map_object) {
    m_vao_map_object = NULL;
  }

  VaoVec::iterator vaoiter = m_vao_lane_meshes.begin();
  for (; vaoiter != m_vao_lane_meshes.end(); ++vaoiter) {
    delete (*vaoiter);
  }
  m_vao_lane_meshes.clear();
}
#endif
