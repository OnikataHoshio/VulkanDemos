#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

#include "VulkanCommon.h"

//Template
template<typename T>
class ArrayRef {
    T* const pArray = nullptr;
    size_t count = 0;
public:
    ArrayRef() = default;
    ArrayRef(T& data) :pArray(&data), count(1) {}
    template<size_t elementCount>
    ArrayRef(T(&data)[elementCount]) : pArray(data), count(elementCount) {}
    ArrayRef(T* pData, size_t elementCount) :pArray(pData), count(elementCount) {}
    ArrayRef(std::vector<T>& data) : pArray(data.data()), count(data.size()) {}
    ArrayRef(const ArrayRef<std::remove_const_t<T>>& other) :pArray(other.data()), count(other.size()) {}
    //Getter
    T* data() const { return pArray; }
    size_t size() const { return count; }
    //Const Function
    T& operator[](size_t index) const { return pArray[index]; }
    T* begin() const { return pArray; }
    T* end() const { return pArray + count; }
    //Non-const Function
    ArrayRef& operator=(const ArrayRef&) = delete;
};


//helper
enum class FONT_COLOR
{
    Black = 0, Blue = 1, Green = 2, Cyan = 3,
    Red = 4, Magenta = 5, Brown = 6, Default = 7,
    Dark_Gray = 8, Bright_Blue = 9, Bright_Green = 10, Bright_Cyan = 11,
    Bright_Red = 12, Bright_Magenta = 13, Bright_Yellow = 14, White = 15
};

enum class OSTREAM_TYPE
{
    COUT = 0, CERR = 1
};

void COLOR_PRINT(std::string content, OSTREAM_TYPE OUT_TYPE, FONT_COLOR COLOR_TYPE);

void PrintProgressBar(int progress, int total, const char* tips = "Progress");

glm::mat4 FlipVertical(const glm::mat4& projection);

#endif // !_COMMON_UTILS_H_

