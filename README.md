# Reflective RapidJSON

The main goal of this project is to provide a code generator for serializing/deserializing C++ objects to/from JSON
using Clang and RapidJSON.

However, extending the generator to generate code for other applications of reflection or to provide generic
reflection would be possible as well.

## Open for other reflection approaches
The reflection implementation used behind the scenes of this library is exchangeable:

* This repository already provides a small, additional header to use RapidJSON with Boost.Hana. This allows to
  serialize or dezerialize simple data structures declared using the `BOOST_HANA_DEFINE_STRUCT` macro rather than
  requiring the code generator.
* When native reflection becomes standardized, it would be possible to make use of it as well. In this case,
  the code generator could still act as a fallback.

## Current state
The basic functionality is implemented, tested and documented:

* serialization and deserialization of datatypes listed above
    * nesting and inheritance is possible
* basic error handling when deserializing
* CMake macro to conveniently include the code generator into the build process
* allow to use Boost.Hana

### TODOs
There are still things missing which would likely be very useful in practise. The following list contains
the most important TODOs:

* [ ] Allow to specify which member variables should be considered.
    * This could work similar to Qt's Signals & Slots
    * but there should also be a way to do this for 3rdparty types.
    * Note that currently, *all* public member variables are (de)serialized.
* [ ] Support getter/setter methods
    * [ ] Allow to serialize the result of methods.
    * [ ] Allow to pass a deserialized value to a method.
* [ ] Validate enum values when deserializing.
* [ ] Untie serialization and deserialization.

## Supported datatypes
The following table shows the mapping of supported C++ types to supported JSON types:

| C++ type                                          | JSON type    |
| ------------------------------------------------- |:------------:|
| custom structures/classes                         | object       |
| `bool`                                            | true/false   |
| signed and unsigned integral types                | number       |
| `float` and `double`                              | number       |
| `enum` and `enum class`                           | number       |
| `std::string`                                     | string       |
| `const char *`                                    | string       |
| iteratable lists (`std::vector`, `std::list`, ...)| array        |
| `std::tuple`                                      | array        |
| `std::unique_ptr`, `std::shared_ptr`              | depends/null |
| `std::map`, `std::unordered_map`                  | object       |
| `JsonSerializable`                                | object       |

### Remarks
* Raw pointer are not supported. This prevents
  forgetting to free memoery which would have to be allocated when deserializing.
* For the same reason `const char *` strings are only supported for serialization.
* Enums are (de)serialized as their underlying integer value. When deserializing, it is currently *not* checked
  whether the present integer value is a valid enumeration item.
* The JSON type for smart pointer depends on the type the pointer refers to. It can also be `null`.
* For deserialization
    * iteratables must provide an `emplace_back` method. So deserialization of eg. `std::forward_list`
      is currently not supported.
    * custom types must provide a default constructor.
    * constant member variables are skipped.
* For custom (de)serialization, see the section below.

## Usage
This example shows how the library can be used to make a `struct` serializable:
```
#include <reflective-rapidjson/json/serializable.h>

// define structures, eg.
struct TestObject : public JsonSerializable<TestObject> {
    int number;
    double number2;
    vector<int> numbers;
    string text;
    bool boolean;
};
struct NestingObject : public JsonSerializable<NestingObject> {
    string name;
    TestObject testObj;
};
struct NestingArray : public JsonSerializable<NestingArray> {
    string name;
    vector<TestObject> testObjects;
};

// serialize to JSON
NestingArray obj{ ... };
cout << "JSON: " << obj.toJson().GetString();

// deserialize from JSON
const auto obj = NestingArray::fromJson(...);

// in exactly one of the project's translation units
#include "reflection/code-defining-structs.h"
```

Note that the header included at the bottom must be generated by invoking the code generator appropriately, eg.:
```
reflective_rapidjson_generator -i "$srcdir/code-defining-structs.cpp" -o "$builddir/reflection/code-defining-structs.h"
```

