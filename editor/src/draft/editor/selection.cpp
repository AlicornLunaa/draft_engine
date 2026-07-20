#include "draft/editor/selection.hpp"

#include <algorithm>

namespace Draft {
    Entity EditorSelection::get() const {
        return m_selected.empty() ? Entity() : m_selected.back();
    }

    bool EditorSelection::is_selected(Entity entity) const {
        return std::find(m_selected.begin(), m_selected.end(), entity) != m_selected.end();
    }

    void EditorSelection::set(Entity entity){
        if(!entity.is_valid()){
            clear();
            return;
        }

        m_selected = {entity};
        notify();
    }

    void EditorSelection::set_range(std::vector<Entity> entities){
        m_selected = std::move(entities);
        notify();
    }

    void EditorSelection::toggle(Entity entity){
        if(!entity.is_valid())
            return;

        auto it = std::find(m_selected.begin(), m_selected.end(), entity);
        if(it != m_selected.end())
            m_selected.erase(it);
        else
            m_selected.push_back(entity);

        notify();
    }

    void EditorSelection::clear(){
        m_selected.clear();
        notify();
    }

    void EditorSelection::on_change(Callback callback){
        m_callbacks.push_back(std::move(callback));
    }

    void EditorSelection::notify(){
        Entity primary = get();
        for(auto& callback : m_callbacks)
            callback(primary);
    }
}
