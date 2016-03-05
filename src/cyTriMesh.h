// cyCodeBase by Cem Yuksel
// [www.cemyuksel.com]
//-------------------------------------------------------------------------------
///
/// \file		cyTriMesh.h
/// \author		Cem Yuksel
/// \version	1.3
/// \date		November 30, 2014
///
/// \brief Triangular Mesh class.
///
//-------------------------------------------------------------------------------

#ifndef _CY_TRIMESH_H_INCLUDED_
#define _CY_TRIMESH_H_INCLUDED_

//-------------------------------------------------------------------------------

#include "cyPoint.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <vector>

//-------------------------------------------------------------------------------

/// Triangular Mesh Class

class cyTriMesh
{
public:
    /// Triangular Mesh Face
    struct cyTriFace
    {
        unsigned int v[3];	///< vertex indices
    };

    /// Mtl Material Definition
    struct cyMtl
    {
        struct Map
        {
            char	name[256];	///< filename of the texture map
            Map()
            {
                name[0] = '\0';
            }
        };
        char name[256];	///< Material name
        float Ka[3];	///< Ambient color
        float Kd[3];	///< Diffuse color
        float Ks[3];	///< Specular color
        float Tf[3];	///< Transmission color
        float Ns;		///< Specular exponent
        float Ni;		///< Index of refraction
        int illum;		///< Illumination model
        Map map_Ka;		///< Ambient texture map
        Map map_Kd;		///< Diffuse texture map
        Map map_Ks;		///< Specular texture map

        cyMtl()
        {
            name[0] = '\0';
            Ka[0] = Ka[1] = Ka[2] = 0;
            Kd[0] = Kd[1] = Kd[2] = 1;
            Ks[0] = Ks[1] = Ks[2] = 0;
            Tf[0] = Tf[1] = Tf[2] = 0;
            Ns = 0;
            Ni = 1;
            illum = 2;
        }
    };

protected:
    cyPoint3f*		v;		///< vertices
    cyTriFace*		f;		///< faces
    cyPoint3f*		vn;	///< vertex normal
    cyTriFace*		fn;	///< normal faces
    cyPoint3f*		vt;	///< texture vertices
    cyTriFace*		ft;	///< texture faces
    cyMtl*			m;		///< materials
    int*				mcfc;	///< material cumulative face count

    unsigned int	nv;		///< number of vertices
    unsigned int	nf;		///< number of faces
    unsigned int	nvn;	///< number of vertex normals
    unsigned int	nvt;	///< number of texture vertices
    unsigned int	nm;		///< number of materials

    cyPoint3f boundMin, boundMax;	///< bounding box

public:

    cyTriMesh() : v(NULL), f(NULL), vn(NULL), fn(NULL), vt(NULL), ft(NULL), m(NULL),
        mcfc(NULL)
        , nv(0), nf(0), nvn(0), nvt(0), nm(0), boundMin(0, 0, 0), boundMax(0, 0, 0) {}
    virtual ~cyTriMesh()
    {
        Clear();
    }

    ///@name Component Access Methods
    cyPoint3f&			V(int i)
    {
        return v[i];    ///< returns the i^th vertex
    }
    const cyPoint3f&	V(int i) const
    {
        return v[i];    ///< returns the i^th vertex
    }
    cyTriFace&			F(int i)
    {
        return f[i];    ///< returns the i^th face
    }
    const cyTriFace&	F(int i) const
    {
        return f[i];    ///< returns the i^th face
    }
    cyPoint3f&			VN(int i)
    {
        return vn[i];    ///< returns the i^th vertex normal
    }
    const cyPoint3f&	VN(int i) const
    {
        return vn[i];    ///< returns the i^th vertex normal
    }
    cyTriFace&			FN(int i)
    {
        return fn[i];    ///< returns the i^th normal face
    }
    const cyTriFace&	FN(int i) const
    {
        return fn[i];    ///< returns the i^th normal face
    }
    cyPoint3f&			VT(int i)
    {
        return vt[i];    ///< returns the i^th vertex texture
    }
    const cyPoint3f&	VT(int i) const
    {
        return vt[i];    ///< returns the i^th vertex texture
    }
    cyTriFace&			FT(int i)
    {
        return ft[i];    ///< returns the i^th texture face
    }
    const cyTriFace&	FT(int i) const
    {
        return ft[i];    ///< returns the i^th texture face
    }
    cyMtl&				M(int i)
    {
        return m[i];    ///< returns the i^th material
    }
    const cyMtl&		M(int i) const
    {
        return m[i];    ///< returns the i^th material
    }