#### Invoking code generator with CMake macro
It is possible to use the provided CMake macro to automate this task:
```
# find the package and make macro available
find_package(reflective-rapidjson REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${REFLECTIVE_RAPIDJSON_MODULE_DIRS})
include(ReflectionGenerator)

# "link" against reflective_rapidjson (it is a header-only lib so this will only add the required include paths to your target)
target_link_libraries(mytarget PRIVATE reflective_rapidjson)

# invoke macro
add_reflection_generator_invocation(
    INPUT_FILES code-defining-structs.cpp
    GENERATORS json
    OUTPUT_LISTS LIST_OF_GENERATED_HEADERS
    CLANG_OPTIONS_FROM_TARGETS mytarget
)
```

This will produce the file `code-defining-structs.h` in the directory `reflection` in the current build directory. So
make sure the current build directory is added to the include directories of your target. The default output directory can
also be overridden by passing `OUTPUT_DIRECTORY custom/directory` to the arguments.

It is possible to specify multiple input files at once. A separate output file is generated for each input. The output files
will always have the extension "`.h`", independently of the extension of the input file.

The full paths of the generated files are also appended to the variable `LIST_OF_GENERATED_HEADERS` which then can be added
to the sources of your target. Of course this can be skipped if not required/wanted.

For an explanation of the `CLANG_OPTIONS_FROM_TARGETS` argument, read the next section.

#### Passing Clang options
It is possible to pass additional options to the Clang tool invocation used by the code generator.
This can be done using the `--clang-opt` argument or the `CLANG_OPTIONS` argument when using the CMake macro.

It makes most sense to specify the same options for the code generator as during the actual compilation. This way the code
generator uses the same flags, defines and include directories as the compiler and hence behaves like the compiler.  
When using the CMake macro, it is possible to automatically pass all compile flags, compile definitions and include directories
from certain targets to the code generator. The targets can be specified using the `CLANG_OPTIONS_FROM_TARGETS` argument.

#### Notes regarding cross-compilation
* For cross compilation, it is required to build the code generator for the platform you're building on.
* Since the code generator is likely not required under the target platform, you should add `-DNO_GENERATOR:BOOL=ON` to the CMake
  arguments when building Reflective RapidJSON for the target platform.
* When using the `add_reflection_generator_invocation` macro, you need to set the following CMake variables:
    * `REFLECTION_GENERATOR_EXECUTABLE:FILEPATH=/path/to/executable`: path of the code generator executable to run under the platform
      you're building on
    * `REFLECTION_GENERATOR_INCLUDE_DIRECTORIES:STRING=/custom/prefix/include`: directories containing header files for target
      platform (not required if you set `CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES` anyways since it defaults to that variable)
* It is likely required to pass additional options for the target platform. For example, to cross compile with MingGW, is is
  required to add `-fdeclspec`, `-D_WIN32` and some more options (see `lib/cmake/modules/ReflectionGenerator.cmake`). The
   `add_reflection_generator_invocation` macro is supposed to take care of this, but currently only MingGW is supported.
* The Arch Linux packages mentioned at the end of the README file also include `mingw-w64` variants which give a concrete example how
  cross-compilation can be done.

### Using Boost.Hana instead of the code generator
The same example as above. However, this time Boost.Hana is used - so it doesn't require invoking the generator.

```
#include "<reflective-rapidjson/json/serializable-boosthana.h>

// define structures using BOOST_HANA_DEFINE_STRUCT, eg.
struct TestObject : public JsonSerializable<TestObject> {
    BOOST_HANA_DEFINE_STRUCT(TestObject,
        (int, number),
        (double, number2),
        (vector<int>, numbers),
        (string, text),
        (bool, boolean)
    );
};
struct NestingObject : public JsonSerializable<NestingObject> {
    BOOST_HANA_DEFINE_STRUCT(NestingObject,
        (string, name),
        (TestObject, testObj)
    );
};
struct NestingArray : public JsonSerializable<NestingArray> {
    BOOST_HANA_DEFINE_STRUCT(NestingArray,
        (string, name),
        (vector<TestObject>, testObjects)
    );
};

// serialize to JSON
NestingArray obj{ ... };
cout << "JSON: " << obj.toJson().GetString();

// deserialize from JSON
const auto obj = NestingArray::fromJson(...);
```

