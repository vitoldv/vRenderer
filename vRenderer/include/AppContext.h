#pragma once

class AppContext
{
public:
    static AppContext& instance()
    {
        if (_instance == nullptr)
        {
            _instance = new AppContext();
        }
        return *_instance;
    }

    float deltaTime = 0;
    
private:

    // Make constructor private to prevent direct instantiation
    AppContext();

    // Delete copy constructor and assignment operator
    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    static AppContext* _instance;

};