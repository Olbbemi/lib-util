#include "util_epoll.h"
#include "util_logger.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/epoll.h>

using namespace util;

/* ====================================================================== */
/* ========================== DEFINE & ENUM ============================= */
/* ====================================================================== */
constexpr std::uint32_t MAX_EVENT_CNT = 1024;

/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */
epoll_c::~epoll_c()
{
	if(-1 == close(_epoll_id))
	{
		char buf[512];
		std::int32_t err_num = errno;

		char* str_err = strerror_r(err_num, buf, sizeof(buf));
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::ERROR, "close() return -1. errno:{}/errstr:{}", err_num, str_err);
	}
}

bool epoll_c::create_epoll()
{
	_epoll_id = epoll_create1(0);
	if(-1 == _epoll_id)
	{
		char buf[512];
		std::int32_t err_num = errno;

		char* str_err = strerror_r(err_num, buf, sizeof(buf));
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::ERROR, "epoll_create1() return -1. errno:{}/errstr:{}", err_num, str_err);

		return false;
	}

	return true;
}

bool epoll_c::regist_fdescriptor(std::int32_t fd, void* user_data)
{
	epoll_event ev;
	memset(&ev, 0x00, sizeof(ev));

	ev.events = EPOLLIN | EPOLLOUT | EPOLLET; // using edge-triggered.
	ev.data.fd = fd;
	ev.data.ptr = user_data;

	std::int32_t result = epoll_ctl(_epoll_id, EPOLL_CTL_ADD, fd, &ev);
	if(-1 == result)
	{
		char buf[512];
		std::int32_t err_num = errno;

		char* str_err = strerror_r(err_num, buf, sizeof(buf));
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::ERROR, "epoll_ctl() return -1. errno:{}/errstr:{}", err_num, str_err);

		return false;
	}

	return true;
}

bool epoll_c::remove_fdescriptor(std::int32_t fd)
{
	std::int32_t result = epoll_ctl(_epoll_id, EPOLL_CTL_DEL, fd, nullptr);
	if(-1 == result)
	{
		char buf[512];
		std::int32_t err_num = errno;

		char* str_err = strerror_r(err_num, buf, sizeof(buf));
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::ERROR, "epoll_ctl() return -1. errno:{}/errstr:{}", err_num, str_err);

		return false;
	}

	return true;
}

std::vector<ev_result> epoll_c::wait_event()
{
	std::vector<ev_result> vec_ev_result;
	vec_ev_result.reserve(MAX_EVENT_CNT);

	epoll_event events[MAX_EVENT_CNT];

	std::int32_t nfds =	epoll_wait(_epoll_id, events, MAX_EVENT_CNT, -1);
	for(std::int32_t idx = 0; idx < nfds; idx++)
	{
		/*
		 * epoll_event is declared with '__attribute__((packed))'.
		 * Structure declared with the '__attribute__((packed))' can't be bound to l-value reference.
		 * But this may vary depending on OS and Compiler version.
		 */
		void* ev_user_data     = events[idx].data.ptr;
		std::int32_t ev_fd     = events[idx].data.fd;
		std::uint32_t ev_event = events[idx].events;

		if(true == (EPOLLIN & ev_event) || true == (EPOLLHUP & ev_event))
		{
			if(true == (EPOLLHUP & ev_event)) {
				U_LOG_ROTATE_FILE(util::LOG_LEVEL::INFO, "event_type is Hangup. fd:{}", ev_fd);
			}

			vec_ev_result.emplace_back(EV_TYPE::EV_TYPE_READ, ev_fd, ev_user_data);
		}
		else if(true == (EPOLLOUT & ev_event)) {
			vec_ev_result.emplace_back(EV_TYPE::EV_TYPE_WRITE, ev_fd, ev_user_data);
		}
		else {
			U_LOG_ROTATE_FILE(util::LOG_LEVEL::ERROR, "event_type is {}. fd:{}",  ev_event, ev_fd);
		}
	}

	return vec_ev_result;
}
