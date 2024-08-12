// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "spatialindex/hadmap_spatialindex.h"
#include "spatialindex/RTree.h"

#include <algorithm>
#include <cfloat>
#include <exception>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hadmap {
#define SP_MAX_GEOM_SIZE 100000

typedef unsigned int sp_pkid;

struct SearchResult {
  size_t size;
  size_t capacity;
  sp_pkid* data;

  explicit SearchResult(size_t cap) : size(0), capacity(cap) { data = new sp_pkid[cap]; }

  ~SearchResult() { delete[] data; }

  void add(sp_pkid d) {
    if (size == capacity) {
      sp_pkid* newData = new sp_pkid[capacity * 2];
      memcpy(newData, data, sizeof(sp_pkid) * capacity);
      delete[] data;
      data = newData;
      capacity *= 2;
    }
    data[size] = d;
    size += 1;
  }
};

static std::vector<sp_pkid> sr;

bool searchCallBack(sp_pkid geomId) {
  // SearchResult *srPtr = ( SearchResult* )resultPtr;
  // srPtr->add( geomId );
  sr.push_back(geomId);
  return true;
}

struct txSpatialIndex::SpatialData {
 private:
  typedef RTree<sp_pkid, double, 2, double> sptree;
  std::vector<sptree> spDataRT;

  typedef std::unordered_map<sp_pkid, txEnvelope> id2envelope;
  typedef std::unordered_map<sp_pkid, pkid_t> spid2pkid;
  typedef std::unordered_map<pkid_t, sp_pkid> pkid2spmax;

  std::vector<id2envelope> spDataId2Envelope;

  std::vector<sp_pkid> spIds;

  std::vector<spid2pkid> spIdMap;

  std::vector<pkid2spmax> pkIdMap;

  std::vector<std::mutex> dataMutex;

  typedef std::unique_lock<std::mutex> mutexLock;

 public:
  SpatialData()
      : spDataRT(3),
        spDataId2Envelope(3, id2envelope()),
        spIds(3, SP_MAX_GEOM_SIZE),
        spIdMap(3, spid2pkid()),
        pkIdMap(3, pkid2spmax()),
        dataMutex(3) {}

  ~SpatialData() {}

 public:
  txEnvelope createEnvelope(const txPoint& p1, const txPoint& p2) {
    txEnvelope envelope;
    envelope.bMin[0] = std::min(p1.x, p2.x);
    envelope.bMin[1] = std::min(p1.y, p2.y);
    envelope.bMax[0] = std::max(p1.x, p2.x);
    envelope.bMax[1] = std::max(p1.y, p2.y);

    if (fabs(envelope.bMin[0] - envelope.bMax[0]) < 1e-6) {
      envelope.bMin[0] -= 1e-6;
      envelope.bMax[0] += 1e-6;
    }
    if (fabs(envelope.bMin[1] - envelope.bMax[1]) < 1e-6) {
      envelope.bMin[1] -= 1e-6;
      envelope.bMax[1] += 1e-6;
    }

    return envelope;
  }

  txEnvelope createEnvelope(const PointVec& points) {
    if (points.empty()) throw std::runtime_error("createEnvelope error, points is empty");
    txEnvelope envelope;
    envelope.bMin[0] = points[0].x;
    envelope.bMin[1] = points[0].y;
    envelope.bMax[0] = points[0].x;
    envelope.bMax[1] = points[0].y;

    for (size_t i = 1; i < points.size(); ++i) {
      envelope.bMin[0] = std::min(envelope.bMin[0], points[i].x);
      envelope.bMin[1] = std::min(envelope.bMin[1], points[i].y);
      envelope.bMax[0] = std::max(envelope.bMax[0], points[i].x);
      envelope.bMax[1] = std::max(envelope.bMax[1], points[i].y);
    }

    if (fabs(envelope.bMin[0] - envelope.bMax[0]) < 1e-6) {
      envelope.bMin[0] -= 1e-6;
      envelope.bMax[0] += 1e-6;
    }
    if (fabs(envelope.bMin[1] - envelope.bMax[1]) < 1e-6) {
      envelope.bMin[1] -= 1e-6;
      envelope.bMax[1] += 1e-6;
    }

    return envelope;
  }

