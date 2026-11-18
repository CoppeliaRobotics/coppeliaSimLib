#include <simInternal.h>
#include <simStrings.h>
#include <utils.h>
#include <marker.h>
#include <global.h>
#include <app.h>
#include <tt.h>
#include <meshManip.h>
#ifdef SIM_WITH_GUI
#include <markerRendering.h>
#include <guiApp.h>
#endif

CMarker::CMarker(int type /*= sim_markertype_points*/, unsigned char col[3] /*= nullptr*/, double size[3] /*= nullptr*/, int maxCnt /*= 0*/, int options /*= 0*/, float duplicateTol /*= 0.0f*/, const std::vector<float>* vertices /*= nullptr*/, const std::vector<int>* indices /*= nullptr*/, const std::vector<float>* normals /*= nullptr*/)
{
    _objectTypeStr = "marker";
    _originalObjectTypeStr = _objectTypeStr;
    _objectType = sim_sceneobject_marker;
    _localObjectSpecialProperty = 0;

    _visibilityLayer = MARKER_LAYER;
    _objectAlias = _objectTypeStr;
    _objectName_old = _objectTypeStr;
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _itemType = type;
    _itemMaxCnt = maxCnt;
    _itemOptions = options;
    _itemDuplicateTol = duplicateTol;
    _itemCol[0] = 255;
    _itemCol[1] = 255;
    _itemCol[2] = 0;
    _itemCol[3] = 255;
    if (col != nullptr)
    {
        for (size_t i = 0; i < 3; i++)
            _itemCol[i] = col[i];
    }
    _itemSize[0] = 0.005;
    _itemSize[1] = 0.005;
    _itemSize[2] = 0.005;
    if (size != nullptr)
    {
        for (size_t i = 0; i < 3; i++)
            _itemSize[i] = size[i];
    }
    if ((_itemType == sim_markertype_custom) && (vertices != nullptr) && (indices != nullptr))
    {
        _vertices = vertices[0];
        _vertices.resize(3 * (_vertices.size() / 3));
        int m = (int(_vertices.size()) / 3) - 1;
        _indices = indices[0];
        _indices.resize(3 * (_indices.size() / 3));
        size_t i = 0;
        while (i < _indices.size() / 3)
        {
            if ((_indices[3 * i + 0] > m) || (_indices[3 * i + 1] > m) || (_indices[3 * i + 2] > m))
                _indices.erase(_indices.begin() + i, _indices.begin() + i + 3);
            else
                i++;
        }
        if (normals != nullptr)
        {
            if (normals->size() == _vertices.size())
            {
                std::vector<float> vert = _vertices;
                std::vector<int> ind = _indices;
                _vertices.clear();
                _indices.clear();
                CMeshManip::generatePerTriangleCornerNormalsAndTextCoords(&vert, &ind, normals, nullptr, &_vertices, &_indices, &_normals);
            }
        }
        if (_normals.empty())
            CMeshManip::getNormals(&_vertices, &_indices, &_normals);

        /* test
        std::vector<float> vert;
        std::vector<int> ind;
        std::vector<float> norm;
        CMeshManip::generatePerVertexNormalsAndTextCoords<float>(&_vertices, &_indices, &_normals, nullptr, &vert, &ind, &norm);
        CMeshManip::generatePerTriangleCornerNormalsAndTextCoords<float>(&vert, &ind, &norm, nullptr, &_vertices, &_indices, &_normals);
        */

        for (size_t i = 0; i < _vertices.size() / 3; i++)
        {
            float xyz[3] = {_vertices[3 * i + 0], _vertices[3 * i + 1], _vertices[3 * i + 2]};
            if (i == 0)
            {
                _customItemExtentX[0] = _customItemExtentX[1] = xyz[0];
                _customItemExtentY[0] = _customItemExtentY[1] = xyz[1];
                _customItemExtentZ[0] = _customItemExtentZ[1] = xyz[2];
            }
            else
            {
                if (xyz[0] < _customItemExtentX[0])
                    _customItemExtentX[0] = xyz[0];
                if (xyz[0] > _customItemExtentX[1])
                    _customItemExtentX[1] = xyz[0];
                if (xyz[1] < _customItemExtentY[0])
                    _customItemExtentY[0] = xyz[1];
                if (xyz[1] > _customItemExtentY[1])
                    _customItemExtentY[1] = xyz[1];
                if (xyz[2] < _customItemExtentZ[0])
                    _customItemExtentZ[0] = xyz[2];
                if (xyz[2] > _customItemExtentZ[1])
                    _customItemExtentZ[1] = xyz[2];
            }
        }
    }
    else
    {
        _customItemExtentX[0] = _customItemExtentX[1] = 0.0;
        _customItemExtentY[0] = _customItemExtentY[1] = 0.0;
        _customItemExtentZ[0] = _customItemExtentZ[1] = 0.0;
    }

    _initialize();
}

int CMarker::getData(const float** pts, const float** quats, const unsigned char** rgbas, const float** sizes, const float** vertices, int* vcnt, const int** indices, int* icnt, const float** normals) const
{
    pts[0] = _pts.data();
    if (_quats.empty())
        quats[0] = nullptr;
    else
        quats[0] = _quats.data();
    if (_rgba.empty())
        rgbas[0] = nullptr;
    else
        rgbas[0] = _rgba.data();
    if (_sizes.empty())
        sizes[0] = nullptr;
    else
        sizes[0] = _sizes.data();
    vertices[0] = _vertices.data();
    vcnt[0] = int(_vertices.size());
    indices[0] = _indices.data();
    icnt[0] = int(_indices.size());
    if (_normals.empty())
        normals[0] = nullptr;
    else
        normals[0] = _normals.data();
    return int(_pts.size());
}

void CMarker::_initialize()
{
    _itemPointCnt = 1;
    if (_itemType == sim_markertype_lines)
        _itemPointCnt = 2;
    else if (_itemType == sim_markertype_triangles)
        _itemPointCnt = 3;
    _nextId = 0;
    _newItemsCnt = 0;
    _sendFullEvent = true;
    _remIds.clear();
    _ids.clear();

    _xs.clear();
    _ys.clear();
    _zs.clear();
    _itemIts.clear();

    std::vector<float> ppts;
    ppts.swap(_pts);
    std::vector<float> qqats;
    qqats.swap(_quats);
    std::vector<unsigned char> rrgba;
    rrgba.swap(_rgba);
    std::vector<float> ssizes;
    ssizes.swap(_sizes);

    addItems(&ppts, &qqats, &rrgba, &ssizes, false);
}

void CMarker::_rebuildMarkerBoundingBox()
{
    _xs.clear();
    _ys.clear();
    _zs.clear();
    _itemIts.clear();
    for (size_t i = 0; i < _ids.size(); i++)
    {
        CItemPointIts its;
        if ( (_itemType != sim_markertype_points) && (_itemType != sim_markertype_lines) && (_itemType != sim_markertype_triangles) )
        {
            if (_itemType != sim_markertype_custom)
            {
                const float* q = _quats.data() + i * 4;
                C3X3Matrix m(CQuaternion(q[3], q[0], q[1], q[2]));
                C3Vector p(_pts.data() + i * 3);
                CMultiSIt it;
                double s[3] = {_sizes[i * 3 + 0] * 0.5, _sizes[i * 3 + 1] * 0.5, _sizes[i * 3 + 2] * 0.5};
                float xm = float(fabs(m.axis[0](0)) * s[0] + fabs(m.axis[1](0)) * s[1] + fabs(m.axis[2](0)) * s[2]);
                float ym = float(fabs(m.axis[0](1)) * s[0] + fabs(m.axis[1](1)) * s[1] + fabs(m.axis[2](1)) * s[2]);
                float zm = float(fabs(m.axis[0](2)) * s[0] + fabs(m.axis[1](2)) * s[1] + fabs(m.axis[2](2)) * s[2]);
                it.itX = _xs.insert(p(0) + xm);
                it.itY = _ys.insert(p(1) + ym);
                it.itZ = _zs.insert(p(2) + zm);
                it.itnX = _xs.insert(p(0) - xm);
                it.itnY = _ys.insert(p(1) - ym);
                it.itnZ = _zs.insert(p(2) - zm);
                its.its[0] = it;
            }
            else
            { // custom, non-symmetrical items
                const float* q = _quats.data() + i * 4;
                C3X3Matrix m(CQuaternion(q[3], q[0], q[1], q[2]));
                C3Vector p(_pts.data() + i * 3);
                CMultiSIt it;
                double s[3] = {_sizes[i * 3 + 0], _sizes[i * 3 + 1], _sizes[i * 3 + 2]};

                double minX =  DBL_MAX, minY =  DBL_MAX, minZ =  DBL_MAX;
                double maxX = -DBL_MAX, maxY = -DBL_MAX, maxZ = -DBL_MAX;

                for (int ix = 0; ix < 2; ++ix)
                {
                    for (int iy = 0; iy < 2; ++iy)
                    {
                        for (int iz = 0; iz < 2; ++iz)
                        {
                            const double x = _customItemExtentX[ix] * s[0];
                            const double y = _customItemExtentY[iy] * s[1];
                            const double z = _customItemExtentZ[iz] * s[2];
                            const double wx = m.axis[0](0) * x + m.axis[1](0) * y + m.axis[2](0) * z;
                            const double wy = m.axis[0](1) * x + m.axis[1](1) * y + m.axis[2](1) * z;
                            const double wz = m.axis[0](2) * x + m.axis[1](2) * y + m.axis[2](2) * z;
                            if (wx < minX)
                                minX = wx;
                            if (wx > maxX)
                                maxX = wx;
                            if (wy < minY)
                                minY = wy;
                            if (wy > maxY)
                                maxY = wy;
                            if (wz < minZ)
                                minZ = wz;
                            if (wz > maxZ)
                                maxZ = wz;
                        }
                    }
                }
                it.itX = _xs.insert(p(0) + maxX);
                it.itY = _ys.insert(p(1) + maxY);
                it.itZ = _zs.insert(p(2) + maxZ);
                it.itnX = _xs.insert(p(0) + minX);
                it.itnY = _ys.insert(p(1) + minY);
                it.itnZ = _zs.insert(p(2) + minZ);
                its.its[0] = it;
            }
        }
        else
        { // points, lines, tubes and triangles
            for (int j = 0; j < _itemPointCnt; j++)
            {
                C3Vector p(_pts.data() + i * 3 * _itemPointCnt + j * 3);
                CMultiSIt it;
                it.itX = _xs.insert(p(0));
                it.itY = _ys.insert(p(1));
                it.itZ = _zs.insert(p(2));
                it.itnX = _xs.insert(p(0));
                it.itnY = _ys.insert(p(1));
                it.itnZ = _zs.insert(p(2));
                its.its[j] = it;
            }
        }
        _itemIts[_nextId] = its;
    }
}

