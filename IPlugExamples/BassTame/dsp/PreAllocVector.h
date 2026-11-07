#pragma once

template <class DataType>
class PreAllocVector
{
public:

    PreAllocVector(const PreAllocVector&) = delete;
    PreAllocVector& operator=(const PreAllocVector&) = delete;

    PreAllocVector(size_t arraySize)
        : maxSize(arraySize)
    {
        data = new DataType[curSize];
        for (int i = 0; i < maxSize; ++i)
            data[i] = new DataType();
    }

    ~PreAllocVector()
    {
        for (int i = 0; i < maxSize; ++i)
            delete data[i];

        delete[] data;
    }

    void clear()
    {
        curSize = 0;
    }

    void resize(size_t newSize)
    {
        curSize = newSize < maxSize ? newSize : maxSize;
    }

    size_t size() const
    {
        return curSize;
    }

    DataType* begin() const
    {
        return data;
    }

    DataType* end() const
    {
        return data + curSize;
    }

    DataType& operator[](const size_t index)
    {
        return data[index];
    }

    const DataType& operator[](const size_t index) const
    {
        return data[index];
    }

    DataType& back()
    {
        return data[curSize > 0 ? curSize - 1 : 0];
    }

    const size_t maxSize;

private:

    size_t curSize = 0;
    DataType* data;
};