#include <eldr/core/logger.hpp>
#include <eldr/core/thread.hpp>
#include <eldr/core/util.hpp>

#include <thread>
#include <unordered_map>

NAMESPACE_BEGIN(eldr)

static std::shared_ptr<Thread>                  main_thread{ nullptr };
static thread_local std::shared_ptr<Thread>     self{ nullptr };
static std::thread::id                          this_thread_id;
static std::unordered_map<std::string, Thread*> thread_map;
static std::mutex                               thread_map_mutex;

class MainThread : public Thread {
public:
  MainThread() : Thread("Main") {};
  virtual ~MainThread() = default;

  virtual void run() override { Log(Error, "Main thread is already running!"); }
};

class WorkerThread : public Thread {
  WorkerThread(const std::string& prefix)
    : Thread(fmt::format("{}{}", prefix, counter_++)) {};

  virtual void run() override
  {
    Log(Error, "Worker thread is already running!");
  }

protected:
  virtual ~WorkerThread()
  {
    std::lock_guard guard{ thread_map_mutex };
    thread_map.erase(this->name());
  };
  static std::atomic<uint32_t> counter_;
};

std::atomic<uint32_t> WorkerThread::counter_{ 0 };

void Thread::createContext()
{
  assert(not main_thread);
  // global_thread_count = util::coreCount();
  //  this_thread_id      = std::this_thread::get_id();

  self = std::make_shared<MainThread>();

  // self->d->running = true;
  //  self->d->fresolver = new FileResolver();
  main_thread = self;
}

Thread* Thread::thread()
{
  // notifier.ensureInitialized();
  Thread* self_val = self.get();
  assert(self_val);
  return self_val;
}

NAMESPACE_END(eldr)