void CMarker::remItems(const std::vector<int64_t>* ids)
{ // remove items based on item ID
    bool updateBB = false;
    for (size_t i = 0; i < ids->size(); i++)
    {
        int64_t id = ids->at(i);
        auto it = _itemIts.find(id);
        if (it != _itemIts.end())
        { // item exists
            // remove it from the bounding box calc struc:
            CItemPointIts& iph = it->second;
            for (int j = 0; j < _itemPointCnt; ++j)
            {
                _xs.erase(iph.its[j].itX);
                _ys.erase(iph.its[j].itY);
                _zs.erase(iph.its[j].itZ);
                _xs.erase(iph.its[j].itnX);
                _ys.erase(iph.its[j].itnY);
                _zs.erase(iph.its[j].itnZ);
            }
            _itemIts.erase(it);
            // Find and remove the item:
            size_t l = 0;
            while (_ids[l] != id)
                l++;
            int newItemsStart = int(_ids.size()) - _newItemsCnt;
            if (l < newItemsStart)
                _remIds.push_back(id); // only if not a newly added item
            else
                _newItemsCnt--;
            _pts.erase(_pts.begin() + l * 3 * _itemPointCnt, _pts.begin() + l * 3 * _itemPointCnt + 3 * _itemPointCnt);
            _rgba.erase(_rgba.begin() + l * 4 * _itemPointCnt, _rgba.begin() + l * 4 * _itemPointCnt + 4 * _itemPointCnt);
            _ids.erase(_ids.begin() + l);
            if (_sizes.size() > 0)
                _sizes.erase(_sizes.begin() + 3 * l * _itemPointCnt, _sizes.begin() + 3 * l * _itemPointCnt + 3 * _itemPointCnt); // because of tubes
            if (_quats.size() > 0)
                _quats.erase(_quats.begin() + 4 * l, _quats.begin() + 4 * l + 4);
            updateBB = true;
        }
    }
    if (updateBB)
    {
        computeBoundingBox();
        _updateMarkerEvent(true);
    }
}

void CMarker::remItems(int itemCntToDelete, bool triggerEvent /*= true*/)
{
    if ((_ids.size() <= itemCntToDelete) || (itemCntToDelete == 0))
    {
        _pts.clear();
        _quats.clear();
        _rgba.clear();
        _sizes.clear();
        _ids.clear();
        _remIds.clear();
        _newItemsCnt = 0;
        _sendFullEvent = true;
        _xs.clear();
        _ys.clear();
        _zs.clear();
        _itemIts.clear();
    }
    else
    {
        for (size_t i = 0; i < itemCntToDelete; i++)
        {
            auto it = _itemIts.find(_ids[i]);
            if (it != _itemIts.end())
            {
                CItemPointIts& iph = it->second;
                for (int j = 0; j < _itemPointCnt; ++j)
                {
                    _xs.erase(iph.its[j].itX);
                    _ys.erase(iph.its[j].itY);
                    _zs.erase(iph.its[j].itZ);
                    _xs.erase(iph.its[j].itnX);
                    _ys.erase(iph.its[j].itnY);
                    _zs.erase(iph.its[j].itnZ);
                }
                _itemIts.erase(it);
            }
        }
        _remIds.insert(_remIds.end(), _ids.begin(), _ids.begin() + itemCntToDelete);
        _pts.erase(_pts.begin(), _pts.begin() + itemCntToDelete * 3 * _itemPointCnt);
        _rgba.erase(_rgba.begin(), _rgba.begin() + itemCntToDelete * 4 * _itemPointCnt);
        _ids.erase(_ids.begin(), _ids.begin() + itemCntToDelete);
        if ( (_itemType != sim_markertype_points) && (_itemType != sim_markertype_lines) && (_itemType != sim_markertype_triangles) )
        {
            _quats.erase(_quats.begin(), _quats.begin() + itemCntToDelete * 4 * _itemPointCnt);
            _sizes.erase(_sizes.begin(), _sizes.begin() + itemCntToDelete * 3 * _itemPointCnt);
        }
        int tot = int(_pts.size()) / (3 * _itemPointCnt);
        if (tot < _newItemsCnt)
            _newItemsCnt = tot;
    }
    if (triggerEvent)
    {
        computeBoundingBox();
        _updateMarkerEvent(true);
    }
}

void CMarker::addItems(const std::vector<float>* pts, const std::vector<float>* quats, const std::vector<unsigned char>* rgbas, const std::vector<float>* sizes, bool transform /*= true*/, std::vector<int64_t>* newIds /*= nullptr*/)
{
    if (_itemOptions & sim_markeropts_tube)
    {
        int lineCnt = int(pts->size()) / 6;
        std::vector<float> npts;
        npts.resize(lineCnt * 3);
        std::vector<float> nquats;
        nquats.resize(lineCnt * 4);
        std::vector<float> nsizes;
        nsizes.resize(lineCnt * 3);
        for (size_t i = 0; i < lineCnt; i++)
        {
            C3Vector v0(pts->data() + 6 * i + 0);
            C3Vector v1(pts->data() + 6 * i + 3);
            const C3Vector dv(v1 - v0);
            const float len = dv.getLength();
            const C3Vector center((v0 + v1) * 0.5f);
            // Orientation: rotate the cylinder's local z-axis onto the segment axis.
            // C4Vector default ctor is identity, which is what we want for a degenerate segment.
            C4Vector q;
            if (len > 1e-6f)
                q.alignQuaternion(v1 -v0);
            else
                q.setIdentity();
            npts[3 * i + 0] = center(0);
            npts[3 * i + 1] = center(1);
            npts[3 * i + 2] = center(2);

            nquats[4 * i + 0] = q(1); // qx
            nquats[4 * i + 1] = q(2); // qy
            nquats[4 * i + 2] = q(3); // qz
            nquats[4 * i + 3] = q(0); // qw

            nsizes[3 * i + 0] = _itemSize[0];  // sr
            nsizes[3 * i + 1] = _itemSize[0];  // sr
            nsizes[3 * i + 2] = len; // sl
        }
        _addItems(&npts, &nquats, rgbas, &nsizes, transform, newIds);
    }
    else if (_itemOptions & sim_markeropts_axis)
    {
        int ptsCnt = int(pts->size()) / 3;
        C3X3Matrix m;
        if ((quats == nullptr) || ((quats->size() / 4) != ptsCnt))
            m.setIdentity();
        std::vector<float> npts;
        npts.resize(ptsCnt * 6 * 3);
        std::vector<unsigned char> nrgbas;
        nrgbas.resize(ptsCnt * 6 * 4);
        float s[3] = {float(_itemSize[0]), float(_itemSize[1]), float(_itemSize[2])};

        for (size_t i = 0; i < ptsCnt; i++)
        {
            if ((quats != nullptr) && ((quats->size() / 4) == (pts->size() / 3)))
                m = C3X3Matrix(CQuaternion(quats->data() + 4 * i, true));
            if ((sizes != nullptr) && (sizes->size() == pts->size()))
            {
                s[0] = sizes->at(3 * i + 0);
                s[1] = sizes->at(3 * i + 1);
                s[2] = sizes->at(3 * i + 2);
            }
            for (size_t j = 0; j < 3; j++)
            {
                npts[18 * i + j + 0] = pts->at(3 * i + j);
                npts[18 * i + j + 3] = pts->at(3 * i + j) + float(m.axis[0](j)) * s[0];
                npts[18 * i + j + 6] = pts->at(3 * i + j);
                npts[18 * i + j + 9] = pts->at(3 * i + j) + float(m.axis[1](j)) * s[1];
                npts[18 * i + j + 12] = pts->at(3 * i + j);
                npts[18 * i + j + 15] = pts->at(3 * i + j) + float(m.axis[2](j)) * s[2];
                for (size_t k = 0; k < 2; k++)
                {
                    nrgbas[24 * i + 8 * j + k * 4 + 0] = (j == 0) ? 255: 0;
                    nrgbas[24 * i + 8 * j + k * 4 + 1] = (j == 1) ? 255: 0;
                    nrgbas[24 * i + 8 * j + k * 4 + 2] = (j == 2) ? 255: 0;
                    nrgbas[24 * i + 8 * j + k * 4 + 3] = 255;
                }
            }
        }
        _addItems(&npts, nullptr, &nrgbas, nullptr, transform, newIds);
    }
    else
        _addItems(pts, quats, rgbas, sizes, transform, newIds);
}

