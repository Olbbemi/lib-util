#ifndef UTIL_SINGLETON_H
#define UTIL_SINGLETON_H

namespace util 
{
	template<typename Class>
	class singleton_c
	{
	public:
		singleton_c() = default;
		~singleton_c() = default;

		static Class* get_instance();
		static void release();

#ifdef UNIT_TEST
		static bool is_released() {
			if(nullptr == _obj)
				return true;
			
			return false;
		}
#endif

	private:
		static Class* _obj;
	};

	template<typename Class> 
	Class* singleton_c<Class>::_obj = nullptr;

	template<typename Class>
	Class* singleton_c<Class>::get_instance()
	{
		if(nullptr == _obj) {
			_obj = new Class();		
		}

		return _obj;
	}

	template<typename Class>
	void singleton_c<Class>::release()
	{
		if(nullptr != _obj)
		{
			delete _obj;
			_obj = nullptr;
		}
	}
}

#endif
