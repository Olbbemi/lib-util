#ifndef UTIL_SINGLETON_H
#define UTIL_SINGLETON_H

#include <mutex>

namespace util 
{
/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */

	template<typename Class>
	class singleton_c
	{
	public:
		singleton_c() = default;
		~singleton_c() = default;

		static Class* get_instance();
		static bool release(Class* obj);

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
	};

	template<typename Class> 
	Class* singleton_c<Class>::_obj = nullptr;

	template <typename Class>
	std::mutex singleton_c<Class>::_mutex;

	template<typename Class>
	Class* singleton_c<Class>::get_instance()
	{
		std::lock_guard<std::mutex> _guard(_mutex);
		if(nullptr == _obj) {
			_obj = new Class();		
		}

		return _obj;
	}

	template<typename Class>
	bool singleton_c<Class>::release(Class* obj)
	{
		std::lock_guard<std::mutex> _guard(_mutex);
		if(nullptr != obj && _obj == obj)
		{
			delete _obj;
			_obj = nullptr;

			return true;
		}
		else {
			return false;
		}
	}
}

#endif
