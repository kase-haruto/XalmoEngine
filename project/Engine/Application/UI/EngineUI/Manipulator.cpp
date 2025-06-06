#include "Manipulator.h"
/* ========================================================================
/* include space
/* ===================================================================== */
#include <Engine/Graphics/Camera/Base/BaseCamera.h>
#include <Engine/Foundation/Math/Matrix4x4.h>
#include <Engine/Objects/Transform/Transform.h>
#include <Engine/Editor/SceneObjectEditor.h>

void Manipulator::SetTarget(WorldTransform* target){
	target_ = target;
}

void Manipulator::SetCamera(BaseCamera* camera){
	camera_ = camera;
}

void Manipulator::SetViewRect(const ImVec2& origin, const ImVec2& size){
	viewOrigin_ = origin;
	viewSize_ = size;
}

void Manipulator::Update(){
	if (!target_ || !camera_) return;

	// View/Projection行列を準備
	float view[16], proj[16], world[16];
	Matrix4x4::Transpose(camera_->GetViewMatrix()).CopyToArray(view);
	Matrix4x4::Transpose(camera_->GetProjectionMatrix()).CopyToArray(proj);
	Matrix4x4::Transpose(target_->matrix.world).CopyToArray(world);

	ImGuizmo::Manipulate(view, proj, operation_, mode_, world);

	if (ImGuizmo::IsUsing()){
		Matrix4x4 worldEdited = ColumnArrayToRow(world);
		Matrix4x4 localEdited = target_->parent
			? Matrix4x4::Inverse(target_->parent->matrix.world) * worldEdited
			: worldEdited;

		float decomposed[16];
		RowToColumnArray(localEdited, decomposed);

		float pos[3], rot[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(decomposed, pos, rot, scale);

		target_->translation = {pos[0], pos[1], pos[2]};
		target_->scale = {scale[0], scale[1], scale[2]};

		constexpr float degToRad = 3.14159265f / 180.0f;
		Vector3 euler = {rot[0] * degToRad, rot[1] * degToRad, rot[2] * degToRad};
		target_->rotation = Quaternion::EulerToQuaternion(euler);
	}
}

void Manipulator::RenderOverlay(){}

void Manipulator::RenderToolbar(){
	if (ImGui::RadioButton("Translate", operation_ == ImGuizmo::TRANSLATE)) operation_ = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", operation_ == ImGuizmo::ROTATE)) operation_ = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", operation_ == ImGuizmo::SCALE)) operation_ = ImGuizmo::SCALE;

	if (ImGui::RadioButton("World", mode_ == ImGuizmo::WORLD)) mode_ = ImGuizmo::WORLD;
	ImGui::SameLine();
	if (ImGui::RadioButton("Local", mode_ == ImGuizmo::LOCAL)) mode_ = ImGuizmo::LOCAL;
}

void Manipulator::RowToColumnArray(const Matrix4x4& m, float out[16]){
	// 回転スケール 3×3 を転置
	out[0] = m.m[0][0]; out[1] = m.m[1][0]; out[2] = m.m[2][0]; out[3] = 0.0f;
	out[4] = m.m[0][1]; out[5] = m.m[1][1]; out[6] = m.m[2][1]; out[7] = 0.0f;
	out[8] = m.m[0][2]; out[9] = m.m[1][2]; out[10] = m.m[2][2]; out[11] = 0.0f;

	// 平行移動はそのまま 12–14
	out[12] = m.m[3][0];
	out[13] = m.m[3][1];
	out[14] = m.m[3][2];
	out[15] = 1.0f;
}

// column-major[16] → DirectX(row-major)
Matrix4x4 Manipulator::ColumnArrayToRow(const float in_[16]){
	Matrix4x4 m;
	// 3×3 転置（＝再転置で元に戻る）
	m.m[0][0] = in_[0];  m.m[0][1] = in_[4];  m.m[0][2] = in_[8];  m.m[0][3] = 0.0f;
	m.m[1][0] = in_[1];  m.m[1][1] = in_[5];  m.m[1][2] = in_[9];  m.m[1][3] = 0.0f;
	m.m[2][0] = in_[2];  m.m[2][1] = in_[6];  m.m[2][2] = in_[10]; m.m[2][3] = 0.0f;

	// 平行移動
	m.m[3][0] = in_[12];
	m.m[3][1] = in_[13];
	m.m[3][2] = in_[14];
	m.m[3][3] = 1.0f;
	return m;
}
