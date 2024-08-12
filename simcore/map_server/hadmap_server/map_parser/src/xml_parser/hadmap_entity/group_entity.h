/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <vector>

class CLinkEntity;
class CMapScene;
class CGroupEntity {
 public:
  typedef std::vector<int> GroupChildren;
  typedef std::vector<CLinkEntity*> LinkEntities;

  CGroupEntity();
  void AddChild(int nID);
  void AddLink(CLinkEntity* link);
  LinkEntities& Links() { return m_links; }
  GroupChildren& Children() { return m_children; }
  std::vector<CLinkEntity*>* FindFromLinks(int id);
  std::vector<CLinkEntity*>* FindToLinks(int id);
  void SetScene(CMapScene* pScene) { m_scene = pScene; }
  CMapScene* Scene() { return m_scene; }
  void GenerateCrossLink();

 protected:
  std::vector<int> m_children;
  std::vector<CLinkEntity*> m_links;
  std::map<int, std::vector<CLinkEntity*> > m_fromLinksMap;
  std::map<int, std::vector<CLinkEntity*> > m_toLinksMap;
  CMapScene* m_scene;
};
