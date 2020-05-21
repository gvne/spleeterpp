#include <mutex>
#include <condition_variable>

namespace artff {

// adapted from
// https://stackoverflow.com/questions/19715873/how-to-force-threads-to-work-in-strict-order
// required for windows compatibility. See
// https://github.com/gvne/spleeterpp/issues/17#issuecomment-631060400
class Semaphore {
 public:
   Semaphore();
   void Notify();
   void Wait();

 private:
  std::mutex m_mutex;
  std::condition_variable m_condition;
  unsigned long m_count;
};

}  // namespace artff
