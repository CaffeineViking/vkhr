#ifndef VKHR_SCENE_GRAPH_HH
#define VKHR_SCENE_GRAPH_HH

#include <vkhr/model.hh>
#include <vkhr/image.hh>

#include <vkhr/camera.hh>

#include <vkhr/hair_style.hh>

#include <nlohmann/json.hpp>

namespace vkhr {
    class SceneGraph final {
    public:
        SceneGraph() = default;
        SceneGraph(const std::string& file_path);

        bool load(const std::string& file_path);
        bool save(const std::string& file_path) const;

    private:
    };
}

#endif
