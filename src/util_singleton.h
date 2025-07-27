#ifndef UTIL_SINGLETON_H
#define UTIL_SINGLETON_H

#include <cstdint>
#include <mutex>

namespace util 
{
/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */

	/* <-- static singleton --> */
	template <typename Class>
	class static_singletonHolder_c
	{
	public:
		static Class& get_instance();

	public:
		static_singletonHolder_c() = default;
		~static_singletonHolder_c() = default;

		static_singletonHolder_c(const static_singletonHolder_c& rhs) = delete;
		static_singletonHolder_c& operator=(const static_singletonHolder_c& rhs) = delete;

		static_singletonHolder_c(static_singletonHolder_c&& rhs) = delete;
		static_singletonHolder_c& operator=(static_singletonHolder_c&& rhs) = delete;
	};

	template <typename Class>
	Class& static_singletonHolder_c<Class>::get_instance()
	{
		static Class obj;
		return obj;
	}

	/* <-- dynamic singleton --> */
	template<typename Class>
	class dynamic_singletonHolder_c
	{
	public:
		static Class* get_instance();
		static bool release(Class* obj);

	public:
		dynamic_singletonHolder_c() = default;
		~dynamic_singletonHolder_c() = default;

		dynamic_singletonHolder_c(const dynamic_singletonHolder_c& rhs) = delete;
		dynamic_singletonHolder_c& operator=(const dynamic_singletonHolder_c& rhs) = delete;

		dynamic_singletonHolder_c(dynamic_singletonHolder_c&& rhs) = delete;
		dynamic_singletonHolder_c& operator=(dynamic_singletonHolder_c&& rhs) = delete;

#ifdef UNIT_TEST
		static bool is_released() {
			if(nullptr == _obj)
				return true;
			
			return false;
		}
#endif

	private:
		static Class* _obj;
		static std::mutex _mutex;
		static std::uint32_t _use_cnt;
	};

	template <typename Class> Class* dynamic_singletonHolder_c<Class>::_obj = nullptr;
	template <typename Class> std::mutex dynamic_singletonHolder_c<Class>::_mutex;
	template <typename Class> std::uint32_t dynamic_singletonHolder_c<Class>::_use_cnt = 0;

	template<typename Class>
	Class* dynamic_singletonHolder_c<Class>::get_instance()
	{
		std::lock_guard<std::mutex> _guard(_mutex);
		if(nullptr == _obj) {
			_obj = new Class();		
		}

		_use_cnt++;
		return _obj;
	}

	template<typename Class>
	bool dynamic_singletonHolder_c<Class>::release(Class* obj)
	{
		std::lock_guard<std::mutex> _guard(_mutex);
		if(nullptr != obj && _obj == obj)
		{
			_use_cnt--;
			if(0 == _use_cnt)
			{
				delete _obj;
				_obj = nullptr;
			}

			return true;
		}
		else {
			return false;
		}
	}
}

#endif
