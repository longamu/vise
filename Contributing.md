## C++ Coding Style Guide
 * Functions and variable are underscore_separated_words (No CamelCase).
 * In the header file, functions should all be written on the same line, up to the 70-character limit. 
 * In the C++ implementation file, the function should have its return data type indicated on a separate line. 
 *  After a function name, the curly brace should follow on the next line, and 
the ending curly brace should be on its own line. Inside of a function, such as 
for an if statement or for loop, the beginning curly brace should come after the 
statement, while the end curly brace should be on its own line. 
```
int foo::bar(int arg1, int arg2)
{
  if(arg1 == 0) {
     ;
  }
  else {
    ;
  }
  return 0;
}
```

 * Data members of classes, both static and non-static, are named like ordinary nonmember variables, but with a trailing underscore.
```
class TableInfo {
  ...
 private:
  string table_name_;  // OK - underscore at end.
  string tablename_;   // OK.
  static Pool<TableInfo>* pool_;  // OK.
};
```

 * The names of all types — classes, structs, type aliases, enums, and type 
template parameters — have the same naming convention. Type names should start 
with a capital letter and have a capital letter for each new word. No underscores.
```
// classes and structs
class UrlTable { ...
class UrlTableTester { ...
struct UrlTableProperties { ...

// typedefs
typedef hash_map<UrlTableProperties *, string> PropertiesMap;

// using aliases
using PropertiesMap = hash_map<UrlTableProperties *, string>;

// enums
enum UrlTableErrors { ...
```

 * The names of variables (including function parameters) and data members are 
all lowercase, with underscores between words. Data members of classes (but not 
structs) additionally have trailing underscores. Variables declared constexpr 
or const, and whose value is fixed for the duration of the program, are named 
with a leading "k" followed by mixed case. For instance: a_local_variable, 
a_struct_data_member, a_class_data_member_, kDaysInWeek.
```
string table_name;  // OK - uses underscore.
string tablename;   // OK - all lowercase.

class TableInfo {
  ...
 private:
  string table_name_;  // OK - underscore at end.
  string tablename_;   // OK.
  static Pool<TableInfo>* pool_;  // OK.
};

struct UrlTableProperties {
  string name;
  int num_entries;
  static Pool<UrlTableProperties>* pool;
};

const int kDaysInAWeek = 7;
```

 * Regular functions have mixed case; accessors and mutators may be named like variables.
Ordinarily, functions should start with a capital letter and have a capital letter 
for each new word (a.k.a. "Camel Case" or "Pascal case"). Such names should not 
have underscores. Prefer to capitalize acronyms as single words (i.e. StartRpc(), not StartRPC()).
```
AddTableEntry()
DeleteUrl()
OpenFileOrDie()
```
Accessors and mutators (get and set functions) may be named like variables. 
These often correspond to actual member variables, but this is not required. 
For example, int count() and void set_count(int count).

## References
 * [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
 * [Gnuradio Style Guide](https://wiki.gnuradio.org/index.php/Coding_guide_impl)
