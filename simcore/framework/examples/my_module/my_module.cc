#include "my_module.h"

#include <iomanip>
#include <iostream>

// since the example would process the two types of message, 
// we need include the two corresponding protobuf headers here.
// if you need process other type of protobuf message that TADSim defines, include them too.
#include "traffic.pb.h"
#include "location.pb.h"
#include "settings.pb.h"
#include "scene.pb.h"


#ifdef _WIN32
#define MODULE_API __declspec(dllexport)
#else
#define MODULE_API
#endif // _WIN32

#define SPLIT_LINE "======================================="
#define SHMEM_TOPIC "MY_SHMEM_TOPIC"


MyModule::MyModule() {}


MyModule::~MyModule() {
  std::cout << "MyModule destroyed." << std::endl;
}


void MyModule::Init(tx_sim::InitHelper& helper) {
  std::cout << SPLIT_LINE << std::endl;

  // get user defined initiation parameters.
  // if we defined the parameters in TADSim UI, override the default values here.
  std::string value = helper.GetParameter("max_step_count");
  if (!value.empty()) {
    max_step_count_ = std::stoi(value);
    std::cout << "Init with parameter max_step_count = " << max_step_count_ << std::endl;
  }
  value = helper.GetParameter("velocity");
  if (!value.empty()) {
    step_velocity_ = std::stod(value);
    std::cout << "Init with parameter velocity = " << step_velocity_ << std::endl;
  }

  puber_ = !helper.GetParameter("pub").empty();
  if (puber_) {
    // publish our topics with messages we produced in Step callback.
    helper.Publish(tx_sim::topic::kLocation);
    // now we could also pub-subs through shared memory!
    helper.PublishShmem(SHMEM_TOPIC, 1024);
    // also subscribe location sent by itself.
    helper.Subscribe(tx_sim::topic::kLocation);
    //helper.ImmediateSubcribe(tx_sim::topic::kLocation);
  } else {
    // by subscribe our interested topics, we expect that the two corresponding messages which defined by
    // traffic.proto and location.proto would received in every Step callback.
    helper.Subscribe(tx_sim::topic::kTraffic);
    helper.Subscribe(tx_sim::topic::kLocation);
    // now we could also pub-subs through shared memory!
    helper.SubscribeShmem(SHMEM_TOPIC);
  }
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);
};


void MyModule::Reset(tx_sim::ResetHelper& helper) {
  std::cout << SPLIT_LINE << std::endl;

  // first we should reset(clear) any data status before starting a new scenario.
  step_count_ = 0;
  last_timestamp_ = 0;
  start_x_ = 0, start_y_ = 0, cur_x_ = 0, cur_y_ = 0;

  // here we could get some information(e.g. the map which the scenario runs on, suggested local origin coordinate
  // and the destination of the ego car) of the current scenario.
  tx_sim::Vector3d o = helper.map_local_origin(), d = helper.ego_destination();
  sim_msg::Location start_location;
  // get start location in reset or in first step. both works.
  start_location.ParseFromString(helper.ego_start_location());
  std::cout << "Reset scenario with parameters:\n"
    << "hadmap path: " << helper.map_file_path() << "\n"
    << "the local origin coordinate of the map: (" << o.x << ", " << o.y << ", " << o.z << ")" << "\n"
    << "ego car's destination (" << d.x << ", " << d.y << ")" << std::endl;
  // we can also get the speed limit(max) info of the ego car in current scenario.
  double speed_limit = helper.ego_speed_limit();
  std::string path = helper.map_file_path();
  //helper.txsim_scence_events();
  // we could throw an exception which must inherited from std::exception type, if there is an error.
  if (helper.map_file_path() == "/some/map/that/we/cannot/load") {
    // std::runtime_error is a subclass of the std::exception.
    throw std::runtime_error("the map " + helper.map_file_path() + " could not load.");
  }

  //helper.setting_pb();
  sim_msg::Setting setting;
  std::string sSetting = helper.setting_pb();
  setting.ParseFromString(sSetting);
  std::cout << setting.DebugString()<<std::endl;

  //helper.scene_pb();
  sim_msg::Scene scene;
  std::string sScene = helper.scene_pb();
  scene.ParseFromString(sScene);
  std::cout << scene.DebugString()<<std::endl;

};


