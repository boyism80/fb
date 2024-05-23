//-------------------------------------------------------------------
//  redis-cpp
//  https://github.com/tdv/redis-cpp
//  Created:     03.2020
//  Copyright 2020 Dmitry Tkachenko (tkachenkodmitryv@gmail.com)
//  Distributed under the MIT License
//  (See accompanying file LICENSE)
//-------------------------------------------------------------------

#ifndef REDISCPP_DETAIL_CONFIG_H_
#define REDISCPP_DETAIL_CONFIG_H_

#if _WIN32
#if _MSVC_LANG < 201703L
#error "RedisCpp. Requires C++ 17 or higher."
#endif
#else
#if __cplusplus < 201703L
#error "RedisCpp. Requires C++ 17 or higher."
#endif
#endif

#endif  // !REDISCPP_DETAIL_CONFIG_H_
