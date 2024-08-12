#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <vector>

class ThreadPool {
 public:
  ThreadPool(size_t);

  /// @note 可变参模板函数
  /// 尾随的返回类型需要 auto 类型说明符
  /// 这种（尾随返回）类型的写法在 C++ 11 中，用于返回类型依赖实参名或者返回类型复杂的时候
  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

  ~ThreadPool();

 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;  ///< 存储任务的队列
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};

template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
  /// @note 别名（类型重定义）
  using return_type = typename std::result_of<F(Args...)>::type;

  /// @note 构造打包任务 task ，其中封装了外部传来的函数和对应参数
  auto task =
      std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  {
    /// @note 上锁
    std::unique_lock<std::mutex> lock(queue_mutex);
    // don't allow enqueueing after stopping the pool
    if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");

    /// @note 将封装了 lambda 表达式（调用了 task ）的 function 对象存入队列当中
    tasks.emplace([task]() { (*task)(); });

    /// @note 解锁
  }
  condition.notify_one();  ///< 通知一个等待条件变量的子线程

  return res;  ///< 返回 future 对象供外部调用
}

inline ThreadPool::ThreadPool(size_t threads) : stop(false) {
  for (size_t i = 0; i < threads; ++i)
    workers.emplace_back(
        /// @note 子线程执行的函数
        [this] {
          for (;;) {
            std::function<void()> task;
            {
              /// @note 上锁
              std::unique_lock<std::mutex> lock(this->queue_mutex);

              this->condition.wait(lock, [this] {
                return this->stop || !this->tasks.empty();
              });  ///< 等到收到条件变量的通知并且 lambda 返回的条件（线程池被析构 或者 任务队列不为空）为真才会往下执行

              if (this->stop && this->tasks.empty()) return;

              task = std::move(this->tasks.front());  ///< 取任务队列的队首
              this->tasks.pop();                      ///< 任务队列出队

              /// @note 解锁
            }
            task();  ///< 执行任务中的可调用实体
          }
        });
}

inline ThreadPool::~ThreadPool() {
  {
    /// @note 上锁
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
    /// @note 解锁
  }
  condition.notify_all();  ///< 通知所有等待条件变量的子线程
  for (std::thread& worker : workers) worker.join();
}