void CMarker::_addItems(const std::vector<float>* pts, const std::vector<float>* quats, const std::vector<unsigned char>* rgbas, const std::vector<float>* sizes, bool transform /*= true*/, std::vector<int64_t>* newIds /*= nullptr*/)
{
    int ptsCnt = int(pts->size()) / 3;
    if (quats != nullptr)
    {
        if (quats->size() != ptsCnt * 4)
            quats = nullptr;
    }
    if (rgbas != nullptr)
    {
        if (rgbas->size() != ptsCnt * 4)
            rgbas = nullptr;
    }
    if (sizes != nullptr)
    {
        if (sizes->size() != ptsCnt * 3)
            sizes = nullptr;
    }
    int totItemCnt = int(_pts.size()) / (3 * _itemPointCnt);
    int itemCnt = ptsCnt / _itemPointCnt;

    if ((_itemMaxCnt != 0) && (totItemCnt + itemCnt > _itemMaxCnt) && ((_itemOptions & sim_markeropts_cyclic) == 0))
        itemCnt = _itemMaxCnt - totItemCnt;

    // handle coords
    CPose tr(getFullCumulativeTransformation());
    CPose trInv(tr.getInverse());
    std::vector<C4X4Matrix> trs;
    if (transform && ((_itemOptions & sim_markeropts_local) == 0))
    {
        for (int i = 0; i < ptsCnt; i++)
        {
            C4X4Matrix t;
            t.X.setData(pts->data() + i * 3);
            if (quats != nullptr)
                t.M = CQuaternion(quats->at(i * 4 + 3), quats->at(i * 4 + 0), quats->at(i * 4 + 1), quats->at(i * 4 + 2)).getMatrix();
            else
                t.M.setIdentity();
            trs.push_back(trInv * t);
        }
    }
    else
    {
        for (int i = 0; i < ptsCnt; i++)
        {
            C4X4Matrix t;
            t.X.setData(pts->data() + i * 3);
            if (quats != nullptr)
                t.M = CQuaternion(quats->at(i * 4 + 3), quats->at(i * 4 + 0), quats->at(i * 4 + 1), quats->at(i * 4 + 2)).getMatrix();
            else
                t.M.setIdentity();
            trs.push_back(t);
        }
    }

    bool hasDim = ((_itemType != sim_markertype_points) && (_itemType != sim_markertype_lines) && (_itemType != sim_markertype_triangles));
    for (int i = 0; i < itemCnt; i++)
    {
        bool skip = false;
        if ((_itemDuplicateTol > 0.0f) && (_itemPointCnt == 1))
        { // optimize later
            float minDD = 1000000000.0f;
            for (int k = 0; k < _pts.size() / 3; k++)
            {
                float dxx[3] = {((float)trs[i].X(0)) - _pts[k * 3 + 0], ((float)trs[i].X(1)) - _pts[k * 3 + 1], ((float)trs[i].X(2)) - _pts[k * 3 + 2]};
                float dd = dxx[0] * dxx[0] + dxx[1] * dxx[1] + dxx[2] * dxx[2];
                if (dd < minDD)
                    minDD = dd;
            }
            skip = (minDD < _itemDuplicateTol * _itemDuplicateTol);
        }
        if (!skip)
        {
            CItemPointIts its;
            if (hasDim)
            {
                if (_itemType != sim_markertype_custom)
                {
                    const C3X3Matrix* m = &trs[i * _itemPointCnt].M;
                    C3Vector p = trs[i * _itemPointCnt].X;
                    _pts.push_back((float)p(0));
                    _pts.push_back((float)p(1));
                    _pts.push_back((float)p(2));
                    CMultiSIt it;
                    double s[3];
                    if (sizes != nullptr)
                    {
                        s[0] = sizes->at(i * 3 + 0) * 0.5;
                        s[1] = sizes->at(i * 3 + 1) * 0.5;
                        s[2] = sizes->at(i * 3 + 2) * 0.5;
                    }
                    else
                    {
                        s[0] = _itemSize[0] * 0.5;
                        s[1] = _itemSize[1] * 0.5;
                        s[2] = _itemSize[2] * 0.5;
                    }
                    float xm = float(fabs(m->axis[0](0)) * s[0] + fabs(m->axis[1](0)) * s[1] + fabs(m->axis[2](0)) * s[2]);
                    float ym = float(fabs(m->axis[0](1)) * s[0] + fabs(m->axis[1](1)) * s[1] + fabs(m->axis[2](1)) * s[2]);
                    float zm = float(fabs(m->axis[0](2)) * s[0] + fabs(m->axis[1](2)) * s[1] + fabs(m->axis[2](2)) * s[2]);
                    it.itX = _xs.insert(p(0) + xm);
                    it.itY = _ys.insert(p(1) + ym);
                    it.itZ = _zs.insert(p(2) + zm);
                    it.itnX = _xs.insert(p(0) - xm);
                    it.itnY = _ys.insert(p(1) - ym);
                    it.itnZ = _zs.insert(p(2) - zm);
                    its.its[0] = it;
                }
                else
                { // custom, non-symmetrical items
                    const C3X3Matrix* m = &trs[i * _itemPointCnt].M;
                    C3Vector p = trs[i * _itemPointCnt].X;
                    _pts.push_back((float)p(0));
                    _pts.push_back((float)p(1));
                    _pts.push_back((float)p(2));
                    CMultiSIt it;
                    double s[3];
                    if (sizes != nullptr)
                    {
                        s[0] = sizes->at(i * 3 + 0);
                        s[1] = sizes->at(i * 3 + 1);
                        s[2] = sizes->at(i * 3 + 2);
                    }
                    else
                    {
                        s[0] = _itemSize[0];
                        s[1] = _itemSize[1];
                        s[2] = _itemSize[2];
                    }

                    double minX =  DBL_MAX, minY =  DBL_MAX, minZ =  DBL_MAX;
                    double maxX = -DBL_MAX, maxY = -DBL_MAX, maxZ = -DBL_MAX;

                    for (int ix = 0; ix < 2; ++ix)
                    {
                        for (int iy = 0; iy < 2; ++iy)
                        {
                            for (int iz = 0; iz < 2; ++iz)
                            {
                                const double x = _customItemExtentX[ix] * s[0];
                                const double y = _customItemExtentY[iy] * s[1];
                                const double z = _customItemExtentZ[iz] * s[2];
                                const double wx = m->axis[0](0) * x + m->axis[1](0) * y + m->axis[2](0) * z;
                                const double wy = m->axis[0](1) * x + m->axis[1](1) * y + m->axis[2](1) * z;
                                const double wz = m->axis[0](2) * x + m->axis[1](2) * y + m->axis[2](2) * z;
                                if (wx < minX)
                                    minX = wx;
                                if (wx > maxX)
                                    maxX = wx;
                                if (wy < minY)
                                    minY = wy;
                                if (wy > maxY)
                                    maxY = wy;
                                if (wz < minZ)
                                    minZ = wz;
                                if (wz > maxZ)
                                    maxZ = wz;
                            }
                        }
                    }
                    it.itX = _xs.insert(p(0) + maxX);
                    it.itY = _ys.insert(p(1) + maxY);
                    it.itZ = _zs.insert(p(2) + maxZ);
                    it.itnX = _xs.insert(p(0) + minX);
                    it.itnY = _ys.insert(p(1) + minY);
                    it.itnZ = _zs.insert(p(2) + minZ);
                    its.its[0] = it;
                }
            }
            else
            { // points, lines, tubes and triangles
                for (int j = 0; j < _itemPointCnt; j++)
                {
                    C3Vector p = trs[i * _itemPointCnt + j].X;
                    _pts.push_back((float)p(0));
                    _pts.push_back((float)p(1));
                    _pts.push_back((float)p(2));
                    CMultiSIt it;
                    it.itX = _xs.insert(p(0));
                    it.itY = _ys.insert(p(1));
                    it.itZ = _zs.insert(p(2));
                    it.itnX = _xs.insert(p(0));
                    it.itnY = _ys.insert(p(1));
                    it.itnZ = _zs.insert(p(2));
                    its.its[j] = it;
                }
            }

            _itemIts[_nextId] = its;
            _ids.push_back(_nextId);
            if (newIds != nullptr)
                newIds->push_back(_nextId);
            if (rgbas != nullptr)
            {
                for (int j = 0; j < 4 * _itemPointCnt; j++)
                    _rgba.push_back(rgbas->at(i * 4 * _itemPointCnt + j));
            }
            else
            {
                for (int j = 0; j < _itemPointCnt; j++)
                {
                    _rgba.push_back(_itemCol[0]);
                    _rgba.push_back(_itemCol[1]);
                    _rgba.push_back(_itemCol[2]);
                    _rgba.push_back(_itemCol[3]);
                }
            }
            if (hasDim)
            {
                CQuaternion q(trs[i].M.getQuaternion());
                _quats.push_back((float)q(1));
                _quats.push_back((float)q(2));
                _quats.push_back((float)q(3));
                _quats.push_back((float)q(0));
                if (sizes != nullptr)
                {
                    for (int j = 0; j < 3 ; j++)
                        _sizes.push_back(sizes->at(i * 3 + j));
                }
                else
                {
                    for (int j = 0; j < 3 ; j++)
                        _sizes.push_back(_itemSize[j]);
                }
            }
            _nextId++;
        }
        else
        {
            if (newIds != nullptr)
                newIds->push_back(-1); // indicates an invalid ID
        }
    }

    totItemCnt = int(_pts.size()) / (3 * _itemPointCnt);
    _newItemsCnt += itemCnt;
    if ((totItemCnt > _itemMaxCnt) && (_itemMaxCnt != 0))
        remItems(totItemCnt - _itemMaxCnt, false);
    computeBoundingBox();
    _updateMarkerEvent(true);
}

CMarker::~CMarker()
{
}

std::string CMarker::getObjectTypeInfoExtended() const
{
    return _objectTypeStr;
}

void CMarker::computeBoundingBox()
{
    C3Vector c, s;
    if (_xs.empty())
    {
        c.clear();
        s.clear();
    }
    else
    {
        double minX = *_xs.begin();
        double maxX = *std::prev(_xs.end());
        double minY = *_ys.begin();
        double maxY = *std::prev(_ys.end());
        double minZ = *_zs.begin();
        double maxZ = *std::prev(_zs.end());
        s(0) = maxX - minX;
        s(1) = maxY - minY;
        s(2) = maxZ - minZ;
        c(0) = (maxX + minX) * 0.5;
        c(1) = (maxY + minY) * 0.5;
        c(2) = (maxZ + minZ) * 0.5;
    }
    s(0) += 0.002;
    s(1) += 0.002;
    s(2) += 0.002;
    CPose tr;
    tr.Q.setIdentity();
    tr.X = c;
    _setBB(tr, s * 0.5);
}