    unsigned int		NV() const
    {
        return nv;    ///< returns the number of vertices
    }
    unsigned int		NF() const
    {
        return nf;    ///< returns the number of faces
    }
    unsigned int		NVN() const
    {
        return nvn;    ///< returns the number of vertex normals
    }
    unsigned int		NVT() const
    {
        return nvt;    ///< returns the number of texture vertices
    }
    unsigned int		NM() const
    {
        return nm;    ///< returns the number of materials
    }

    bool HasNormals() const
    {
        return NVN() > 0;    ///< returns true if the mesh has vertex normals
    }
    bool HasTextureVertices() const
    {
        return NVT() > 0;    ///< returns true if the mesh has texture vertices
    }

    ///@name Set Component Count
    void Clear()
    {
        SetNumVertex(0);
        SetNumFaces(0);
        SetNumNormals(0);
        SetNumTexVerts(0);
        SetNumMtls(0);
        boundMin.Zero();
        boundMax.Zero();
    }
    void SetNumVertex  (unsigned int n)
    {
        Allocate(n, v, nv);
    }
    void SetNumFaces   (unsigned int n)
    {
        if ( Allocate(n, f, nf) )
        {
            if (fn)
            {
                Allocate(n, fn);
            }

            if (ft)
            {
                Allocate(n, ft);
            }
        }
    }
    void SetNumNormals (unsigned int n)
    {
        Allocate(n, vn, nvn);

        if (!fn)
        {
            Allocate(nf, fn);
        }
    }
    void SetNumTexVerts(unsigned int n)
    {
        Allocate(n, vt, nvt);

        if (!ft)
        {
            Allocate(nf, ft);
        }
    }
    void SetNumMtls    (unsigned int n)
    {
        Allocate(n, m, nm);
        Allocate(n, mcfc);
    }

    ///@name Get Property Methods
    bool		IsBoundBoxReady() const
    {
        return boundMin.x != 0 && boundMin.y != 0 && boundMin.z != 0 && boundMax.x != 0
               && boundMax.y != 0 && boundMax.z != 0;
    }
    cyPoint3f	GetBoundMin() const
    {
        return boundMin;    ///< Returns the minimum values of the bounding box
    }
    cyPoint3f	GetBoundMax() const
    {
        return boundMax;    ///< Returns the maximum values of the bounding box
    }
    cyPoint3f	GetPoint   (int faceID, const cyPoint3f& bc) const
    {
        return Interpolate(faceID, v, f,
                           bc);    ///< Returns the point on the given face with the given barycentric coordinates (bc).
    }
    cyPoint3f	GetNormal  (int faceID, const cyPoint3f& bc) const
    {
        return Interpolate(faceID, vn, fn,
                           bc);    ///< Returns the the surface normal on the given face at the given barycentric coordinates (bc). The returned vector is not normalized.
    }
    cyPoint3f	GetTexCoord(int faceID, const cyPoint3f& bc) const
    {
//        qDebug() << "face: " << faceID;
//        qDebug() << "vt: "<< vt[faceID].x << vt[faceID].y << vt[faceID].z;
        return Interpolate(faceID, vt, ft,
                           bc);    ///< Returns the texture coordinate on the given face at the given barycentric coordinates (bc).
    }
    int			GetMaterialIndex(int faceID)
    const;				///< Returns the material index of the face. This method goes through material counts of all materials to find the material index of the face. Returns a negaive number if the face as no material
    int			GetMaterialFaceCount(int mtlID) const
    {
        return mtlID > 0 ? mcfc[mtlID] - mcfc[mtlID - 1] :
               mcfc[0];    ///< Returns the number of faces associated with the given material ID.
    }
    int			GetMaterialFirstFace(int mtlID) const
    {
        return mtlID > 0 ? mcfc[mtlID - 1] :
               0;    ///< Returns the first face index associated with the given material ID. Other faces associated with the same material are placed are placed consecutively.
    }

