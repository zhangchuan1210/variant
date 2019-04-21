// variant.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
template<typename T>
struct function_traits :public function_traits<decltype(&T::operator())> {};
template<typename classType,typename ReturnType,typename... Args>
struct function_traits<ReturnType(classType::*)(Args...) const> : {
	enum {arity=sizeof...(Args) };
	typedef std::function<RetureType(Args...)> FunType;
	typedef RetureType result_type;
	typedef std::tuple<Args...> ArgTupleType;
	template<size_t i>
	struct arg {
		typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;

	};

};
//获取最大的整数
template<size_t arg,size_t... rest>
struct IntegerMax :public std::integral_constant<size_t, arg>IntegerMax<rest...>::value ? arg : IntegerMax<rest... >> {};

template<size_t arg>
struct IntegerMax<arg>:pulbic std::integral_constant<size_t, arg> {};

//获取最大对齐数
template<typename... Args>
struct MaxAlign :std::integral_constant<size_t, IntegerMax<std::alignment_of<Args>::value...>::value> {};
//是否包含某个类型
template<typename T,typename... List>
struct Contains :std::true_type {};
template<typename T,typename Head,typename... Rest>
struct Contains : std::conditional<std::is_same<T, Head>::value, true_type, Contains<T, Rest...>>::type {};
template<typename T>
struct Contains<T> :std::false_type {};
//获取第一个T的索引位置
template<typename Type,typename... Types>
struct GetLeftSize;
template<typename Type,typename First,typename... Rest>
struct GetLeftSize<Type, First, Rest...> :public GetLeftSize<Type,Rest...> {};
template<typename Type,typename... Types>
struct GetLeftSize<Type, Type, Types...> :std::integral_constant<size_t, sizeof...(Types)> {};
template<typename Type>
struct GetLeftSize<Type> :std::integral_constant<size_t, -1> {};
template<typename T,typename... Types>
struct Index :std::integral_constant<int, sizeof...(Types) - GetLeftSize<T, Types...>::value - 1> {};
//根据索引获取索引位置的类型
template<int index,typename... Types>
struct IndexType;

template<int Index,typename First,typename... Rest>
struct IndexType<Index, First, Rest...> :public struct IndexType<Index - 1, Rest...> {};
template<typename Type,typename... Rest>
struct IndexType<0, Type, Rest...> {
	typedef First DataType;

};






















int main()
{
    std::cout << "Hello World!\n"; 
}
