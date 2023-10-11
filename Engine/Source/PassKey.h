#pragma once

template <typename KeyUser>
class PassKey
{
public:
    PassKey(PassKey&&) = delete;
    PassKey& operator=(const PassKey&) = delete;
    PassKey& operator=(PassKey&&) = delete;

    ~PassKey() = default;

private:
    friend KeyUser;

    PassKey()
    {
    }

    PassKey(const PassKey&)
    {
    }
};
