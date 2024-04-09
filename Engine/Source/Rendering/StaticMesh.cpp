#include "StaticMesh.h"
#include "StaticMeshRenderingData.h"
#include "Util.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Engine/Engine.h"
#include "MaterialParameterTypes.h"
#include "AssetPtr.h"

StaticMesh::StaticMesh()
{
    SetImporterType(StaticMeshImporter::StaticType());
}

StaticMesh::StaticMesh(const StaticMesh& other) : Asset(other)
{
    _vertices = other._vertices;
    _indices = other._indices;
    _material = other._material;
}

StaticMesh& StaticMesh::operator=(const StaticMesh& other)
{
    if (this == &other)
    {
        return *this;
    }

    _vertices = other._vertices;
    _indices = other._indices;
    _material = other._material;

    return *this;
}

StaticMesh::StaticMesh(Name name) : Asset(name)
{
}

bool StaticMesh::Initialize()
{
    RenderingSubsystem& renderingSubsystem = RenderingSubsystem::Get();

    _renderingData = renderingSubsystem.CreateStaticMeshRenderingData();
    if (_renderingData == nullptr)
    {
        return false;
    }

    _renderingData->SetMesh(std::dynamic_pointer_cast<StaticMesh>(shared_from_this()), {});
    _renderingData->UploadToGPU(renderingSubsystem);

    if (_material != nullptr)
    {
        MaterialParameterMap* paramMap = _material->GetParameterMap();
        if (paramMap != nullptr)
        {
            paramMap->SetSharedParameter("GPerPassConstants", renderingSubsystem.GetPerPassConstants(), true);
        }
    }

    return true;
}

bool StaticMesh::Serialize(MemoryWriter& writer) const
{
    if (!Asset::Serialize(writer))
    {
        return false;
    }

    writer << _vertices;
    writer << _indices;

    if (_material == nullptr)
    {
        writer << static_cast<uint64>(0u);
    }
    else
    {
        writer << _material->GetAssetID();
    }

    return true;
}

bool StaticMesh::Deserialize(MemoryReader& reader)
{
    if (!Asset::Deserialize(reader))
    {
        return false;
    }

    reader >> _vertices;
    reader >> _indices;

    uint64 materialAssetID;
    reader >> materialAssetID;
    _material = AssetManager::Get().FindAsset<Material>(materialAssetID);

    return true;
}

const std::vector<Vertex>& StaticMesh::GetVertices() const
{
    return _vertices;
}

const std::vector<uint32_t>& StaticMesh::GetIndices() const
{
    return _indices;
}

void StaticMesh::SetMaterial(const std::shared_ptr<Material>& material)
{
    if (_material == material)
    {
        return;
    }

    _material = material;

    const RenderingSubsystem& renderingSubsystem = RenderingSubsystem::Get();
    _material->GetParameterMap()->SetSharedParameter("GPerPassConstants", renderingSubsystem.GetPerPassConstants(), true);

    MarkDirtyForAutosave();
}

std::shared_ptr<Material> StaticMesh::GetMaterial() const
{
    return _material;
}

StaticMeshRenderingData* StaticMesh::GetRenderingData() const
{
    return _renderingData.get();
}

std::vector<std::shared_ptr<Asset>> StaticMesh::Import(const std::shared_ptr<Importer>& importer) const
{
    const std::shared_ptr<StaticMeshImporter> smImporter = std::dynamic_pointer_cast<StaticMeshImporter>(importer);
    if (smImporter == nullptr)
    {
        DEBUG_BREAK();
        return {};
    }

    const std::filesystem::path& path = smImporter->Path;

    Assimp::Importer assimpImporter;
    const aiScene* scene = assimpImporter.ReadFile(path.string(),
                                                   aiProcess_CalcTangentSpace |
                                                   aiProcess_Triangulate |
                                                   aiProcess_JoinIdenticalVertices |
                                                   aiProcess_SortByPType |
                                                   aiProcess_FlipUVs);

    if (scene == nullptr)
    {
        LOG(L"Failed to import static meshes: {}", Util::ToWString(assimpImporter.GetErrorString()));
        return {};
    }

    if (!scene->HasMeshes())
    {
        return {};
    }

    std::vector<std::shared_ptr<Asset>> meshes;
    for (uint32 i = 0; i < scene->mNumMeshes; ++i)
    {
        std::wstring meshName = Util::ToWString(scene->mMeshes[i]->mName.C_Str());
        std::shared_ptr newMesh = AssetManager::Get().NewAsset<StaticMesh>(Name(meshName));
        if (newMesh == nullptr)
        {
            LOG(L"Error creating new static mesh: {}", meshName);
            continue;
        }

        // todo import path, reimport (save index of mesh in scene?)

        if (!newMesh->ImportInternal(scene->mMeshes[i]))
        {
            AssetManager::Get().DeleteAsset(newMesh);
            LOG(L"Failed to import static mesh: {}", Util::ToWString(scene->mMeshes[i]->mName.C_Str()));
            continue;
        }

        std::shared_ptr<Material> defaultMaterial = AssetManager::Get().FindAssetByName<Material>(
            Name(L"DefaultMaterial"));
        if (defaultMaterial == nullptr)
        {
            LOG(L"Failed to find default material!");
            return {};
        }
        defaultMaterial->Load();

        newMesh->SetMaterial(defaultMaterial);
        newMesh->SetImportPath(path);

        meshes.emplace_back(newMesh);
    }

    return meshes;
}

bool StaticMesh::ImportInternal(const aiMesh* assimpMesh)
{
    if (assimpMesh == nullptr)
    {
        return false;
    }

    SetName(Name(Util::ToWString(assimpMesh->mName.C_Str())));

    if (assimpMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
    {
        LOG(L"Failed to import Static Mesh {}: Unsupported primitive type!", GetName().ToString());
        return false;
    }

    _vertices.reserve(assimpMesh->mNumVertices);
    _indices.reserve(3 * assimpMesh->mNumFaces);

    for (uint32 i = 0; i < assimpMesh->mNumVertices; i++)
    {
        _vertices.emplace_back();
        Vertex& vertex = _vertices.back();

        vertex.Position = Vector3(
            assimpMesh->mVertices[i].x,
            assimpMesh->mVertices[i].y,
            assimpMesh->mVertices[i].z);

        if (assimpMesh->HasNormals())
        {
            vertex.Normal = Vector3(
                assimpMesh->mNormals[i].x,
                assimpMesh->mNormals[i].y,
                assimpMesh->mNormals[i].z);
        }

        if (assimpMesh->HasVertexColors(0))
        {
            vertex.Color = Vector4(
                assimpMesh->mColors[0][i].r,
                assimpMesh->mColors[0][i].g,
                assimpMesh->mColors[0][i].b,
                assimpMesh->mColors[0][i].a);
        }

        if (assimpMesh->HasTextureCoords(0))
        {
            vertex.UV = Vector2(
                assimpMesh->mTextureCoords[0][i].x,
                assimpMesh->mTextureCoords[0][i].y);
        }
    }

    for (uint32 faceIndex = 0; faceIndex < assimpMesh->mNumFaces; ++faceIndex)
    {
        for (uint32 j = 0; j < assimpMesh->mFaces[faceIndex].mNumIndices; ++j)
        {
            _indices.emplace_back(assimpMesh->mFaces[faceIndex].mIndices[j]);
        }
    }
    _indices.shrink_to_fit();

    SetIsLoaded(true);

    if (!Initialize())
    {
        LOG(L"Failed to initialize static mesh: {}", GetName().ToString());
        return false;
    }

    return true;
}