void CMarker::scaleObject(double scalingFactor)
{
    _itemSize[0] *= scalingFactor;
    _itemSize[1] *= scalingFactor;
    _itemSize[2] *= scalingFactor;
    for (size_t i = 0; i < 2; i++)
    {
        _customItemExtentX[i] *= scalingFactor;
        _customItemExtentY[i] *= scalingFactor;
        _customItemExtentZ[i] *= scalingFactor;
    }
    _itemDuplicateTol *= scalingFactor;

    for (size_t i = 0; i < _pts.size(); i++)
        _pts[i] *= scalingFactor;
    for (size_t i = 0; i < _sizes.size(); i++)
        _sizes[i] *= scalingFactor;
    for (size_t i = 0; i < _vertices.size(); i++)
        _vertices[i] *= scalingFactor;

    CSceneObject::scaleObject(scalingFactor);
    _initialize();
    if ( (_isInScene && App::scenes->getEventsEnabled()) && (_itemType == sim_markertype_custom) )
    {
        CCbor* ev = App::scenes->createSceneObjectChangedEvent(this, false, "vertices", true);
        std::vector<float> vert;
        std::vector<int> ind;
        std::vector<float> norm;
        CMeshManip::generatePerVertexNormalsAndTextCoords(&_vertices, &_indices, &_normals, nullptr, &vert, &ind, &norm);
        ev->appendKeyMatrix(prop(PropMarker::vertices).name, vert.data(), 3, vert.size() / 3, false);
        ev->appendKeyInt32Array(prop(PropMarker::indices).name, ind.data(), ind.size());
        ev->appendKeyMatrix(prop(PropMarker::normals).name, norm.data(), 3, norm.size() / 3, false);
        App::scenes->pushEvent();
    }
}

void CMarker::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CMarker::pushNakedGenesisEvents(CCbor* ev /*= nullptr*/)
{
    if (_isInScene && App::scenes->getEventsEnabled())
    {
        bool createdHere = (ev == nullptr);
        if (createdHere)
            ev = App::scenes->createSceneObjectAddEvent(this);
        ev->appendKeyText(prop(PropMarker::type).name, getMarkerTypeStr().c_str());
        ev->appendKeyBool(prop(PropMarker::cyclic).name, _itemOptions & sim_markeropts_cyclic);
        ev->appendKeyBool(prop(PropMarker::local).name, _itemOptions & sim_markeropts_local);
        ev->appendKeyBool(prop(PropMarker::overlay).name, _itemOptions & sim_markeropts_overlay);
        ev->appendKeyBool(prop(PropMarker::notShaded).name, _itemOptions & sim_markeropts_notshaded);
        if (_itemType == sim_markertype_custom)
        {
            std::vector<float> vert;
            std::vector<int> ind;
            std::vector<float> norm;
            CMeshManip::generatePerVertexNormalsAndTextCoords(&_vertices, &_indices, &_normals, nullptr, &vert, &ind, &norm);
            ev->appendKeyMatrix(prop(PropMarker::vertices).name, vert.data(), 3, vert.size() / 3, false);
            ev->appendKeyInt32Array(prop(PropMarker::indices).name, ind.data(), ind.size());
            ev->appendKeyMatrix(prop(PropMarker::normals).name, norm.data(), 3, norm.size() / 3, false);
        }
        _updateMarkerEvent(false, ev);
        CSceneObject::pushNakedGenesisEvents(ev);
        if (createdHere)
            App::scenes->pushEvent();
    }
}

CSceneObject* CMarker::copyYourself()
{
    CMarker* newMarker = (CMarker*)CSceneObject::copyYourself();

    newMarker->_itemType = _itemType;
    newMarker->_itemMaxCnt = _itemMaxCnt;
    newMarker->_itemOptions = _itemOptions;
    newMarker->_itemDuplicateTol = _itemDuplicateTol;
    newMarker->_itemPointCnt = _itemPointCnt;
    for (size_t i = 0; i < 4; i++)
        newMarker->_itemCol[i] =_itemCol[i];
    for (size_t i = 0; i < 3; i++)
        newMarker->_itemSize[i] =_itemSize[i];
    for (size_t i = 0; i < 2; i++)
    {
        newMarker->_customItemExtentX[i] = _customItemExtentX[i];
        newMarker->_customItemExtentY[i] = _customItemExtentY[i];
        newMarker->_customItemExtentZ[i] = _customItemExtentZ[i];
    }

    newMarker->_pts.assign(_pts.begin(), _pts.end());
    newMarker->_quats.assign(_quats.begin(), _quats.end());
    newMarker->_rgba.assign(_rgba.begin(), _rgba.end());
    newMarker->_sizes.assign(_sizes.begin(), _sizes.end());
    newMarker->_ids.assign(_ids.begin(), _ids.end());
    newMarker->_vertices.assign(_vertices.begin(), _vertices.end());
    newMarker->_indices.assign(_indices.begin(), _indices.end());
    newMarker->_normals.assign(_normals.begin(), _normals.end());
    newMarker->_rebuildMarkerBoundingBox();
    newMarker->_sendFullEvent = true;
    newMarker->_nextId = _nextId;

    return newMarker;
}

void CMarker::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
}

void CMarker::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CMarker::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
        }
    }
    CSceneObject::simulationEnded();
}

void CMarker::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Var");
            unsigned char dummy = 0;
            // SIM_SET_CLEAR_BIT(dummy, 0, _resetAfterSimError);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Set");
            ar << _itemType;
            ar << _itemMaxCnt;
            ar << _itemOptions;
            ar << _itemDuplicateTol;
            for (size_t i = 0; i < 4; i++)
                ar << _itemCol[i];
            for (size_t i = 0; i < 3; i++)
                ar << _itemSize[i];
            ar.flush();

            ar.storeDataName("Ext");
            ar << _customItemExtentX[0];
            ar << _customItemExtentX[1];
            ar << _customItemExtentY[0];
            ar << _customItemExtentY[1];
            ar << _customItemExtentZ[0];
            ar << _customItemExtentZ[1];
            ar.flush();

            ar.storeDataName("Pts");
            ar << int(_pts.size());
            for (size_t i = 0; i < _pts.size(); i++)
                ar << _pts[i];
            ar.flush();

            ar.storeDataName("Qts");
            ar << int(_quats.size());
            for (size_t i = 0; i < _quats.size(); i++)
                ar << _quats[i];
            ar.flush();

            ar.storeDataName("Cls");
            ar << int(_rgba.size());
            for (size_t i = 0; i < _rgba.size(); i++)
                ar << _rgba[i];
            ar.flush();

            ar.storeDataName("Siz");
            ar << int(_sizes.size());
            for (size_t i = 0; i < _sizes.size(); i++)
                ar << _sizes[i];
            ar.flush();

            ar.storeDataName("Ver");
            ar << int(_vertices.size());
            for (size_t i = 0; i < _vertices.size(); i++)
                ar << _vertices[i];
            ar.flush();

            ar.storeDataName("Ind");
            ar << int(_indices.size());
            for (size_t i = 0; i < _indices.size(); i++)
                ar << _indices[i];
            ar.flush();

            ar.storeDataName("Nor");
            ar << int(_normals.size());
            for (size_t i = 0; i < _normals.size(); i++)
                ar << _normals[i];
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            bool tubeOrAxis_old = false;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Var") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        // _resetAfterSimError = SIM_IS_BIT_SET(dummy, 0);
                    }

                    if (theName.compare("Set") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _itemType;
                        ar >> _itemMaxCnt;
                        ar >> _itemOptions;
                        ar >> _itemDuplicateTol;
                        for (size_t i = 0; i < 4; i++)
                            ar >> _itemCol[i];
                        for (size_t i = 0; i < 3; i++)
                            ar >> _itemSize[i];
                        if (_itemType == 14)
                        {
                            _itemType = sim_markertype_cylinders;
                            _itemOptions |= sim_markeropts_tube;
                            tubeOrAxis_old = true;
                        }
                        else if (_itemType == 15)
                        {
                            _itemType = sim_markertype_lines;
                            _itemOptions |= sim_markeropts_axis;
                            tubeOrAxis_old = true;
                        }
                    }

                    if (theName.compare("Ext") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _customItemExtentX[0];
                        ar >> _customItemExtentX[1];
                        ar >> _customItemExtentY[0];
                        ar >> _customItemExtentY[1];
                        ar >> _customItemExtentZ[0];
                        ar >> _customItemExtentZ[1];
                    }

                    if (theName.compare("Pts") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _pts.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                            ar >> _pts[i];
                    }

                    if (theName.compare("Qts") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _quats.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                            ar >> _quats[i];
                    }

                    if (theName.compare("Cls") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _rgba.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                            ar >> _rgba[i];
                    }

                    if (theName.compare("Siz") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _sizes.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                            ar >> _sizes[i];
                    }

                    if (theName.compare("Ver") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _vertices.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                            ar >> _vertices[i];
                    }

                    if (theName.compare("Ind") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _indices.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                            ar >> _indices[i];
                    }

                    if (theName.compare("Nor") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _normals.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                            ar >> _normals[i];
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (tubeOrAxis_old)
                remItems(0);
            _initialize();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            ar.xmlAddNode_comment(" 'type' tag: can be 'points', 'lines', 'triangles', 'spheres', 'squares', 'discs', 'cubes'", exhaustiveXml);
            ar.xmlAddNode_enum("type", _itemType, {{sim_markertype_points, "points"}, {sim_markertype_lines, "lines"}, {sim_markertype_triangles, "triangles"}, {sim_markertype_spheres, "spheres"}, {sim_markertype_squares, "squares"}, {sim_markertype_discs, "discs"}, {sim_markertype_cubes, "cubes"}, {sim_markertype_cylinders, "cylinders"}, {sim_markertype_custom, "custom"}});

            ar.xmlAddNode_int("maxCnt", _itemMaxCnt);
            ar.xmlAddNode_int("options", _itemOptions);
            ar.xmlAddNode_float("duplicateTolerance", _itemDuplicateTol);

            if (_itemType == sim_markertype_custom)
            {
                ar.xmlAddNode_floats("vertices", _vertices.data(), _vertices.size());
                ar.xmlAddNode_ints("indices", _indices.data(), _indices.size());
                ar.xmlAddNode_floats("normals", _normals.data(), _normals.size());
            }

            int rgba[4];
            for (size_t i = 0; i< 4; i++)
                rgba[i] = _itemCol[i];
            ar.xmlAddNode_ints("defaultColor", rgba, 4);
            ar.xmlAddNode_floats("defaultSize", _itemSize, 3);

            ar.xmlAddNode_floats("customItemExtentX", _customItemExtentX, 2);
            ar.xmlAddNode_floats("customItemExtentY", _customItemExtentY, 2);
            ar.xmlAddNode_floats("customItemExtentZ", _customItemExtentZ, 2);

            ar.xmlAddNode_floats("points", _pts.data(), _pts.size());
            ar.xmlAddNode_floats("quaternions", _quats.data(), _quats.size());
            std::vector<int> cols;
            for (size_t i = 0; i < _rgba.size(); i++)
                cols.push_back(_rgba[i]);
            ar.xmlAddNode_ints("colors", cols.data(), cols.size());
            ar.xmlAddNode_floats("sizes", _sizes.data(), _sizes.size());
        }
        else
        {
            ar.xmlGetNode_enum("type", _itemType, exhaustiveXml,
                               {
                                   {"points", sim_markertype_points},
                                   {"lines", sim_markertype_lines},
                                   {"triangles", sim_markertype_triangles},
                                   {"spheres", sim_markertype_spheres},
                                   {"squares", sim_markertype_squares},
                                   {"discs", sim_markertype_discs},
                                   {"cubes", sim_markertype_cubes},
                                   {"cylinders", sim_markertype_cylinders},
                                   {"custom", sim_markertype_custom}
                               });

            if (_itemType == sim_markertype_custom)
            {
                ar.xmlGetNode_floats("vertices", _vertices, exhaustiveXml);
                ar.xmlGetNode_ints("indices", _indices, exhaustiveXml);
                ar.xmlGetNode_floats("normals", _normals, exhaustiveXml);
            }

            ar.xmlGetNode_int("maxCnt", _itemMaxCnt, exhaustiveXml);
            if (_itemMaxCnt < 0)
                _itemMaxCnt = 0;
            ar.xmlGetNode_int("options", _itemOptions, exhaustiveXml);
            ar.xmlGetNode_float("duplicateTolerance", _itemDuplicateTol, exhaustiveXml);
            if (_itemDuplicateTol < 0.0)
                _itemDuplicateTol = 0.0;

            int rgba[4];
            ar.xmlGetNode_ints("defaultColor", rgba, 4, exhaustiveXml);
            for (size_t i = 0; i< 4; i++)
                _rgba[i] = (unsigned char)rgba[i];
            ar.xmlGetNode_floats("defaultSize", _itemSize, 3, exhaustiveXml);

            ar.xmlGetNode_floats("customItemExtentX", _customItemExtentX, 2, exhaustiveXml);
            ar.xmlGetNode_floats("customItemExtentY", _customItemExtentY, 2, exhaustiveXml);
            ar.xmlGetNode_floats("customItemExtentZ", _customItemExtentZ, 2, exhaustiveXml);

            ar.xmlGetNode_floats("points", _pts, exhaustiveXml);
            ar.xmlGetNode_floats("quaternions", _quats, exhaustiveXml);
            std::vector<int> cols;
            ar.xmlGetNode_ints("colors", cols, exhaustiveXml);
            for (size_t i = 0; i < cols.size(); i++)
                _rgba.push_back((unsigned char)cols[i]);
            ar.xmlGetNode_floats("sizes", _sizes, exhaustiveXml);

            _initialize();
        }
    }
}

