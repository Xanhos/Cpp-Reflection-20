#include "Reflection.h"

enum class Test
{
    a,b,c = 454156
};

REFLECT_ENUM(Test,
    ENUM_FIELD(Test::a),
    ENUM_FIELD(Test::b),
    ENUM_FIELD(Test::c)
    );

int main()
{
    auto t = Reflection::EnumInfo<Test>::fields;

    auto EnumName = Reflection::GetEnumName(Test::c);
    std::cout <<  EnumName << "\n";
    std::cout << (int)Reflection::GetEnumValue<Test>(EnumName) << "\n";

}