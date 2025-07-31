#include "util_thread_pool.h"

#include <chrono>
#include <future>
#include <limits>

using namespace util;

/* ====================================================================== */
/* ========================== DEFINE & ENUM ============================= */
/* ====================================================================== */
typedef struct
{
    std::uint16_t min;
    std::uint16_t max;
} monitoring;

const monitoring Green{1, 3};
const monitoring Yellow{4, 6};
const monitoring Red{7, 10};

/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */
thread_pool_c::~thread_pool_c()
{
    _shutdown.exchange(true);

    // awake producer thread.
    std::uint64_t value = 1;
    static_assert(sizeof(value) == FD_EVENT_TYPE_SIZE, "[ERROR] value-size mismatch for write(event_fd). need_size: FD_EVENT_TYPE_SIZE");

	ssize_t result = write(_event_fd, &value, sizeof(value));
	if(FD_EVENT_TYPE_SIZE != result) {
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::CRITICAL, "write() is weird. return_value: {}.", result);
	}

    if(true == _task_producer.joinable()) {
        _task_producer.join();
    }

    // awake consumer thread.
    _task_cv.notify_all();
    for(auto& task_consumer : _task_consumer_pool)
    {
        if(true == task_consumer.joinable()) {
            task_consumer.join();
        }
    }

    close(_event_fd);
    _task_consumer_pool.clear();
}

bool thread_pool_c::create_pool(const std::string& identification, std::uint16_t max_cnt)
{
    _shutdown       = false;
    _ready_task     = false;
    _identification = identification;
    _max_cnt        = max_cnt;

    // create event_fd with semaphore option.
    _event_fd = eventfd(0, EFD_SEMAPHORE);
    _consumer_elapsed_time.resize(max_cnt, 0);

    // checking all consumer thread is ready.
    std::vector<std::future<void>> check_consumer_ready;
    for(std::uint16_t idx = 0; idx < max_cnt; idx++)
    {
        std::promise<void> ready_signal;
        check_consumer_ready.push_back(ready_signal.get_future());

        _task_consumer_pool.push_back(std::thread(&thread_pool_c::_task_consumer_thread, this, std::move(ready_signal), idx));
    }

    _task_producer = std::thread(&thread_pool_c::_task_producer_thread, this);
    for(auto& consumer_future : check_consumer_ready)
    {
        if(std::future_status::ready == consumer_future.wait_for(std::chrono::milliseconds(500))) {
            consumer_future.get();
        }
        else
        {
            U_LOG_ROTATE_FILE(util::LOG_LEVEL::ERROR, "[{}] partial consumer thread isn't ready.", _identification);
            return false;
        }
    }

    U_LOG_ROTATE_FILE(util::LOG_LEVEL::DEBUG, "[{}] all consumer thread is ready. count:{}", _identification, _max_cnt);
    return true;
}

void thread_pool_c::_task_consumer_thread(std::promise<void> ready_signal, std::uint16_t index)
{
    std::uint16_t idx = index;
    auto          _id = std::this_thread::get_id();

    {
        std::lock_guard<std::mutex> _guard(_mgr_mutex);
        _consumer_mgr.insert(std::make_pair(_id, true));
    }

    ready_signal.set_value();

    // proceed task
    std::function<void()> task;
    while(true)
    {
        {
            std::unique_lock<std::mutex> lock_obj(_task_mutex);

            _consumer_mgr[_id].exchange(true);
            _task_cv.wait(lock_obj, [&]() {
                	bool shutdown   = _shutdown.load();
                	bool ready_task = _ready_task.load();

                	return shutdown | ready_task;
            	});

            // thread shutdown.
            if(bool shutdown = _shutdown.load(); true == shutdown)
            {
                U_LOG_ROTATE_FILE(util::LOG_LEVEL::DEBUG, "[{}] consumer thread_id({}) is shutdown.", _identification, _id);
                break;
            }

            // handle task.
            if(bool ready_task = _ready_task.load(); false == ready_task)
            {
                U_LOG_ROTATE_FILE(util::LOG_LEVEL::WARNING, "[{}] consumer thread_id({}) is wake up, but ready_task is false.", _identification, _id);
                continue;
            }

            _ready_task.exchange(false);
            if(true == _task_queue.empty()) // this code never execute. (maybe)
            {
                U_LOG_ROTATE_FILE(util::LOG_LEVEL::WARNING, "[{}] consumer thread_id({}) is wake up, but task_queue is empty.", _identification, _id);
                continue;
            }

            task = _task_queue.front();
            _task_queue.pop();

            _consumer_mgr[_id].exchange(false);
        }

        auto begin_time = std::chrono::system_clock::now();
        task();
        auto end_time = std::chrono::system_clock::now();

        // calc elpased time for the task.
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - begin_time);
        _consumer_elapsed_time[idx] = elapsed_time.count();
    }
}

