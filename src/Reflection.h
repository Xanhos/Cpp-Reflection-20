#pragma once
#include <optional>
#include <type_traits>
#include <any>
#include <functional>
#include <iostream>


#define ALLOW_PRIVATE_TEMPLATE_REFLECT_ACCESS(Template_) \
    template<typename T> friend struct Template_##Fields; \
    friend struct Reflection::TemplateInfo<Template_>

#define ALLOW_PRIVATE_REFLECT_ACCESS(Type_) \
	friend struct Reflection::TypeInfo<Type_>


#define REFLECT_TEMPLATE_IMPL(Template_,Base_ ,FieldGenerator_) \
template<> \
struct Reflection::TemplateInfo<Template_> { \
    static constexpr bool exist_v = true; \
    static inline constexpr auto template_name = #Template_; \
    \
    template<typename T> \
    static constexpr auto generate_fields() { \
        return FieldGenerator_<T>()(); \
    } \
}; \
\
template<typename T> \
struct Reflection::TypeInfo<Template_<T>> { \
    using Type = Template_<T>; \
	using BaseType = Base_; \
    using TemplateBase = Reflection::TemplateInfo<Template_>; \
    \
    static inline constexpr auto own_fields = TemplateBase::template generate_fields<T>();\
	static inline constexpr auto fields = Reflection::priv::get_all_fields<void>(own_fields);\
	static inline constexpr auto members = Reflection::filter_fields<std::is_member_object_pointer>(fields);\
	static inline constexpr auto methods = Reflection::filter_fields<std::is_member_function_pointer>(fields);\
	static inline constexpr auto static_members = Reflection::filter_fields<Reflection::is_object_pointer>(fields);\
	static inline constexpr auto static_methods = Reflection::filter_fields<Reflection::is_function_pointer>(fields);\
	static inline constexpr uint32_t hash_type = Reflection::hash_string(#Template_); \
	static inline constexpr auto name = #Template_;\
	static constexpr bool exist_v = true;\
}


#define REFLECT_TEMPLATE(Template_, Base_,...)\
template<typename T>\
struct Template_##Fields {\
	using Type = Template_<T>;\
	constexpr auto operator()() const {\
		return std::make_tuple(__VA_ARGS__);\
	}\
};\
REFLECT_TEMPLATE_IMPL(Template_,Base_,Template_##Fields)

#define REFLECT(Type_, Base_,...)\
template<>\
struct Reflection::TypeInfo<Reflection::get_pure_type_t<Type_>>\
{\
public:\
using Type = Type_;\
using BaseType = Base_;\
static inline constexpr auto own_fields = std::make_tuple(__VA_ARGS__);\
static inline constexpr auto fields = Reflection::priv::get_all_fields<BaseType>(own_fields);\
static inline constexpr auto members = Reflection::filter_fields<std::is_member_object_pointer>(fields); \
static inline constexpr auto methods = Reflection::filter_fields<std::is_member_function_pointer>(fields); \
static inline constexpr auto static_members = Reflection::filter_fields<Reflection::is_object_pointer>(fields); \
static inline constexpr auto static_methods = Reflection::filter_fields<Reflection::is_function_pointer>(fields); \
static inline constexpr uint32_t hash_type = Reflection::hash_string(#Type_); \
static inline constexpr auto name = #Type_; \
static constexpr bool exist_v = true; \
};


#define FIELD(Name,...) std::make_tuple(#Name, \
        Reflection::remove_member_function_qualifiers_t<decltype(&Type::Name)>(&Type::Name),\
		Reflection::hash_string(#Name),\
		Reflection::field_meta_data{}__VA_ARGS__)

#define REFLECT_ENUM(Type_, ...)\
	template<>\
	struct Reflection::EnumInfo<Type_>{\
	using Type = Type_;\
	constexpr inline static bool exist_v = true;\
	static inline constexpr auto fields = std::make_tuple(__VA_ARGS__);}

#define ENUM_FIELD(Value_, ...) std::make_tuple(#Value_,\
	static_cast<int>(Type::Value_),\
	Reflection::hash_string(#Value_),\
    Reflection::enum_meta_data{.custom_name = #Value_}__VA_ARGS__)


namespace Reflection
{


    template<template<typename> typename Template>
    struct TemplateInfo
    {
        static constexpr bool exist_v = false;
    };

    template<typename T>
    struct TypeInfo
    {
        static constexpr bool exist_v = false;
    };

    template<typename T>
    struct EnumInfo
    {
        static constexpr bool exist_v = false;
    };

    using hash32 = uint32_t;


    static constexpr hash32 hash_string(const char *str)
    {
        hash32 hash = 2166136261u;
        while (*str)
        {
            hash ^= static_cast<hash32>(*str++);
            hash *= 16777619u;
        }
        return hash;
    }

    template<typename EnumType_>
    concept IsReflectedEnum = std::is_enum_v<EnumType_> && EnumInfo<EnumType_>::exist_v;

    template<typename ReflectedType_>
    concept IsReflectedType = TypeInfo<ReflectedType_>::exist_v;


    namespace priv
    {
        template<typename T>
        constexpr auto get_all_fields(auto own_fields)
        {
            using Parent = T;
            if constexpr (!std::is_void_v<Parent> && TypeInfo<Parent>::exist_v)
            {
                return std::tuple_cat(TypeInfo<Parent>::fields, own_fields);
            } else
            {
                return own_fields;
            }
        }

        template <size_t N>
        struct FixedString {
            char data[N]{};

            constexpr const char* c_str() const { return data; }

            constexpr operator std::string_view() const { return std::string_view(data); }
        };

        constexpr bool is_same_string(std::string_view string_one, std::string_view string_two)
        {
            return string_one == string_two;
        }
    }


    template<typename T>
    using get_clean_type_t = std::remove_cvref_t<T>;

    template<typename T>
    using get_pure_type_t = std::remove_pointer_t<get_clean_type_t<T> >;

    template<typename T>
    struct TypeInfo<const T> : TypeInfo<T>
    {
    };

    template<typename T>
    struct TypeInfo<const T &> : TypeInfo<T>
    {
    };

    template<typename T>
    struct TypeInfo<T &> : TypeInfo<T>
    {
    };

    template<typename T>
    struct TypeInfo<T *> : TypeInfo<T>
    {
    };


    struct field_meta_data
    {
    public:
        struct slider_meta
        {
            float min = 0.f;
            float max = 1.f;
        };

        slider_meta slider;
        bool serializable = true;
        const char *tooltip = nullptr;
        const char *category = nullptr;

        constexpr field_meta_data &set_slider(float min, float max)
        {
            slider = {.min = min, .max = max};
            return *this;
        }

        constexpr field_meta_data &set_tooltip(const char *t)
        {
            tooltip = t;
            return *this;
        }

        constexpr field_meta_data &set_category(const char *c)
        {
            category = c;
            return *this;
        }

        constexpr field_meta_data &set_serializable(bool s)
        {
            serializable = s;
            return *this;
        }
    };

    struct enum_meta_data
    {
        const char* custom_name = "";
        hash32 custom_name_hash = hash_string("");

        constexpr enum_meta_data& set_custom_name(const char* name)
        {
            custom_name = name;
            custom_name_hash = hash_string(name);
            return *this;
        }
     };

    static inline field_meta_data invalid_meta_data{};



    enum class fields_getter_info : uint8_t
    {
        name = 0,
        pointer = 1,
        hash_name = 2,
        meta_data = 3
    };

    enum class enum_fields_getter_info : uint8_t
    {
        name = 0,
        value = 1,
        hash_name = 2,
        meta_data = 3
    };

    template<fields_getter_info Index, typename Tuple>
    constexpr decltype(auto) Get(Tuple &&tuple) noexcept
    {
        return std::get<static_cast<size_t>(Index)>(std::forward<Tuple>(tuple));
    }

    template<enum_fields_getter_info Index, typename Tuple>
    constexpr decltype(auto) Get(Tuple &&tuple) noexcept
    {
        return std::get<static_cast<size_t>(Index)>(std::forward<Tuple>(tuple));
    }


    template<typename T>
    struct has_arguments : std::false_type
    {
    };

    template<typename Ret>
    struct has_arguments<Ret()> : std::false_type
    {
    };

    template<typename Ret, typename... Args>
    struct has_arguments<Ret(Args...)> : std::bool_constant<(sizeof...(Args) > 0)>
    {
    };

    template<typename T>
    inline constexpr bool has_arguments_v = has_arguments<T>::value;

    template<typename T>
    struct argument_count;

    template<typename Ret, typename... Args>
    struct argument_count<Ret(Args...)>
    {
        static constexpr size_t value = sizeof...(Args);
    };

    template<typename T>
    inline constexpr size_t argument_count_v = argument_count<T>::value;

    template<typename T>
    inline constexpr bool takes_void_v = !has_arguments_v<T>;

    template<typename T>
    using is_object_pointer = std::conjunction<std::is_pointer<T>, std::is_object<std::remove_pointer_t<T> > >;


    template<typename T>
    struct remove_member_function_qualifiers
    {
        using type = T;
    };

    template<typename Class, typename Ret, typename... Args>
    struct remove_member_function_qualifiers<Ret(Class::*)(Args...) const>
    {
        using type = Ret(Class::*)(Args...);
    };

    template<typename Class, typename Ret, typename... Args>
    struct remove_member_function_qualifiers<Ret(Class::*)(Args...) volatile>
    {
        using type = Ret(Class::*)(Args...);
    };

    template<typename Class, typename Ret, typename... Args>
    struct remove_member_function_qualifiers<Ret(Class::*)(Args...) const volatile>
    {
        using type = Ret(Class::*)(Args...);
    };

    template<typename Class, typename Ret, typename... Args>
    struct remove_member_function_qualifiers<Ret(Class::*)(Args...) &>
    {
        using type = Ret(Class::*)(Args...);
    };

    template<typename Class, typename Ret, typename... Args>
    struct remove_member_function_qualifiers<Ret(Class::*)(Args...) const &>
    {
        using type = Ret(Class::*)(Args...);
    };

    template<typename Class, typename Ret, typename... Args>
    struct remove_member_function_qualifiers<Ret(Class::*)(Args...) &&>
    {
        using type = Ret(Class::*)(Args...);
    };

    template<typename Class, typename Ret, typename... Args>
    struct remove_member_function_qualifiers<Ret(Class::*)(Args...) const &&>
    {
        using type = Ret(Class::*)(Args...);
    };

    template<typename T>
    using remove_member_function_qualifiers_t = typename remove_member_function_qualifiers<T>::type;


    template<typename T>
    using is_function_pointer = std::conjunction<std::is_pointer<T>, std::is_function<std::remove_pointer_t<T> > >;

    template<typename Field>
    struct get_field_pointer_type
    {
        using type = std::decay_t<decltype(Get<fields_getter_info::pointer>(std::declval<Field>()))>;
    };

    template<typename Field>
    using get_field_pointer_type_t = typename get_field_pointer_type<Field>::type;

    template<typename Tuple, typename Indices, template<typename> class Constraint>
    struct filter_fields_impl;

    template<typename Tuple, std::size_t... Is, template<typename> class Constraint>
    struct filter_fields_impl<Tuple, std::index_sequence<Is...>, Constraint>
    {
        template<std::size_t I>
        static constexpr bool respect_constraint()
        {
            using Field = std::tuple_element_t<I, Tuple>;
            using PointerType = get_field_pointer_type_t<Field>;
            return Constraint<PointerType>::value;
        }

        template<std::size_t I>
        static constexpr auto get_if_field(const Tuple &tuple)
        {
            if constexpr (respect_constraint<I>())
            {
                return std::make_tuple(std::get<I>(tuple));
            } else
            {
                return std::make_tuple();
            }
        }

        static constexpr auto filter(const Tuple &tuple)
        {
            return std::tuple_cat(get_if_field<Is>(tuple)...);
        }
    };

    template<template<typename> class Constraint = std::is_member_object_pointer, typename Tuple>
    constexpr auto filter_fields(const Tuple &tuple)
    {
        using Indices = std::make_index_sequence<std::tuple_size_v<Tuple> >;
        return filter_fields_impl<Tuple, Indices, Constraint>::filter(tuple);
    }

    //Functor need to take 3 arguments; name, value, hash_name
    template<IsReflectedEnum T, typename F>
    constexpr void for_each_enum_fields(F &&func)
    {
        std::apply([&](auto &&... field)
        {
            (..., [&](auto &&f)
            {
                func(
                    Get<enum_fields_getter_info::name>(f),
                    Get<enum_fields_getter_info::value>(f),
                    Get<enum_fields_getter_info::hash_name>(f),
                    Get<enum_fields_getter_info::meta_data>(f)
                );
            }(field));
        }, EnumInfo<T>::fields);
    }


    //Functor need to take 4 arguments; name, value, hash_name, meta_data
    template<IsReflectedType T, typename F>
    constexpr void for_each_fields(F &&func)
    {
        std::apply([&](auto &&... field)
        {
            (..., [&](auto &&f)
            {
                func(
                    Get<fields_getter_info::name>(f),
                    Get<fields_getter_info::pointer>(f),
                    Get<fields_getter_info::hash_name>(f),
                    Get<fields_getter_info::meta_data>(f)
                );
            }(field));
        }, TypeInfo<T>::fields);
    }

    template<IsReflectedType T, typename F>
    constexpr void for_each_member(T &obj, F &&func)
    {
        std::apply([&](auto &&... field)
        {
            (..., [&](auto &&f)
            {
                func(
                    Get<fields_getter_info::name>(f),
                    obj.*Get<fields_getter_info::pointer>(f)
                );
            }(field));
        }, TypeInfo<T>::members);
    }

    template<typename Tuple, typename Predicate, typename Callback, size_t Index = 0>
    constexpr void find_index_field_in_tuple(Tuple &&t, Predicate &&pred, Callback &&callback)
    {
        if constexpr (Index < std::tuple_size_v<std::remove_reference_t<Tuple> >)
        {
            auto &element = std::get<Index>(t);
            if (pred(element))
            {
                callback(element);
            }
            return find_index_field_in_tuple<Tuple, Predicate, Callback, Index + 1>(
                std::forward<Tuple>(t),
                std::forward<Predicate>(pred),
                std::forward<Callback>(callback)
            );
        }
    }

    template<typename Tuple, typename Predicate>
    constexpr std::optional<std::any> find_pointer_in_tuple(Tuple &&t, Predicate &&pred)
    {
        std::optional<std::any> result = std::nullopt;
        find_index_field_in_tuple(std::forward<Tuple>(t), std::forward<Predicate>(pred), [&](const auto &e)
        {
            result = Get<fields_getter_info::pointer>(e);
        });

        return result;
    }

    template<IsReflectedType ObjectType>
    constexpr field_meta_data GetFieldMetaData(const char *Name)
    {
        hash32 hash_name = hash_string(Name);
        field_meta_data result = invalid_meta_data;
        auto predicate = [=](const auto &e)
        {
            return Get<fields_getter_info::hash_name>(e) == hash_name;
        };

        find_index_field_in_tuple(TypeInfo<ObjectType>::fields, predicate, [&](const auto &e)
        {
            result = Get<fields_getter_info::meta_data>(e);
        });
        return result;
    }

    template<typename Tuple>
    constexpr field_meta_data GetFieldMetaData(const char *Name, Tuple &&tuple)
    {
        hash32 hash_name = hash_string(Name);
        field_meta_data result = invalid_meta_data;
        auto predicate = [=](const auto &e)
        {
            return Get<fields_getter_info::hash_name>(e) == hash_name;
        };

        find_index_field_in_tuple(std::forward<Tuple>(tuple), predicate, [&](const auto &e)
        {
            result = Get<fields_getter_info::meta_data>(e);
        });
        return result;
    }

    template<typename MethodSignature_, IsReflectedType ObjectType_, typename ReturnType = MethodSignature_ *>
    constexpr auto GetStaticMethod(const char *Name) -> ReturnType
    {
        auto hash_wanted_name = hash_string(Name);
        auto value = find_pointer_in_tuple(TypeInfo<ObjectType_>::static_methods, [=](const auto &e)
        {
            return Get<fields_getter_info::hash_name>(e) == hash_wanted_name;
        });
        if (value)
        {
            try
            {
                auto returnValue = std::any_cast<ReturnType>(*value);
                return returnValue;
            } catch (std::bad_any_cast &exception)
            {
                std::cerr << exception.what() << " for " << Name << " static method in the class " << typeid(
                            ObjectType_).name()
                        << ". Type used for the cast : " << typeid(MethodSignature_).name() << "\n";
                return nullptr;
            }
        }
        return nullptr;
    }

    template<typename MemberType_, IsReflectedType ObjectType_, typename ReturnType = MemberType_ *>
    constexpr auto GetStaticMember(const char *Name) -> ReturnType
    {
        auto hash_wanted_name = hash_string(Name);
        auto value = find_pointer_in_tuple(TypeInfo<ObjectType_>::static_members, [=](const auto &e)
        {
            return Get<fields_getter_info::hash_name>(e) == hash_wanted_name;
        });

        if (value)
        {
            try
            {
                auto returnValue = std::any_cast<ReturnType>(*value);
                return returnValue;
            } catch (std::bad_any_cast &exception)
            {
                std::cerr << exception.what() << " for " << Name << " static member in the " << typeid(ObjectType_).
                        name()
                        << ". Type used for the cast : " << typeid(MemberType_).name() << "\n";
                return nullptr;
            }
        }
        return nullptr;
    }

    template<typename MethodSignature_, IsReflectedType ObjectType_, typename ReturnType = MethodSignature_ ObjectType_::*>
    constexpr auto GetMethod(const char *Name) -> ReturnType
    {
        auto hash_wanted_name = hash_string(Name);
        auto value = find_pointer_in_tuple(TypeInfo<ObjectType_>::methods, [=](const auto &e)
        {
            return Get<fields_getter_info::hash_name>(e) == hash_wanted_name;
        });

        if (value)
        {
            try
            {
                auto returnValue = std::any_cast<ReturnType>(*value);
                return returnValue;
            } catch (std::bad_any_cast &exception)
            {
                std::cerr << exception.what() << " for " << Name << " method in the " << typeid(ObjectType_).name()
                        << ". Type used for the cast : " << typeid(MethodSignature_).name() << "\n";
                return nullptr;
            }
        }
        return nullptr;
    }

    template<typename MemberType_, IsReflectedType ObjectType_, typename ReturnType = MemberType_ ObjectType_::*>
    ReturnType GetMember(const char *Name)
    {
        auto hash_wanted_name = hash_string(Name);
        auto value = find_pointer_in_tuple(TypeInfo<ObjectType_>::members, [=](const auto &e)
        {
            return Get<fields_getter_info::hash_name>(e) == hash_wanted_name;
        });

        if (value)
        {
            try
            {
                auto returnValue = std::any_cast<ReturnType>(*value);
                return returnValue;
            } catch (std::bad_any_cast &exception)
            {
                std::cerr << exception.what() << " for " << Name << " member in the " << typeid(ObjectType_).name()
                        << ". Type used for the cast : " << typeid(MemberType_).name() << "\n";
                return nullptr;
            }
        }
        return nullptr;
    }

    template<typename MethodSignature_, IsReflectedType ObjectType_, typename... Args>
    void InvokeMethod(ObjectType_ &Object, const char *Name, Args &&... args)
    {
        auto Method = GetMethod<MethodSignature_, ObjectType_>(Name);
        if (!Method)
        {
            throw std::exception(
                (std::string("No method named ") + Name + " found in the object " + typeid(ObjectType_).name() + ".").
                c_str());
        }

        (Object.*Method)(std::forward<Args>(args)...);
    }

    template<typename Value_, IsReflectedType Type_>
    constexpr void SetValue(Type_ &Obj, const char *Name, const Value_ &Value)
    {
        auto Member = GetMember<Value_, Type_>(Name);
        if (Member)
        {
            Obj.*Member = Value;
        }
    }


    template<typename Value_, IsReflectedType Type_>
    constexpr Value_ *GetValue(Type_ &Obj, const char *Name)
    {
        auto Member = GetMember<Value_, Type_>(Name);
        return Member ? &(Obj.*Member) : nullptr;
    }

    template<typename Type_>
    class Enable_Reflection_For_This
    {
        Type_ *GetRealType()
        {
            return reinterpret_cast<Type_ *>(this);
        }

    public:
        template<typename MemberType_>
        std::optional<std::reference_wrapper<std::decay_t<MemberType_> > > GetMember(const char *Name)
        {
            Type_ *realType = GetRealType();
            auto member = Reflection::GetMember<MemberType_, Type_>(Name);
            if (member)
            {
                return (realType->*member);
            }
            return std::nullopt;
        }

        template<typename MemberType_>
        std::optional<std::reference_wrapper<std::decay_t<MemberType_> > > GetStaticMember(const char *Name)
        {
            auto member = Reflection::GetStaticMember<MemberType_, Type_>(Name);
            if (member)
            {
                return (*member);
            }
            return std::nullopt;
        }

        template<typename MethodSignature_>
        std::function<MethodSignature_> GetMethod(const char *Name)
        {
            Type_ *realType = GetRealType();
            auto function = Reflection::GetMethod<MethodSignature_, Type_>(Name);
            if (function)
            {
                std::function<MethodSignature_> returnValue;
                if constexpr (takes_void_v<MethodSignature_>)
                {
                    returnValue = [&]
                    {
                        return (realType->*function)();
                    };
                } else
                {
                    returnValue = [&]<typename... T0>(T0 &&... args)
                    {
                        return (realType->*function)(std::forward<T0>(args)...);
                    };
                }
                return returnValue;
            }
            return std::function<MethodSignature_>();
        }

        template<typename MethodSignature_>
        std::function<MethodSignature_> GetStaticMethod(const char *Name)
        {
            auto function = Reflection::GetStaticMethod<MethodSignature_, Type_>(Name);
            if (function)
            {
                std::function<MethodSignature_> returnValue;
                if constexpr (takes_void_v<MethodSignature_>)
                {
                    returnValue = [&]
                    {
                        return function();
                    };
                } else
                {
                    returnValue = [&]<typename... T0>(T0 &&... args)
                    {
                        return function(std::forward<T0>(args)...);
                    };
                }
                return returnValue;
            }
            return std::function<MethodSignature_>();
        }
    };


    template<IsReflectedEnum EnumType_>
    constexpr const char* GetEnumName(EnumType_ EnumValue)
    {
        int ValueToSearch = static_cast<int>(EnumValue);
        {
            const char* result = nullptr;
            find_index_field_in_tuple(EnumInfo<EnumType_>::fields,
                [=](auto& element){return Get<enum_fields_getter_info::value>(element) == ValueToSearch;},
                [&](auto& element)
                {
                    result = Get<enum_fields_getter_info::meta_data>(element).custom_name;
                });
            return result;
        }
    }


    template<IsReflectedEnum EnumType_>
    constexpr const char* GetEnumTrueName(EnumType_ EnumValue)
    {
        int ValueToSearch = static_cast<int>(EnumValue);
        {
            const char* result = nullptr;
            find_index_field_in_tuple(EnumInfo<EnumType_>::fields,
                [=](auto& element){return Get<enum_fields_getter_info::value>(element) == ValueToSearch;},
                [&](auto& element)
                {
                    result = Get<enum_fields_getter_info::name>(element);
                });
            return result;
        }
    }

    template<IsReflectedEnum EnumType_>
    constexpr EnumType_ GetEnumValue(const char* Name)
    {
        auto hash_name = hash_string(Name);
        {
            EnumType_ result{0};
            find_index_field_in_tuple(EnumInfo<EnumType_>::fields,
                [=](auto& element)
                {
                    return Get<enum_fields_getter_info::hash_name>(element) == hash_name ||
                        Get<enum_fields_getter_info::meta_data>(element).custom_name_hash == hash_name;
                },
                [&](auto& element)
                {
                    result = static_cast<EnumType_>(Get<enum_fields_getter_info::value>(element));
                });
            return result;
        }
    }

    template<IsReflectedEnum EnumType_>
    constexpr std::vector<const char*> GetAllEnumNames()
    {
        std::vector<const char*> result;
        for_each_enum_fields<EnumType_>([&](auto name, auto value, auto hash_name, auto meta_data)
        {
            result.push_back(meta_data.custom_name);
        });

        return result;
    }

    template<IsReflectedEnum EnumType_>
 constexpr std::vector<const char*> GetAllEnumTrueNames()
    {
        std::vector<const char*> result;
        for_each_enum_fields<EnumType_>([&](auto name, auto value, auto hash_name, auto meta_data)
        {
            result.push_back(name);
        });

        return result;
    }

    template<IsReflectedEnum EnumType_>
    constexpr std::vector<EnumType_> GetAllEnumValue()
    {
        std::vector<EnumType_> result;
        for_each_enum_fields<EnumType_>([&](auto name, auto value, auto hash_name, auto meta_data)
        {
            result.push_back(static_cast<EnumType_>(value));
        });

        return result;
    }
}
