#include "util_logger.h"

#include <chrono>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/details/thread_pool.h>

using namespace util;

/* ====================================================================== */
/* ========================== GLOBAL & STATIC =========================== */
/* ====================================================================== */

std::set<std::string> logger_mgr::_tag_checker{UTIL_LOGGER, NETWORK_LOGGER};
std::map< std::string, std::unique_ptr<logger_c> > logger_mgr::_map_logger_obj{};

static spdlog::level::level_enum convert_spdlog_level(LOG_LEVEL level)
{
	switch(level)
	{
		case util::LOG_LEVEL::TRACE:	return spdlog::level::trace;
		case util::LOG_LEVEL::DEBUG:	return spdlog::level::debug;
		case util::LOG_LEVEL::INFO:		return spdlog::level::info;
		case util::LOG_LEVEL::WARNING:	return spdlog::level::warn;
		case util::LOG_LEVEL::ERROR:	return spdlog::level::err;
		case util::LOG_LEVEL::CRITICAL:	return spdlog::level::critical;
	}

	return spdlog::level::off;
}

/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */

/* =============== logger_mgr class =============== */
bool logger_mgr::init_logger(const std::string& tag, std::vector<VARIANT_SINK>& vec_logger_data)
{
	auto find_iter = _tag_checker.find(tag);	
	if(find_iter == _tag_checker.end()) {
		return false;
	}

	std::unique_ptr<logger_c> logger_obj = std::make_unique<logger_c>();
	for(auto& _data : vec_logger_data)
	{
		bool result = logger_obj->initialize(tag, _data);
		if(false == result) {
			return false;
		}
	}

	_map_logger_obj.insert(std::make_pair(tag, std::move(logger_obj)));
	return true;
}

void logger_mgr::release_logger()
{
	spdlog::shutdown();

	_tag_checker.clear();
	_map_logger_obj.clear();
}

logger_c* logger_mgr::_find_logger(const std::string& tag)
{
	auto find_iter = _map_logger_obj.find(tag);
	if(find_iter == _map_logger_obj.end())
		return nullptr;

	return find_iter->second.get();
}

/* ================ logger_c class ================ */
bool logger_c::initialize(const std::string& tag, VARIANT_SINK& sink_data)
{
	spdlog::flush_every(std::chrono::milliseconds(500));

	/**/
	console_sink_data_st* console_data = std::get_if<console_sink_data_st>(&sink_data);
	if(nullptr != console_data)
	{
		_console_thread_pool = std::make_shared<spdlog::details::thread_pool>(8192, 1);

		std::string logger_name = tag + std::string("_console");
		auto dist_sink_mt = std::make_shared<spdlog::sinks::dist_sink_mt>();
		spdlog::level::level_enum spdlog_level = convert_spdlog_level(console_data->standard_level);

		//
		auto console_sink_mt = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink_mt->set_pattern(console_data->pattern);
		console_sink_mt->set_level(spdlog_level);
		dist_sink_mt->add_sink(console_sink_mt);

		//
		_console_logger = std::make_shared<spdlog::async_logger>(logger_name, dist_sink_mt, _console_thread_pool, spdlog::async_overflow_policy::block);
		_console_logger->set_level(spdlog_level);
		_console_logger->flush_on(spdlog_level);

		spdlog::register_logger(_console_logger);
		return true;
	}
	
	/**/
	rotate_file_sink_data_st* rotate_file_data = std::get_if<rotate_file_sink_data_st>(&sink_data);
	if(nullptr != rotate_file_data)
	{
		_rotate_file_thread_pool = std::make_shared<spdlog::details::thread_pool>(8192, 1);

		std::string logger_name = tag + std::string("_rotate_file");
		auto dist_sink_mt = std::make_shared<spdlog::sinks::dist_sink_mt>();
		spdlog::level::level_enum spdlog_level = convert_spdlog_level(rotate_file_data->standard_level);

		//
		auto rotate_file_sink_mt = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(rotate_file_data->file_path, rotate_file_data->max_file_size, rotate_file_data->max_file_count, false);
		rotate_file_sink_mt->set_pattern(rotate_file_data->pattern);
		rotate_file_sink_mt->set_level(spdlog_level);
		dist_sink_mt->add_sink(rotate_file_sink_mt);

		if(true == rotate_file_data->need_console)
		{
			auto console_sink_mt = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			console_sink_mt->set_pattern(rotate_file_data->pattern);
			console_sink_mt->set_level(spdlog_level);
			dist_sink_mt->add_sink(console_sink_mt);
		}
		
		//
		_rotate_file_logger = std::make_shared<spdlog::async_logger>(logger_name, dist_sink_mt, _rotate_file_thread_pool, spdlog::async_overflow_policy::block);
		_rotate_file_logger->set_level(spdlog_level);
		_rotate_file_logger->flush_on(spdlog_level);

		spdlog::register_logger(_rotate_file_logger);
		return true;
	}

	/**/
	daily_file_sink_data_st* daily_file_data = std::get_if<daily_file_sink_data_st>(&sink_data);
	if(nullptr != daily_file_data)
	{
		_daily_file_thread_pool = std::make_shared<spdlog::details::thread_pool>(8192, 1);

		std::string logger_name = tag + std::string("_daily_file");
		auto dist_sink_mt = std::make_shared<spdlog::sinks::dist_sink_mt>();
		spdlog::level::level_enum spdlog_level = convert_spdlog_level(daily_file_data->standard_level);

		//
		auto daily_file_sink_mt = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(daily_file_data->file_path, daily_file_data->rotation_hour, daily_file_data->rotation_minute, false, daily_file_data->max_file_count);
		daily_file_sink_mt->set_pattern(daily_file_data->pattern);
		daily_file_sink_mt->set_level(spdlog_level);
		dist_sink_mt->add_sink(daily_file_sink_mt);

		if(true == daily_file_data->need_console)
		{
			auto console_sink_mt = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			console_sink_mt->set_pattern(daily_file_data->pattern);
			console_sink_mt->set_level(spdlog_level);
			dist_sink_mt->add_sink(console_sink_mt);
		}

		//		
		_daily_file_logger = std::make_shared<spdlog::async_logger>(logger_name, dist_sink_mt, _daily_file_thread_pool, spdlog::async_overflow_policy::block);
		_daily_file_logger->set_level(spdlog_level);
		_daily_file_logger->flush_on(spdlog_level);

		spdlog::register_logger(_daily_file_logger);
		return true;
	}

	return false;
}

