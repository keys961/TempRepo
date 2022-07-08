#ifndef CPPDEMO_REFLECT_H
#define CPPDEMO_REFLECT_H

// Get a struct field info in compile time

#include <tuple>

template<typename T>
inline constexpr auto struct_schema() {
    return std::make_tuple();
}

#define DEFINE_STRUCT_SCHEMA(Struct, ...)           \
    template<>                                      \
    inline constexpr auto struct_schema<Struct>() {  \
        using _Struct = Struct;                     \
        return std::make_tuple(__VA_ARGS__);        \
    }

#define DEFINE_STRUCT_FIELD(StructField, FieldName) \
    std::make_tuple(&StructField, FieldName)

template<typename Fn, typename Tuple, std::size_t... I>
inline constexpr void for_each_tuple(Tuple&& tuple, Fn&& fn, std::index_sequence<I...>) {
    using Expander = int[];
    Expander {0, ((void)fn(std::get<I>(std::forward<Tuple>(tuple))), 0)...};
}

template<typename Fn, typename Tuple>
inline constexpr void for_each_tuple(Tuple&& tuple, Fn&& fn) {
    for_each_tuple(std::forward<Tuple>(tuple), std::forward<Fn>(fn),
                   std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
}

template<typename T, typename Fn>
inline constexpr void for_each_field(T&& value, Fn&& fn) {
    constexpr auto schema = struct_schema<std::decay_t<T>>();
    // struct_schema = tuple of (field ref, field name)
    for_each_tuple(schema, [&value, &fn](auto &&field_schema) {
        fn(value.*(std::get<0>(std::forward<decltype(field_schema)>(field_schema))),
           (std::get<1>(std::forward<decltype(field_schema)>(field_schema))));
    });
}

struct Data {
    int a;
    std::string b;
};

/**
 * We Use this macro to specialize the struct_schema function with type Data. <br>
 * So we can fetch a tuple with a list of (field, field name) in compile-tile.
 *
 * <code>
 *  template<>
 *  inline constexpr auto struct_schema<Data>() {
 *  using _Struct = Data;
 *  return std::make_tuple(std::make_tuple(&Data::a, "a"), std::make_tuple(&Data::b, "b"));
 * }
 * </code>
 */
DEFINE_STRUCT_SCHEMA(Data,
                     DEFINE_STRUCT_FIELD(Data::a, "a"),
                     DEFINE_STRUCT_FIELD(Data::b, "b"))


#endif //CPPDEMO_REFLECT_H
