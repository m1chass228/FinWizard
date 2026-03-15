#include <unordered_map>

class PluginManager {

    struct loadedExecutor {
        LibHandle handle;
        ExecutorHandle* instance;
        const ExecutorVTable* vtable;
    };

    bool load(const std::string& lib_path, const std::string& executor_name);

private:
    unordered_map<std::string, LoadedExecutor> loaded;
};
