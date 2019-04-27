// variant.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <typeindex>
#include<functional>
#include<type_traits>
//template<typename T>
//struct function_traits :public function_traits<decltype(&T::operator())> {};
//template<typename classType,typename ReturnType,typename... Args>
//struct function_traits<ReturnType(classType::*)(Args...) const> {
//	enum {arity=sizeof...(Args) };
//	typedef std::function<ReturnType(Args...)> FunType;
//	typedef ReturnType result_type;
//	typedef std::tuple<Args...> ArgTupleType;
//	template<size_t i>
//	struct arg {
//		typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
//
//	};
//
//};

template<typename F>
struct function_traits;

template<typename Ret, typename... Args>
struct function_traits<Ret(Args...)>
{
public:
	enum { arity = sizeof...(Args) };
	typedef Ret function_type(Args...);
	typedef Ret return_type;
	using stl_function_type = std::function<function_type>;
	typedef Ret(*pointer)(Args...);

	template<size_t I>
	struct args
	{
		static_assert(I < arity, "index is out of range, index must less than sizeof Args");
		using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
	};
};

//����ָ��
template<typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

//std::function
template <typename Ret, typename... Args>
struct function_traits<std::function<Ret(Args...)>> : function_traits<Ret(Args...)> {};

//member function
#define FUNCTION_TRAITS(...) \
    template <typename ReturnType, typename ClassType, typename... Args>\
    struct function_traits<ReturnType(ClassType::*)(Args...) __VA_ARGS__> : function_traits<ReturnType(Args...)>{}; \

FUNCTION_TRAITS()
FUNCTION_TRAITS(const)
FUNCTION_TRAITS(volatile)
FUNCTION_TRAITS(const volatile)

template<typename Callable>
struct function_traits :function_traits<decltype(&Callable::operator())> {};


//
template<size_t arg,size_t... rest>
struct IntegerMax :public std::integral_constant<size_t, (arg>(IntegerMax<rest...>::value ))? arg : IntegerMax<rest... >::value> {};

template<size_t arg>
struct IntegerMax<arg>:public std::integral_constant<size_t, arg> {};

//获取最大对齐数
template<typename... Args>
struct MaxAlign :std::integral_constant<size_t, IntegerMax<std::alignment_of<Args>::value...>::value> {};
//是否包含某个类型
template<typename T,typename... List>
struct Contains :std::true_type {};
template<typename T,typename Head,typename... Rest>
struct Contains<T,Head,Rest...> : std::conditional<std::is_same<T, Head>::value, std::true_type, Contains<T, Rest...>>::type {};
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
struct IndexType<Index, First, Rest...> :public IndexType<Index - 1, Rest...> {};
template<typename Type,typename... Rest>
struct IndexType<0, Type, Rest...> {
	typedef Type DataType;

};

template<typename... Args>
struct VariantHepler;
template<typename T,typename... Args>
struct VariantHepler<T, Args...> {
	inline static void Destory(std::type_index id, void * data) {
		if (id == type_index(typeid(T))) {
			reinterpret_cast<T*>(data)->~T();
		}
		else {
			VariantHepler<Args...>::Destory(id, data);
		}
	}
	inline static void move(std::type_index old_t,void *old_v,void * new_v) {
		if (old_t==type_index(typeid(T))) {
			new (new_v)T(std::move(*reinterpret_cast<T*>(old_v)));

		}
		else {
			VariantHepler<Args...>::move(old_t, old_v, new_v);
		}
	}
	inline static void copy(std::type_index old_t, const void * old_v, void * new_v) {
		if (old_t==type_index(typeid(T))) {

			new (new_v)T(*reinterpret_cast<const T*>(old_v));
		}
		else {
			VariantHepler<Args...>::copy(old_t, old_v, new_v);
		}
	}
	
};
template<> struct VariantHepler<> {
	inline static void Destory(std::type_index id, void * data) {
	    
	}
	inline static void move(std::type_index old_t, void* old_v, void * new_v) {}
	inline static void copy(std::type_index old_t, const void * old_v, void * new_v) {}

};

