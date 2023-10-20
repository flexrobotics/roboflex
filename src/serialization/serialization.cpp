#include <iostream>

template <typename T>
concept DataUnitConcept = requires (T v) {
    { v.is_bool() } -> std::convertible_to<bool>;
    { v.is_uint8() } -> std::convertible_to<bool>;

    { v.as_bool() } -> std::convertible_to<bool>;
    { v.as_uint8() } -> std::same_as<uint8_t>;
};

class DataUnit {
    
};

class DataUnitNull: public DataUnit {
public:
    bool is_bool() const { return false; }
    bool is_uint8() const { return false; }

    bool as_bool() const { return false; }
    uint8_t as_uint8() const { return 0; }
};

class DataUnitFlexBuffers: public DataUnit {
public:
    bool is_bool() const { return false; }
    bool is_uint8() const { return false; }

    bool as_bool() const { return false; }
    uint8_t as_uint8() const { return 0; }
};

class DataUnitCBOR: public DataUnit {
public:
    bool is_bool() const { return false; }
    bool is_uint8() const { return false; }

    bool as_bool() const { return false; }
    uint8_t as_uint8() const { return 0; }
};





template <DataUnitConcept T=DataUnitFlexBuffers>
class MsgBase {
public:
    MsgBase(T data) : _data(data) {}
protected:
    T _data;
};

template <DataUnitConcept T=DataUnitFlexBuffers>
class ImageTensorMessage: public MsgBase<T> {
public:
    ImageTensorMessage(T data) : MsgBase<T>(data) {}
    bool is_depth_image() { return this->_data.is_bool(); }
};




class Interpreter {
public:
    bool is_bool() const { return false; }
    bool is_uint8() const { return false; }

    bool as_bool() const { return false; }
    uint8_t as_uint8() const { return 0; }
};



class MsgBase2 {
public:
    MsgBase2(Interpreter i): _i(i) {}
protected:
    Interpreter _i;
};

class ImageTensorMessage2: public MsgBase2 {
public:
    ImageTensorMessage2(Interpreter i) : MsgBase2(i) {}
    bool is_depth_image() { return this->_i.is_bool(); }
};

// using MsgNull = MsgBase<DataUnitNull>;
// using MsgFlex = MsgBase<DataUnitFlexBuffers>;
// using MsgCBOR = MsgBase<DataUnitCBOR>;

// DataUnitConcept f() {
//     return DataUnitFlexBuffers();
// }

template <DataUnitConcept T>
MsgBase<T> Msg(T data) {
    return MsgBase<T>(data);
}

int main() {
    auto k = Msg(DataUnitFlexBuffers());
    std::cout << "DONE\n";
    return 0;
}

// #include <string>
// #include <variant>
// #include <iostream>

// // Abstract template class using CRTP
// template <typename Derived>
// class MapInterface {
// public:
//     // Get an object based on the key
//     template <typename Key>
//     auto get(const Key& key) const -> decltype(static_cast<const Derived*>(this)->getImpl(key)) {
//         return static_cast<const Derived*>(this)->getImpl(key);
//     }

//     // Pure virtual function for getting an object based on the key (to be implemented in the derived class)
//     template <typename Key>
//     virtual auto getImpl(const Key& key) const -> decltype(getImpl(key)) = 0;
// };

// // Example derived class implementing the map-like interface
// class MyMap : public MapInterface<MyMap> {
// public:
//     template <typename Key>
//     auto getImpl(const Key& key) const -> decltype(getImpl(key)) override {
//         // Implement the object retrieval based on the key
//         if constexpr (std::is_same_v<Key, std::string>) {
//             if (key == "integer") {
//                 return 42;  // Return an integer
//             }
//             else if (key == "float") {
//                 return 3.14f;  // Return a float
//             }
//             else if (key == "string") {
//                 return std::string("Hello");  // Return a string
//             }
//             else if (key == "char_array") {
//                 return "World";  // Return an unsigned character array (C-string)
//             }
//         }
//         // Handle additional key types and objects as needed

//         // Default case: return an empty object
//         return decltype(getImpl(key)){};
//     }
// };

// int main() {
//     MyMap myMap;
//     int intValue = myMap.get("integer");
//     float floatValue = myMap.get("float");
//     std::string stringValue = myMap.get("string");
//     const char* charArrayValue = myMap.get("char_array");

//     std::cout << "Integer: " << intValue << std::endl;
//     std::cout << "Float: " << floatValue << std::endl;
//     std::cout << "String: " << stringValue << std::endl;
//     std::cout << "Character Array: " << charArrayValue << std::endl;

//     return 0;
// }
