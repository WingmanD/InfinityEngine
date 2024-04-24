#pragma once

#include "DX12RenderingCore.h"
#include "NonCopyable.h"
#include "StructuredBuffer.h"
#include <atlcomcli.h>
#include <filesystem>

class DX12RenderingSubsystem;

class WorkGraphBase : public NonCopyable<WorkGraphBase>
{
public:
    explicit WorkGraphBase() = default;
    virtual ~WorkGraphBase() = default;
    
    explicit WorkGraphBase(std::filesystem::path libraryPath, std::wstring name);
    
    virtual bool Initialize();

    void SetLibraryPath(std::filesystem::path libraryPath);
    void SetName(std::wstring name);
    
protected:
    virtual void PreDispatch(DX12GraphicsCommandList* commandList);
    void SetupDispatch(DX12GraphicsCommandList* commandList) const;
    
private:
    CComPtr<ID3D12StateObject> _pso;
    CComPtr<ID3D12RootSignature> _rootSignature;
    
    CComPtr<ID3D12Resource> _backingMemory;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE _backingMemoryGPUAddressRange{};
    D3D12_PROGRAM_IDENTIFIER _programID{};
    D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS _memoryRequirements{};

    std::filesystem::path _libraryPath;
    std::wstring _name;

    ComPtr<ID3DBlob> _library;
};

template<typename T>
class WorkGraph : public WorkGraphBase
{
public:
    explicit WorkGraph() = default;
    explicit WorkGraph(std::filesystem::path libraryPath, std::wstring name) : WorkGraphBase(std::move(libraryPath), std::move(name))
    {
    }
    
    void Dispatch(DX12GraphicsCommandList* commandList, T* data, uint32 count)
    {
        PreDispatch(commandList);
        
        SetupDispatch(commandList);

        BindBuffers(commandList);
        
        DispatchImplementation(commandList, data, count);
    }

    virtual void BindBuffers(DX12GraphicsCommandList* commandList) const = 0;

protected:
    virtual void DispatchImplementation(DX12GraphicsCommandList* commandList, T* data, uint32 count)
    {
        D3D12_DISPATCH_GRAPH_DESC desc = {};
        desc.Mode = D3D12_DISPATCH_MODE_NODE_CPU_INPUT;
        desc.NodeCPUInput.EntrypointIndex = 0;
        desc.NodeCPUInput.NumRecords = count;
        desc.NodeCPUInput.RecordStrideInBytes = sizeof(T);
        desc.NodeCPUInput.pRecords = data;
        commandList->DispatchGraph(&desc);
    }
};
