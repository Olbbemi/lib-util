#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>

#include <cstdint>
#include <sys/mman.h>

namespace util
{
	/*
	 * if a particular class use this memory-pool, muse inherit this class.
	 */
	class base_node_c
	{
	public:
		friend class memory_pool_c;

		base_node_c(std::string& grp_name) : _grp_name(std::move(grp_name)) {};
		virtual ~base_node_c() = default;

	private:
		std::string _grp_name;
	};

	/*
	 * memory-pool obj can be created anywhere.
	 * because of using mmap, if there is no more space for virtual memory, some obj are unavailable.
	 * each node is managed by shared_ptr and release by custom-deleter(memory_pool_c::_free).
	 */
	class memory_pool_c
	{
	public:
		template<typename U, typename... Args>
		std::shared_ptr<U> alloc(std::string& grp_name, Args... args);

	public:
		memory_pool_c(std::string& grp_name, int max_cnt);
		~memory_pool_c();

		memory_pool_c(const memory_pool_c&) = delete;
		memory_pool_c(memory_pool_c&&) = delete;

	private:
		static void _free(base_node_c* node, std::string& grp_name);
		static long _get_pageSize();
		static long _get_osBit();
	
	private:
		template<typename U>
		void _free(U* obj);

	private:
		std::string _grp_name;
		std::unordered_multimap<size_t, base_node_c*> _mpool;

		void* _base_ptr;
		void* _last_ptr;

		uint32_t _mpool_max_cnt = 0;
		uint32_t _mpool_alloc_cnt = 0;

		uint64_t _mpool_max_byte = 0;
		uint64_t _mpool_cur_byte = 0;
		uint64_t _mpool_adjust_byte = 0;

		std::mutex _mpool_lock;
	};
}

#endif