void MyModule::Step(tx_sim::StepHelper& helper) {
  std::cout << SPLIT_LINE << std::endl;

  // 1. get current simulation timestamp.
  double time_stamp = helper.timestamp();
  std::cout << "time stamp: " << time_stamp << "\n";
  int64_t curtime_stamp = helper.current_timestamp();
  //std::cout << "curtime_stamp : " << curtime_stamp << "\n";

  // 2. get messages we subscribed.
  helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload_);
  sim_msg::Location loc;
  loc.ParseFromString(payload_);
  cur_x_ = loc.position().x();
  cur_y_ = loc.position().y();
  std::cout << std::fixed << std::setprecision(12) << "received location: x -> " \
   << cur_x_ << " y -> " << cur_y_ << std::endl;

  helper.GetSubscribedMessage(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation, payload_);
  std::cout << "union message length: " << payload_.size() << std::endl;
  if (puber_) {
    // 3. here should put the actual user algorithm, do some computing according to the subscribed messages we received.
    // for explanatory simplicity, it only moves a little by a constant velocity, no matter what happens.
    double move_distance = step_velocity_ * (time_stamp - last_timestamp_); // s = v * t
    std::cout << "ego car moved " << move_distance << std::endl;
    double next_x = cur_x_ + move_distance, next_y = cur_y_ + move_distance;

    // 4. put our results into output messages and publish them.
    loc.mutable_position()->set_x(next_x);
    loc.mutable_position()->set_y(next_y);
    // since the message we want to publish is a protobuf message type
    // and the PublishMessage API only accepts the std::string type,
    // we need serialize it to std::string using google::protobuf::MessageLite::SerializeToString method.
    loc.SerializeToString(&payload_);
    helper.PublishMessage(tx_sim::topic::kLocation, payload_);

    last_timestamp_ = time_stamp;
    std::cout << "expected next position: (" << next_x << ", " << next_y << ")" << std::endl;

    // we could also send data via shared memory ...
    char* shm_buf = nullptr;
    uint32_t shm_size = helper.GetPublishedShmemBuffer(SHMEM_TOPIC, &shm_buf);
    if (shm_buf == nullptr) {
      std::cerr << "no shm available!" << std::endl;
    } else {
      std::string shm_data = "this is a string data on step " + std::to_string(step_count_);
      size_t str_len = shm_data.size();
      memcpy(shm_buf, &str_len, sizeof(size_t));
      memcpy(shm_buf + sizeof(size_t), shm_data.data(), str_len);
    }
  } else {
    helper.GetSubscribedMessage(tx_sim::topic::kTraffic, payload_);
    // since the traffic and location message is defined by TADSim, the payload string we get here is
    // in serialized protobuf bytes form, we need deserialize them manually.
    // using google::protobuf::MessageLite::ParseFromString method to parse from a std::string type.
    sim_msg::Traffic traffic;
    traffic.ParseFromString(payload_);
    std::cout << tx_sim::topic::kTraffic << ": "
      << traffic.cars_size() << " cars, "
      << traffic.staticobstacles_size() << " static obstacles, "
      << traffic.dynamicobstacles_size() << " dynamic obstacles, "
      << traffic.trafficlights_size() << " traffic lights.\n";

    const char* shm_buf = nullptr;
    uint32_t shm_size = helper.GetSubscribedShmemData(SHMEM_TOPIC, &shm_buf);
    if (shm_buf == nullptr) {
      std::cerr << "no shm available!" << std::endl;
    } else {
      std::cout << "shm buf address: " << (void*)shm_buf << std::endl;
      size_t data_len = 0;
      memcpy(&data_len, shm_buf, sizeof(size_t));
      std::cout << "copied length from shm buf: " << data_len << std::endl;
      std::unique_ptr<char[]> read_data(new char[data_len + 1]);  // don't forget the last '\0'
      memcpy(read_data.get(), shm_buf + sizeof(size_t), data_len);
      read_data[data_len] = '\0';
      std::cout << "read data from shared memory: " << read_data.get() << std::endl;
    }
  }

  // this is just for simplicity. we should stop the scenario when we reached the destination point
  // which we received in Reset() by helper.ego_destination() method.
  step_count_++;
  if (step_count_ >= max_step_count_) {
    helper.StopScenario("we have reached our destination.");
  }
};


void MyModule::Stop(tx_sim::StopHelper& helper) {
  std::cout << SPLIT_LINE << std::endl;
  std::cout << name_ << " module stopped." << std::endl;
  // here we could send out any feedback data we want the TADSim to display on front-end UI.
  helper.set_feedback("stepCounts", std::to_string(step_count_));

  std::string str = helper.get_module_status();
  std::cout << " module status: " << str << std::endl;
};


TXSIM_MODULE(MyModule)

