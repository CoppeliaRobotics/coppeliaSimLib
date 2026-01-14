#include <interfaceStackMatrix.h>
#include <interfaceStackString.h>


CInterfaceStackMatrix::CInterfaceStackMatrix(const double* matrix, size_t rows, size_t cols)
{
    _objectType = sim_stackitem_matrix;
    if (matrix != nullptr)
    {
        _matrix.resize(rows, cols, 0.0);
        _matrix.data.assign(matrix, matrix + rows * cols);
    }
    else
        _matrix.resize(1, 1, 0.0);
}

CInterfaceStackMatrix::~CInterfaceStackMatrix()
{
}

const CMatrix* CInterfaceStackMatrix::getValue() const
{
    return &_matrix;
}

void CInterfaceStackMatrix::setValue(const CMatrix* matrix)
{
    _matrix = matrix[0];
}

CInterfaceStackObject* CInterfaceStackMatrix::copyYourself() const
{
    CInterfaceStackMatrix* retVal = new CInterfaceStackMatrix(_matrix.data.data(), _matrix.rows, _matrix.cols);
    return retVal;
}

CInterfaceStackObject* CInterfaceStackMatrix::getTypeEquivalent() const
{
    std::string str("m");
    str += std::to_string(_matrix.rows);
    str += "x";
    str += std::to_string(_matrix.cols);
    CInterfaceStackString* retVal = new CInterfaceStackString(str.c_str());
    return retVal;
}

void CInterfaceStackMatrix::printContent(int spaces, std::string& buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "MATRIX(";
    buffer += std::to_string(_matrix.rows) + "*" + std::to_string(_matrix.cols) + "): ";
    for (size_t i = 0; i < _matrix.data.size(); i++)
    {
        buffer += std::to_string(_matrix.data[i]);
        if (i != _matrix.data.size() - 1)
            buffer += ", ";
    }
    buffer += "\n";
}

std::string CInterfaceStackMatrix::getObjectData(std::string& /*auxInfos*/) const
{
    std::string retVal;
    retVal.append(reinterpret_cast<const char*>(&_matrix.rows), sizeof(_matrix.rows));
    retVal.append(reinterpret_cast<const char*>(&_matrix.cols), sizeof(_matrix.cols));
    for (size_t i = 0; i < _matrix.data.size(); i++)
        retVal.append(reinterpret_cast<const char*>(&_matrix.data[i]), sizeof(double));
    return retVal;
}

void CInterfaceStackMatrix::addCborObjectData(CCbor* cborObj) const
{
    cborObj->appendMatrix(_matrix.data.data(), _matrix.rows, _matrix.cols);
}

unsigned int CInterfaceStackMatrix::createFromData(const char* data, unsigned char /*version*/, std::vector<CInterfaceStackObject*>& allCreatedObjects)
{
    allCreatedObjects.push_back(this);

    std::size_t pos = 0;
    size_t rows, cols;
    std::memcpy(&rows, data + pos, sizeof(rows));
    pos += sizeof(rows);
    std::memcpy(&cols, data + pos, sizeof(cols));
    pos += sizeof(cols);
    _matrix.resize(rows, cols, 0.0);
    size_t cnt = rows * cols;
    std::memcpy(_matrix.data.data(), data + pos, cnt * sizeof(double));
    return (unsigned int)(sizeof(rows) + sizeof(cols) + cnt * sizeof(double));
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
