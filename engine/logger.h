#pragma once
#include <iostream>

#ifdef ASCII_DEBUG_MODE
#define ASCII_COUT std::cout
#define ASCII_CERR std::cerr
#define ASCII_WCERR std::wcerr
#else
#define ASCII_COUT                                                                                 \
    if (false)                                                                                     \
    std::cout
#define ASCII_CERR                                                                                 \
    if (false)                                                                                     \
    std::cerr
#define ASCII_WCERR                                                                                \
    if (false)                                                                                     \
    std::wcerr
#endif