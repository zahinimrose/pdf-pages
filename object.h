#include <vector>
#include <string>
#include <unordered_map>

typedef std::vector<char> Data;

class Object {
    bool indirect;
};

class Data_object : public Object {
    Data data;
    Data_object(Data data) : data(data) {};
};

class Name_object: public Data_object {
    std::string name;
};

class Ref_object: public Object {};

class Array_object: public Ref_object {
    std::vector<Object> list;
};

class Dict_object : public Ref_object {
    std::unordered_map<Name_object, Object> map;
};

class Stream_object: public Ref_object {
    Dict_object dict;
    Data data;
};