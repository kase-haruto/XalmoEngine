#include "GameScene.h"
/////////////////////////////////////////////////////////////////////////////////////////
//	include
/////////////////////////////////////////////////////////////////////////////////////////

// scene
#include <Engine/Scene/System/SceneManager.h>
#include <Engine/Scene/Utirity/SceneUtility.h>

// engine
#include <Engine/Graphics/Context/GraphicsGroup.h>
#include <Engine/Application/Input/Input.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Objects/3D/Actor/SceneObjectManager.h>
#include <Engine/Collision/CollisionManager.h>

/////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ/デストラクタ
/////////////////////////////////////////////////////////////////////////////////////////
GameScene::GameScene() {
	// シーン名を設定
	//IScene::SetSceneName("GameScene");
	SetSceneName("GameScene");

}

/////////////////////////////////////////////////////////////////////////////////////////
//	アセットのロード
/////////////////////////////////////////////////////////////////////////////////////////
void GameScene::LoadAssets() {
}


/////////////////////////////////////////////////////////////////////////////////////////
//	初期化処理
/////////////////////////////////////////////////////////////////////////////////////////
void GameScene::Initialize(){
	LoadAssets();

	CameraManager::GetInstance()->SetType(CameraType::Type_Default);
	
	//=========================
	// グラフィック関連
	//=========================
	railCamera_ = std::make_unique<RailCamera>();
	railCamera_->Initialize();

	modelField_ = sceneContext_->GetObjectLibrary()->CreateAndAddObject<BaseGameObject>("terrain.obj", "field");
	modelField_->SetScale({300.0f,300.0f,300.0f});

	//player
	player_ = sceneContext_->GetObjectLibrary()->CreateAndAddObject<Player>("player.obj", "player");
	player_->Initialize();
	player_->SetParent(&railCamera_->GetWorldTransform());

	playerBulletContainer_ = sceneContext_->AddEditorObject(std::make_unique<BulletContainer>("playerBulletContainer"));
	playerBulletContainer_->SetSceneContext(sceneContext_.get());
	player_->SetBulletContainer(playerBulletContainer_);

	//===================================================================*/
	//                    editor
	//===================================================================*/
}

void GameScene::Update(){
	/* カメラ関連更新 ============================*/
	railCamera_->Update();
	CameraManager::GetCamera3d()->SetCamera(railCamera_->GetPosition(), railCamera_->GetRotation());
	CameraManager::Update();

	skyBox_->Update();
	
	/* 3dObject ============================*/
	//地面の更新
	modelField_->Update();
	//プレイヤーの更新
	player_->Update();

	/* その他 ============================*/

	sceneContext_->Update();
	CollisionManager::GetInstance()->UpdateCollisionAllCollider();
}


void GameScene::CleanUp(){
	// 3Dオブジェクトの描画を終了
	sceneContext_->GetObjectLibrary()->Clear();
	CollisionManager::GetInstance()->ClearColliders();
}

