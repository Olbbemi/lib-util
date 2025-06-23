#ifndef THREAD_POOL_GTEST_CPP
#define THREAD_POOL_GTEST_CPP

#include <gtest/gtest.h>
#include <iostream>

#include "util_thread_pool.h"

/* ====================================================================== */
/* ========================== DEFINE & ENUM ============================= */
/* ====================================================================== */


/* ====================================================================== */
/* =============================== GTEST ================================ */
/* ====================================================================== */
TEST(ThreadPoolTest, MemoryAdjustInAlloc) 
{
	/*
	 */
	util::thread_pool_c thread_pool;
	thread_pool.create_pool("", 10);

	int test_data = 0;
	auto lambda = [&test_data](int data)
	{
		std::cout << "HIHI" << std::endl;
		test_data = data;
		std::cout << test_data << std::endl;
	};

	thread_pool.async_dispatch(lambda, 10);

	EXPECT_EQ(test_data, 10);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