int CMarker::getMarkerOptions() const
{
    return _itemOptions;
}

int CMarker::getMarkerType() const
{
    return _itemType;
}

std::string CMarker::getMarkerTypeStr() const
{
    std::string retVal = "invalidEnum";
    auto enum_value = magic_enum::enum_cast<markerType>(_itemType);
    if (enum_value.has_value())
        retVal = magic_enum::enum_name(enum_value.value()).data();
    return retVal;
}

int CMarker::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setBoolProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::getBoolProperty(const char* ppName, bool& pState) const
{
    int retVal = CSceneObject::getBoolProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(prop(PropMarker::cyclic).name, ppName) == 0)
        {
            pState = _itemOptions & sim_markeropts_cyclic;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(prop(PropMarker::overlay).name, ppName) == 0)
        {
            pState = _itemOptions & sim_markeropts_overlay;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(prop(PropMarker::notShaded).name, ppName) == 0)
        {
            pState = _itemOptions & sim_markeropts_notshaded;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(prop(PropMarker::local).name, ppName) == 0)
        {
            pState = _itemOptions & sim_markeropts_local;
            retVal = sim_propertyret_ok;
        }
    }

    return retVal;
}

int CMarker::setIntProperty(const char* ppName, int pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setIntProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getIntProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::setLongProperty(const char* ppName, int64_t pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setLongProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::getLongProperty(const char* ppName, int64_t& pState) const
{
    int retVal = CSceneObject::getLongProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::getHandleProperty(const char* ppName, int64_t& pState) const
{
    int retVal = CSceneObject::getHandleProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::setStringProperty(const char* ppName, const std::string& pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setStringProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getStringProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (_pName == prop(PropMarker::type).name)
        {
            pState = getMarkerTypeStr();
            retVal = sim_propertyret_ok;
        }
    }

    return retVal;
}

int CMarker::getBufferProperty(const char* pName, std::string& pState) const
{
    int retVal = CSceneObject::getBufferProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(pName, prop(PropMarker::colors).name) == 0)
        {
            pState.assign(_rgba.begin(), _rgba.end());
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, prop(PropMarker::packedPoints).name) == 0)
        {
            pState.assign((char*)_pts.data(), _pts.size() * sizeof(float));
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, prop(PropMarker::packedQuaternions).name) == 0)
        {
            pState.assign((char*)_quats.data(), _quats.size() * sizeof(float));
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, prop(PropMarker::packedSizes).name) == 0)
        {
            pState.assign((char*)_sizes.data(), _sizes.size() * sizeof(float));
            retVal = sim_propertyret_ok;
        }
    }

    return retVal;
}

int CMarker::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }
    return retVal;
}

int CMarker::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }
    return retVal;
}

int CMarker::getIntArrayProperty(const char* pName, std::vector<int>& pState) const
{
    int retVal = CSceneObject::getIntArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(pName, prop(PropMarker::indices).name) == 0)
        {
            std::vector<float> vert;
            std::vector<int> ind;
            std::vector<float> norm;
            CMeshManip::generatePerVertexNormalsAndTextCoords(&_vertices, &_indices, &_normals, nullptr, &vert, &ind, &norm);
            pState = ind;
            retVal = sim_propertyret_ok;
        }
    }
    return retVal;
}

int CMarker::getLongArrayProperty(const char* pName, std::vector<int64_t>& pState) const
{
    int retVal = CSceneObject::getLongArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(pName, prop(PropMarker::ids).name) == 0)
        {
            pState = _ids;
            retVal = sim_propertyret_ok;
        }
    }
    return retVal;
}

int CMarker::getFloatArrayProperty(const char* pName, std::vector<double>& pState) const
{
    int retVal = CSceneObject::getFloatArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }
    return retVal;
}

int CMarker::getVector3Property(const char* pName, C3Vector& pState) const
{
    int retVal = CSceneObject::getVector3Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CMarker::getMatrixProperty(const char* pName, CMatrix& pState) const
{
    int retVal = CSceneObject::getMatrixProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(pName, prop(PropMarker::points).name) == 0)
        {
            size_t n = _pts.size() / 3;
            pState.resize(3, n, 0.0);
            for (size_t i = 0; i < n; i++)
            {
                pState.data[n * 0 + i] = (double)_pts[3 * i + 0];
                pState.data[n * 1 + i] = (double)_pts[3 * i + 1];
                pState.data[n * 2 + i] = (double)_pts[3 * i + 2];
            }
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, prop(PropMarker::quaternions).name) == 0)
        {
            size_t n = _quats.size() / 4;
            pState.resize(4, n, 0.0);
            for (size_t i = 0; i < n; i++)
            {
                pState.data[n * 0 + i] = (double)_quats[4 * i + 0];
                pState.data[n * 1 + i] = (double)_quats[4 * i + 1];
                pState.data[n * 2 + i] = (double)_quats[4 * i + 2];
                pState.data[n * 3 + i] = (double)_quats[4 * i + 3];
            }
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, prop(PropMarker::sizes).name) == 0)
        {
            size_t n = _sizes.size() / 3;
            pState.resize(3, n, 0.0);
            for (size_t i = 0; i < n; i++)
            {
                pState.data[n * 0 + i] = (double)_sizes[3 * i + 0];
                pState.data[n * 1 + i] = (double)_sizes[3 * i + 1];
                pState.data[n * 2 + i] = (double)_sizes[3 * i + 2];
            }
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, prop(PropMarker::vertices).name) == 0)
        {
            std::vector<float> vert;
            std::vector<int> ind;
            std::vector<float> norm;
            CMeshManip::generatePerVertexNormalsAndTextCoords(&_vertices, &_indices, &_normals, nullptr, &vert, &ind, &norm);
            size_t n = vert.size() / 3;
            pState.resize(3, n, 0.0);
            for (size_t i = 0; i < n; i++)
            {
                pState.data[n * 0 + i] = (double)vert[3 * i + 0];
                pState.data[n * 1 + i] = (double)vert[3 * i + 1];
                pState.data[n * 2 + i] = (double)vert[3 * i + 2];
            }
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, prop(PropMarker::normals).name) == 0)
        {
            std::vector<float> vert;
            std::vector<int> ind;
            std::vector<float> norm;
            CMeshManip::generatePerVertexNormalsAndTextCoords(&_vertices, &_indices, &_normals, nullptr, &vert, &ind, &norm);
            size_t n = norm.size() / 3;
            pState.resize(3, n, 0.0);
            for (size_t i = 0; i < n; i++)
            {
                pState.data[n * 0 + i] = (double)norm[3 * i + 0];
                pState.data[n * 1 + i] = (double)norm[3 * i + 1];
                pState.data[n * 2 + i] = (double)norm[3 * i + 2];
            }
            retVal = sim_propertyret_ok;
        }
    }

    return retVal;
}