template<typename... Types>
class Variant {
	typedef VariantHepler<Types...> Helper_t;
	enum {
		data_size = IntegerMax<sizeof(Types)...>::value,
		align_size = MaxAlign<Types...>::value

	};
	using data_t = typename std::aligned_storage<data_size, align_size>::type;
public:

	template<int index>
	using IndexType = typename IndexType<index, Types...>::DataType;
	Variant(void) :m_typeIndex(typeid(void)),m_data(-1) {}

	~Variant()
	{
		Helper_t::Destory(m_typeIndex, &m_data);
	}
	Variant(Variant<Types...>&& old) :m_typeIndex(old.m_typeIndex) {
		Helper_t::move(old.m_typeIndex, &old.m_data, &m_data);
	}
	Variant(const Variant<Types...>& old) :m_typeIndex(old.m_typeIndex) {
		Helper_t::copy(old.m_typeIndex, &old.m_data, &m_data);
	}
	Variant& operator=(const Variant& old) {
		Helper_t::copy(old.m_typeIndex, &old.m_data, &m_data);
		m_typeIndex = old.m_typeIndex;
		return *this;
	}
	Variant& operator=(Variant&& old) {
		Helper_t::move(old.m_typeIndex, &old.m_typeIndex, &m_data);
		m_typeIndex = old.m_typeIndex;
		return *this;
	}
	template<class T,class=std::enable_if<Contains<typename std::remove_reference<T>::type,Types...>::value>::type>
	Variant(T&& value) :m_typeIndex(typeid(void)) {
		Helper_t::Destory(m_typeIndex, &m_data);
		typedef typename std::remove_reference<T>::type U;
		new (&m_data)U(std::forward<T>(value));
		m_typeIndex = type_index(typeid(T));

	}
	template<typename T>
	bool Is() const {
		return (m_typeIndex == type_index(typeid(T)));
	}
	bool Empty() const {
		return m_typeIndex == type_index(typeid(void));
	}

	std::type_index Type() const {
		return m_typeIndex;
	}
	template<typename T>
	typename std::decay<T>::type& Get()
	{
		using U = typename std::decay<T>::type;
		if (Is<U>()) {
			std::cout<<typeid(U).name()<<"is not defined.."<<std::endl;
		}
		return *(U*)(&m_data);

	}
	template<typename T>
	int GetIndexOf() {
		return Index<T, Types...>::value;
	}
	/*template<typename F>
	void Visit(F&& f) {
		using T = typename function_traits<F>::arg<0>::type;
		if (Is<T>()) {
			f(Get<T>());
		}
	}
	template<typename F,typename... Rest>
	void Visit(F&& f, Rest&&... rest) {
		using T = typename function_traits<F>::arg<0>::type;
		if (Is<T>()) {
			Visit(std::forward<F>(f));
		}
		else
		{
			Visit(std::forward<Rest>(rest)...);
		}

	}*/
	bool operator==(const Variant& rhs) const {
		return m_typeIndex == rhs.m_typeIndex;
	}

	bool operator<(const Variant& rhs) const {
		return m_typeIndex == rhs.m_typeIndex;
	}



private:
	data_t m_data;
	std::type_index m_typeIndex;


};
template<typename F>
typename function_traits<F>::stl_function_type to_function(F&& f) {
	return static_cast<typename function_traits<F>::stl_function_type>(std::forward<F>(f));
}

template<typename F>
typename function_traits<F>::stl_function_type to_function(const F& f) {
	return static_cast<typename function_traits<F>::stl_function_type>(std::move(f));
}


int main()
{
	typedef Variant<int, double, std::string, int> cv;


	//std::cout<<typeid(cv::IndexType<1>)<<std::endl;
	/*cv v = 10;
	int i = v.GetIndexOf<std::string>();
	v.Visit([](double i) {std::cout<<i<<std::endl; },
		[](short i) {std::cout <<i << std::endl; },
		[](int i) {std::cout<<i<<std::endl; }
		
		);
	bool empl = v.Empty();
	std::cout<<v.Type().name()<<std::endl;
*/
	
	auto f=to_function([](int i) {return i; });
	std::cout << "Hello World!\n"; 
}
