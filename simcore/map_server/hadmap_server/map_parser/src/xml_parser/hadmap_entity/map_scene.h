/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <hadmap.h>
#include <vector>

class CMapElement;
class CStraightRoad;
class CCurveRoad;
class CCross;
class CGroupEntity;
class TiXmlElement;
namespace boost {
namespace filesystem {
class path;
};
};  // namespace boost

class CMapScene {
 public:
  typedef std::vector<CStraightRoad*> StragihtRoads;
  typedef std::vector<CCurveRoad*> CurveRoads;
  typedef std::vector<CCross*> Crosses;
  typedef std::vector<CGroupEntity*> Groups;

  CMapScene();
  ~CMapScene();

  void AddStraight(CStraightRoad* pStraightRoad);
  void AddCurve(CCurveRoad* pCurveRoad);
  void AddCross(CCross* pCross);
  void AddGroup(CGroupEntity* pGroup);
  CMapElement* FindElement(int elementID);
  CStraightRoad* FindStraight(int straightID);
  StragihtRoads& Straights() { return m_straight_roads; }
  CCurveRoad* FindCurve(int curveID);
  CurveRoads& Curves() { return m_curve_roads; }
  CCross* FindCross(int crossID);
  Crosses& AllCrosses() { return m_crosses; }
  Groups& AllGroups() { return m_groups; }

  hadmap::txRoads& TXRoads() { return m_txroads; }
  hadmap::txLaneBoundaries& TXLaneBoundaries() { return m_boundaries; }
  hadmap::txLaneLinks& TXLaneLinks() { return m_txLaneLinks; }

  int SaveToXml(const char* strSceneName);
  int SaveToSqlite(const char* strSceneName);

  void SetRef(double lon, double lat, double alt);
  double RefLon() { return m_dRefLon; }
  double RefLat() { return m_dRefLat; }
  double RefAlt() { return m_dRefAlt; }

  int ParseFromXml(const char* strSceneName);
  std::string ToJson();

 protected:
  int CopyHadmap(const char* strFilePath);
  int AddToConfigFile(const char* strFileName);
  int AddToConfigFile(boost::filesystem::path& p);
  int ParseStraights(TiXmlElement* elemStraights, std::vector<CStraightRoad*>& straights);
  int ParseCurves(TiXmlElement* elemCurves, std::vector<CCurveRoad*>& cruves);
  int ParseCrosses(TiXmlElement* elemCrosses, std::vector<CCross*>& crosses);
  int ParseGroups(TiXmlElement* elemGroups, std::vector<CGroupEntity*>& groups);

  std::map<int, CMapElement*> m_map_elements;
  std::vector<CStraightRoad*> m_straight_roads;
  std::map<int, CStraightRoad*> m_map_straight_roads;
  CurveRoads m_curve_roads;
  std::map<int, CCurveRoad*> m_map_curve_roads;
  Crosses m_crosses;
  std::map<int, CCross*> m_map_crosses;
  std::vector<CGroupEntity*> m_groups;

  hadmap::txRoads m_txroads;
  hadmap::txLaneLinks m_txLaneLinks;
  hadmap::txLaneBoundaries m_boundaries;

  double m_dRefLon;
  double m_dRefLat;
  double m_dRefAlt;
};
