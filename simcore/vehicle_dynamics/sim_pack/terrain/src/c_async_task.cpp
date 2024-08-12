#include "c_async_task.h"
#include "c_terrain_common.h"

namespace terrain {
CAsyncTask::CAsyncTask() {
  m_ready = true;
  m_stop = false;
  m_task.reset();
  m_thread = std::thread(&CAsyncTask::taskRun, this);
}
CAsyncTask::~CAsyncTask() {
  stopTask();
  if (m_thread.joinable()) m_thread.join();
}
void CAsyncTask::stopTask() {
  m_stop = true;
  m_task_finished_cv.notify_all();
}
bool CAsyncTask::ready() {
  {
    std::lock_guard<std::mutex> lock(m_task_mutex);
    if (m_task.get() != nullptr) return m_task->m_ready;
  }
  return false;
}
void CAsyncTask::consume() {
  std::lock_guard<std::mutex> lock(m_task_mutex);
  if (m_task.get() != nullptr && m_task->m_ready) {
    m_task.reset(nullptr);
  }
}
void CAsyncTask::wait() {
  std::unique_lock<std::mutex> lock(m_task_finish_mutex);
  m_task_finished_cv.wait(lock, [=] {
    bool ready = false;

    // check if task is ready, return imediatelly if no task present
    {
      std::lock_guard<std::mutex> lock(m_task_mutex);
      if (m_task.get() == nullptr)
        ready = true;
      else
        return ready = m_task->m_ready;
    }

    // sleep 1 microseconds if not ready
    if (!ready) std::this_thread::sleep_for(std::chrono::microseconds(1));
    return ready;
  });
}
void CAsyncTask::taskRun() {
  while (!m_stop) {
    // wait until task submitted or timeout
    std::unique_lock<std::mutex> lock(m_task_submit_mutex);
    m_task_submit_cv.wait_for(lock, std::chrono::microseconds(10));

    // task configuratrion
    TaskFun task_func = nullptr;

    // try to get task
    {
      std::lock_guard<std::mutex> lock(m_task_mutex);
      if (m_task.get() != nullptr && m_task->m_task_fun != nullptr && !m_task->m_ready) {
        task_func = m_task->m_task_fun;  // retrive task
      }
    }

    // do task
    if (task_func != nullptr) {
      try {
        task_func();
      } catch (const std::exception& e) {
        LOG(ERROR) << "fail to run task, " << e.what();
      }

      // set state to idle
      {
        std::lock_guard<std::mutex> lock(m_task_mutex);
        if (m_task.get() != nullptr) m_task->m_ready = true;
      }
    } else {
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    // notify waiter
    m_task_finished_cv.notify_one();
  }

  // notify waiter
  m_task_finished_cv.notify_all();
}
}  // namespace terrain
