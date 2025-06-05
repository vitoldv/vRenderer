#pragma once

// AI GENERATED

#include <stdexcept>
#include <mutex>

/**
 * @brief Singleton class with support for external initialization
 *
 * This pattern allows passing external data once for initialization.
 * Users must call `initialize()` before using `getInstance()`.
 */
template <typename T>
class Singleton
{
public:

	/**
	 * @brief Returns the singleton instance of the class
	 * @return A reference to the single instance of the class.
	 * @throw std::logic_error if the singleton has not been initialized yet.
	 */
	static T& instance()
	{
		if (!s_instance)
		{
			throw std::logic_error("Singleton has not been initialized. Call initialize() first.");
		}
		return *s_instance;
	}

	/**
	 * @brief Initialize the singleton instance with external data
	 * @param args Arguments to construct the singleton instance
	 * @tparam Args Variadic template for constructor arguments
	 * @throw std::logic_error if already initialized.
	 */
	template <typename... Args>
	static void initialize(Args&&... args) 
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		if (s_instance)
		{
			throw std::logic_error("Singleton has already been initialized.");
		}
		s_instance = new T(std::forward<Args>(args)...);
	}

	/**
	 * @brief Destroy the singleton instance (optional).
	 * This should rarely be used, but it can help in certain controlled shutdown scenarios.
	 */
	static void destroy()
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		delete s_instance;
		s_instance = nullptr;
	}

	// Prevent copy and move construction/assignment
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

protected:

	// Protected constructor & destructor for child classes
	Singleton() = default;
	virtual ~Singleton() = default;

private:

	static T* s_instance;
	static std::mutex s_mutex;
};

// Static member initialization
template <typename T>
T* Singleton<T>::s_instance = nullptr;

template <typename T>
std::mutex Singleton<T>::s_mutex;