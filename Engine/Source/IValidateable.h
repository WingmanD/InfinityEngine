#pragma once

class IValidateable
{
public:
    template <typename Self>
    void SetValid(this Self&& self, bool value)
    {
        self.SetValidImplementation(value);
    }

    template <typename Self>
    bool IsValid(this Self&& self)
    {
        return self.IsValidImplementation();
    }
};