So beside the `BOOST_HANA_DEFINE_STRUCT` macro, the usage remains the same.

#### Disadvantages
* Use of ugly macro required
* No context information for errors like type-mismatch available
* Inherited members not considered
* Support for enums is unlikely
* Attempt to access private members can not be prevented

### Enable reflection for 3rd party classes/structs
It is obvious that the previously shown examples do not work for classes
defined in 3rd party header files as it requires adding an additional
base class.

To work around this issue, one can use the `REFLECTIVE_RAPIDJSON_MAKE_JSON_SERIALIZABLE`
macro. It will enable the `toJson` and `fromJson` methods for the specified class
in the `ReflectiveRapidJSON::JsonReflector` namespace:

```
// somewhere in included header
struct ThridPartyStruct
{ ... };

// somewhere in own header or source file
REFLECTIVE_RAPIDJSON_MAKE_JSON_SERIALIZABLE(ThridPartyStruct)

// (de)serialization
ReflectiveRapidJSON::JsonReflector::toJson(...).GetString();
ReflectiveRapidJSON::JsonReflector::fromJson<ThridPartyStruct>("...");
```

The code generator will emit the code in the same way as if `JsonSerializable` was
used.

By the way, the functions in the `ReflectiveRapidJSON::JsonReflector` namespace can also
be used when inheriting from `JsonSerializable` (instead of the member functions).

### (De)serializing private members
By default, private members are not considered for (de)serialization. However, it is possible
to enable this by adding `friend` methods for the helper functions of Reflective RapidJSON.

To make things easier, there's a macro provided:
```
struct SomeStruct : public JsonSerializable<SomeStruct> {
    REFLECTIVE_RAPIDJSON_ENABLE_PRIVATE_MEMBERS(SomeStruct);

public:
    std::string publicMember = "will be (de)serialized anyways";

private:
    std::string privateMember = "will be (de)serialized with the help of REFLECTIVE_RAPIDJSON_ENABLE_PRIVATE_MEMBERS macro";
};
```

#### Caveats
* It will obviously not work for 3rd party structs.
* This way to allow (de)serialization of private members must be applied when using Boost.Hana
  and there are any private members present. The reason is that accessing the private members can
  currently not prevented when using Boost.Hana.

### Custom (de)serialization
Sometimes it is appropriate to implement custom (de)serialization. For instance, a
custom object representing a time value should likey be serialized as a string rather
than an object with the internal data members.

An example for such custom (de)serialization can be found in the file
`json/reflector-chronoutilities.h`. It provides (de)serialization of `DateTime` and
`TimeSpan` objects from the C++ utilities library.

### Remarks
* Static member variables are currently ignored by the generator.
* It is currently not possible to ignore a specific member.

### Further examples
* Checkout the test cases for further examples. Relevant files are in
  the directories `lib/tests` and `generator/tests`.
