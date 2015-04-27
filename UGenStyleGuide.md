# Developing UGen Style Guide #

## Preface ##
This style guide is intended to help those new to UGen or C++ programming in general to code in a consistent and logical manner. While some aspects of coding are discussed in great detail here with fairly stringent guidelines these are not meant to be the last word. The key to this guide is that it is a reference if you are unsure of what style to code in but you must make deacons for yourself as well. If something looks more readable a different way, do it that way. Do not get too heavily tied down to how many spaces to leave between if statements and brackets.

Try to be consistent with what you are coding. If you are modifying somebody else's code use the style they have to avoid confusion. Having a consistent style improves code readability, understandably and speed.

This guide will try to outline the main coding styles and practices used in UGen and you should aim to follow these when developing UGen to maintain a consistent code base. Code submissions may be rejected if their style is too at odds with existing code. While is will outline general C++ conventions and styles used there will also be specific areas that will act like a tutorial for developing UGens.

Above all try to have fun when programming and don't let style conformities get in the way of inventive and creative programming.

# Table of Contents #

|[Header Files](#Header_Files.md)|[The #define Guard](#Header_Files.md) [Header File Dependencies](#Header_File_Dependencies.md) [Inline Functions](#Inline_Functions.md) [Function Parameter Ordering](#Function_Parameter_Ordering.md) [Names and Order of Includes](#Names_and_Order_of_Includes.md)|
|:-------------------------------|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|[Scoping](#Scoping.md)|[Namespaces](#Namespaces.md) [Nested Classes](#Nested_Classes.md) [Nonmember, Static Member, and Global Functions](#Nonmember,_Static_Member,_and_Global_Functions.md) [Local Variables](#Local_Variables.md) [Static and Global Variables](#Static_and_Global_Variables.md)|
|[Classes](#Classes.md)|[Doing Work in Constructors](#Doing_Work_in_Constructors.md) [Default Constructors](#Default_Constructors.md) [Explicit Constructors](#Explicit_Constructors.md) [Copy Constructors](#Copy_Constructors.md) [Structs vs. Classes](#Structs_vs._Classes.md) [Inheritance](#Inheritance.md) [Multiple Inheritance](#Multiple_Inheritance.md) [Interfaces](#Interfaces.md) [Operator Overloading](#Operator_Overloading.md) [Access Control](#Access_Control.md) [Declaration Order](#Declaration_Order.md) [Write Short Functions](#Write_Short_Functions.md)|
|[Other C++ Features](#Other_C++_Features.md)|[Reference Arguments](#Reference_Arguments.md) [Function Overloading](#Function_Overloading.md) [Default Arguments](#Default_Arguments.md) [Variable-Length Arrays and alloca()](#Variable-Length_Arrays_and_alloca().md) [Friends](#Friends.md) [Exceptions](#Exceptions.md) [Run-Time Type Information (RTTI)](#Run-Time_Type_Information_(RTTI).md) [Casting](#Casting.md) [Streams](#Streams.md) [Preincrement and Predecrement](#Preincrement_and_Predecrement.md) [Use of const](#Use_of_const.md) [Integer Types](#Integer_Types.md) [64-bit Portability](#64-bit_Portability.md) [Preprocessor Macros](#Preprocessor_Macros.md) [0 and NULL](#0_and_NULL.md) [sizeof](#sizeof.md)|
|[Naming](#Naming.md)|[General Naming Rules](#General_Naming_Rules.md) [File Names](#File_Names.md) [Type Names](#Type_Names.md) [Variable Names](#Variable_Names.md) [Constant Names](#Constant_Names.md) [Function Names](#Function_Names.md) [Namespace Names](#Namespace_Names.md) [Enumerator Names](#Enumerator_Names.md) [Macro Names](#Macro_Names.md) [Exceptions to Naming Rules](#Exceptions_to_Naming_Rules.md)|
|[Comments](#Comments.md)|[Comment Style](#Comment_Style.md) [File Comments](#File_Comments.md) [Class Comments](#Class_Comments.md) [Function Comments](#Function_Comments.md) [Variable Comments](#Variable_Comments.md) [Implementation Comments](#Implementation_Comments.md) [Punctuation, Spelling and Grammar](#Punctuation,_Spelling_and_Grammar.md) [TODO Comments](#TODO_Comments.md)|
|[Formatting](#Formatting.md)|[Line Length](#Line_Length.md) [Non-ASCII Characters](#Non-ASCII_Characters.md) [Spaces vs. Tabs](#Spaces_vs._Tabs.md) [Function Declarations and Definitions](#Function_Declarations_and_Definitions.md) [Function Calls](#Function_Calls.md) [Conditionals](#Conditionals.md) [Loops and Switch Statements](#Loops_and_Switch_Statements.md) [Reference\_Expressions Pointer and Reference Expressions](#Pointer_and.md) [Boolean Expressions](#Boolean_Expressions.md) [Return Values](#Return_Values.md) [Variable and Array Initialization](#Variable_and_Array_Initialization.md) [Preprocessor Directives](#Preprocessor_Directives.md) [Class Format](#Class_Format.md) [Constructor Initializer Lists](#Constructor_Initializer_Lists.md) [Namespace Formatting](#Namespace_Formatting.md) [Horizontal Whitespace](#Horizontal_Whitespace.md) [Vertical Whitespace](#Vertical_Whitespace.md)|
|[Exceptions to the Rules](#Exceptions_to_the_Rules.md)|[Existing Non-conformant Code](#Existing_Non-conformant_Code.md)|


# Header Files #

In general, every .cpp file should have an associated .h file. There are some common exceptions, such as small classes and abstract base classes.

Correct use of header files can make a huge difference to the readability, size and performance of your code.

The following rules will guide you through the various pitfalls of using header files.

## The #define Guard ##

All header files should have `#define` guards to prevent multiple inclusion. The format of the symbol name should be `_UGEN_<FILENAME>_H_`.

Guard ends should have the associated guard symbol as a comment next to them e.g.

`#endif //_UGEN_<FILENAME>_H_`

## Header File Dependencies ##

Try to minimise the use of header file includes to reduce the amount of dependencies. This can significantly speed up build times and reduce binary file size.

To do this only include header files that are actually needed. If you are just declaring a pointer to a class and not actually calling any of its methods then use a forward declaration of the class instead. e.g. class File;

It is cleaner to include necessary header files required in both .h files and their corresponding .cpp files in just the header file. Then only the header file and the ugen\_StandardHeader.h need be included in the .cpp file. Doing this makes it easier to spot what classes the file uses and manipulates.

## Inline Functions ##

Define functions inline only when they are small, say, 10 lines or less.

Function declared inline will indicate to the compiler that they should be expanded inline rather than calling them through the usual function call mechanism. This can speed up code but may increase code size.

In general small functions, especially accessors, mutators and time critical functions such as maths ones should be declared inline

## Function Parameter Ordering ##

When defining a function, parameter order is: inputs, then outputs. In general more important parameters come first.

Parameters to C/C++ functions are either input to the function, output from the function, or both. Input parameters are usually values or const references, while output and input/output parameters will be non-const pointers. When ordering function parameters, put all input-only parameters before any output parameters. In particular, do not add new parameters to the end of the function just because they are new; place new input-only parameters before the output parameters.

## Names and Order of Includes ##

Use standard order for readability and to avoid hidden dependencies: C library, C++ library, other libraries' .h, your project's .h.

Header files should be included as relative path names to the current file using UNIX directory shortcuts (.. for the parent directory) #include "base/logging.h"


# Scoping #

## Namespaces ##

The use of namespaces is encouraged in UGen, especially around constants and stateless functions i.e. those that are not part of a class.

Try to avoid using the using directive as this pollutes the namespace. Instead when accessing function and variable within a namespace use the scope identifier e.g. Constant::pi

When creating UGens they should be within the UGen namespace. For ease of use and namespace consistency two macros defined in the ugen\_StandardHeader.h can be used around all of your code, BEGIN\_UGEN\_NAMESPACE and END\_UGEN\_NAMESPACE.

The content of a namespace should not be indented e.g.

```
// In the .h file
namespace UGEN_NAMESPACE {

// All declarations are within the namespace scope.
// Notice the lack of indentation.
class MyClass
{
public:
  ...
  void Foo();
};

}  // namespace UGEN_NAMESPACE

// In the .cpp file
namespace UGEN_NAMESPACE
{

// Definition of functions is within scope of the namespace.
void MyClass::Foo() {
...
}

}  // namespace UGEN_NAMESPACE
```

## Nested Classes ##

Nested classes are encouraged when the nested class is related only to its parent class e.g. a listener interface.

As with normal class variables a nested class's scope should be as narrow as possible. If only the parent class requires the use of it it should remain private. e.g.

```
class Foo {
private:
  // Bar is a member class, nested within Foo.
  class Bar {
    ...
  };
};
```

## Nonmember, Static Member, and Global Functions ##

Prefer nonmember functions within a namespace or static member functions to global functions; use completely global functions rarely.

Nonmember and static member functions can be useful in some situations. Putting nonmember functions in a namespace avoids polluting the global namespace.

Nonmember and static member functions may make more sense as members of a new class, especially if they access external resources or have significant dependencies.

## Local Variables ##

Place a function's variables in the narrowest scope possible, and initialise variables in the declaration.

C++ allows you to declare variables anywhere in a function. We encourage you to declare them in as local a scope as possible, and as close to the first use as possible. This makes it easier for the reader to find the declaration and see what type the variable is and what it was initialised to. In particular, initialisation should be used instead of declaration and assignment, e.g.

```
int i;
i = f();      // Bad -- initialization separate from declaration.
int j = g();  // Good -- declaration has initialization.
```

Note that gcc implements for (int i = 0; i < 10; ++i) correctly (the scope of i is only the scope of the for loop), so you can then reuse i in another for loop in the same scope. It also correctly scopes declarations in if and while statements, e.g. while (const char**p = strchr(str, '/')) str = p + 1;**

There is one caveat: if the variable is an object, its constructor is invoked every time it enters scope and is created, and its destructor is invoked every time it goes out of scope.

```
// Inefficient implementation:
for (int i = 0; i < 1000000; ++i) {
  Foo f;  // My ctor and dtor get called 1000000 times each.
  f.DoSomething(i);
}
```

It may be more efficient to declare such a variable used in a loop outside that loop:

```
Foo f;  // My ctor and dtor get called once each.
for (int i = 0; i < 1000000; ++i) {
  f.DoSomething(i);
}
```


## Static and Global Variables ##

Static or global variables of class type are forbidden: they cause hard-to-find bugs due to indeterminate order of construction and destruction. Consider refactoring or grouping these variables into a class or namespace or use a singleton instance.


# Classes #

Classes are the fundamental unit of code in C++. Naturally, we use them extensively. This section lists the main dos and don'ts you should follow when writing a class.

## Doing Work in Constructors ##

In general, constructors should merely set member variables to their initial values. Although light work can go in a constructor any complex initialisation should go in an explicit Init() method or several methods for different stages of initialisation. This makes constructor readability much simpler.

## Default Constructors ##

If your class defines member variables and has no other constructors you must define a default constructor (one that takes no arguments). It should preferably initialise the object in such a way that its internal state is consistent and valid.

The reason for this is that if you have no other constructors and do not define a default constructor, the compiler will generate one for you. This compiler generated constructor may not initialise your object sensibly.

If your class inherits from an existing class but you add no new member variables, you are not required to have a default constructor.

## Explicit Constructors ##

Use the C++ keyword explicit for constructors with one argument.

Normally, if a constructor takes one argument, it can be used as a conversion. For instance, if you define Foo::Foo(string name) and then pass a string to a function that expects a Foo, the constructor will be called to convert the string into a Foo and will pass the Foo to your function for you. This can be convenient but is also a source of trouble when things get converted and new objects created without you meaning them to. Declaring a constructor explicit prevents it from being invoked implicitly as a conversion.

## Copy Constructors ##

Provide a copy constructor and assignment operator when necessary. If a class should not be copied disable it by declaring a default copy constructor private.

## Structs vs. Classes ##

In general use a struct only for passive objects that carry data; everything else is a class.

## Inheritance ##

Composition is often more appropriate than inheritance. When using inheritance, make it public.

Definition:

When a sub-class inherits from a base class, it includes the definitions of all the data and operations that the parent base class defines. In practice, inheritance is used in two major ways in C++: implementation inheritance, in which actual code is inherited by the child, and interface inheritance, in which only method names are inherited.

Pros:

Implementation inheritance reduces code size by re-using the base class code as it specialises an existing type. Because inheritance is a compile-time declaration, you and the compiler can understand the operation and detect errors. Interface inheritance can be used to programmatically enforce that a class expose a particular API. Again, the compiler can detect errors, in this case, when a class does not define a necessary method of the API.

Cons:

For implementation inheritance, because the code implementing a sub-class is spread between the base and the sub-class, it can be more difficult to understand an implementation. The sub-class cannot override functions that are not virtual, so the sub-class cannot change implementation. The base class may also define some data members, so that specifies physical layout of the base class.
Decision:

All inheritance should be public. If you want to do private inheritance, you should be including an instance of the base class as a member instead.

Do not overuse implementation inheritance. Composition is often more appropriate. Try to restrict use of inheritance to the "is-a" case: Bar subclasses Foo if it can reasonably be said that Bar "is a kind of" Foo.

Make your destructor virtual if necessary. If your class has virtual methods, its destructor should be virtual.

Limit the use of protected to those member functions that might need to be accessed from subclasses. Note that data members should be private.

When redefining an inherited virtual function, explicitly declare it virtual in the declaration of the derived class. Rationale: If virtual is omitted, the reader has to check all ancestors of the class in question to determine if the function is virtual or not.

## Multiple Inheritance ##

Multiple inheritance should be used sparingly. Subclassing from many base classes can cause complex implementations and make bug-tracing hard. Try to keep inheritance to a minimum, sticking to inheriting mainly from pure interface classes.

## Access Control ##

Make all data members private, and provide access to them through accessor functions as needed. Typically a variable would be called foo and the accessor function foo(). You may also want a mutator function set\_foo().

The definitions of accessors are usually inlined in the header file. See also Inheritance and Function Names.

## Declaration Order ##

Use the specified order of declarations within a class: public: before private:, methods before data members (variables), etc.

Your class definition should start with its public: section, followed by its protected: section and then its private: section. If any of these sections are empty, omit them. Within each section, the declarations generally should be in the following order:

**Typedefs and Enums** Constants
**Constructors** Destructor
**Methods, including static methods** Data Members, including static data members

Do not put large method definitions inline in the class definition. Usually, only trivial or performance-critical, and very short, methods may be defined inline. See Inline Functions for more details.

## Write Short Functions ##

Prefer small and focused functions.

We recognise that long functions are sometimes appropriate, so no hard limit is placed on functions length. If a function exceeds about 40 lines, think about whether it can be broken up without harming the structure of the program.

Even if your long function works perfectly now, someone modifying it in a few months may add new behaviour. This could result in bugs that are hard to find. Keeping your functions short and simple makes it easier for other people to read and modify your code. You could find long and complicated functions when working with some code. Do not be intimidated by modifying existing code: if working with such a function proves to be difficult, you find that errors are hard to debug, or you want to use a piece of it in several different contexts, consider breaking up the function into smaller and more manageable pieces.

# Other C++ Features #

## Reference Arguments ##

Sometimes it is appropriate to pass parameters by reference. This can be used to avoid ugly syntax like (pval)++. Where possible these should be labelled const to indicate that a function will not change the variables. If it is going to change them e.g. in some geometric operation this should be clearly labelled in the documentation.

## Function Overloading ##

Use overloaded functions (including constructors) only in cases where input can be specified in different types that contain the same information. Do not use function overloading to simulate default function parameters unless inline performance is required (functions with default parameters can not be inlined).

## Default Arguments ##

Default arguments are used heavily throughout the UGen library mainly to avoid duplicate code such as overloaded functions. When using default arguments make sure that they are sensible and documented where necessary. They should initialise the function to a workable state.

One specific example of their use is when default arguments are used to simulate variable-length argument lists.

// Support up to 4 params by using a default INFINITY.
```
Buffer(double i00,
       double i01, 
       double i02 = INFINITY, 
       double i03 = INFINITY);
```

## Variable-Length Arrays and alloca() ##

For cross platform compatibility and code safety make sure any variable-length arrays are defined with a const variable.

## Friends ##

The use of friend classes is allowed but within reason. The purpose of the friend class should be closely related to what they are befriending. One example of this is when defining the internal workings of a class that will be held within many classes e.g. BufferChannelInternal.

Another common use of friend is to have a FooBuilder? class be a friend of Foo so that it can construct the inner state of Foo correctly, without exposing this state to the world.
Friends extend, but do not break, the encapsulation boundary of a class. In some cases this is better than making a member public when you want to give only one other class access to it. However, most classes should interact with other classes solely through their public members.

## Exceptions ##

We do not use C++ exceptions.

Pros:

  * Exceptions allow higher levels of an application to decide how to handle "can't happen" failures in deeply nested functions, without the obscuring and error-prone bookkeeping of error codes.
  * Exceptions are used by most other modern languages. Using them in C++ would make it more consistent with Python, Java, and the C++ that others are familiar with.
  * Some third-party C++ libraries use exceptions, and turning them off internally makes it harder to integrate with those libraries.
  * Exceptions are the only way for a constructor to fail. We can simulate this with a factory function or an Init() method, but these require heap allocation or a new "invalid" state, respectively.
  * Exceptions are really handy in testing frameworks.

Cons:

  * When you add a throw statement to an existing function, you must examine all of its transitive callers. Either they must make at least the basic exception safety guarantee, or they must never catch the exception and be happy with the program terminating as a result. For instance, if f() calls g() calls h(), and h throws an exception that f catches, g has to be careful or it may not clean up properly.
  * More generally, exceptions make the control flow of programs difficult to evaluate by looking at code: functions may return in places you don't expect. This results maintainability and debugging difficulties. You can minimize this cost via some rules on how and where exceptions can be used, but at the cost of more that a developer needs to know and understand.
  * Exception safety requires both RAII and different coding practices. Lots of supporting machinery is needed to make writing correct exception-safe code easy. Further, to avoid requiring readers to understand the entire call graph, exception-safe code must isolate logic that writes to persistent state into a "commit" phase. This will have both benefits and costs (perhaps where you're forced to obfuscate code to isolate the commit). Allowing exceptions would force us to always pay those costs even when they're not worth it.
  * Turning on exceptions adds data to each binary produced, increasing compile time (probably slightly) and possibly increasing address space pressure.
  * The availability of exceptions may encourage developers to throw them when they are not appropriate or recover from them when it's not safe to do so. For example, invalid user input should not cause exceptions to be thrown. We would need to make the style guide even longer to document these restrictions!

Decision:

On their face, the benefits of using exceptions outweigh the costs, especially in new projects. However, for existing code, the introduction of exceptions has implications on all dependent code. If exceptions can be propagated beyond a new project, it also becomes problematic to integrate the new project into existing exception-free code.

Given that UGen's existing code is not exception-tolerant, the costs of using exceptions are somewhat greater than the costs in in a new project. The conversion process would be slow and error-prone. We don't believe that the available alternatives to exceptions, such as error codes and assertions, introduce a significant burden.

## Run-Time Type Information (RTTI) ##

RTTI is used internally within UGen in order for it to work in a very end-user friendly manner however when extending UGen or using the library you should think carefully about whether this is the best method.

## Casting ##

Use C++ casts like static\_cast<>() preferably. If the long syntax make code difficult to read it is possible to use C-type casts for type conversion only.

C++ introduced a different cast system from C that distinguishes the types of cast operations. The problem with C casts is the ambiguity of the operation; sometimes you are doing a conversion (e.g., (int)3.5) and sometimes you are doing a cast (e.g., (int)"hello"); C++ casts avoid this. Additionally C++ casts are more visible when searching for them.

## Streams ##

Preincrement and Predecrement

Use suffix form (i++) of the increment and decrement operators where the return value is ignored where possible for readability. Otherwise use the prefix form to distinguish what it is doing and for the performance boost it will give.

## Use of const ##

We strongly recommend that you use const whenever it makes sense to do so.

Definition:

Declared variables and parameters can be preceded by the keyword const to indicate the variables are not changed (e.g., const int foo). Class functions can have the const qualifier to indicate the function does not change the state of the class member variables (e.g., class Foo { int Bar(char c) const; };).
Pros:

Easier for people to understand how variables are being used. Allows the compiler to do better type checking, and, conceivably, generate better code. Helps people convince themselves of program correctness because they know the functions they call are limited in how they can modify your variables. Helps people know what functions are safe to use without locks in multi-threaded programs.
Cons:

const is viral: if you pass a const variable to a function, that function must have const in its prototype (or the variable will need a const\_cast). This can be a particular problem when calling library functions.
Decision:

const variables, data members, methods and arguments add a level of compile-time type checking; it is better to detect errors as soon as possible. Therefore we strongly recommend that you use const whenever it makes sense to do so:

  * If a function does not modify an argument passed by reference or by pointer, that argument should be const.
  * Declare methods to be const whenever possible. Accessors should almost always be const. Other methods should be const if they do not modify any data members, do not call any non-const methods, and do not return a non-const pointer or non-const reference to a data member.
  * Consider making data members const whenever they do not need to be modified after construction.

However, do not go crazy with const. Something like const int const const x; is likely overkill, even if it accurately describes how const x is. Focus on what's really useful to know: in this case, const int x is probably sufficient. The mutable keyword is allowed but is unsafe when used with threads, so thread safety should be carefully considered first.
Where to put the const

Some people favour the form int const foo to const int foo. They argue that this is more readable because it's more consistent: it keeps the rule that const always follows the object it's describing. However, this consistency argument doesn't apply in this case, because the "don't go crazy" dictum eliminates most of the uses you'd have to be consistent with. Putting the const first is arguably more readable, since it follows English in putting the "adjective" (const) before the "noun" (int). That said, while we encourage putting const first, we do not require it. But be consistent with the code around you!

## Integer Types ##

Use the type of integer that is appropriate. If you know it should never go below 0 use an unsigned int as a form of self documentation. However be carful when mixing types such as comparing an unsigned int to an int. Remember that the ranges and overflow behaviour will be different to avoid this where possible.

## Preprocessor Macros ##

Be very cautious with macros. Prefer inline functions, enums, and const variables to macros. One special case is when describing platform specific code. Try and keep this code separate from standard implementations within classes though.

Macros mean that the code you see is not the same as the code the compiler sees. This can introduce unexpected behaviour, especially since macros have global scope.

One large exception to these rules are the way UGen internally uses macros to reduce code size by expanding reused code. Although this is used extensively throughout the library you should not define your own macros where another method is possible. If you can't think of another way, rethink your design.

## 0 and NULL ##

Use 0 for integers, 0.0 for reals, NULL for pointers, and '\0' for chars.

Use 0 for integers and 0.0 for reals. This is not controversial.

For pointers (address values), there is a choice between 0 and NULL. Bjarne Stroustrup prefers an unadorned 0. We prefer NULL because it looks like a pointer. In fact, some C++ compilers, such as gcc 4.1.0, provide special definitions of NULL which enable them to give useful warnings, particularly in situations where sizeof(NULL) is not equal to sizeof(0).

Use '\0' for chars. This is the correct type and also makes code more readable.

## sizeof ##

Use sizeof(varname) instead of sizeof(type) unless dealing with primitive types.

Use sizeof(varname) because it will update appropriately if the type of the variable changes. sizeof(type) may make sense in some cases, but should generally be avoided because it can fall out of sync if the variable's type changes.

```
memset(bufferData, 0, numChannels * sizeof(float*));

// or

Struct data;
memset(&data, 0, sizeof(data));
memset(&data, 0, sizeof(Struct));
```

# Naming #

The most important consistency rules are those that govern naming. The style of a name immediately informs us what sort of thing the named entity is: a type, a variable, a function, a constant, a macro, etc., without requiring us to search for the declaration of that entity. The pattern-matching engine in our brains relies a great deal on these naming rules.

Naming rules are pretty arbitrary, but we feel that consistency is the most important thing in this area. If you are editing a file, make sure you continue with its naming scheme.

The following rules govern general naming conventions however in UGen there are specific ways of naming classes depending on their job and use these will be covered under the Developing UGen wiki.

## General Naming Rules ##
Function names, variable names, and filenames should be descriptive; eschew abbreviation. Types and variables should be nouns, while functions should be "command" verbs.

#### How to Name ####
Give as descriptive a name as possible, within reason. Do not worry about saving horizontal space as it is far more important to make your code immediately understandable by a new reader. Examples of well-chosen names:
```
int numErrors;                  // Good.
int numCompletedConnections;    // Good.
```
Poorly-chosen names use ambiguous abbreviations or arbitrary characters that do not convey meaning:
```
int n;                           // Bad - meaningless.
int nerr;                        // Bad - ambiguous abbreviation.
int n_comp_conns;                // Bad - ambiguous abbreviation.
```
Type and variable names should typically be nouns: e.g., FileOpener, numErrors.
Function names should typically be imperative (that is they should be commands): e.g., openFile(), setNumErrors(). There is an exception for accessors, which, described more completely in Function Names, should be named the same as the variable they access.

#### Abbreviations ####
Do not use abbreviations unless they are extremely well known outside your project. For example:
```
// Good
// These show proper names with no abbreviations.
int numDnsConnections;  // Most people know what "DNS" stands for.
int priceCountReader;   // OK, price count. Makes sense.

// Bad!
// Abbreviations can be confusing or ambiguous outside a small group.
int wgc_connections;  // Only your group knows what this stands for.
int pc_reader;        // Lots of things can be abbreviated "pc".

///Never abbreviate by leaving out letters:
int errorCount;  // Good.
int errorCnt;    // Bad.
```

## File Names ##
Filenames should start with lowercase `ugen_` then the name of the file in camel case starting with a capital. It is usual practice to name the file after the class it contains.

Eg. `ugen_MyUsefulClass.cpp` 

C++ files should end in .cpp and header files should end in .h.

Do not use filenames that already exist in /usr/include, such as db.h.

In general, make your filenames very specific. For example, use HttpServerLogs.h rather than logs.h. A very common case is to have a pair of files called, e.g., fooBar.h and fooBar.cpp, defining a class called FooBar.

Inline functions should be included in the .h header file along with the class definition.

## Type Names ##
Type names start with a capital letter and have a capital letter for each new word, with no underscores: `MyExcitingClass`, `MyExcitingEnum`. The internal workings of a `ugen` have the same name as the interface class with `UGenInternal` appended

The names of all types — classes, structs, typedefs, and enums — have the same naming convention. Type names should start with a capital letter and have a capital letter for each new word. No underscores. For example:
```
// classes and structs
class DataRecorderUGenInternal { ...
class PlayBufUGenInternal { ...
struct AQCallbackStruct { ...

// typedefs
typedef ObjectArray<BufferReceiver*> BufferReceiverArray;

// enums
enum Inputs { Input, NumInputs };
```

## Variable Names ##
Variable names are all camel case starting with a lowercase letter. Class member variables can have trailing underscores but do not require them. For instance: `myExcitingLocalVariable`.

Although not required it is sometimes useful to prepend a letter describing the variable's data type to its name e.g. i or f. This is particularly useful if there are two variables describing the same thing e.g. buffer positions: `iBufferPosition`, `fBufferPosition`

This is usually only done for basic types where it is immediately obvious what the letter refers to.

## Constant Names ##
In general there are no fixed rules regarding constants but as with variable names a `k` could prepend the constant: `kDaysInAWeek`.

This rule can apply to compile-time constants, whether they are declared locally, globally, or as part of a class. Use a k followed by words with uppercase first letters:
`const int kDaysInAWeek = 7;`

## Function Names ##
Regular functions have camel case; accessors and mutators generally prepend the word get or set to the name of the variable: `myExcitingFunction(), myExcitingMethod(), getMyExcitingMemberVariable(), setMyExcitingMemberVariable()`.

#### Regular Functions ####
Functions should start with a lower case letter and have a capital letter for each new word. No underscores:
```
processBlock(...)
handleBuffer(...)
```

#### Accessors and Mutators ####
Accessors and mutators (get and set functions) should match the name of the variable they are getting and setting. This shows an excerpt of a class whose instance variable is numEntries.
```
class MyClass {
 public:
  ...
  int getNumEntries() const { return numEntries; }
  void setNumEntries(int numEntries) { numEntries = numEntries; }

 private:
  int numEntries;
};
```

## Namespace Names ##
Namespace names are all lower-case, and based on project names and possibly their directory structure: `ugen_project`.

See Namespaces for a discussion of namespaces and how to name them.

## Enumerator Names ##
Enumerators should be named either like types eg. camel case beginning with a capital.

```
enum Inputs
{
    Input,
    Trig,
    NumInputs
};
```

## Macro Names ##
Macros should be defined all upper case with words separated by underscores: `MY_MACRO_THAT_SCARES_SMALL_CHILDREN`.

Please see the description of macros; in general macros should not be used. However, if they are absolutely needed, then they should be named like enum value names with all capitals and underscores.
```
#define ROUND(x) ...
#define PI_ROUNDED 3.0
```

One exception to this rule is the use of Macros internally within UGen for quick class creation. These are named like Types because they create a Type eg.`DirectMulAddUGenDeclaration`.

# Comments #
Though a pain to write, comments are absolutely vital to keeping code readable. The following rules describe what you should comment and where. But remember: while comments are very important, the best code is self-documenting. Giving sensible names to types and variables is much better than using obscure names that you must then explain through comments.

When writing your comments, write for your audience: the next contributor who will need to understand your code. Be generous — the next one may be you!

## Comment Style ##
Use either the `//` or `/*` `*/` syntax, as long as you are consistent.

You can use either the `//` or the `/* */` syntax; however, `//` is much more common. Be consistent with how you comment and what style you use where.

In general use `//` for inline comments within code and `/* */` for multi-line comments above classes and methods. Try to use doxygen style comments with a title line if possible for easy documenting.
```
e.g. 
/** 	Quick despription.
	Full and complete description of what the method does.
 */
```

## File Comments ##
Start each file with a copyright notice, followed by a description of the contents of the file.

#### Legal Notice and Author Line ####
Every file should contain the following items, in order:
  * a copyright statement (for example, Copyright 2008 UGen Inc.)
  * a license boilerplate. Choose the appropriate boilerplate for the license used by the project (for example, Apache 2.0, BSD, LGPL, GPL)
  * an author line to identify the original author of the file

If you make significant changes to a file that someone else originally wrote, add yourself to the author line. This can be very helpful when another contributor has questions about the file and needs to know whom to contact about it.

#### File Contents ####
Every file should have a comment at the top, below the copyright notice and author line, that describes the contents of the file.

Generally a `.h` file will describe the classes that are declared in the file with an overview of what they are for and how they are used. A `.cpp` file should contain more information about implementation details or discussions of tricky algorithms. If you feel the implementation details or a discussion of the algorithms would be useful for someone reading the `.h`, feel free to put it there instead, but mention in the `.cpp` that the documentation is in the `.h` file.

Do not duplicate comments in both the `.h` and the `.cpp`. Duplicated comments diverge.

## Class Comments ##
Every class definition should have an accompanying comment that describes what it is for and how it should be used. It can optionally include an example.
```
/**	Iterates over the contents of a GargantuanTable.
	
	Sample usage:
    	GargantuanTable_Iterator* iter = table->NewIterator();
    	for (iter->Seek("foo"); !iter->done(); iter->Next()) {
    		process(iter->key(), iter->value());
    	}
   	delete iter;
*/
class GargantuanTable_Iterator {
  ...
};
```

If you have already described a class in detail in the comments at the top of your file feel free to simply state "See comment at top of file for a complete description", but be sure to have some sort of comment.

Document the synchronisation assumptions the class makes, if any. If an instance of the class can be accessed by multiple threads, take extra care to document the rules and invariants surrounding multithreaded use.

## Function Comments ##
Declaration comments describe use of the function; comments at the definition of a function describe operation.

#### Function Declarations ####
Every function declaration should have comments immediately preceding it that describe what the function does and how to use it. These comments should be descriptive ("Opens the file") rather than imperative ("Open the file"); the comment describes the function, it does not tell the function what to do. In general, these comments do not describe how the function performs its task. Instead, that should be left to comments in the function definition.

Types of things to mention in comments at the function declaration:
  * What the inputs and outputs are.
  * For class member functions: whether the object remembers reference arguments beyond the duration of the method call, and whether it will free them or not.
  * If the function allocates memory that the caller must free.
  * Whether any of the arguments can be `NULL`.
  * If there are any performance implications of how a function is used.
  * If the function is re-entrant. What are its synchronisation assumptions?

Here is an example:
```
/** 	Returns an iterator for this table.
	
	It is the client's
	responsibility to delete the iterator when it is done with it,
	and it must not use the iterator once the GargantuanTable object
	on which the iterator was created has been deleted.

	The iterator is initially positioned at the beginning of the table.

	This method is equivalent to:
	Iterator* iter = table->NewIterator();
	iter->Seek("");
	return iter;
	If you are going to immediately seek to another place in the
	returned iterator, it will be faster to use NewIterator()
	and avoid the extra seek.
 */
Iterator* getIterator() const;
```
However, do not be unnecessarily verbose or state the completely obvious. Notice below that it is not necessary to say "returns false otherwise" because this is implied.
```
// Returns true if the table cannot hold any more entries.
bool IsTableFull();
```
When commenting constructors and destructors, remember that the person reading your code knows what constructors and destructors are for, so comments that just say something like "destroys this object" are not useful. Document what constructors do with their arguments (for example, if they take ownership of pointers), and what cleanup the destructor does. If this is trivial, just skip the comment. It is quite common for destructors not to have a header comment.

#### Function Definitions ####
Each function definition should have a comment describing what the function does and anything tricky about how it does its job. For example, in the definition comment you might describe any coding tricks you use, give an overview of the steps you go through, or explain why you chose to implement the function in the way you did rather than using a viable alternative. For instance, you might mention why it must acquire a lock for the first half of the function but why it is not needed for the second half.

Note you should not just repeat the comments given with the function declaration, in the `.h` file or wherever. It's okay to recapitulate briefly what the function does, but the focus of the comments should be on how it does it.

## Variable Comments ##
In general the actual name of the variable should be descriptive enough to give a good idea of what the variable is used for. In certain cases, more comments are required.

#### Class Data Members ####
Each class data member (also called an instance variable or member variable) may have a comment describing what it is used for. If the variable can take sentinel values with special meanings, such as `NULL` or `-1`, document this. For example:
```
private:
 // Keeps track of the total number of entries in the table.
 // Used to ensure we do not go over the limit. -1 means
 // that we don't yet know how many entries the table has.
 int numTotalEntries;
```

#### Global Variables ####
As with data members, all global variables should have a comment describing what they are and what they are used for. For example:
```
// The total number of tests cases that we run through in this regression test.
const int kNumTestCases = 6;
```

## Implementation Comments ##
In your implementation you should have comments in tricky, non-obvious, interesting, or important parts of your code.

#### Class Data Members ####
Tricky or complicated code blocks should have comments before them. Example:
```
// Divide result by two, taking into account that x
// contains the carry from the add.
for (int i = 0; i < result->size(); i++) {
  x = (x << 8) + (*result)[i];
  (*result)[i] = x >> 1;
  x &= 1;
}
```

#### Line Comments ####
Also, lines that are non-obvious should get a comment at the end of the line. These end-of-line comments should be separated from the code by 2 spaces. Example:
```
// If we have enough memory, map the data portion too.
mapBudget = max(0, mapBudget - index->getLength());
if (mapBudget >= dataSize_ && !MapData(mapChunkBytes, mlock))
  return;  // Error already logged.
```

Note that there are both comments that describe what the code is doing, and comments that mention that an error has already been logged when the function returns.
If you have several comments on subsequent lines, it can often be more readable to line them up:
```
doSomething();                  // Comment here so the comments line up.
doSomethingElseThatIsLonger();  // Comment here so there are two spaces between
                                // the code and the comment.
{ // One space before comment when opening a new scope is allowed,
  // thus the comment lines up with the following comments and code.
  doSomethingElse();  // Two spaces before line comments normally.
}
```

#### NULL, true/false, 1, 2, 3... ####
When you pass in NULL, boolean, or literal integer values to functions, you should consider adding a comment about what they are, or make your code self-documenting by using constants. For example, compare:
```
bool success = calculateSomething(interestingValue,
                                  10,
                                  false,
                                  NULL);  // What are these arguments??
```
versus:
```
bool success = calculateSomething(interestingValue,
                                  10,     // Default base value.
                                  false,  // Not the first time we're calling this.
                                  NULL);  // No callback.
```
Or alternatively, constants or self-describing variables:
```
const int kDefaultBaseValue = 10;
const bool kFirstTimeCalling = false;
Callback *nullCallback = NULL;
bool success = calculateSomething(interesting_value,
                                  kDefaultBaseValue,
                                  kFirstTimeCalling,
                                  null_callback);
```

#### Don'ts ####
Note that you should never describe the code itself. Assume that the person reading the code knows C++ better than you do, even though he or she does not know what you are trying to do:
```
// Now go through the b array and make sure that if i occurs,
// the next element is i+1.
...        // Geez.  What a useless comment.
```

## Punctuation, Spelling and Grammar ##
Pay attention to punctuation, spelling, and grammar; it is easier to read well-written comments than badly written ones.

Comments should usually be written as complete sentences with proper capitalisation and periods at the end. Shorter comments, such as comments at the end of a line of code, can sometimes be less formal, but you should be consistent with your style. Complete sentences are more readable, and they provide some assurance that the comment is complete and not an unfinished thought.

Although it can be frustrating to have a code reviewer point out that you are using a comma when you should be using a semicolon, it is very important that source code maintain a high level of clarity and readability. Proper punctuation, spelling, and grammar help with that goal.

## TODO Comments ##
Use TODO comments for code that is temporary, a short-term solution, or good-enough but not perfect.

TODOs should include the string TODO in all caps, followed by your name, e-mail address, or other identifier in parentheses. A colon is optional. The main purpose is to have a consistent TODO format searchable by the person adding the comment (who can provide more details upon request). A TODO is not a commitment to provide the fix yourself.
```
// TODO(kl@gmail.com): Use a "*" here for concatenation operator.
// TODO(Zeke) change this to use relations.
```
If your TODO is of the form "At a future date do something" make sure that you either include a very specific date ("Fix by November 2005") or a very specific event ("Remove this code when all clients can handle XML responses.").

#### Formatting ####
Coding style and formatting are pretty arbitrary, but a project is much easier to follow if everyone uses the same style. Individuals may not agree with every aspect of the formatting rules, and some of the rules may take some getting used to, but it is important that all project contributors follow the style rules so that they can all read and understand everyone's code easily.

## Line Length ##
Each line of text in your code should be at most 80 characters long.

We recognise that this rule is controversial, but so much existing code already adheres to it, and we feel that consistency is important.

#### Pros: ####
Those who favour this rule argue that it is rude to force them to resize their windows and there is no need for anything longer. Some folks are used to having several code windows side-by-side, and thus don't have room to widen their windows in any case. People set up their work environment assuming a particular maximum window width, and 80 columns has been the traditional standard. Why change it?

#### Cons: ####
Proponents of change argue that a wider line can make code more readable. The 80-column limit is an hidebound throwback to 1960s mainframes; modern equipment has wide screens that can easily show longer lines.

#### Decision: ####
80 characters is the maximum.

Exception: if a comment line contains an example command or a literal URL longer than 80 characters, that line may be longer than 80 characters for ease of cut and paste.
Exception: an #include statement with a long path may exceed 80 columns. Try to avoid situations where this becomes necessary.
Exception: you needn't be concerned about header guards that exceed the maximum length.

## Non-ASCII Characters ##
Non-ASCII characters should be rare, and must use UTF-8 formatting.

You shouldn't hard-code user-facing text in source, even English, so use of non-ASCII characters should be rare. However, in certain cases it is appropriate to include such words in your code. For example, if your code parses data files from foreign it may be appropriate to hard-code the non-ASCII string(s) used in those data files as delimiters. More commonly, unittest code (which does not need to be localized) might contain non-ASCII strings. In such cases, you should use UTF-8, since that is an encoding understood by most tools able to handle more than just ASCII. Hex encoding is also OK, and encouraged where it enhances readability — for example, "\xEF\xBB\xBF" is the Unicode zero-width no-break space character, which would be invisible if included in the source as straight UTF-8.

## Spaces vs. Tabs ##
Tabs can be used but should be 4 spaces in width.

## Function Declarations and Definitions ##
Return type on the same line as function name, parameters on the same line if they fit.
```
Functions look like this:
ReturnType ClassName::functionName(Type parName1, Type parName2) {
  doSomething();
  ...
}
```
If you have too much text to fit on one line:
```
ReturnType ClassName::reallyLongFunctionName(Type parName1,
                                             Type parName2,
                                             Type parName3)
{
  doSomething();
  ...
}
```
or if you cannot fit even the first parameter:
```
ReturnType LongClassName::reallyReallyReallyLongFunctionName(
           Type parName1,
	   Type parName2,
	   Type parName3)
{
  doSomething();  // 4 space indent
  ...
}
```
Some points to note:
  * The return type is always on the same line as the function name.
  * The open parenthesis is always on the same line as the function name.
  * There is never a space between the function name and the open parenthesis.
  * There is never a space between the parentheses and the parameters.
  * The open curly brace is always on a new line to the last parameter.
  * The close curly brace is either on the last line by itself or (if other style rules permit) on the same line as the open curly brace.
  * All parameters should be named, with identical names in the declaration and implementation.
  * All parameters should be aligned if possible.
  * Default indentation is 4 spaces.
  * Wrapped parameters are inline with the first.
If your function is const, the const keyword should be on the same line as the last parameter:
```
// Everything in this function signature fits on a single line
ReturnType functionName(Type par) const
{
  ...
}

// This function signature requires multiple lines, but
// the const keyword is on the line with the last parameter.
ReturnType reallyLongFunctionName(Type par1,
                                  Type par2) const
{
  ...
}
```
If some parameters are unused, comment out the variable name in the function definition:
```
// Always have named parameters in interfaces.
class Shape {
 public:
  virtual void rotate(double radians) = 0;
}

// Always have named parameters in the declaration.
class Circle : public Shape
{
public:
	virtual void rotate(double radians);
}

// Comment out unused named parameters in definitions.
void Circle::rotate(double /*radians*/) {}
// Bad - if someone wants to implement later, it's not clear what the
// variable means.
void Circle::rotate(double) {}
```

## Function Calls ##
On one line if it fits; otherwise, wrap arguments at the parenthesis.

Function calls have the following format:
`bool retval = doSomething(argument1, argument2, argument3);`

If the arguments do not all fit on one line, they should be broken up onto multiple lines, with each subsequent line aligned with the first argument. Do not add spaces after the open paren or before the close paren:
```
bool retval = doSomething(averyveryveryverylongargument1,
                          argument2, argument3);
```

If the function has many arguments, consider having one per line if this makes the code more readable:
```
bool retval = doSomething(argument1,
                          argument2,
                          argument3,
                          argument4);
```
If the function signature is so long that it cannot fit within the maximum line length, you may place all arguments on subsequent lines:
```
if (...) {
  ...
  ...
  if (...) {
    doSomethingThatRequiresALongFunctionName(
        veryLongArgument1,  // 4 space indent
        argument2,
        argument3,
        argument4);
  }
```

## Conditionals ##
Prefer no spaces inside parentheses. The else keyword belongs on a new line.

There are two acceptable formats for a basic conditional statement. One includes spaces between the parentheses and the condition, and one does not.

The most common form is without spaces. Either is fine, but be consistent. If you are modifying a file, use the format that is already present. If you are writing new code, use the format that the other files in that directory or project use. If in doubt and you have no personal preference, do not add the spaces.
```
if (condition) {  // no spaces inside parentheses
	...  // 4 space indent.
}
else {  // The else goes on a new line
	...
}
```
If you prefer you may add spaces inside the parentheses:
```
if ( condition ) {  // spaces inside parentheses - rare
    ...  // 4 space indent.
}
else {  // The else goes on a new line
    ...
}
```
Note that in all cases you must have a space between the if and the open parenthesis. You must also have a space between the close parenthesis and the curly brace, if you're using one.
```
if(condition)     // Bad - space missing after IF.
if (condition){   // Bad - space missing before {.
if(condition){    // Doubly bad.
if (condition) {  // Good - proper space after IF and before {.
```
Short conditional statements may be written on two lines if this enhances readability. You may use this only when the line is brief and the statement does not use the else clause.
```
if (x == kFoo)
    return new Foo();
if (x == kBar)
    return new Bar();

// Not allowed - IF statement on one line when there is an ELSE clause
if (x) DoThis();
else DoThat();
```
In general, curly braces are not required for single-line statements, but they are allowed if you like them; conditional or loop statements with complex conditions or statements may be more readable with curly braces.
```
if (condition)
    doSomething();  // 4 space indent.

if (condition) {
    doSomething();  // 4 space indent.
}
```
However, if one part of an if-else statement uses curly braces, the other part must too:
```
// Not allowed - curly on IF but not ELSE
if (condition) {
  foo;
} else
  bar;

// Not allowed - curly on ELSE but not IF
if (condition)
  foo;
else {
  bar;
}
// Curly braces around both IF and ELSE required because
// one of the clauses used braces.
if (condition) {
  foo;
} else {
  bar;
}
```

## Loops and Switch Statements ##
Switch statements may use braces for blocks. Empty loop bodies should use `{}` or continue.

case blocks in switch statements can have curly braces or not, depending on your preference. If you do include curly braces they should be placed as shown below.

If not conditional on an enumerated value, switch statements should always have a default case (in the case of an enumerated value, the compiler will warn you if any values are not handled). If the default case should never execute, simply assert:
```
switch (var) {
  case 0: {  // 2 space indent
    ...      // 4 space indent
    break;
  }
  case 1: {
    ...
    break;
  }
  default: {
    assert(false);
  }
}
```
Empty loop bodies should use {} or continue, but not a single semicolon.
```
while (condition) {
  // Repeat test until it returns false.
}
for (int i = 0; i < kSomeNumber; ++i) {}  // Good - empty body.
while (condition) continue;  // Good - continue indicates no logic.
while (condition);  // Bad - looks like part of do/while loop.
```

## Pointer and Reference Expressions ##
No spaces around period or arrow. Pointer operators do not have trailing spaces.

The following are examples of correctly-formatted pointer and reference expressions:
```
x = *p;
p = &x;
x = r.y;
x = r->y;
```
Note that:
  * There are no spaces around the period or arrow when accessing a member.
  * Pointer operators have no space after the `*` or &.
When declaring a pointer variable or argument, you may place the asterisk adjacent to either the type or to the variable name:
```
// These are fine, space preceding.
char *c;
const string &str;

// These are fine, space following.
char* c;    // but remember to do "char* c, *d, *e, ...;"!
const string& str;
char * c;  // Bad - spaces on both sides of *
const string & str;  // Bad - spaces on both sides of &
```
You should do this consistently within a single file, so, when modifying an existing file, use the style in that file.

## Boolean Expressions ##
When you have a boolean expression that is longer than the standard line length, be consistent in how you break up the lines.

In this example, the logical AND operator is always at the end of the lines:
```
if (this_one_thing > this_other_thing &&
    a_third_thing == a_fourth_thing &&
    yet_another && last_one) {
  ...
}
```
Note that when the code wraps in this example, both of the && logical AND operators are at the end of the line. Wrapping all operators at the beginning of the line is also allowed. Feel free to insert extra parentheses judiciously, because they can be very helpful in increasing readability when used appropriately.

## Return Values ##
Do not surround the return expression with parentheses.

Return values should not have parentheses:
```
return x;  // not return(x);
```

## Variable and Array Initialisation ##
Your choice of = or () although = is preferential.

You may choose between = and (); the following are all correct:
```
int x = 3;
int x(3);
string name("Some Name");
string name = "Some Name";
```

## Preprocessor Directives ##
Preprocessor directives should not be indented but should instead start at the beginning of the line.

Even when pre-processor directives are within the body of indented code, the directives should start at the beginning of the line.
```
// Good - directives at beginning of line
  if (lopsided_score) {
#if DISASTER_PENDING      // Correct -- Starts at beginning of line
    DropEverything();
#endif
    BackToNormal();
  }
// Bad - indented directives
  if (lopsided_score) {
    #if DISASTER_PENDING  // Wrong!  The "#if" should be at beginning of line
    DropEverything();
    #endif                // Wrong!  Do not indent "#endif"
    BackToNormal();
  }
```

## Class Format ##
Sections in public, protected and private order, not indented.

The basic format for a class declaration (lacking the comments, see Class Comments for a discussion of what comments are needed) is:
```
class MyClass : public OtherClass
{
public:
    MyClass();  // Regular 4 space indent.
    explicit MyClass(int var);
    ~MyClass() {}

    void someFunction();
    void someFunctionThatDoesNothing()
    {
    }

    void setSomeVar(int var) { someVar = var; }
    int getSomeVar() const { return someVar_; }

private:
    bool someInternalFunction();

    int someVar_;
    int someOtherVar_;
};
```

Things to note:
  * Any base class names apart from the first should be on a new line.
  * The public:, protected:, and private: keywords should not be indented.
  * Except for the first instance, these keywords should be preceded by a blank line. This rule is optional in small classes.
  * Do not leave a blank line after these keywords.
  * The public section should be first, followed by the protected and finally the private section.
  * See Declaration Order for rules on ordering declarations within each of these sections.

## Constructor Initialiser Lists ##
Constructor initialiser lists should all be on a new line with subsequent lines indented four spaces.

```
// When it requires multiple lines, indent 4 spaces, putting the colon on
// the first initialiser line:
MyClass::MyClass(int var)
    : someVar(var),             // 4 space indent
      someOtherVar(var + 1)
{  // lined up
  ...
  doSomething();
  ...
}
```

## Namespace Formatting ##
The contents of namespaces are not indented.

Namespaces do not add an extra level of indentation. For example, use:
```
namespace {

void foo() {  // Correct.  No extra indentation within namespace.
  ...
}

}  // namespace
```
Do not indent within a namespace:
```
namespace {

  // Wrong.  Indented when it should not be.
  void foo() {
    ...
  }

}  // namespace
```
When declaring nested namespaces, put each namespace on its own line.
```
namespace foo {
namespace bar {
```

## Horizontal Whitespace ##
Use of horizontal whitespace depends on location. Never put trailing whitespace at the end of a line.

#### General ####
```
void f(bool b) {  // Open braces should always have a space before them or begin on a new line.
  ...
int i = 0;        // Semicolons usually have no space before them.
int x[] = { 0 };  // Spaces inside braces for array initialisation are
int x[] = {0};    // optional.  If you use them, put them on both sides!

// Spaces around the colon in inheritance and initialiser lists.
class Foo : public Bar {
public:
  // For inline function implementations, put spaces between the braces
  // and the implementation itself.
  Foo(int b) : Bar(), baz_(b) {}  // No spaces inside empty braces.
  void reset() { baz_ = 0; }  // Spaces separating braces from implementation.
  ...
```
Adding trailing whitespace can cause extra work for others editing the same file, when they merge, as can removing existing trailing whitespace. So: Don't introduce trailing whitespace. Remove it if you're already changing that line, or do it in a separate clean-up operation (preferably when no-one else is working on the file).

#### Loops and Conditionals ####
```
if(b)          // No space after the keyword in conditions and loops.
{
}
else          // Else on new line.
{
}

while(test) {}   // There is usually no space inside parentheses.
switch(i) {
for (int i = 0; i < 5; ++i) {
switch ( i ) {    // Loops and conditions may have spaces inside
if ( test ) {     // parentheses, but this is rare.  Be consistent.
for ( int i = 0; i < 5; ++i ) {
for ( ; i < 5 ; ++i) {  // For loops always have a space after the
  ...                   // semicolon, and may have a space before the
                        // semicolon.
switch(i) {
  case 1:         // No space before colon in a switch case.
    ...
  case 2: break;  // Use a space after a colon if there's code after it.
```

#### Operators ####
```
x = 0;              // Assignment operators usually have spaces around
                    // them.
x = -5;             // No spaces separating unary operators and their
++x;                // arguments.
if(x && !y)
  ...
v = w * x + y / z;  // Binary operators usually have spaces around them,
v = w*x + y/z;      // but it's okay to remove spaces around factors.
v = w * (x + z);    // Parentheses should have no spaces inside them.
```

#### Templates and Casts ####
```
vector<string> x;           // No spaces inside the angle
y = static_cast<char*>(x);  // brackets (< and >), before
                            // <, or between >( in a cast.
vector<char *> x;           // Spaces between type and pointer are
                            // okay, but be consistent.
set<list<string> > x;       // C++ requires a space in > >.
set< list<string> > x;      // You may optionally use
                            // symmetric spacing in < <.
```

## Vertical Whitespace ##
Minimize use of vertical whitespace.

This is more a principle than a rule: don't use blank lines when you don't have to. In particular, don't put more than one or two blank lines between functions, don't start or end functions with a blank line, and be discriminating with your use of blank lines inside functions.

The basic principle is: The more code that fits on one screen, the easier it is to follow and understand the control flow of the program. Of course, readability can suffer from code being too dense as well as too spread out, so use your judgement. But in general, minimise use of vertical whitespace.

Don't start or end functions with blank lines:
```
void function() {

  // Unnecessary blank lines before and after

}
```
Don't start and end blocks with blank lines either:
```
while (condition) {
  // Unnecessary blank line after

}
if (condition) {

  // Unnecessary blank line before
}
```
However, it's okay to add blank lines between a chain of if-else blocks:
```
if (condition) {
  // Some lines of code too small to move to another function,
  // followed by a blank line.

} else {
  // Another block of code
}
```

# Exceptions to the Rules #
The coding conventions described above are mandatory. However, like all good rules, these sometimes have exceptions, which we discuss here.

## Existing Non-conformant Code ##
You may diverge from the rules when dealing with code that does not conform to this style guide.

If you find yourself modifying code that was written to specifications other than those presented by this guide, you may have to diverge from these rules in order to stay consistent with the local conventions in that code. If you are in doubt about how to do this, ask the original author or the person currently responsible for the code. Remember that consistency includes local consistency, too.

# Parting Words #
Use common sense and BE CONSISTENT.
If you are editing code, take a few minutes to look at the code around you and determine its style. If they use spaces around their if clauses, you should, too. If their comments have little boxes of stars around them, make your comments have little boxes of stars around them too.
The point of having style guidelines is to have a common vocabulary of coding so people can concentrate on what you are saying, rather than on how you are saying it. We present global style rules here so people know the vocabulary. But local style is also important. If code you add to a file looks drastically different from the existing code around it, the discontinuity throws readers out of their rhythm when they go to read it. Try to avoid this.
OK, enough writing about writing code; the code itself is much more interesting. Have fun!