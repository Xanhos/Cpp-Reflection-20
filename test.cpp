#include "Reflection.h"

enum class Test
{
    a,b,c = 454156
};

REFLECT_ENUM(Test,
    ENUM_FIELD(a),
    ENUM_FIELD(b),
    ENUM_FIELD(c)
    );

using Type = Test;


int main()
{
    auto t = Reflection::EnumInfo<Test>::fields;

    auto EnumName = Reflection::GetEnumName(Test::c);
    std::cout <<  EnumName << "\n";
    std::cout << (int)Reflection::GetEnumValue<Test>(EnumName) << "\n";

    for (auto enum_name: Reflection::GetAllEnumNames<Test>())
    {
        std::cout << enum_name << "\n";
    }
    for (auto enum_value : Reflection::GetAllEnumValue<Test>())
    {
        std::cout << (int)enum_value << "\n";

    }

}