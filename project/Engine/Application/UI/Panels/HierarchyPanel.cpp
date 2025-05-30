/* ========================================================================
/*  include space
/* ===================================================================== */

// engine
#include <Engine/Application/UI/Panels/HierarchyPanel.h>
#include <Engine/Application/UI/Panels/InspectorPanel.h>
#include <Engine/Application/UI/EngineUI/Context/EditorContext.h>
#include <Engine/objects/3D/Actor/Library/SceneObjectLibrary.h>

// lib
#include <externals/imgui/imgui.h>
#include <map>

int HierarchyPanel::selectedObjectIndex_ = -1;

HierarchyPanel::HierarchyPanel()
	:IEngineUI("Hierarchy"){}

void HierarchyPanel::Render(){
	ImGui::Begin(panelName_.c_str(), nullptr, ImGuiWindowFlags_NoDecoration);
	ImGui::Text("Scene Hierarchy");

	const auto& allObjects = pSceneObjectLibrary_->GetAllObjects();

	// 順序保証付きのカテゴリ（表示順: Camera → Light → Game Object）
	std::vector<std::pair<std::string, std::vector<SceneObject*>>> categorizedObjects = {
		{ "Cameras", {} },
		{ "Lights", {} },
		{ "Game Objects", {} }
	};

	// 各カテゴリに振り分け
	for (SceneObject* obj : allObjects){
		if (!obj) continue;

		switch (obj->GetObjectType()){
			case ObjectType::Camera:
				categorizedObjects[0].second.push_back(obj);
				break;
			case ObjectType::Light:
				categorizedObjects[1].second.push_back(obj);
				break;
			case ObjectType::GameObject:
				categorizedObjects[2].second.push_back(obj);
				break;
			default:
				break;
		}
	}

	// 選択中オブジェクト取得
	SceneObject* selected = pEditorContext_->GetSelectedObject();

	for (const auto& [category, objects] : categorizedObjects){
		if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen)){
			for (SceneObject* obj : objects){
				bool isSelected = (selected == obj);
				if (ImGui::Selectable(obj->GetName().c_str(), isSelected)){
					pEditorContext_->SetSelectedObject(obj);
					pEditorContext_->SetSelectedEditor(nullptr);
				}
				if (isSelected){
					ImGui::SetItemDefaultFocus();
				}
			}
		}
	}

	ImGui::End();
}


const std::string& HierarchyPanel::GetPanelName() const{
	return panelName_;
}

void HierarchyPanel::SetEditorContext(EditorContext* context){
	pEditorContext_ = context;
}

void HierarchyPanel::SetSceneObjectLibrary(const SceneObjectLibrary* library){
	pSceneObjectLibrary_ = library;
}
