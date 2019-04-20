// variant.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
template<typename T>
struct function_traits :public function_traits<decltype(&T::operator())> {};









int main()
{
    std::cout << "Hello World!\n"; 
}