* There's also my
  [tag editor](https://github.com/Martchus/tageditor), which uses Reflective RapidJSON to provide
  a JSON export.
  See [json.h](https://github.com/Martchus/tageditor/blob/master/cli/json.h) and
  [mainfeatures.cpp#exportToJson](https://github.com/Martchus/tageditor/blob/master/cli/mainfeatures.cpp#L856).

## Architecture
The following diagram gives an overview about the architecture of the code generator and wrapper library
around RapidJSON:

![Architectue overview](/doc/arch.svg)

* blue: classes from LibTooling/Clang
* grey: conceivable extension or use

## Install instructions

### Dependencies
The following dependencies are required at build time. Note that Reflective RapidJSON itself
and *none* of these dependencies are required at runtime by an application which makes use of
Reflective RapidJSON.

* C++ compiler and C++ standard library supporting at least C++14
* the [CMake](https://cmake.org) build system
* LibTooling from [Clang](https://clang.llvm.org) for the code generator (optional when using
  Boost.Hana)
* [RapidJSON](https://github.com/Tencent/rapidjson) for JSON (de)serialization
* [C++ utilities](https://github.com/Martchus/cpp-utilities) for various helper functions

#### Optional
* [Boost.Hana](http://www.boost.org/doc/libs/1_65_1/libs/hana/doc/html/index.html) for using
  `BOOST_HANA_DEFINE_STRUCT` instead of code generator
* [CppUnit](https://www.freedesktop.org/wiki/Software/cppunit) for building and running the tests
* [Doxygen](http://www.doxygen.org) for generating API documentation
* [Graphviz](http://www.graphviz.org) for diagrams in the API documentation

#### Remarks
* It is not required to use CMake as build system for your own project. However, when using a
  different build system, there is no helper for adding the code generator to the build process
  provided (so far).
* I usually develop using the latest version of those dependencies. So it is recommend to get the
  the latest versions as well. I tested the following versions so far:
    * GCC 7.2.1 or Clang 5.0 as compiler
    * libstdc++ from GCC 7.2.1
    * CMake 3.10.1
    * Clang 5.0.0/5.0.1 for LibTooling
    * RapidJSON 1.1.0
    * C++ utilities 4.12
    * Boost.Hana 1.65.1 and 1.66.0
    * CppUnit 1.14.0
    * Doxygen 1.8.13
    * Graphviz 2.40.1

### How to build
#### 1. Install dependencies
Install all required dependencies. Under a typical GNU/Linux system most of these dependencies
can be installed via the package manager. Otherwise follow the links in the "Dependencies" section
above.

C++ utilities is likely not available as package. However, it is possible to build C++ utilities
together with `reflective-rapidjson` to simplify the build process. The following build script makes
use of this. (To use system C++ utilities, just skip any lines with "`c++utilities`" in the following
examples.)

#### 2. Make dependencies available

When installing (some) of the dependencies at custom locations, it is likely neccassary to tell
CMake where to find them. If you installed everything using packages provided by the system,
you can skip this step of course.

To specify custom locations, just set some environment variables before invoking CMake. This
can likely be done in your IDE settings and of course at command line. Here is a Bash example:
```
export PATH=$CUSTOM_INSTALL_PREFIX/bin:$PATH
export CMAKE_PREFIX_PATH=$CUSTOM_INSTALL_PREFIX:$CMAKE_PREFIX_PATH
export CMAKE_LIBRARY_PATH=$CUSTOM_INSTALL_PREFIX/lib:$CMAKE_LIBRARY_PATH
export CMAKE_INCLUDE_PATH=$CUSTOM_INSTALL_PREFIX/include:$CMAKE_INCLUDE_PATH
```

There are also a lot of [useful variables](https://cmake.org/Wiki/CMake_Useful_Variables)
that can be specified as CMake arguments. It is also possible to create a
[toolchain file](https://cmake.org/cmake/help/v3.10/manual/cmake-toolchains.7.html).


#### 3. Get sources, eg. using Git:
```
cd $SOURCES
git clone https://github.com/Martchus/cpp-utilities.git c++utilities
git clone https://github.com/Martchus/reflective-rapidjson.git
```

If you don't want to build the development version, just checkout the desired version tag.

#### 4. Run the build script
Here is an example for building with GNU Make:
```
cd $BUILD_DIR
# generate Makefile
cmake \
 -DCMAKE_BUILD_TYPE:STRING=Release \
 -DCMAKE_INSTALL_PREFIX:PATH="/final/install/prefix" \
 -DBUNDLED_CPP_UTILITIES_PATH:PATH="$SOURCES/c++utilities" \
 "$SOURCES/reflective-rapidjson"
# build library and generators
make
# build and run tests (optional, requires CppUnit)
make check
# build tests but do not run them (optional, requires CppUnit)
make tests
# generate API documentation (optional, reqquires Doxygen)
make apidoc
# install header files, libraries and generator
make install DESTDIR="/temporary/install/location"
```
Add eg. `-j$(nproc)` to `make` arguments for using all cores.

### Packages
I currently only provide an
[Arch Linux package](https://github.com/Martchus/PKGBUILDs/blob/master/reflective-rapidjson/git/PKGBUILD)
for the current Git version. This package shows the required dependencies and commands to build
in a plain way. So it might be useful for making Reflective RapidJSON available under other platforms,
too.