int CMarker::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance, excludeFlags);
    if (_isInScene && (retVal == sim_propertyret_unknownproperty))
    {
        appartenance = _originalObjectTypeStr;
        for (size_t i = 0; i < allProps_marker.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_marker[i].name, pName.c_str()))
            {
                if ((allProps_marker[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_marker[i].name;
                        retVal = sim_propertyret_ok;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CMarker::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = CSceneObject::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
    {
        for (size_t i = 0; i < allProps_marker.size(); i++)
        {
            if (strcmp(allProps_marker[i].name, ppName) == 0)
            {
                retVal = allProps_marker[i].type;
                info = allProps_marker[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_marker[i].info.json;
                else
                {
                    auto w = allProps_marker[i].info.map;
                    std::string descr = w["description"].toString().toStdString();
                    std::string label = w["label"].toString().toStdString();
                    if ( (infoTxt == "s") || (descr == "") )
                        infoTxt = label;
                    else
                        infoTxt = descr;
                }
                break;
            }
        }
    }
    return retVal;
}

void CMarker::_updateMarkerEvent(bool incremental, CCbor* evv /*= nullptr*/)
{
    CCbor* ev = evv;
    if ((evv != nullptr) || (_isInScene && App::scenes->getEventsEnabled()))
    {
        if (!incremental)
            _sendFullEvent = true;
        if (_sendFullEvent)
        {
            _sendFullEvent = false;
            if (evv == nullptr)
                ev = App::scenes->createSceneObjectChangedEvent(this, false, "set", true);
            ev->openKeyMap("set");
            ev->appendKeyMatrix(prop(PropMarker::points).name, _pts.data(), 3, _pts.size() / 3, false);
            ev->appendKeyMatrix(prop(PropMarker::quaternions).name, _quats.data(), 4, _quats.size() / 4, false);
            ev->appendKeyMatrix(prop(PropMarker::sizes).name, _sizes.data(), 3, _sizes.size() / 3, false);
            ev->appendKeyUint8Array(prop(PropMarker::colors).name, _rgba.data(), _rgba.size());
            ev->appendKeyInt64Array(prop(PropMarker::ids).name, _ids.data(), _ids.size());
            ev->closeArrayOrMap();
            if (evv == nullptr)
                App::scenes->pushEvent();
        }
        else
        {
            if ( (_newItemsCnt > 0) || (_remIds.size() > 0) )
            {
                if (evv == nullptr)
                    ev = App::scenes->createSceneObjectChangedEvent(this, false, "addRemove", true);
                if (_newItemsCnt > 0)
                {
                    ev->openKeyMap("add");
                    ev->appendKeyMatrix(prop(PropMarker::points).name, _pts.data() + _pts.size() - (_newItemsCnt * 3 * _itemPointCnt), 3, _newItemsCnt * _itemPointCnt, false);
                    if (_quats.size() > 0)
                        ev->appendKeyMatrix(prop(PropMarker::quaternions).name, _quats.data() + _quats.size() - (_newItemsCnt * 4 * _itemPointCnt), 4, _newItemsCnt * _itemPointCnt, false);
                    else
                        ev->appendKeyMatrix(prop(PropMarker::quaternions).name, (float*)nullptr, 4, 0, false);
                    if (_sizes.size() > 0)
                        ev->appendKeyMatrix(prop(PropMarker::sizes).name, _sizes.data() + _sizes.size() - (_newItemsCnt * 3 * _itemPointCnt), 3, _newItemsCnt * _itemPointCnt, false);
                    else
                        ev->appendKeyMatrix(prop(PropMarker::sizes).name, (float*)nullptr, 3, 0);
                    ev->appendKeyUint8Array(prop(PropMarker::colors).name, (unsigned char*)(_rgba.data() + _rgba.size() - (_newItemsCnt * 4 * _itemPointCnt)), _newItemsCnt * 4 * _itemPointCnt);
                    ev->appendKeyInt64Array(prop(PropMarker::ids).name, _ids.data() + _ids.size() - _newItemsCnt, _newItemsCnt);
                    ev->closeArrayOrMap();
                }
                if (_remIds.size() > 0)
                {
                    ev->openKeyMap("rem");
                    ev->appendKeyInt64Array(prop(PropMarker::ids).name, _remIds.data(), _remIds.size());
                    ev->closeArrayOrMap();
                }
                if (evv == nullptr)
                    App::scenes->pushEvent();
            }
        }
        _newItemsCnt = 0;
        _remIds.clear();
    }
}

void CMarker::display_extRenderer(CViewableBase* renderingObject, int displayAttrib)
{
    if (getShouldObjectBeDisplayed(renderingObject->getObjectHandle(), displayAttrib))
    {
        void* data[20];
        std::vector<float> tmp;
        data[0] = &_itemType;
        data[1] = &_itemOptions;
        const float* pts = nullptr;
        const float* quats = nullptr;
        const unsigned char* rgbas = nullptr;
        const float* sizes = nullptr;
        const float* vertices = nullptr;
        int vCnt = 0;
        const int* indices = nullptr;
        int iCnt = 0;
        const float* normals = nullptr;
        int ptDataCnt = getData(&pts, &quats, &rgbas, &sizes, &vertices, &vCnt, &indices, &iCnt, &normals);
        data[2] = &ptDataCnt; // number of points*3
        data[3] = &pts; // has ptDataCnt*3 values
        data[4] = &quats; // is either nullptr or has ptDataCnt*4 values (for items that are not points, lines, tubes or triangles)
        data[5] = &rgbas; // is either nullptr or has ptDataCnt*4 values (lines and tubes have 2*4 floats, triangles have 3*4 floats)
        data[6] = &sizes; // is either nullptr or has ptDataCnt*3 values (for items that are not points, lines or triangles. Tubes use just the first float, as a radius of the tube)
        data[8] = &vertices; // has vCnt*3 values
        data[9] = &vCnt;
        data[10] = &indices; // has iCnt*3 values
        data[11] = &iCnt;
        data[12] = &normals; // is either nullptr or has iCnt*9 values (one normal per triangle corner)
        CPose p = getCumulativeTransformation();
        double q[4];
        data[13] = p.X.data;
        p.Q.getData(q, true);
        data[14] = q;
        int oh = _objectHandle;
        data[15] = &oh;
        App::scenes->pluginContainer->extRenderer(sim_message_eventcallback_extrenderer_marker, data);
    }
}

#ifdef SIM_WITH_GUI
void CMarker::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayMarker(this, renderingObject, displayAttrib, false);
}

void CMarker::displayOverlay(CViewableBase* renderingObject, int displayAttrib)
{
    displayMarker(this, renderingObject, displayAttrib, true);
}

namespace
{
constexpr float kPi = 3.14159265358979f;

inline bool colorCoded(int displayAttrib)
{
    return (displayAttrib & sim_displayattribute_colorcoded) != 0;
}

// Fixed-function: only ambient+diffuse track glColor (alpha included).
// Specular/emission/shininess are set directly (GL_SHININESS is NOT a valid glColorMaterial mode).
void beginColorMaterial(int displayAttrib)
{
    if (colorCoded(displayAttrib))
        return;
    static const GLfloat black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

inline void setItemColor(int displayAttrib, const unsigned char* rgba)
{
    if (!colorCoded(displayAttrib))
        glColor4ubv(reinterpret_cast<const GLubyte*>(rgba));
}

// Multiplies the current matrix by T(p) * R(q) * S(s), built directly (no acos/sqrt/glRotatef chain)
void multItemMatrix(const float* p, const float* q, const float* s)
{
    float x = q[0], y = q[1], z = q[2], w = q[3];
    float n2 = x * x + y * y + z * z + w * w;
    if (n2 > 1e-12f)
    {
        float inv = 1.0f / sqrtf(n2);
        x *= inv; y *= inv; z *= inv; w *= inv;
    }
    else
    {
        x = y = z = 0.0f; w = 1.0f;
    }
    const float xx = x * x, yy = y * y, zz = z * z;
    const float xy = x * y, xz = x * z, yz = y * z;
    const float wx = w * x, wy = w * y, wz = w * z;

    const GLfloat m[16] = {
                           (1.0f - 2.0f * (yy + zz)) * s[0], (2.0f * (xy + wz)) * s[0], (2.0f * (xz - wy)) * s[0], 0.0f,
                           (2.0f * (xy - wz)) * s[1], (1.0f - 2.0f * (xx + zz)) * s[1], (2.0f * (yz + wx)) * s[1], 0.0f,
                           (2.0f * (xz + wy)) * s[2], (2.0f * (yz - wx)) * s[2], (1.0f - 2.0f * (xx + yy)) * s[2], 0.0f,
                           p[0], p[1], p[2], 1.0f};
    glMultMatrixf(m);
}

// Static, CCW-wound (outward facing) triangle meshes with per-vertex normals
struct StaticMesh
{
    std::vector<float> v;
    std::vector<float> n;
    GLenum mode = GL_TRIANGLES;

    void vert(float x, float y, float z, float nx, float ny, float nz)
    {
        v.push_back(x); v.push_back(y); v.push_back(z);
        n.push_back(nx); n.push_back(ny); n.push_back(nz);
    }
    void tri(const float* a, const float* b, const float* c, const float* nn)
    {
        vert(a[0], a[1], a[2], nn[0], nn[1], nn[2]);
        vert(b[0], b[1], b[2], nn[0], nn[1], nn[2]);
        vert(c[0], c[1], c[2], nn[0], nn[1], nn[2]);
    }
    void quad(const float* a, const float* b, const float* c, const float* d, const float* nn)
    {
        tri(a, b, c, nn);
        tri(a, c, d, nn);
    }
    void draw() const
    {
        glVertexPointer(3, GL_FLOAT, 0, v.data());
        glNormalPointer(GL_FLOAT, 0, n.data());
        glDrawArrays(mode, 0, GLsizei(v.size() / 3));
    }
};

const StaticMesh& quadMesh()
{
    static const StaticMesh m = [] {
        StaticMesh s;
        const float a[3] = {-0.5f, -0.5f, 0.0f}, b[3] = {0.5f, -0.5f, 0.0f};
        const float c[3] = {0.5f, 0.5f, 0.0f}, d[3] = {-0.5f, 0.5f, 0.0f};
        const float nz[3] = {0.0f, 0.0f, 1.0f};
        s.quad(a, b, c, d, nz);
        return s;
    }();
    return m;
}

const StaticMesh& discMesh()
{
    static const StaticMesh m = [] {
        StaticMesh s;
        constexpr int seg = 12;
        const float nz[3] = {0.0f, 0.0f, 1.0f};
        const float ctr[3] = {0.0f, 0.0f, 0.0f};
        for (int j = 0; j < seg; j++)
        {
            float a0 = 2.0f * kPi * j / seg, a1 = 2.0f * kPi * (j + 1) / seg;
            const float p0[3] = {0.5f * cosf(a0), 0.5f * sinf(a0), 0.0f};
            const float p1[3] = {0.5f * cosf(a1), 0.5f * sinf(a1), 0.0f};
            s.tri(ctr, p0, p1, nz);
        }
        return s;
    }();
    return m;
}

const StaticMesh& cubeMesh()
{
    static const StaticMesh m = [] {
        StaticMesh s;
        const float P[8][3] = {
                               {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
                               {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}};
        const float N[6][3] = {{0, 0, 1}, {0, 0, -1}, {0, 1, 0}, {0, -1, 0}, {1, 0, 0}, {-1, 0, 0}};
        s.quad(P[4], P[5], P[6], P[7], N[0]); // +z
        s.quad(P[0], P[3], P[2], P[1], N[1]); // -z
        s.quad(P[3], P[7], P[6], P[2], N[2]); // +y
        s.quad(P[0], P[1], P[5], P[4], N[3]); // -y
        s.quad(P[1], P[2], P[6], P[5], N[4]); // +x
        s.quad(P[0], P[4], P[7], P[3], N[5]); // -x
        return s;
    }();
    return m;
}

const StaticMesh& sphereMesh()
{
    static const StaticMesh m = [] {
        StaticMesh s;
        constexpr int slices = 12, stacks = 12;
        auto pt = [](float lat, float lng, float* p, float* n) {
            n[0] = cosf(lat) * cosf(lng);
            n[1] = cosf(lat) * sinf(lng);
            n[2] = sinf(lat);
            p[0] = 0.5f * n[0]; p[1] = 0.5f * n[1]; p[2] = 0.5f * n[2];
        };
        for (int k = 0; k < stacks; k++)
        {
            float lat0 = kPi * (-0.5f + float(k) / stacks);
            float lat1 = kPi * (-0.5f + float(k + 1) / stacks);
            for (int j = 0; j < slices; j++)
            {
                float lng0 = 2.0f * kPi * j / slices, lng1 = 2.0f * kPi * (j + 1) / slices;
                float p00[3], n00[3], p01[3], n01[3], p10[3], n10[3], p11[3], n11[3];
                pt(lat0, lng0, p00, n00); pt(lat0, lng1, p01, n01);
                pt(lat1, lng0, p10, n10); pt(lat1, lng1, p11, n11);
                // two CCW (outward) triangles, smooth normals
                s.vert(p00[0], p00[1], p00[2], n00[0], n00[1], n00[2]);
                s.vert(p01[0], p01[1], p01[2], n01[0], n01[1], n01[2]);
                s.vert(p11[0], p11[1], p11[2], n11[0], n11[1], n11[2]);
                s.vert(p00[0], p00[1], p00[2], n00[0], n00[1], n00[2]);
                s.vert(p11[0], p11[1], p11[2], n11[0], n11[1], n11[2]);
                s.vert(p10[0], p10[1], p10[2], n10[0], n10[1], n10[2]);
            }
        }
        return s;
    }();
    return m;
}

// Closed unit cylinder: radius 0.5, z in [-0.5,0.5]. Also used for tubes.
const StaticMesh& cylinderMesh()
{
    static const StaticMesh m = [] {
        StaticMesh s;
        constexpr int slices = 16;
        const float up[3] = {0, 0, 1}, dn[3] = {0, 0, -1};
        const float ct[3] = {0, 0, 0.5f}, cb[3] = {0, 0, -0.5f};
        for (int j = 0; j < slices; j++)
        {
            float a0 = 2.0f * kPi * j / slices, a1 = 2.0f * kPi * (j + 1) / slices;
            float c0 = cosf(a0), s0 = sinf(a0), c1 = cosf(a1), s1 = sinf(a1);
            const float b0[3] = {0.5f * c0, 0.5f * s0, -0.5f}, b1[3] = {0.5f * c1, 0.5f * s1, -0.5f};
            const float t0[3] = {0.5f * c0, 0.5f * s0, 0.5f}, t1[3] = {0.5f * c1, 0.5f * s1, 0.5f};
            // side (smooth normals)
            s.vert(b0[0], b0[1], b0[2], c0, s0, 0); s.vert(b1[0], b1[1], b1[2], c1, s1, 0); s.vert(t1[0], t1[1], t1[2], c1, s1, 0);
            s.vert(b0[0], b0[1], b0[2], c0, s0, 0); s.vert(t1[0], t1[1], t1[2], c1, s1, 0); s.vert(t0[0], t0[1], t0[2], c0, s0, 0);
            // caps
            s.tri(ct, t0, t1, up);
            s.tri(cb, b1, b0, dn);
        }
        return s;
    }();
    return m;
}

// Scratch buffers (rendering is single-threaded); keep capacity between frames
std::vector<float>& scratchVerts() { static std::vector<float> v; return v; }
std::vector<float>& scratchNormals() { static std::vector<float> v; return v; }

// Expands indexed triangles into flat (per-face normal) arrays
void buildFlatTriangles(const float* verts, const int* indices, size_t triCount,
                        std::vector<float>& outV, std::vector<float>& outN)
{
    outV.resize(triCount * 9);
    outN.resize(triCount * 9);
    for (size_t t = 0; t < triCount; t++)
    {
        const float* v0 = verts + size_t(indices ? indices[3 * t + 0] : int(3 * t + 0)) * 3;
        const float* v1 = verts + size_t(indices ? indices[3 * t + 1] : int(3 * t + 1)) * 3;
        const float* v2 = verts + size_t(indices ? indices[3 * t + 2] : int(3 * t + 2)) * 3;
        float e1[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
        float e2[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};
        float nn[3] = {e1[1] * e2[2] - e1[2] * e2[1], e1[2] * e2[0] - e1[0] * e2[2], e1[0] * e2[1] - e1[1] * e2[0]};
        float len = sqrtf(nn[0] * nn[0] + nn[1] * nn[1] + nn[2] * nn[2]);
        if (len > 0.0f) { nn[0] /= len; nn[1] /= len; nn[2] /= len; }
        else { nn[0] = 0.0f; nn[1] = 0.0f; nn[2] = 1.0f; }
        float* dv = outV.data() + t * 9;
        float* dn = outN.data() + t * 9;
        for (int c = 0; c < 3; c++)
        {
            dv[c] = v0[c]; dv[3 + c] = v1[c]; dv[6 + c] = v2[c];
            dn[c] = nn[c]; dn[3 + c] = nn[c]; dn[6 + c] = nn[c];
        }
    }
}
} // namespace

// ---------------------------------------------------------------------------

void CMarker::_drawPoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    const size_t cnt = _pts.size() / 3;
    if (cnt == 0)
        return;
    glPointSize(2.0f);
    beginColorMaterial(displayAttrib);
    glNormal3dv(normalVectorForLinesAndPoints);
    glVertexPointer(3, GL_FLOAT, 0, _pts.data());
    if (!colorCoded(displayAttrib))
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, _rgba.data()); // alpha included
    }
    glDrawArrays(GL_POINTS, 0, GLsizei(cnt));
    glDisableClientState(GL_COLOR_ARRAY);
}

void CMarker::_drawLines(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    const size_t cnt = (_pts.size() / 6) * 2; // vertices
    if (cnt == 0)
        return;
    glLineWidth(1.0f);
    beginColorMaterial(displayAttrib);
    glNormal3dv(normalVectorForLinesAndPoints);
    glVertexPointer(3, GL_FLOAT, 0, _pts.data());
    if (!colorCoded(displayAttrib))
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, _rgba.data());
    }
    glDrawArrays(GL_LINES, 0, GLsizei(cnt));
    glDisableClientState(GL_COLOR_ARRAY);
}

void CMarker::_drawTriangles(int displayAttrib, const double /*normalVectorForLinesAndPoints*/[3]) const
{
    // User triangles must be CCW (front faces are culled otherwise)
    const size_t triCount = _pts.size() / 9;
    if (triCount == 0)
        return;
    beginColorMaterial(displayAttrib);
    std::vector<float>& V = scratchVerts();
    std::vector<float>& N = scratchNormals();
    buildFlatTriangles(_pts.data(), nullptr, triCount, V, N);
    glVertexPointer(3, GL_FLOAT, 0, V.data());
    glNormalPointer(GL_FLOAT, 0, N.data());
    if (!colorCoded(displayAttrib))
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, _rgba.data());
    }
    glDrawArrays(GL_TRIANGLES, 0, GLsizei(triCount * 3));
    glDisableClientState(GL_COLOR_ARRAY);
}

