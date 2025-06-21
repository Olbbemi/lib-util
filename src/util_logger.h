#ifndef UTIL_LOG
#define UTIL_LOG

#include <set>
#include <map>
#include <thread>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <variant>
#include <sstream>

#include <spdlog/async_logger.h>
#include <spdlog/fmt/fmt.h>

/* =============== forward-declear =============== */
namespace util { class logger_c; }
namespace internal {
	inline util::logger_c* get_logger_obj(const std::string& TAG);
}

namespace util
{
/* ====================================================================== */
/* ========================== DEFINE & ENUM ============================= */
/* ====================================================================== */

	// tags
	inline const std::string UTIL_LOGGER = "util_logger";
	inline const std::string NETWORK_LOGGER = "network_logger";

	//  log_level
	enum class LOG_LEVEL :std::uint16_t
	{
		TRACE = 1,
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		CRITICAL
	};

/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */

	typedef struct console_sink_data_st
	{
		LOG_LEVEL standard_level;
		std::string pattern;

		console_sink_data_st(LOG_LEVEL standard_level_, const std::string& pattern_)
			:standard_level(standard_level_), pattern(pattern_) {}
	} console_sink_data_st;

	typedef struct rotate_file_sink_data_st
	{
		LOG_LEVEL standard_level;
		std::string pattern;

		std::string file_path;
		std::uint32_t max_file_size;
		std::uint32_t max_file_count;
		bool need_console;

		rotate_file_sink_data_st(LOG_LEVEL standard_level_, const std::string& pattern_, const std::string& file_path_, std::uint32_t max_file_size_, std::uint32_t max_file_count_, bool need_console_ = false)
			: standard_level(standard_level_), pattern(pattern_), file_path(file_path_), max_file_size(max_file_size_), max_file_count(max_file_count_), need_console(need_console_) {}
	} rotate_file_sink_data_st;

	typedef struct daily_file_sink_data_st
	{
		LOG_LEVEL standard_level;
		std::string pattern;

		std::string file_path;
		std::uint32_t max_file_count;
		std::uint32_t rotation_hour;
		std::uint32_t rotation_minute;
		bool need_console;

		daily_file_sink_data_st(LOG_LEVEL standard_level_, const std::string& pattern_, const std::string& file_path_, std::uint32_t max_file_count_, std::uint32_t rotation_hour_, std::uint32_t rotation_minute_, bool need_console_ = false)
			: standard_level(standard_level_), pattern(pattern_), file_path(file_path_), max_file_count(max_file_count_), rotation_hour(rotation_hour_), rotation_minute(rotation_minute_), need_console(need_console_) {}
	} daily_file_sink_data_st;

	/* logger mgr class */
	using VARIANT_SINK = std::variant<console_sink_data_st, rotate_file_sink_data_st, daily_file_sink_data_st>;
	class logger_c;
	class logger_mgr
	{
	public:
		/*
		 * init_logger : when process is executed, this function is called with tags in top-level project.
		 * release_logger : when process will be graceful shutdown, this function is called only once in top-level project. 
		 */
		static bool init_logger(const std::string& tag, std::vector<VARIANT_SINK>& vec_logger_data);
		static void release_logger();

		friend inline logger_c* internal::get_logger_obj(const std::string& TAG);

	public:
		logger_mgr() = delete;
		~logger_mgr() = delete;

		logger_mgr(const logger_mgr& rhs) = delete;
		logger_mgr& operator=(const logger_mgr& rhs) = delete;

		logger_mgr(logger_mgr&& rhs) = delete;
		logger_mgr& operator=(logger_mgr&& rhs) = delete;

	private:
		static logger_c* _find_logger(const std::string& tag);

	private:
		static std::set<std::string> _tag_checker;
		static std::map< std::string, std::unique_ptr<logger_c> > _map_logger_obj;
	};

	/* logger_c class */
	class logger_c
	{
	public:
		bool initialize(const std::string& tag, VARIANT_SINK& sink_data);
		
		void write_console_log(const LOG_LEVEL level, std::string log_str);
		void write_rotate_file_log(const LOG_LEVEL level, std::string log_str);
		void write_daily_file_log(const LOG_LEVEL level, std::string log_str);
	
		logger_c() = default;
		~logger_c() = default;

	private:
		std::shared_ptr<spdlog::async_logger> _console_logger = nullptr; 
		std::shared_ptr<spdlog::async_logger> _rotate_file_logger = nullptr; 
		std::shared_ptr<spdlog::async_logger> _daily_file_logger = nullptr; 

		std::shared_ptr<spdlog::details::thread_pool> _console_thread_pool = nullptr;
		std::shared_ptr<spdlog::details::thread_pool> _rotate_file_thread_pool = nullptr;
		std::shared_ptr<spdlog::details::thread_pool> _daily_file_thread_pool = nullptr;
	};
}

/* ====================================================================== */
/* ========================== GLOBAL & STATIC =========================== */
/* ====================================================================== */

/* this function is never called from outside. */
namespace internal
{
	inline util::logger_c* get_logger_obj(const std::string& TAG)
	{
		return util::logger_mgr::_find_logger(TAG);
	}
}

/* template formatter for std::thread::id */
template <>
struct fmt::formatter<std::thread::id> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const std::thread::id& id, FormatContext& ctx) const // format() is 'const' member-func
	{
        std::ostringstream oss;
        oss << id;
        return fmt::format_to(ctx.out(), "{}", oss.str());
    }
};

/* =============== util logger =============== */
template <typename... Args>
inline void U_LOG_CONSOLE(const util::LOG_LEVEL level, const std::string& format, Args&&... args)
{
	util::logger_c* logger_obj = internal::get_logger_obj(util::UTIL_LOGGER);	
	if(nullptr == logger_obj)
		return;

	logger_obj->write_console_log(level, fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>	
inline void U_LOG_ROTATE_FILE(const util::LOG_LEVEL level, const std::string& format, Args&&... args)
{
	util::logger_c* logger_obj = internal::get_logger_obj(util::UTIL_LOGGER);
	if(nullptr == logger_obj)
		return;

	logger_obj->write_rotate_file_log(level, fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>	
inline void U_LOG_DAILY_FILE(const util::LOG_LEVEL level, const std::string& format, Args&&... args)
{
	util::logger_c* logger_obj = internal::get_logger_obj(util::UTIL_LOGGER);
	if(nullptr == logger_obj)
		return;

	logger_obj->write_daily_file_log(level, fmt::format(format, std::forward<Args>(args)...));
}

/* =============== network logger =============== */


#endif