  size_t type2Index(SpatialDataType type) {
    switch (type) {
      case LANE_SP:
        return 0;
      case LINK_SP:
        return 1;
      case OBJ_SP:
        return 2;
      default:
        throw std::runtime_error("SpatialDataType Error");
    }
  }

  bool insert(SpatialDataType type, const txLineCurve* curvePtr, pkid_t curveIndex) {
    size_t typeIndex = type2Index(type);
    sptree& curTree = spDataRT[typeIndex];
    id2envelope& curId2Envelope = spDataId2Envelope[typeIndex];
    spid2pkid& curSp2Pk = spIdMap[typeIndex];
    pkid2spmax& curPk2SpMax = pkIdMap[typeIndex];

    if (curvePtr == NULL) throw std::runtime_error("SpatialData insert error, curvePtr is null");

    // if curve has been inserted into cur rtree, then return
    {
      mutexLock lck(dataMutex[typeIndex]);
      if (curPk2SpMax.find(curveIndex) != curPk2SpMax.end()) return false;
    }

    // get curve points
    // if curve is curb | barrier, then resample
    PointVec points;
    curvePtr->getPoints(points);
    if (points.size() == 1) {
      hadmap::txPoint ptNext = points.front();
      ptNext.x = ptNext.x + 0.01;
      ptNext.y = ptNext.y + 0.01;
      points.push_back(ptNext);
    }

    // generate bbox
    // if curve is polygon, then generate only one bbox
    // else generate size - 1 bboxes to cover each single line in this curve
    size_t envelopeSize;
    envelopeSize = points.size() - 1;
    std::vector<txEnvelope> geomEnvelopes(envelopeSize, txEnvelope());
    for (size_t i = 0; i < points.size() - 1; ++i) geomEnvelopes[i] = createEnvelope(points[i], points[i + 1]);

    // generate bbox for whole curve
    txEnvelope wholeEnvelope = createEnvelope(points);

    // insert data into spatialindex
    // thread safe
    mutexLock lck(dataMutex[typeIndex]);
    sp_pkid curSpId = spIds[typeIndex];
    spIds[typeIndex] += SP_MAX_GEOM_SIZE;
    curSp2Pk.insert(std::make_pair(curSpId, curveIndex));
    curPk2SpMax.insert(std::make_pair(curveIndex, curSpId + geomEnvelopes.size() - 1));
    for (size_t i = 0; i < geomEnvelopes.size(); ++i)
      curTree.Insert(geomEnvelopes[i].bMin, geomEnvelopes[i].bMax, curSpId + i);
    curId2Envelope.insert(std::make_pair(curSpId, wholeEnvelope));
    return true;
  }

  bool remove(SpatialDataType type, pkid_t curveIndex) {
    size_t typeIndex = type2Index(type);
    sptree& curTree = spDataRT[typeIndex];
    id2envelope& curId2Envelope = spDataId2Envelope[typeIndex];
    spid2pkid& curSp2Pk = spIdMap[typeIndex];
    pkid2spmax& curPk2SpMax = pkIdMap[typeIndex];

    mutexLock lck(dataMutex[typeIndex]);
    if (curPk2SpMax.find(curveIndex) == curPk2SpMax.end()) return false;

    // spGeomId & geomSize
    sp_pkid curSpId = curPk2SpMax[curveIndex];
    size_t geomSize = curSpId % SP_MAX_GEOM_SIZE + 1;
    curSpId = curSpId - geomSize;

    // txEnvelope removeEnvelope( -DBL_MAX, -DBL_MAX, DBL_MAX, DBL_MAX );
    txEnvelope removeEnvelope = curId2Envelope[curSpId];
    for (size_t i = 0; i < geomSize; ++i) {
      curTree.Remove(removeEnvelope.bMin, removeEnvelope.bMax, curSpId + i);
    }
    curId2Envelope.erase(curSpId);
    curSp2Pk.erase(curSpId);
    curPk2SpMax.erase(curveIndex);
    return true;
  }

