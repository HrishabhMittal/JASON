# JASON: A JSON parser written in C++
A Parser for JSON. Testing for this parser is done using this repo: [https://github.com/briandfoy/json-acceptance-tests.git].


# How to use
in your project include src/parser.cpp (yes i am too lazy to properly make header files)

```cpp
JsonParser jp("json as string")
Value v=jp.parseJSON();
```
```cpp
JsonParser jp(ifstream_of_file)
Value v=jp.parseJSON();
```

# To run tests
```bash
git clone https://github.com/briandfoy/json-acceptance-tests.git tests
./run.sh
```
