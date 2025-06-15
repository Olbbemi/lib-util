#ifndef UTIL_THREAD_POOL
#define UTIL_THREAD_POOL

#include <mutex>
#include <queue>
#include <string>
#include <atomic>
#include <thread>
#include <future>
#include <cstdint>
#include <functional>
#include <condition_variable>

#include <unistd.h>
#include <sys/eventfd.h>

#include "util_logger.h"

namespace util 
{
/* ====================================================================== */
/* ========================== DEFINE & ENUM ============================= */
/* ====================================================================== */
#define  FD_EVENT_TYPE_SIZE 8
using EVENT_FD = int;

/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */

	class thread_pool_c
	{
	public:
		thread_pool_c() = default;
		~thread_pool_c();

		thread_pool_c(const thread_pool_c&& rhs) = delete;

	public:
		bool create_pool(const std::string& identification, uint16_t max_cnt);

		template<typename Func, typename... Args>
		bool async_dispatch(Func&& func, Args&&... args)
		{
			// when shutdown flag is true, no more tasks should be enqueued.
			bool shutdown = _shutdown.load();
			if(true == shutdown)
			{
				U_LOG_ROTATE_FILE(util::LOG_LEVEL::DEBUG, "[{}] thread_pool is shutdown.", _identificaion);
				return false;
			}

			// make task with function and variable.
			auto task = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
			{
				std::lock_guard<std::mutex> lock_obj(_task_mutex);
				_task_queue.push(task);
			}

			// awake producer thread with event_fd.
			uint64_t value = 1;
			static_assert(sizeof(value) == FD_EVENT_TYPE_SIZE, "[ERROR] value-size mismatch for write(event_fd). need_size: FD_EVENT_TYPE_SIZE");
			ssize_t result = write(_event_fd, &value, sizeof(value));
			return true;
		}

	private:
		void _task_consumer_thread(std::promise<void> ready_signal, uint16_t index);
		void _task_producer_thread();
		bool _check_exec_right_now();

	private:
		std::atomic_bool _shutdown;
		uint16_t _max_cnt;
		EVENT_FD _event_fd;

		std::string _identificaion;
		std::vector<int64_t> _consumer_elapsed_time;
		std::vector<std::thread> _task_consumer_pool;
		std::queue<std::function<void()>> _task_queue;
		std::map<std::thread::id, std::atomic_bool> _consumer_mgr;

		std::mutex _task_mutex, _mgr_mutex;
		std::thread _task_producer;

		std::condition_variable _task_cv;
	};
}

#endif