    ///@name Compute Methods
    void ComputeBoundingBox();						///< Computes the bounding box
    void ComputeNormals(bool clockwise = false);		///< Computes and stores vertex normals

    ///@name Load and Save methods
    bool LoadFromFileObj( const char* filename,
                          bool loadMtl =
                              true );	///< Loads the mesh from an OBJ file. Automatically converts all faces to triangles.

private:
    template <class T> void Allocate(unsigned int n, T*& t)
    {
        if (t)
        {
            delete [] t;
        }

        if (n > 0)
        {
            t = new T[n];
        }
        else
        {
            t = NULL;
        }
    }
    template <class T> bool Allocate(unsigned int n, T*& t, unsigned int& nt)
    {
        if (n == nt)
        {
            return false;
        }

        nt = n;
        Allocate(n, t);
        return true;
    }
    static cyPoint3f Interpolate( int i, const cyPoint3f* v, const cyTriFace* f,
                                  const cyPoint3f& bc )
    {
        return v[f[i].v[0]] * bc.x + v[f[i].v[1]] * bc.y + v[f[i].v[2]] * bc.z;
    }
};

//-------------------------------------------------------------------------------

inline int cyTriMesh::GetMaterialIndex(int faceID) const
{
    for ( unsigned int i = 0; i < nm; i++ )
    {
        if ( faceID < mcfc[i] )
        {
            return (int) i;
        }
    }

    return -1;
}

inline void cyTriMesh::ComputeBoundingBox()
{
    boundMin = v[0];
    boundMax = v[0];

    for ( unsigned int i = 1; i < nv; i++ )
    {
        if ( boundMin.x > v[i].x )
        {
            boundMin.x = v[i].x;
        }

        if ( boundMin.y > v[i].y )
        {
            boundMin.y = v[i].y;
        }

        if ( boundMin.z > v[i].z )
        {
            boundMin.z = v[i].z;
        }

        if ( boundMax.x < v[i].x )
        {
            boundMax.x = v[i].x;
        }

        if ( boundMax.y < v[i].y )
        {
            boundMax.y = v[i].y;
        }

        if ( boundMax.z < v[i].z )
        {
            boundMax.z = v[i].z;
        }
    }
}

inline void cyTriMesh::ComputeNormals(bool clockwise)
{
    SetNumNormals(nv);

    for ( unsigned int i = 0; i < nvn; i++ )
    {
        vn[i].Set(0, 0, 0);    // initialize all normals to zero
    }

    for ( unsigned int i = 0; i < nf; i++ )
    {
        cyPoint3f N = (v[f[i].v[1]] - v[f[i].v[0]]) ^ (v[f[i].v[2]] -
                                                       v[f[i].v[0]]);	// face normal (not normalized)

        if ( clockwise )
        {
            N = -N;
        }

        vn[f[i].v[0]] += N;
        vn[f[i].v[1]] += N;
        vn[f[i].v[2]] += N;
        fn[i] = f[i];
    }

    for ( unsigned int i = 0; i < nvn; i++ )
    {
        vn[i].Normalize();
    }
}

#include <QFile>
#include <QString>
#include <QChar>

