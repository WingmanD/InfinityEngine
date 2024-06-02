#include "StaticMesh.h"
#include "StaticMeshRenderingData.h"
#include "Util.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Engine/Engine.h"
#include "AssetPtr.h"
#include "Math/Math.h"

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
    _lods = other._lods;
    _material = other._material;
}

StaticMesh& StaticMesh::operator=(const StaticMesh& other)
{
    if (this == &other)
    {
        return *this;
    }

    _lods = other._lods;
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

    for (uint32 i = 0; i < _lods.Count(); ++i)
    {
        LOD& lod = _lods[i];
        
        lod.RenderingData = renderingSubsystem.CreateStaticMeshRenderingData();
        if (lod.RenderingData == nullptr)
        {
            return false;
        }

        lod.RenderingData->SetMesh(std::dynamic_pointer_cast<StaticMesh>(shared_from_this()), i, {});

        if (!lod.Vertices.IsEmpty() && !lod.Indices.IsEmpty())
        {
            lod.RenderingData->UploadToGPU(renderingSubsystem);
        }
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

    writer << _lods;

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
    
    reader >> _lods;

    uint64 materialAssetID;
    reader >> materialAssetID;
    _material = AssetManager::Get().FindAsset<Material>(materialAssetID);

    reader >> _boundingBox;

    return true;
}

const StaticMesh::LOD& StaticMesh::GetLOD(uint8 lod) const
{
    const uint8 lodIndex = Math::Clamp(lod, 0, static_cast<uint8>(_lods.Count() - 1));
    return _lods[lodIndex];
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

const BoundingBox& StaticMesh::GetBoundingBox() const
{
    return _boundingBox;
}

DArray<std::shared_ptr<Asset>> StaticMesh::Import(const std::shared_ptr<Importer>& importer) const
{
    const std::shared_ptr<StaticMeshImporter> smImporter = std::dynamic_pointer_cast<StaticMeshImporter>(importer);
    if (smImporter == nullptr)
    {
        DEBUG_BREAK();
        return {};
    }

    const std::filesystem::path& path = smImporter->Path;

    Assimp::Importer assimpImporter;
    const aiScene* scene = assimpImporter.ReadFile(
        path.string(),
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_FlipUVs
    );

    if (scene == nullptr)
    {
        LOG(L"Failed to import static meshes: {}", Util::ToWString(assimpImporter.GetErrorString()));
        return {};
    }

    if (!scene->HasMeshes())
    {
        return {};
    }

    DArray<std::shared_ptr<Asset>> meshes;
    for (uint32 i = 0; i < scene->mNumMeshes; ++i)
    {
        std::wstring meshName = Util::ToWString(scene->mMeshes[i]->mName.C_Str());

        uint8 lodIndex = 0;
        size_t lodOffset = meshName.find(L"_LOD");
        if (lodOffset != std::wstring::npos)
        {
            const std::wstring_view meshNameView = std::wstring_view(meshName.begin() + static_cast<int64>(lodOffset) + 4, meshName.end());
            lodIndex = static_cast<uint8>(std::stoi(meshNameView.data()));
            
            meshName = meshName.substr(0, lodOffset);
        }

        if (lodIndex >= 10)
        {
            LOG(L"Failed to import static mesh: {} - unsupported LOD index!", meshName);
            continue;
        }

        const Name name = Name(meshName);
        auto it = meshes.FindIf([&name](const std::shared_ptr<Asset>& asset)
        {
            return asset->GetName() == name;
        });

        bool existing = false;
        std::shared_ptr<StaticMesh> mesh;
        if (it != meshes.end())
        {
            mesh = std::dynamic_pointer_cast<StaticMesh>(*it);
            existing = true;
        }
        else
        {
            mesh = AssetManager::Get().NewAsset<StaticMesh>(name);
            if (mesh == nullptr)
            {
                LOG(L"Error creating new static mesh: {}", meshName);
                continue;
            }
        }

        // todo import path, reimport (save index of mesh in scene?)

        if (!mesh->ImportLOD(scene->mMeshes[i], lodIndex))
        {
            AssetManager::Get().DeleteAsset(mesh);
            LOG(L"Failed to import static mesh: {}", Util::ToWString(scene->mMeshes[i]->mName.C_Str()));
            continue;
        }

        if (existing)
        {
            continue;
        }
        
        std::shared_ptr<Material> defaultMaterial = AssetManager::Get().FindAssetByName<Material>(
            Name(L"DefaultMaterial"));
        if (defaultMaterial == nullptr)
        {
            LOG(L"Failed to find default material!");
            continue;
        }
        defaultMaterial->Load();

        mesh->SetMaterial(defaultMaterial);
        mesh->SetImportPath(path);
        
        meshes.Emplace(mesh);
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

StaticMesh::LOD::LOD(const LOD& other)
{
    Vertices = other.Vertices;
    Indices = other.Indices;
}

StaticMesh::LOD& StaticMesh::LOD::operator=(const LOD& other)
{
    Vertices = other.Vertices;
    Indices = other.Indices;

    return *this;
}

StaticMesh::LOD::LOD(LOD&& other) noexcept
{
    Vertices = std::move(other.Vertices);
    Indices = std::move(other.Indices);
}

StaticMesh::LOD& StaticMesh::LOD::operator=(LOD&& other) noexcept
{
    Vertices = std::move(other.Vertices);
    Indices = std::move(other.Indices);

    return *this;
}

bool StaticMesh::ImportLOD(const aiMesh* assimpMesh, uint8 lodIndex)
{
    if (assimpMesh == nullptr)
    {
        return false;
    }

    if (assimpMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
    {
        LOG(L"Failed to import Static Mesh {}: Unsupported primitive type!", GetName().ToString());
        return false;
    }
    
    _lods.Resize(Math::Max(lodIndex + 1, _lods.Count()));
    
    LOD& newLod = _lods[lodIndex];
    
    newLod.Vertices.Reserve(assimpMesh->mNumVertices);
    newLod.Indices.Reserve(3 * assimpMesh->mNumFaces);

    for (uint32 i = 0; i < assimpMesh->mNumVertices; i++)
    {
        newLod.Vertices.Emplace();
        Vertex& vertex = newLod.Vertices.Back();

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
            newLod.Indices.Emplace(assimpMesh->mFaces[faceIndex].mIndices[j]);
        }
    }
    newLod.Indices.ShrinkToFit();

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
    
    for (const Vertex& vertex : _lods[0].Vertices)
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

MemoryWriter& operator<<(MemoryWriter& writer, const StaticMesh::LOD& lod)
{
    writer << lod.Vertices;
    writer << lod.Indices;
    
    return writer;
}

MemoryReader& operator>>(MemoryReader& reader, StaticMesh::LOD& lod)
{
    reader >> lod.Vertices;
    reader >> lod.Indices;

    return reader;
}