void thread_pool_c::_task_producer_thread()
{
    while(true)
    {
        std::uint64_t value = 0;
        static_assert(sizeof(value) == FD_EVENT_TYPE_SIZE, "[ERROR] value-size mismatch for write(event_fd). need_size: FD_EVENT_TYPE_SIZE");

		ssize_t result = read(_event_fd, &value, sizeof(value));
		if(FD_EVENT_TYPE_SIZE != result || 1 != value)
		{
			U_LOG_ROTATE_FILE(util::LOG_LEVEL::CRITICAL, " read() is weird. return_value: {} value: {}", result, value);
			break;
		}

        if(bool shutdown = _shutdown.load(); true == shutdown)
        {
            U_LOG_ROTATE_FILE(util::LOG_LEVEL::DEBUG, "[{}] producer thread is shutdown.", _identification);
            break;
        }

        std::uint16_t wait_cnt = 0;
        std::int64_t  wait_min_time_ms = std::numeric_limits<std::uint64_t>::max();
        std::int64_t  wait_mean_time_ms = 0;
        std::int64_t  wait_max_time_ms  = 0;

        while(true)
        {
            std::int64_t total_elapse_time = 0;
            for(auto& elapse_time : _consumer_elapsed_time)
            {
                if(elapse_time < wait_min_time_ms) {
                    wait_min_time_ms = elapse_time;
                }

                if(wait_max_time_ms < elapse_time) {
                    wait_max_time_ms = elapse_time;
                }

                total_elapse_time += elapse_time;
            }

            wait_mean_time_ms = total_elapse_time / static_cast<std::int64_t>(_consumer_elapsed_time.size());

            // try to awake consumer thread.
            if(true == _check_exec_right_now())
            {
                _ready_task.exchange(true);
                _task_cv.notify_one();
                break;
            }

            wait_cnt++;
            std::int64_t wait_time_ms = 0;

            // decide on sleep time.
            if(Green.min <= wait_cnt && wait_cnt <= Green.max) {
                wait_time_ms = wait_min_time_ms;
            }
            else if(Yellow.min <= wait_cnt && wait_cnt <= Yellow.max) {
                wait_time_ms = wait_mean_time_ms;
            }
            else if(Red.min <= wait_cnt && wait_cnt <= Red.max) {
                wait_time_ms = wait_max_time_ms;
            }
            else
            {
                U_LOG_ROTATE_FILE(util::LOG_LEVEL::CRITICAL, "[{}] wait_cnt({}) is over the Red limit({}).", _identification, wait_cnt, Red.max);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_ms));
        }
    }
}

bool thread_pool_c::_check_exec_right_now()
{
    std::lock_guard<std::mutex> _guard(_task_mutex);

    /* std::map<std::thread::id, std::atomic_bool> */
    for(auto& consumer : _consumer_mgr)
    {
        bool is_idle = consumer.second.load();
        if(true == is_idle) {
            return true;
		}
    }

    return false;
}
