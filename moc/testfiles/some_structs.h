#ifndef SOME_STRUCTS_H
#define SOME_STRUCTS_H

//#include <string>
#include "../../lib/reflectable.h"

namespace TestNamespace1 {

#define SOME_MACRO

struct Person : public ReflectiveRapidJSON::Reflectable<Person>
{
    SOME_MACRO
    //std::string name;
    int age;
    bool alive;
};

struct NonReflectableClass
{
    int foo;
}

struct SomeOtherNonReflectableClass : public NonReflectableClass
{
    int bar;
}

namespace TestNamespace2 {

}

#endif // SOME_STRUCTS_H