// Shared instancing loop for all "one mesh per point" item types
namespace
{
void drawInstancedMesh(const StaticMesh& mesh, int displayAttrib,
                       const std::vector<float>& pts, const std::vector<float>& quats,
                       const std::vector<float>& sizes, const std::vector<unsigned char>& rgba)
{
    const size_t cnt = pts.size() / 3;
    if (cnt == 0)
        return;
    beginColorMaterial(displayAttrib);
    glVertexPointer(3, GL_FLOAT, 0, mesh.v.data());
    glNormalPointer(GL_FLOAT, 0, mesh.n.data());
    const GLsizei nVerts = GLsizei(mesh.v.size() / 3);
    for (size_t i = 0; i < cnt; i++)
    {
        glPushMatrix();
        multItemMatrix(pts.data() + 3 * i, quats.data() + 4 * i, sizes.data() + 3 * i);
        setItemColor(displayAttrib, rgba.data() + 4 * i);
        glDrawArrays(mesh.mode, 0, nVerts);
        glPopMatrix();
    }
}
} // namespace

void CMarker::_drawQuadPoints(int displayAttrib, const double[3]) const
{
    drawInstancedMesh(quadMesh(), displayAttrib, _pts, _quats, _sizes, _rgba);
}

void CMarker::_drawDiscPoints(int displayAttrib, const double[3]) const
{
    drawInstancedMesh(discMesh(), displayAttrib, _pts, _quats, _sizes, _rgba);
}

