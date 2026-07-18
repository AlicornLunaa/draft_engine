#include "draft/editor/selection.hpp"

namespace Draft {
    void EditorSelection::set(Entity entity){
        m_selected = entity;

        for(auto& callback : m_callbacks)
            callback(m_selected);
    }

    void EditorSelection::clear(){
        set(Entity());
    }

    void EditorSelection::on_change(Callback callback){
        m_callbacks.push_back(std::move(callback));
    }
}