  bool removeAll(SpatialDataType type) {
    size_t typeIndex = type2Index(type);
    sptree& curTree = spDataRT[typeIndex];
    spid2pkid& curSp2Pk = spIdMap[typeIndex];
    pkid2spmax& curPk2SpMax = pkIdMap[typeIndex];
    id2envelope& curId2Envelope = spDataId2Envelope[typeIndex];

    mutexLock lck(dataMutex[typeIndex]);
    curTree.RemoveAll();
    curSp2Pk.clear();
    curPk2SpMax.clear();
    curId2Envelope.clear();
    return true;
  }

  int search(SpatialDataType type, const txEnvelope& envelope, std::vector<pkid_t>& curveIndexes,
             std::vector<std::pair<size_t, size_t> >& pointRanges) {
    size_t typeIndex = type2Index(type);
    sptree& curTree = spDataRT[typeIndex];
    spid2pkid& curSp2Pk = spIdMap[typeIndex];

    std::vector<sp_pkid> sr;
    // SearchResult sr(20);
    sr.clear();
    auto searchCallBack = [&](sp_pkid geomId) {
      // SearchResult *srPtr = ( SearchResult* )resultPtr;
      // srPtr->add( geomId );
      sr.push_back(geomId);
      return true;
    };
    int hits = curTree.Search(envelope.bMin, envelope.bMax, searchCallBack);
    if (hits > 0) {
      curveIndexes.clear();
      pointRanges.clear();
      std::unordered_map<sp_pkid, std::pair<size_t, size_t> > indexMap;
      for (size_t i = 0; i < sr.size(); ++i) {
        size_t pointIndex = sr[i] % SP_MAX_GEOM_SIZE;
        sp_pkid spId = sr[i] - pointIndex;
        if (indexMap.find(spId) == indexMap.end()) {
          indexMap.insert(std::make_pair(spId, std::make_pair(pointIndex, pointIndex)));
        } else {
          indexMap[spId].first = std::min(pointIndex, indexMap[spId].first);
          indexMap[spId].second = std::max(pointIndex, indexMap[spId].second);
        }
      }
      for (auto itr = indexMap.begin(); itr != indexMap.end(); ++itr) {
        curveIndexes.push_back(curSp2Pk[itr->first]);
        pointRanges.push_back(itr->second);
      }
    }
    return hits;
  }
};

txSpatialIndex::txSpatialIndex() : spDataPtr(new SpatialData) {}

txSpatialIndex::txSpatialIndex(const txSpatialIndex& sp) {}

txSpatialIndex::~txSpatialIndex() {}

bool txSpatialIndex::insert(SpatialDataType type, const txLineCurve* curve, pkid_t curveIndex) {
  return spDataPtr->insert(type, curve, curveIndex);
}

bool txSpatialIndex::remove(SpatialDataType type, pkid_t curveIndex) { return spDataPtr->remove(type, curveIndex); }

bool txSpatialIndex::removeAll(SpatialDataType type) { return spDataPtr->removeAll(type); }

bool txSpatialIndex::search(SpatialDataType type, const txEnvelope& envelope, std::vector<pkid_t>& curveIndexes) {
  std::vector<std::pair<size_t, size_t> > ranges;
  return spDataPtr->search(type, envelope, curveIndexes, ranges) != 0;
}

bool txSpatialIndex::search(SpatialDataType type, const txEnvelope& envelope, std::vector<pkid_t>& curveIndexes,
                            std::vector<std::pair<size_t, size_t> >& pointRanges) {
  return spDataPtr->search(type, envelope, curveIndexes, pointRanges) != 0;
}
}  // namespace hadmap