inline bool cyTriMesh::LoadFromFileObj( const char* filename, bool loadMtl )
{
//    QFile myFile(filename);
//    myFile.open(QIODevice::ReadOnly);
//    int fileHandle = myFile.handle();
//    FILE* fp = fdopen(fileHandle, "r");
//    FILE *fp = fopen(filename,"r");
//    if ( !fp ) return false;

    QFile fp(filename);

    if (!fp.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream in(&fp);


    Clear();

    class Buffer
    {
        char data[1024];
        int readLine;
    public:
        int ReadLine(QString line)
        {
            int k = 0;
            int i = 0;

            while(line.at(k).isSpace() && k < line.length())
            {
                ++k;
            }

            bool inspace = false;

            while(k < line.length())
            {
                QChar c = line.at(k);

                if ( c == '#' )
                {
                    break;
                }

                if( i < 1024 - 1 )
                {
                    if ( c.isSpace() )  	// only use a single space as the space character
                    {
                        inspace = true;
                    }
                    else
                    {
                        if ( inspace )
                        {
                            data[i++] = ' ';
                        }

                        inspace = false;
                        data[i++] = c.toLatin1();
                    }

                    ++k;
                }
            }

            data[i] = '\0';
            readLine = i;
            return i;
        }
        char& operator[](int i)
        {
            return data[i];
        }
        void ReadVertex( cyPoint3f& v ) const
        {
            sscanf( data + 2, "%f %f %f", &v.x, &v.y, &v.z );
        }
        void ReadFloat3( float f[3] ) const
        {
            sscanf( data + 2, "%f %f %f", &f[0], &f[1], &f[2] );
        }
        void ReadFloat( float* f ) const
        {
            sscanf( data + 2, "%f", f );
        }
        void ReadInt( int* i, int start ) const
        {
            sscanf( data + start, "%d", i );
        }
        bool IsCommand( const char* cmd ) const
        {
            int i = 0;

            while ( cmd[i] != '\0' )
            {
                if ( cmd[i] != data[i] )
                {
                    return false;
                }

                i++;
            }

            return (data[i] == '\0' || data[i] == ' ');
        }
        void Copy( char* a, int count, int start = 0 ) const
        {
            strncpy( a, data + start, count - 1 );
            a[count - 1] = '\0';
        }
    };
    Buffer buffer;


    unsigned int numVerts = 0, numTVerts = 0, numNormals = 0, numFaces = 0, numMtls = 0;

    struct MtlData
    {
        char mtlName[256];
        int faceCount;
        MtlData()
        {
            mtlName[0] = '\0';
            faceCount = 0;
        }
    };
    struct MtlList
    {
        std::vector<MtlData> mtlData;
        int GetMtlIndex(const char* mtlName)
        {
            for ( unsigned int i = 0; i < mtlData.size(); i++ )
            {
                if ( strcmp(mtlName, mtlData[i].mtlName) == 0 )
                {
                    return (int)i;
                }
            }

            return -1;
        }
        MtlData* CreateMtl(const char* mtlName)
        {
            if ( mtlName[0] == '\0' )
            {
                return NULL;
            }

            int i = GetMtlIndex(mtlName);

            if ( i >= 0 )
            {
                return &mtlData[i];
            }

            MtlData m;
            strncpy(m.mtlName, mtlName, 256);
            mtlData.push_back(m);
            return &mtlData.back();
        }
    };
    MtlList mtlList;
    MtlData* currentMtlData = NULL;
    struct MtlLibName
    {
        char filename[1024];
    };
    std::vector<MtlLibName> mtlFiles;


//	while ( int rb = buffer.ReadLine(fp) ) {
    while (!in.atEnd())
    {
        QString line = in.readLine();

        if(line.trimmed().length() == 0)
        {
            continue;
        }

        int rb = buffer.ReadLine(line);

        if(rb == 0)
        {
            continue;
        }


        if ( buffer.IsCommand("v") )
        {
            numVerts++;
        }
        else if ( buffer.IsCommand("vt") )
        {
            numTVerts++;
        }
        else if ( buffer.IsCommand("vn") )
        {
            numNormals++;
        }
        else if ( buffer.IsCommand("f") )
        {
            int nFaceVerts = 0; // count face vertices
            bool inspace = true;

            for ( int i = 2; i < rb - 1; i++ )
            {
                if ( buffer[i] == ' ' || buffer[i] == '\t' )
                {
                    inspace = true;
                }
                else if ( inspace )
                {
                    nFaceVerts++;
                    inspace = false;
                }
            }

            if ( nFaceVerts > 2 )
            {
                numFaces += nFaceVerts - 2; // non-triangle faces will %be multiple triangle faces

                if ( currentMtlData )
                {
                    currentMtlData->faceCount += nFaceVerts - 2;
                }
            }
        }

//        else if ( loadMtl )
//        {
//            if ( buffer.IsCommand("usemtl") )
//            {
//                char mtlName[256];
//                buffer.Copy(mtlName, 256, 7);
//                currentMtlData = mtlList.CreateMtl(mtlName);
//            }

//            if ( buffer.IsCommand("mtllib") )
//            {
//                MtlLibName libName;
//                buffer.Copy(libName.filename, 1024, 7);
//                mtlFiles.push_back(libName);
//            }
//        }

//		if ( feof(fp) ) break;
    }

    if ( numFaces == 0 )
    {
        return true;    // No faces found
    }

    SetNumVertex(numVerts);
    SetNumFaces(numFaces);
    SetNumNormals(numNormals);
    SetNumTexVerts(numTVerts);

    if ( loadMtl )
    {
        SetNumMtls(mtlList.mtlData.size());
    }

    unsigned int readVerts = 0;
    unsigned int readTVerts = 0;
    unsigned int readNormals = 0;
    unsigned int readFaces = 0;

    int currentMtlIndex = -1;
    std::vector<int> mtlFaceIndex;

    if ( loadMtl )
    {
        mtlFaceIndex.resize( NM() );

        for ( unsigned int i = 0; i < NM(); i++ )
        {
            mtlFaceIndex[i] = readFaces;
            readFaces += mtlList.mtlData[i].faceCount;
            mcfc[i] = readFaces;
        }
    }

//	rewind(fp);

    in.device()->seek( 0 );

    while (!in.atEnd())
    {
        QString line = in.readLine();
        if(line.trimmed().length() == 0)
        {
            continue;
        }
//qDebug() << line;
        int rb = buffer.ReadLine(line);

        if(rb == 0)
        {
            continue;
        }


        if ( buffer.IsCommand("v") )
        {
            buffer.ReadVertex(v[readVerts++]);
        }
        else if ( buffer.IsCommand("vt") )
        {
            buffer.ReadVertex(vt[readTVerts++]);
        }
        else if ( buffer.IsCommand("vn") )
        {
            buffer.ReadVertex(vn[readNormals++]);
        }
        else if ( buffer.IsCommand("f") )
        {
            int facevert = -1;
            bool inspace = true;
            int type = 0;
            unsigned int index;
            int rf = currentMtlIndex >= 0 ? mtlFaceIndex[currentMtlIndex] : readFaces;

            for ( int i = 2; i < rb; i++ )
            {
                if ( buffer[i] == ' ' )
                {
                    inspace = true;
                }
                else
                {
                    if ( inspace )
                    {
                        inspace = false;
                        type = 0;
                        index = 0;

                        switch ( facevert )
                        {
                        case -1:
                            // initialize face
                            f[rf].v[0] = f[rf].v[1] = f[rf].v[2] = 0;

                            if ( ft )
                            {
                                ft[rf].v[0] = ft[rf].v[1] = ft[rf].v[2] = 0;
                            }

                            if ( fn )
                            {
                                fn[rf].v[0] = fn[rf].v[1] = fn[rf].v[2] = 0;
                            }

                        case 0:
                        case 1:
                            facevert++;
                            break;

                        case 2:
                            // copy the first two vertices from the previous face
                            rf++;
                            f[rf].v[0] = f[rf - 1].v[0];
                            f[rf].v[1] = f[rf - 1].v[2];

                            if ( ft )
                            {
                                ft[rf].v[0] = ft[rf - 1].v[0];
                                ft[rf].v[1] = ft[rf - 1].v[2];
                            }

                            if ( fn )
                            {
                                fn[rf].v[0] = fn[rf - 1].v[0];
                                fn[rf].v[1] = fn[rf - 1].v[2];
                            }

                            break;
                        }
                    }

                    if ( buffer[i] == '/' )
                    {
                        type++;
                        index = 0;
                    }

                    if ( buffer[i] >= '0' && buffer[i] <= '9' )
                    {
                        index = index * 10 + (buffer[i] - '0');

                        switch ( type )
                        {
                        case 0:
                            f[rf].v[facevert] = index - 1;
                            break;

                        case 1:
                            if (ft)
                            {
                                ft[rf].v[facevert] = index - 1;
//                                qDebug() << "facetex " << rf << ", " << facevert << " = "
//                                         << index - 1;
                            }

                            break;

                        case 2:
                            if (fn)
                            {
                                fn[rf].v[facevert] = index - 1;
                            }

                            break;
                        }
                    }
                }
            }

            rf++;

            if ( currentMtlIndex >= 0 )
            {
                mtlFaceIndex[currentMtlIndex] = rf;
            }
            else
            {
                readFaces = rf;
            }
        }

//        else if ( loadMtl && buffer.IsCommand("usemtl") )
//        {
//            char mtlName[256];
//            buffer.Copy(mtlName, 256, 7);
//            currentMtlIndex = mtlList.GetMtlIndex(mtlName);
//        }

//		if ( feof(fp) ) break;
    }

//	fclose(fp);

    // Load the .mtl files
//	if ( loadMtl ) {
//		// get the path from filename
//		char *mtlFullFilename = NULL;
//		char *mtlFilename = NULL;
//		const char* pathEnd = strrchr(filename,'\\');
//		if ( !pathEnd ) pathEnd = strrchr(filename,'/');
//		if ( pathEnd ) {
//			int n = pathEnd-filename + 1;
//			mtlFullFilename = new char[n+1024];
//			strncpy(mtlFullFilename,filename,n);
//			mtlFilename = &mtlFullFilename[n];
//		} else {
//			mtlFullFilename = new char[1024];
//			mtlFilename = mtlFullFilename;
//		}
//		for ( unsigned int mi=0; mi<mtlFiles.size(); mi++ ) {
//			strncpy( mtlFilename, mtlFiles[mi].filename, 1024 );
//			FILE *fp = fopen(mtlFullFilename,"r");
//			if ( !fp ) continue;
//			int mtlID = -1;
//			while ( int rb = buffer.ReadLine(fp) ) {
//				if ( buffer.IsCommand("newmtl") ) {
//					char mtlName[256];
//					buffer.Copy(mtlName,256,7);
//					mtlID = mtlList.GetMtlIndex(mtlName);
//					if ( mtlID >= 0 ) strncpy(m[mtlID].name,mtlName,256);
//				} else if ( mtlID >= 0 ) {
//					if ( buffer.IsCommand("Ka") ) buffer.ReadFloat3( m[mtlID].Ka );
//					else if ( buffer.IsCommand("Kd") ) buffer.ReadFloat3( m[mtlID].Kd );
//					else if ( buffer.IsCommand("Ks") ) buffer.ReadFloat3( m[mtlID].Ks );
//					else if ( buffer.IsCommand("Tf") ) buffer.ReadFloat3( m[mtlID].Tf );
//					else if ( buffer.IsCommand("Ns") ) buffer.ReadFloat( &m[mtlID].Ns );
//					else if ( buffer.IsCommand("Ni") ) buffer.ReadFloat( &m[mtlID].Ni );
//					else if ( buffer.IsCommand("illum") ) buffer.ReadInt( &m[mtlID].illum, 5 );
//					else if ( buffer.IsCommand("map_Ka") ) buffer.Copy( m[mtlID].map_Ka.name, 256, 7 );
//					else if ( buffer.IsCommand("map_Kd") ) buffer.Copy( m[mtlID].map_Kd.name, 256, 7 );
//					else if ( buffer.IsCommand("map_Ks") ) buffer.Copy( m[mtlID].map_Ks.name, 256, 7 );
//				}
//			}
//			fclose(fp);
//		}
//		delete [] mtlFullFilename;
//	}
    return true;
}

//-------------------------------------------------------------------------------

namespace cy
{
typedef cyTriMesh TriMesh;
}

//-------------------------------------------------------------------------------

#endif

