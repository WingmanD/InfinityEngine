#include "StaticMesh.h"
#include "StaticMeshRenderingData.h"
#include "Util.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Engine/Engine.h"
#include "MaterialParameterTypes.h"
#include "AssetPtr.h"

IDGenerator<uint32> StaticMesh::_meshIDGenerator = IDGenerator<uint32>(0);
std::unordered_map<uint32, std::weak_ptr<StaticMesh>> StaticMesh::_meshIDToStaticMesh;
DynamicGPUBuffer2<StaticMesh::MeshInfo> StaticMesh::_meshInfoBuffer;

std::shared_ptr<StaticMesh> StaticMesh::GetMeshByID(uint32 meshID)
{
    return _meshIDToStaticMesh[meshID].lock();
}

DynamicGPUBuffer2<StaticMesh::MeshInfo>& StaticMesh::GetMeshInfoBuffer()
{
    return _meshInfoBuffer;
}

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
    SetImporterType(StaticMeshImporter::StaticType());
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

    if (!_vertices.IsEmpty() && !_indices.IsEmpty())
    {
        _renderingData->UploadToGPU(renderingSubsystem);
    }
    
    if (_material != nullptr)
    {
        MaterialParameterMap* paramMap = _material->GetParameterMap();
        if (paramMap != nullptr)
        {
            paramMap->SetSharedParameter("GScene", renderingSubsystem.GetScene(), true);
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

    writer << _boundingBox;

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

    reader >> _boundingBox;

    return true;
}

const DArray<Vertex>& StaticMesh::GetVertices() const
{
    return _vertices;
}

const DArray<uint32_t>& StaticMesh::GetIndices() const
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
    _material->GetParameterMap()->SetSharedParameter("GPerPassConstants", renderingSubsystem.GetScene(), true);

    MarkDirtyForAutosave();
}

std::shared_ptr<Material> StaticMesh::GetMaterial() const
{
    return _material;
}

uint32 StaticMesh::GetMeshID() const
{
    return _meshID;
}

StaticMeshRenderingData* StaticMesh::GetRenderingData() const
{
    return _renderingData.get();
}

const BoundingBox& StaticMesh::GetBoundingBox() const
{
    return _boundingBox;
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

void StaticMesh::PostLoad()
{
    Asset::PostLoad();

    _meshID = _meshIDGenerator.GenerateID();
    _meshIDToStaticMesh[_meshID] = SharedFromThis();

    // todo we need to handle removal of meshes - we need to set mesh info at _meshID index, but DArray might get in the way
    _meshInfoBuffer.Add({_boundingBox.GetMin(), _boundingBox.GetMax()});
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

    _vertices.Reserve(assimpMesh->mNumVertices);
    _indices.Reserve(3 * assimpMesh->mNumFaces);

    for (uint32 i = 0; i < assimpMesh->mNumVertices; i++)
    {
        _vertices.Emplace();
        Vertex& vertex = _vertices.Back();

        vertex.Position = Vector3(
            assimpMesh->mVertices[i].x,
            assimpMesh->mVertices[i].y,
            assimpMesh->mVertices[i].z
        );

        if (assimpMesh->HasNormals())
        {
            vertex.Normal = Vector3(
                assimpMesh->mNormals[i].x,
                assimpMesh->mNormals[i].y,
                assimpMesh->mNormals[i].z
            );
        }

        if (assimpMesh->HasTangentsAndBitangents())
        {
            vertex.Tangent = Vector3(
                assimpMesh->mTangents[i].x,
                assimpMesh->mTangents[i].y,
                assimpMesh->mTangents[i].z
            );

            vertex.Bitangent = Vector3(
                assimpMesh->mBitangents[i].x,
                assimpMesh->mBitangents[i].y,
                assimpMesh->mBitangents[i].z
            );
        }

        if (assimpMesh->HasVertexColors(0))
        {
            vertex.Color = Vector4(
                assimpMesh->mColors[0][i].r,
                assimpMesh->mColors[0][i].g,
                assimpMesh->mColors[0][i].b,
                assimpMesh->mColors[0][i].a
            );
        }

        if (assimpMesh->HasTextureCoords(0))
        {
            vertex.UV = Vector2(
                assimpMesh->mTextureCoords[0][i].x,
                assimpMesh->mTextureCoords[0][i].y
            );
        }
    }

    for (uint32 faceIndex = 0; faceIndex < assimpMesh->mNumFaces; ++faceIndex)
    {
        for (uint32 j = 0; j < assimpMesh->mFaces[faceIndex].mNumIndices; ++j)
        {
            _indices.Emplace(assimpMesh->mFaces[faceIndex].mIndices[j]);
        }
    }
    _indices.ShrinkToFit();

    UpdateBoundingBox();

    SetIsLoaded(true);

    if (!Initialize())
    {
        LOG(L"Failed to initialize static mesh: {}", GetName().ToString());
        return false;
    }

    return true;
}

void StaticMesh::UpdateBoundingBox()
{
    Vector3 aabbMin = Vector3::Zero;
    Vector3 aabbMax = Vector3::Zero;
    
    for (const Vertex& vertex : _vertices)
    {
        if (vertex.Position.x < aabbMin.x)
        {
            aabbMin.x = vertex.Position.x;
        }
        else if (vertex.Position.x > aabbMax.x)
        {
            aabbMax.x = vertex.Position.x;
        }

        if (vertex.Position.y < aabbMin.y)
        {
            aabbMin.y = vertex.Position.y;
        }
        else if (vertex.Position.y > aabbMax.y)
        {
            aabbMax.y = vertex.Position.y;
        }

        if (vertex.Position.z < aabbMin.z)
        {
            aabbMin.z = vertex.Position.z;
        }
        else if (vertex.Position.z > aabbMax.z)
        {
            aabbMax.z = vertex.Position.z;
        }
    }

    _boundingBox = BoundingBox(aabbMin, aabbMax);
}
