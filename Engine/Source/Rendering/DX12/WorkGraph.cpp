#include "WorkGraph.h"
#include "Core.h"
#include "DX12RenderingSubsystem.h"

WorkGraphBase::WorkGraphBase(std::filesystem::path libraryPath, std::wstring name) :
    _libraryPath(std::move(libraryPath)),
    _name(std::move(name))
{
}

bool WorkGraphBase::Initialize()
{
#if !GPU_DEBUG
    const DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    DX12Device* device = renderingSubsystem.GetDevice();

    _library = renderingSubsystem.CompileDXILLibrary(_libraryPath, L"lib_6_8");

    CD3DX12_STATE_OBJECT_DESC stateObjectDesc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);
    CD3DX12_DXIL_LIBRARY_SUBOBJECT* librarySubobject = stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

    const CD3DX12_SHADER_BYTECODE libraryBytecode(_library.Get());
    librarySubobject->SetDXILLibrary(&libraryBytecode);
    HRESULT result = device->CreateRootSignatureFromSubobjectInLibrary(
        0,
        libraryBytecode.pShaderBytecode,
        libraryBytecode.BytecodeLength,
        L"GGlobalRootSignature",
        IID_PPV_ARGS(&_rootSignature)
    );

    if (FAILED(result))
    {
        LOG(L"Failed to create root signature for work graph!");
        return false;
    }

    CD3DX12_WORK_GRAPH_SUBOBJECT* workGraph = stateObjectDesc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
    workGraph->IncludeAllAvailableNodes();
    workGraph->SetProgramName(_name.c_str());

    result = device->CreateStateObject(stateObjectDesc, IID_PPV_ARGS(&_pso));
    if (FAILED(result))
    {
        LOG(L"Failed to create state object for work graph!");
        return false;
    }

    CComPtr<ID3D12StateObjectProperties1> psoProperties;
    psoProperties = _pso;
    _programID = psoProperties->GetProgramIdentifier(_name.c_str());

    CComPtr<ID3D12WorkGraphProperties> workGraphProperties;
    workGraphProperties = _pso;

    const uint32 workGraphIndex = workGraphProperties->GetWorkGraphIndex(_name.c_str());

    workGraphProperties->GetWorkGraphMemoryRequirements(workGraphIndex, &_memoryRequirements);

    _backingMemoryGPUAddressRange.SizeInBytes = _memoryRequirements.MaxSizeInBytes;


    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(_backingMemoryGPUAddressRange.SizeInBytes);
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

    result = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        __uuidof(ID3D12Resource),
        reinterpret_cast<void**>(&_backingMemory));

    if (FAILED(result))
    {
        LOG(L"Failed to create backing memory for work graph!");
        return false;
    }

    _backingMemoryGPUAddressRange.StartAddress = _backingMemory->GetGPUVirtualAddress();
#endif
    return true;
}

void WorkGraphBase::SetLibraryPath(std::filesystem::path libraryPath)
{
    _libraryPath = std::move(libraryPath);
}

void WorkGraphBase::SetName(std::wstring name)
{
    _name = std::move(name);
}

void WorkGraphBase::PreDispatch(DX12GraphicsCommandList* commandList)
{
}

void WorkGraphBase::SetupDispatch(DX12GraphicsCommandList* commandList) const
{
#if !GPU_DEBUG
    commandList->SetComputeRootSignature(_rootSignature);
    
    D3D12_SET_PROGRAM_DESC programDesc = {};
    programDesc.Type = D3D12_PROGRAM_TYPE_WORK_GRAPH;
    programDesc.WorkGraph.ProgramIdentifier = _programID;
    programDesc.WorkGraph.Flags = D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;
    programDesc.WorkGraph.BackingMemory = _backingMemoryGPUAddressRange;
    commandList->SetProgram(&programDesc);
#endif
}