void CMarker::_drawCubePoints(int displayAttrib, const double[3]) const
{
    drawInstancedMesh(cubeMesh(), displayAttrib, _pts, _quats, _sizes, _rgba);
}

void CMarker::_drawSpherePoints(int displayAttrib, const double[3]) const
{
    drawInstancedMesh(sphereMesh(), displayAttrib, _pts, _quats, _sizes, _rgba);
}

void CMarker::_drawCylinderPoints(int displayAttrib, const double[3]) const
{
    drawInstancedMesh(cylinderMesh(), displayAttrib, _pts, _quats, _sizes, _rgba);
}

void CMarker::_drawAxesPoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    const size_t cnt = _pts.size() / 3;
    if (cnt == 0)
        return;
    static const GLfloat axesVerts[18] = {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1};
    glLineWidth(1.0f);
    glDisable(GL_LIGHTING); // restored by glPopAttrib in drawItems
    glNormal3dv(normalVectorForLinesAndPoints);
    glVertexPointer(3, GL_FLOAT, 0, axesVerts);
    const bool cc = colorCoded(displayAttrib);
    GLubyte cols[18];
    for (size_t i = 0; i < cnt; i++)
    {
        glPushMatrix();
        multItemMatrix(_pts.data() + 3 * i, _quats.data() + 4 * i, _sizes.data() + 3 * i);
        if (!cc)
        {
            // keep the item's alpha so axes can be translucent too
            const GLubyte a = (_rgba.size() >= 4 * (i + 1)) ? _rgba[4 * i + 3] : 255;
            const GLubyte c[6][4] = {{255, 0, 0, a}, {255, 0, 0, a}, {0, 255, 0, a}, {0, 255, 0, a}, {0, 0, 255, a}, {0, 0, 255, a}};
            // glColorPointer needs 4 bytes per vertex:
            static GLubyte colBuf[24];
            for (int k = 0; k < 6; k++)
                for (int j = 0; j < 4; j++)
                    colBuf[4 * k + j] = c[k][j];
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4, GL_UNSIGNED_BYTE, 0, colBuf);
        }
        glDrawArrays(GL_LINES, 0, 6);
        glPopMatrix();
    }
    (void)cols;
    glDisableClientState(GL_COLOR_ARRAY);
}

void CMarker::_drawTubeLines(int displayAttrib, const double[3]) const
{
    const size_t cnt = _pts.size() / 6;
    if (cnt == 0 || _sizes.empty())
        return;
    beginColorMaterial(displayAttrib);
    const StaticMesh& mesh = cylinderMesh(); // closed, CCW, radius 0.5, z in [-0.5,0.5]
    glVertexPointer(3, GL_FLOAT, 0, mesh.v.data());
    glNormalPointer(GL_FLOAT, 0, mesh.n.data());
    const GLsizei nVerts = GLsizei(mesh.v.size() / 3);
    const float d = _sizes[0]; // diameter

    for (size_t i = 0; i < cnt; i++)
    {
        const float* p0 = _pts.data() + i * 6;
        const float* p1 = p0 + 3;
        float z[3] = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
        float len = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
        if (len < 1e-6f)
            continue;
        z[0] /= len; z[1] /= len; z[2] /= len;

               // orthonormal basis with z = tube axis (no acos)
        float x[3];
        if (fabsf(z[2]) < 0.9f) { x[0] = -z[1]; x[1] = z[0]; x[2] = 0.0f; }
        else                    { x[0] = 1.0f;  x[1] = 0.0f; x[2] = 0.0f; }
        // x = normalize(x - (x.z) z)
        float dot = x[0] * z[0] + x[1] * z[1] + x[2] * z[2];
        x[0] -= dot * z[0]; x[1] -= dot * z[1]; x[2] -= dot * z[2];
        float xl = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
        x[0] /= xl; x[1] /= xl; x[2] /= xl;
        const float y[3] = {z[1] * x[2] - z[2] * x[1], z[2] * x[0] - z[0] * x[2], z[0] * x[1] - z[1] * x[0]};

        const GLfloat m[16] = {
                               x[0] * d, x[1] * d, x[2] * d, 0.0f,
                               y[0] * d, y[1] * d, y[2] * d, 0.0f,
                               z[0] * len, z[1] * len, z[2] * len, 0.0f,
                               p0[0] + 0.5f * z[0] * len, p0[1] + 0.5f * z[1] * len, p0[2] + 0.5f * z[2] * len, 1.0f};

        glPushMatrix();
        glMultMatrixf(m);
        setItemColor(displayAttrib, _rgba.data() + i * 8);
        glDrawArrays(mesh.mode, 0, nVerts);
        glPopMatrix();
    }
}

namespace
{
// De-indexes triangle positions only (normals are supplied per corner elsewhere)
void expandTrianglePositions(const float* verts, const int* indices, size_t triCount, std::vector<float>& outV)
{
    outV.resize(triCount * 9);
    float* dv = outV.data();
    for (size_t k = 0; k < triCount * 3; k++, dv += 3)
    {
        const float* v = verts + size_t(indices[k]) * 3;
        dv[0] = v[0];
        dv[1] = v[1];
        dv[2] = v[2];
    }
}
} // namespace

void CMarker::_drawCustomPoints(int displayAttrib, const double[3]) const
{ // custom mesh must be CCW wound (culled rendering)
    const size_t cnt = _pts.size() / 3;
    const size_t triCount = _indices.size() / 3;
    if (cnt == 0 || triCount == 0)
        return;
    beginColorMaterial(displayAttrib);

    std::vector<float>& V = scratchVerts();
    const float* normals = nullptr;

    if (_normals.size() == triCount * 9)
    {
        // One normal per triangle corner, already in de-indexed order: bind directly
        expandTrianglePositions(_vertices.data(), _indices.data(), triCount, V);
        normals = _normals.data();
    }
    else
    {
        // Fallback: flat per-face normals computed once per call
        std::vector<float>& N = scratchNormals();
        buildFlatTriangles(_vertices.data(), _indices.data(), triCount, V, N);
        normals = N.data();
    }

    glVertexPointer(3, GL_FLOAT, 0, V.data());
    glNormalPointer(GL_FLOAT, 0, normals);

    for (size_t i = 0; i < cnt; i++)
    {
        glPushMatrix();
        multItemMatrix(_pts.data() + 3 * i, _quats.data() + 4 * i, _sizes.data() + 3 * i);
        setItemColor(displayAttrib, _rgba.data() + 4 * i);
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(triCount * 3));
        glPopMatrix();
    }
}

void CMarker::drawItems(int displayAttrib, const double normalVectorForLinesAndPoints[3], bool overlay) const
{
    const bool isOverlay = (_itemOptions & sim_markeropts_overlay) != 0;
    if (isOverlay != overlay)
        return;

    const bool cc = colorCoded(displayAttrib);
    const bool depthPass = (displayAttrib & sim_displayattribute_depthpass) != 0;

           // Any translucent item? (cheap scan; not relevant for picking/depth passes)
    bool transparent = false;
    if (!cc && !depthPass)
    {
        for (size_t i = 3; i < _rgba.size(); i += 4)
        {
            if (_rgba[i] < 255)
            {
                transparent = true;
                break;
            }
        }
    }

    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                 GL_LIGHTING_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_CURRENT_BIT);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY); // harmless for points/lines (overridden by glNormal* where no pointer set)

    if (cc)
    {
        ogl::setMaterialColor(ogl::colorBlack, ogl::colorBlack, ogl::colorBlack);
        glColor3ub(_objectHandle & 255, (_objectHandle >> 8) & 255, (_objectHandle >> 16) & 255);
        glLoadName(-1);
    }

    bool wire = ((displayAttrib & sim_displayattribute_forcewireframe) != 0) && !depthPass;
    if (displayAttrib & sim_displayattribute_forbidwireframe)
        wire = false;
    if (wire)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (isOverlay)
        glDisable(GL_DEPTH_TEST);

           // Always culled; all built-in geometry is CCW/outward
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    // Always culled; all built-in geometry is CCW/outward
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

           // Plain (unlit) colour if requested: like the axes frames, just output glColor as-is.
           // Not applied in colour-coded (picking) passes, where the caller controls lighting state.
    const bool notShaded = (_itemOptions & sim_markeropts_notshaded) != 0;
    if (notShaded && !cc)
    {
        glDisable(GL_LIGHTING);
        // GL_COLOR_MATERIAL / normals are ignored with lighting off; nothing else to do
    }
    else
        glEnable(GL_NORMALIZE); // non-uniform scales in the item matrices (only matters when lit)

    if (transparent)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // translucent items don't write depth
    }

           // Points and lines need normal arrays off (they use a single glNormal)
    if (_itemType == sim_markertype_points || _itemType == sim_markertype_lines)
        glDisableClientState(GL_NORMAL_ARRAY);

    switch (_itemType)
    {
    case sim_markertype_points:    _drawPoints(displayAttrib, normalVectorForLinesAndPoints); break;
    case sim_markertype_lines:     _drawLines(displayAttrib, normalVectorForLinesAndPoints); break;
    case sim_markertype_triangles: _drawTriangles(displayAttrib, normalVectorForLinesAndPoints); break;
    case sim_markertype_squares:   _drawQuadPoints(displayAttrib, normalVectorForLinesAndPoints); break;
    case sim_markertype_discs:     _drawDiscPoints(displayAttrib, normalVectorForLinesAndPoints); break;
    case sim_markertype_cubes:     _drawCubePoints(displayAttrib, normalVectorForLinesAndPoints); break;
    case sim_markertype_spheres:   _drawSpherePoints(displayAttrib, normalVectorForLinesAndPoints); break;
    case sim_markertype_cylinders: _drawCylinderPoints(displayAttrib, normalVectorForLinesAndPoints); break;
    case sim_markertype_custom:    _drawCustomPoints(displayAttrib, normalVectorForLinesAndPoints); break;
    default: break;
    }

    glPopClientAttrib();
    glPopAttrib(); // restores cull/blend/depth mask/lighting/color material/line & point size/polygon mode
    glPopMatrix();
}

#endif
