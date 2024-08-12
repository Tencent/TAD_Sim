/**
 * @file data_queue.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "data_queue.h"
#include <boost/algorithm/hex.hpp>
#include <limits>

#define MAX_N 5000
#define EARSE_TO(data)                   \
  if (data.size() > MAX_N) {             \
    auto eraseIter = data.begin();       \
    std::advance(eraseIter, MAX_N / 2);  \
    data.erase(data.begin(), eraseIter); \
  }
/**
 * @brief Construct a new Data Queue:: Data Queue object
 *
 */
DataQueue::DataQueue() {
  _job_worker = std::thread([this] {
    std::uint64_t frameID_image = 0;
    std::uint64_t frameID_pcd = 0;
    for (;;) {
      std::vector<ImagePackage> img_pkg;
      std::vector<PcdPackage> pcd_pkg;
      // mutex lock
      {
        std::unique_lock<std::mutex> lock(this->_mutex);
        this->_job_condition.wait(lock, [this] {
          return this->_stop || !this->_semantics.empty() || !this->_cameras.empty() || !this->_fisheyes.empty() ||
                 !this->_lidars.empty();
        });
        if (this->_stop && this->_semantics.empty() && _cameras.empty() && this->_fisheyes.empty() &&
            this->_lidars.empty())
          return;
        EARSE_TO(this->_objects);
        // handle semantic
        for (auto &semans : this->_semantics) {
          for (auto &sem : semans.second) {
            ImagePackage pkg;
            pkg.frame_c = frameID_image;
            pkg.image = std::move(sem.second);
            pkg.type = sim_msg::SensorRaw_Type_TYPE_SEMANTIC;
            auto timestamp = pkg.image.timestamp;
            if (this->_objects.find(timestamp) == this->_objects.end()) {
              continue;
            }
            pkg.obj = this->_objects.at(timestamp);
            img_pkg.emplace_back(std::move(pkg));
          }
        }
        this->_semantics.clear();
        // handle camera
        for (auto &cams : this->_cameras) {
          for (auto &sem : cams.second) {
            ImagePackage pkg;
            pkg.frame_c = frameID_image;
            pkg.image = std::move(sem.second);
            pkg.type = sim_msg::SensorRaw_Type_TYPE_CAMERA;
            auto timestamp = pkg.image.timestamp;
            if (this->_objects.find(timestamp) == this->_objects.end()) {
              continue;
            }
            pkg.obj = this->_objects.at(timestamp);
            img_pkg.emplace_back(std::move(pkg));
          }
        }
        this->_cameras.clear();
        // handle fisheye
        for (auto &cams : this->_fisheyes) {
          for (auto &sem : cams.second) {
            ImagePackage pkg;
            pkg.frame_c = frameID_image;
            pkg.image = std::move(sem.second);
            pkg.type = sim_msg::SensorRaw_Type_TYPE_FISHEYE;
            auto timestamp = pkg.image.timestamp;
            if (this->_objects.find(timestamp) == this->_objects.end()) {
              continue;
            }
            pkg.obj = this->_objects.at(timestamp);
            img_pkg.emplace_back(std::move(pkg));
          }
        }
        this->_fisheyes.clear();
        // handle lidar
        for (auto &lidars : this->_lidars) {
          for (auto &lid : lidars.second) {
            PcdPackage pkg;
            pkg.frame_c = frameID_pcd;
            pkg.lidar = std::move(lid.second);
            auto timestamp = lid.second.timestamp;
            if (this->_objects.size() == 0) continue;
            auto fd = this->_objects.end();
            if (this->_objects.find(timestamp) == this->_objects.end()) {
              auto it_upper = this->_objects.upper_bound(timestamp);
              if (it_upper != this->_objects.end() && it_upper->first >= lid.second.timestamp_bg &&
                  it_upper->first <= lid.second.timestamp_ed) {
                fd = it_upper;
              } else if (it_upper != this->_objects.begin()) {
                --it_upper;
                if (it_upper->first >= lid.second.timestamp_bg && it_upper->first <= lid.second.timestamp_ed) {
                  fd = it_upper;
                }
              }
            }
            if (fd == this->_objects.end()) {
              continue;
            }
            pkg.obj = fd->second;
            pcd_pkg.emplace_back(std::move(pkg));
          }
        }
        this->_lidars.clear();
      }

      // package to callback
      if (this->_callback_image && !img_pkg.empty()) {
        for (const auto &pkg : img_pkg) {
          this->_callback_image(pkg);
        }
        frameID_image++;
      }
      if (this->_callback_pcd && !pcd_pkg.empty()) {
        for (const auto &pkg : pcd_pkg) {
          this->_callback_pcd(pkg);
        }
        frameID_pcd++;
      }
    }
  });
}

/**
 * @brief Destroy the Data Queue:: Data Queue object
 *
 */
DataQueue::~DataQueue() {
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _stop = true;
  }
  _job_condition.notify_all();
  if (_job_worker.joinable()) _job_worker.join();
}

/**
 * @brief set the callback function for image package
 *
 * @param callback the callback function
 */
void DataQueue::setImageCallback(const std::function<void(const ImagePackage &)> &callback) {
  std::unique_lock<std::mutex> lock(_mutex);
  _callback_image = callback;
}

/**
 * @brief set the callback function for pcd package
 *
 * @param callback the callback function
 */
void DataQueue::setPcdCallback(const std::function<void(const PcdPackage &)> &callback) {
  std::unique_lock<std::mutex> lock(_mutex);
  _callback_pcd = callback;
}

/**
 * @brief add camera info
 *
 * @param info the camera information
 */
void DataQueue::addCamera(const ImageInfo &info) {
  std::unique_lock<std::mutex> lock(_mutex);
  _cameras[info.id][info.timestamp] = std::move(info);
}

/**
 * @brief add fisheye info
 *
 * @param info the fisheye information
 */
void DataQueue::addFisheye(const ImageInfo &info) {
  std::unique_lock<std::mutex> lock(_mutex);
  _fisheyes[info.id][info.timestamp] = std::move(info);
}

/**
 * @brief add senmantic info
 *
 * @param info the semantic information
 */
void DataQueue::addSenmantic(const ImageInfo &info) {
  std::unique_lock<std::mutex> lock(_mutex);
  _semantics[info.id][info.timestamp] = std::move(info);
}

/**
 * @brief add lidar info
 *
 * @param info the lidar information
 */
void DataQueue::addLidar(const PcInfo &info) {
  std::unique_lock<std::mutex> lock(_mutex);
  _lidars[info.id][info.timestamp] = std::move(info);
}

/**
 * @brief add object info
 *
 * @param info the object information
 */
void DataQueue::addObject(const sim_msg::DisplayPose &info) {
  std::int64_t t = static_cast<std::int64_t>(info.timestamp());
  std::unique_lock<std::mutex> lock(_mutex);
  _objects[t] = std::move(info);
}

/**
 * @brief update the data queue
 *
 */
void DataQueue::update() { _job_condition.notify_all(); }
