// variant.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <typeindex>
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

template<typename... Args>
struct VariantHepler;
template<typename T,typename... Args>
struct VariantHepler<T, Args...> {
	inline static void Destory(type_index id, void * data) {
		if (id == type_index(typeid(T))) {
			reinterpret_cast<T*>(data)->~T();
		}
		else {
			VariantHepler<Args...>::Destory(id, data);
		}
	}
	inline static void move(type_index old_t,void *old_v,void * new_v) {
		if (old_t==type_index(typeid(T))) {
			new (new_v)T(std::move(*reinterpret_cast<T*>(old_v)));

		}
		else {
			VariantHepler<Args...>::move(old_t, old_v, new_v);
		}
	}
	inline static void copy(type_index old_t, const void * old_v, void * new_v) {
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
	using IndexType = typename IndexType<Index, Types...>::DataType;
	Variant(void) :m_typeIndex(typeid(void)),m_index(-1) {}

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
		retrun m_typeIndex == type_index(typeid(void));
	}

	type_index Type() const {
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
	template<typename F>
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

	}
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


int main()
{
	typedef Variant<int, double, std::string, int> cv;
	//std::cout<<typeid(cv::IndexType<1>)<<std::endl;
	cv v = 10;
	int i = v.GetIndexOf<std::string>();
	v.Visit([&](double i) {std::cout<<i<<std::endl; },
		[&](short i) {std::cout <<i << std::endl; },
		[](int i) {std::cout<<i<<std::endl; },
		[](std::string i) {std::cout<<i<<std::endl; }
		);
	bool empl = v.Empty();
	std::cout<<v.Type()().name()<<std::endl;

	std::cout << "Hello World!\n"; 
}
