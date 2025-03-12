#pragma once

#include <sstream>
#include <mutex>

class Logger {
    public:
        virtual ~Logger() = default;
    
        void Log(const std::string& text) {
            std::lock_guard lock(mutex_);
            DoLog(text);
        }
    
        template<typename T>
        Logger& operator<<(const T& value) {
            {
                std::lock_guard lock(mutex_);
                std::ostringstream oss;
                oss << value;
                DoLog(oss.str());
            }
            return *this;
        }
    
        using StreamManipulator = std::ostream& (*)(std::ostream&);
        Logger& operator<<(StreamManipulator manip) {
            {
                std::lock_guard lock(mutex_);
                std::ostringstream oss;
                oss << manip;
                DoLog(oss.str());
            }
            return *this;
        }
    
    protected:
        virtual void DoLog(const std::string& text) = 0;
    
    private:
        std::mutex mutex_;
    };
    
