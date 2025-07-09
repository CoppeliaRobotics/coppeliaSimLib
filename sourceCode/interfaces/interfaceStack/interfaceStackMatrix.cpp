#include <interfaceStackMatrix.h>

CInterfaceStackMatrix::CInterfaceStackMatrix(const double* matrix, size_t rows, size_t cols)
{
    _objectType = sim_stackitem_matrix;
    _rows = 0;
    _cols = 0;
    if (matrix != nullptr)
        setValue(matrix, rows, cols);
}

CInterfaceStackMatrix::~CInterfaceStackMatrix()
{
}

const double* CInterfaceStackMatrix::getValue(size_t& rows, size_t& cols) const
{
    rows = _rows;
    cols = _cols;
    return _data.data();
}

void CInterfaceStackMatrix::setValue(const double* matrix, size_t rows, size_t cols)
{
    _data.assign(matrix, matrix + rows * cols);
    _rows = rows;
    _cols = cols;
}

CInterfaceStackObject* CInterfaceStackMatrix::copyYourself() const
{
    CInterfaceStackMatrix* retVal = new CInterfaceStackMatrix(_data.data(), _rows, _cols);
    return retVal;
}

void CInterfaceStackMatrix::printContent(int spaces, std::string& buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "MATRIX(";
    buffer += std::to_string(_rows) + "*" + std::to_string(_cols) + "): ";
    for (size_t i = 0; i < _data.size(); i++)
    {
        buffer += std::to_string(_data[i]);
        if (i != _data.size() - 1)
            buffer += ", ";
    }
    buffer += "\n";
}

std::string CInterfaceStackMatrix::getObjectData(std::string& /*auxInfos*/) const
{
    std::string retVal;
    retVal.append(reinterpret_cast<const char*>(&_rows), sizeof(_rows));
    retVal.append(reinterpret_cast<const char*>(&_cols), sizeof(_cols));
    for (size_t i = 0; i < _data.size(); i++)
        retVal.append(reinterpret_cast<const char*>(&_data[i]), sizeof(double));
    return retVal;
}

void CInterfaceStackMatrix::addCborObjectData(CCbor* cborObj) const
{
    cborObj->appendMatrix(_data.data(), _rows, _cols);
}

unsigned int CInterfaceStackMatrix::createFromData(const char* data, unsigned char /*version*/, std::vector<CInterfaceStackObject*>& allCreatedObjects)
{
    allCreatedObjects.push_back(this);

    std::size_t pos = 0;
    std::memcpy(&_rows, data + pos, sizeof(_rows));
    pos += sizeof(_rows);
    std::memcpy(&_cols, data + pos, sizeof(_cols));
    pos += sizeof(_cols);
    size_t cnt = _rows * _cols;
    _data.resize(cnt);
    std::memcpy(_data.data(), data + pos, cnt * sizeof(double));
    return (unsigned int)(sizeof(_rows) + sizeof(_cols) + cnt * sizeof(double));
}

bool CInterfaceStackMatrix::checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version)
{
    bool retVal = false;
    if (l >= 2 * sizeof(size_t) + sizeof(double))
    {
        size_t r, c;
        std::memcpy(&r, data + 0, sizeof(r));
        std::memcpy(&c, data + sizeof(r), sizeof(c));
        w = (unsigned int)(sizeof(r) + sizeof(c) + (r * c) * sizeof(double));
        retVal = l >= w;
    }
    return retVal;
}
