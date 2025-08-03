#ifndef UTIL_EPOLL
#define UTIL_EPOLL

#include <cstdint>
#include <vector>

namespace util
{

	/* ====================================================================== */
    /* ========================== DEFINE & ENUM ============================= */
    /* ====================================================================== */
	using EPOLL_ID = std::int32_t;

	enum class EV_TYPE
	{
		EV_TYPE_READ = 1,
		EV_TYPE_WRITE,
	};

    /* ====================================================================== */
    /* ========================== CLASS & STRUCT ============================ */
    /* ====================================================================== */
	struct ev_result
	{
		EV_TYPE ev_type;
		std::int32_t fd;
		void* user_data;

		ev_result(EV_TYPE ev_type_, std::int32_t fd_, void* user_data_)
			: ev_type(ev_type_), fd(fd_), user_data(user_data_) {}
	};

	class epoll_c
	{
	public:
		bool create_epoll();
		bool regist_fdescriptor(std::int32_t fd, void* user_data);
		bool remove_fdescriptor(std::int32_t fd);
		std::vector<ev_result> wait_event();

		/* <-- special member functions --> */
		epoll_c() = default;
		~epoll_c();

	private:
		EPOLL_ID _epoll_id;
	};
}

#endif
