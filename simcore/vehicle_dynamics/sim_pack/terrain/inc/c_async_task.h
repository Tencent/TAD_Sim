#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace terrain {
// one task function
using TaskFun = std::function<void()>;

/// <summary>
/// async task configuration
/// </summary>
struct ASyncTaskCfg {
  bool m_ready = false;          // ready
  TaskFun m_task_fun = nullptr;  // task

  ASyncTaskCfg(const TaskFun& func = nullptr, bool ready = false) : m_task_fun(func), m_ready(ready) {}
};

class CAsyncTask;
class CThreadPool;

using CAsyncTaskPtr = std::unique_ptr<CAsyncTask>;
using CThreadPoolPtr = std::shared_ptr<CThreadPool>;
using ASyncTaskCfgPtr = std::unique_ptr<ASyncTaskCfg>;

/// <summary>
/// an async task runner, user can submit task to this runner.
/// user can use "wait" to block waiting.
/// user can alse use "ready" to check if submitted task is ready(finished),
/// if task is ready, user must "consume" it, otherwise user can not submit any new task.
/// task state become "unready" after "consumed".
/// </summary>
class CAsyncTask final {
 public:
  CAsyncTask();
  virtual ~CAsyncTask();

  CAsyncTask(const CAsyncTask&) = delete;
  CAsyncTask& operator=(const CAsyncTask&) = delete;

 public:
  // submit one task and run
  template <typename FuncTy, typename... Args>
  bool async(FuncTy&& func, Args&&... args) {
    // bind and create function job
    TaskFun task_func = std::bind(std::forward<FuncTy>(func), std::forward<Args>(args)...);

    // try to submit task
    if (task_func != nullptr) {
      std::lock_guard<std::mutex> lock(m_task_mutex);
      if (m_task.get() != nullptr) {
        // LOG(INFO) << "please wait async task ready and consume the result.";
        return false;
      } else {
        m_task = ASyncTaskCfgPtr(new ASyncTaskCfg(task_func, false));
        m_task_submit_cv.notify_one();
      }
      return true;
    }

    return false;
  }
  // if submitted task is finished
  bool ready();
  // consume task result
  void consume();
  // block wait
  void wait();

 protected:
  void taskRun();
  void stopTask();

 private:
  std::thread m_thread;  // worker

  std::atomic<bool> m_stop;   // if stop async task
  std::atomic<bool> m_ready;  // indicator if this worker is in ready state

  ASyncTaskCfgPtr m_task;   // task
  std::mutex m_task_mutex;  // task mutex

  std::condition_variable m_task_finished_cv;  // conditional variable, notify when task quit
  std::mutex m_task_finish_mutex;              // conditional variable mutex

  std::condition_variable m_task_submit_cv;  // conditional variable, notify when task submitted
  std::mutex m_task_submit_mutex;            // conditional variable mutex
};
}  // namespace terrain
