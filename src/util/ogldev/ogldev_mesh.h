/*

	Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MESH_H
#define	MESH_H

#include <map>
#include <vector>
#include <Importer.hpp>      // C++ importer interface
#include <scene.h>       // Output data structure
#include <postprocess.h> // Post processing flags

#include "ogldev_util.h"
#include "ogldev_math_3d.h"
#include "ogldev_texture.h"

class Mesh
{
public:
    Mesh(GLint attrPosition, GLint attrNormal, GLint attrTextureCoord);

    ~Mesh();

    bool LoadMesh(const std::string& Filename);

    void Render();

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF

    struct MeshEntry {
        MeshEntry();

        ~MeshEntry();

        void Init(const std::vector<GLfloat>& position,
                const std::vector<GLfloat>& textureCoord,
                const std::vector<GLfloat>& normal,
                const std::vector<unsigned int>& Indices);

        std::vector<GLfloat> mPosition;
        std::vector<GLfloat> mTextureCoord;
        std::vector<GLfloat> mNormal;
        std::vector<unsigned int> mIndices;
        unsigned int mMaterialIndex;
    };

    std::vector<MeshEntry> mEntries;
    std::vector<Texture*> mTextures;

    GLint mAttrPosition;
    GLint mAttrNormal;
    GLint mAttrTextureCoord;
};


#endif	/* MESH_H */

