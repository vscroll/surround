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



#include "ogldev_mesh.h"

Mesh::MeshEntry::MeshEntry()
{
    mMaterialIndex = INVALID_MATERIAL;
};

Mesh::MeshEntry::~MeshEntry()
{

}

void Mesh::MeshEntry::Init(const std::vector<GLfloat>& position,
                const std::vector<GLfloat>& textureCoord,
                const std::vector<GLfloat>& normal,
                const std::vector<unsigned int>& indices)
{
    mPosition = position;
    mTextureCoord = textureCoord;
    mNormal = normal;
    mIndices = indices;
}

Mesh::Mesh(GLint attrPosition, GLint attrNormal, GLint attrTextureCoord)
{
    mAttrPosition = attrPosition;
    mAttrNormal = attrNormal;
    mAttrTextureCoord = attrTextureCoord;
}


Mesh::~Mesh()
{
    Clear();
}


void Mesh::Clear()
{
    for (unsigned int i = 0 ; i < mTextures.size() ; i++) {
        SAFE_DELETE(mTextures[i]);
    }
}


bool Mesh::LoadMesh(const std::string& Filename)
{
    // Release the previously loaded mesh (if it exists)
    Clear();
    
    bool Ret = false;
    Assimp::Importer Importer;

    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), ASSIMP_LOAD_FLAGS);
    
    if (pScene) {
        Ret = InitFromScene(pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    return Ret;
}

bool Mesh::InitFromScene(const aiScene* pScene, const std::string& Filename)
{  
    mEntries.resize(pScene->mNumMeshes);
    mTextures.resize(pScene->mNumMaterials);

    printf("InitFromScene mNumMeshes:%d, mNumMaterials:%d\n", pScene->mNumMeshes, pScene->mNumMaterials);

    // Initialize the meshes in the scene one by one
    for (unsigned int i = 0 ; i < mEntries.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh);
    }

    return InitMaterials(pScene, Filename);
}

void Mesh::InitMesh(unsigned int Index, const aiMesh* paiMesh)
{
    mEntries[Index].mMaterialIndex = paiMesh->mMaterialIndex;
    
    std::vector<GLfloat> position;
    std::vector<GLfloat> textureCoord;
    std::vector<GLfloat> normal;
    std::vector<unsigned int> indices;

    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

        position.push_back(pPos->x);
        position.push_back(pPos->y);
        position.push_back(pPos->z);

        textureCoord.push_back(pTexCoord->x);
        textureCoord.push_back(pTexCoord->y);

        normal.push_back(pNormal->x);
        normal.push_back(pNormal->y);
        normal.push_back(pNormal->z);
    }

    for (unsigned int i = 0 ; i < paiMesh->mNumFaces ; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        indices.push_back(Face.mIndices[0]);
        indices.push_back(Face.mIndices[1]);
        indices.push_back(Face.mIndices[2]);
    }

    printf("InitMesh %d, MaterialIndex:%d, position:%d textureCoord:%d normal:%d face:%d\n",
        Index, mEntries[Index].mMaterialIndex, position.size(), textureCoord.size(), normal.size(), paiMesh->mNumFaces);

    printf("InitMesh %d, indices first:%d %d %d, last:%d %d %d\n",
        Index, indices[0], indices[1], indices[2],
        indices[indices.size()-3], indices[indices.size()-2], indices[indices.size()-1]);

    mEntries[Index].Init(position, textureCoord, normal, indices);
}

bool Mesh::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
    // Extract the directory part from the file name
    std::string::size_type SlashIndex = Filename.find_last_of("/");
    std::string Dir;

    if (SlashIndex == std::string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;

    // Initialize the materials
    for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        mTextures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string FullPath = Dir + "/" + Path.data;
                mTextures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                if (!mTextures[i]->Load()) {
                    printf("Error loading texture '%s'\n", FullPath.c_str());
                    delete mTextures[i];
                    mTextures[i] = NULL;
                    Ret = false;
                }
                else {
                    printf("Loaded texture '%s'\n", FullPath.c_str());
                }
            }
        }

        // Load a white texture in case the model does not include its own texture
        if (!mTextures[i]) {
            printf("Loaded texture ./Content/white.png \n");
            mTextures[i] = new Texture(GL_TEXTURE_2D, "./Content/white.png");
            Ret = mTextures[i]->Load();
        }
    }

    return Ret;
}

void Mesh::Render()
{

    glEnableVertexAttribArray(mAttrPosition);
    glEnableVertexAttribArray(mAttrTextureCoord);
    glEnableVertexAttribArray(mAttrNormal);

    for (unsigned int i = 0 ; i < mEntries.size() ; i++) {

        glVertexAttribPointer(mAttrPosition, 3,  GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), &(mEntries[i].mPosition[0]));
        glVertexAttribPointer(mAttrTextureCoord, 2,  GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), &(mEntries[i].mTextureCoord[0]));
        glVertexAttribPointer(mAttrNormal, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), &(mEntries[i].mNormal[0]));

        const unsigned int materialIndex = mEntries[i].mMaterialIndex;

        if (materialIndex < mTextures.size() && mTextures[materialIndex]) {
            mTextures[materialIndex]->Bind(GL_TEXTURE0);
        }

        glDrawElements(GL_TRIANGLES, mEntries[i].mIndices.size(), GL_UNSIGNED_INT, &(mEntries[i].mIndices[0]));
    }

    glDisableVertexAttribArray(mAttrPosition);
    glDisableVertexAttribArray(mAttrTextureCoord);
    glDisableVertexAttribArray(mAttrNormal);
}