void logger_c::write_console_log(LOG_LEVEL level, std::string log_str)
{
	if(nullptr == _console_logger)
		return;

	switch (level) 
	{
		case LOG_LEVEL::TRACE: 		_console_logger->trace(log_str);	break;
		case LOG_LEVEL::DEBUG:		_console_logger->debug(log_str);	break;
		case LOG_LEVEL::INFO:		_console_logger->info(log_str);		break;
		case LOG_LEVEL::WARNING:	_console_logger->warn(log_str);		break;
		case LOG_LEVEL::CRITICAL:	_console_logger->critical(log_str);	break;
		case LOG_LEVEL::ERROR:		_console_logger->error(log_str);	break;
	}
}

void logger_c::write_rotate_file_log(LOG_LEVEL level, std::string log_str)
{
	if(nullptr == _rotate_file_logger)
		return;

	switch (level) 
	{
		case LOG_LEVEL::TRACE: 		_rotate_file_logger->trace(log_str);	break;
		case LOG_LEVEL::DEBUG:		_rotate_file_logger->debug(log_str);	break;
		case LOG_LEVEL::INFO:		_rotate_file_logger->info(log_str);		break;
		case LOG_LEVEL::WARNING:	_rotate_file_logger->warn(log_str);		break;
		case LOG_LEVEL::CRITICAL:	_rotate_file_logger->critical(log_str);	break;
		case LOG_LEVEL::ERROR:		_rotate_file_logger->error(log_str);	break;
	}
}

void logger_c::wirte_daily_file_log(LOG_LEVEL level, std::string log_str)
{
	if(nullptr == _daily_file_logger)
		return;

	switch (level) 
	{
		case LOG_LEVEL::TRACE: 		_daily_file_logger->trace(log_str);		break;
		case LOG_LEVEL::DEBUG:		_daily_file_logger->debug(log_str);		break;
		case LOG_LEVEL::INFO:		_daily_file_logger->info(log_str);		break;
		case LOG_LEVEL::WARNING:	_daily_file_logger->warn(log_str);		break;
		case LOG_LEVEL::CRITICAL:	_daily_file_logger->critical(log_str);	break;
		case LOG_LEVEL::ERROR:		_daily_file_logger->error(log_str);		break;
	}
}
