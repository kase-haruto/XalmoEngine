#pragma once
/* ========================================================================
/* include space
/* ===================================================================== */

/* engine */
#include <Engine/Assets/Model/ModelData.h>
#include <Engine/Graphics/Buffer/DxConstantBuffer.h>
#include <Engine/Graphics/Buffer/DxStructuredBuffer.h>
#include <Engine/Graphics/Buffer/DxVertexBuffer.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Graphics/Pipeline/BlendMode/BlendMode.h>
#include <Engine/Objects/Transform/Transform.h>
#include <Engine/Objects/Transform/TransformationMatrix.h>

/* math */
#include <Engine/Foundation/Math/Vector4.h>
#include <Engine/Foundation/Utility/Random/Random.h>
#include <Engine/Objects/3D/Geometory/AABB.h>
#include <Engine/Objects/3D/Geometory/Shape.h>

/* c++ */
#include <cstdint>
#include <d3d12.h>
#include <list>
#include <memory>
#include <vector>
#include <wrl.h>

/* externals */
#include <externals/nlohmann/json.hpp>

enum class EmitterShape{
	OBB,
	Sphere
};

/* ========================================================================
/*	namespace
/* ===================================================================== */
namespace ParticleData{
	struct Parameters{
		EulerTransform transform {{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}};
		Vector3 rotationSpeed {0.0f,0.0f,0.0f};
		Vector3 velocity {};
		Vector4 color {1.0f, 1.0f, 1.0f, 1.0f};
		float lifeTime = 1.0f;
		float currentTime = 0.0f;
		Vector3 maxScale = {1.0f, 1.0f, 1.0f};

		void SetColorRandom();          // 色をランダムで初期化
		void SetColorInitialize();      // 色を初期化する(白)

		void SetVelocityRandom(float min, float max);
	};

	struct ParticleForGPU{
		Matrix4x4 world;
		Vector4 color;
	};

	struct EmittedPtlData{

		// 回転
		bool useRotation = true;              // 回転を使用するか
		bool rotateContinuously = false;      // 回転し続けるか（毎フレーム加算）
		bool randomizeInitialRotation = true; // 初期回転をランダムにするか
		Vector3 initialRotation = {0.0f, 0.0f, 0.0f}; // 初期回転（ランダムでなければ使用）
		Vector3 rotationSpeed = {0.0f, 0.0f, 0.0f};   // 連続回転する場合の速度（度/秒など）
	};

	struct Emitter{
		EulerTransform transform {{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}};    //エミッタのtransform
		uint32_t count = 1;				//発生数
		float frequency = 0.1f;			//発生頻度
		float frequencyTime = 0.1f;		//頻度用時刻
		EmitterShape shape = EmitterShape::Sphere; // エミッタの形状
		ParticleData::EmittedPtlData parmData;		//発生させるパーティクルのデータ
		float emissionCounter = 0.0f; // 発生カウンター（頻度管理用）
		Vector3 prevPosition = {0.0f, 0.0f, 0.0f}; // 追加: 前フレーム位置
		bool isFirstFrame = true; // 初回フレームかどうか

		void Initialize(uint32_t count);
		void Initialize(const EulerTransform& transform, const float frequency, const float frequencyTime, uint32_t count);
	};
}

enum class EmitType{ Once, Auto, Both };

/* ========================================================================
/* base class
/* ===================================================================== */
class BaseParticle{
protected:
	// カラーモード用のenumを定義
	enum class ColorMode{
		Random,
		SingleColor,
		SimilarColor
	};

	enum class BillboardAxis{
		AllAxis,
		YAxis,
		XAxis,
		ZAxis
	};

	enum class ScaleMode {
		FixedMaxScale, // 固定最大スケール
		RandomScale, // ランダムスケール
		influencedByLifePlus, // 寿命に応じたスケール
		influencedByLifeMinus // 寿命に応じたスケール（逆）
	};

public:
	//===================================================================*/
	//                    public methods
	//===================================================================*/
	BaseParticle();
	BaseParticle(const BaseParticle& other);
	virtual void Initialize(const std::string& modelName, const std::string& texturePath, const uint32_t count);
	virtual ~BaseParticle() = default;
	virtual void Update();
	virtual void Draw(ID3D12GraphicsCommandList* cmdList);

	virtual void ImGui();

	void VisualSettingGui();
	void ParameterGui();
	void EmitterGui();

	virtual void Emit(ParticleData::Emitter& emitter,std::optional<Vector3> position = std::nullopt);
	void EmitAll();
	void Play(EmitType emitType);

	//--------- accessor -----------------------------------------------------
	virtual float SetParticleLifeTime() const{ return Random::Generate(0.5f, 1.0f); }
	virtual Vector3 GenerateVelocity(float speed);
	virtual bool GetUseRandomColor() const{ return true; }
	virtual Vector4 GetSelectedColor() const{ return Vector4(1.0f, 1.0f, 1.0f, 1.0f); }
	BlendMode GetBlendMode() const{ return blendMode_; }
	void SetUseRandomScale(bool useRandom){ useRandomScale_ = useRandom; }
	void SetFixedMaxScale(Vector3 scale){ fixedMaxScale_ = scale; }
	void SetRandomScaleRange(Vector3 minScale, Vector3 maxScale){
		randomScaleMin_ = minScale;
		randomScaleMax_ = maxScale;
	}
	void SetEmitterShape(EmitterShape shape){ currentShape_ = shape; }
	void SetEmitPos(const Vector3& pos);
	//--------- json -----------------------------------------------------
	virtual nlohmann::json SaveToJson() const = 0;
	virtual void LoadFromJson(const nlohmann::json& j) = 0;

protected:
	//===================================================================*/
	//                    protected methods
	//===================================================================*/

	/* resources =======================*/
	void CreateBuffer();

public:
	//===================================================================*/
	//				public methods
	//===================================================================*/
	BillboardAxis billboardAxis_ = BillboardAxis::AllAxis;
	std::vector<ParticleData::Parameters> particles_;
	bool isStatic_ = false;
	int32_t kMaxInstanceNum_ = 4096;
	int32_t instanceNum_ = 0;

	bool useRotation_ = false;

	bool useRandomScale_ = false;
	Vector3 fixedMaxScale_ = {1.0f, 1.0f, 1.0f};
	Vector3 randomScaleMin_ = {1.0f, 1.0f, 1.0f};
	Vector3 randomScaleMax_ = {6.0f, 6.0f, 6.0f};

	bool useGravity_ = false; // デフォルトは false（オフ）
	float gravity_ = -9.8f;   // 重力加速度（Y軸下向き）
	bool usePopVelocity_ = false;   // ポンっと上方向初速を使うか
	float popVelocityY_ = 3.0f;     // 初速の強さ

	float lifeTime_ = 1.0f; // パーティクルの寿命

	bool isRandomLifeTime_ = true;
	bool isComplement_ = true; // パーティクルをストレージに保存するかどうか
	float maxLifeTime_ = 3.0f;
	float minLifeTime_ = 1.0f;
	bool flyToEmitter_ = false; // エミッタに向かうかどうか
	ScaleMode scaleMode_ = ScaleMode::FixedMaxScale; // スケールの方式
	std::string name_;                                  // システム名
	bool useRandomColor_ = true;                        // ランダムカラーを使用するか
	Vector4 selectedColor_ = {1.0f, 1.0f, 1.0f, 1.0f};  // ランダムでない場合に使う色
	ColorMode colorMode_ = ColorMode::Random; // 現在のカラー方式
	//Vector4 selectedColor_ = {1.0f,1.0f,1.0f,1.0f}; // SINGLEまたはSIMILAR用の基準色
	float colorVariation_ = 0.1f; // 類似色モードでのバラつき度合い(0.0f〜1.0f程度)
protected:
	//===================================================================*/
	//					protected methods
	//===================================================================*/
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle; // テクスチャハンドル

	//反対側に回す回転行列
	Matrix4x4 backToFrontMatrix_;

	/* resources =======================*/
	DxConstantBuffer<Material> materialBuffer_;
	DxStructuredBuffer<ParticleData::ParticleForGPU> instancingBuffer_;

	/* data =======================*/
	std::string modelName_;                           // ▼ロードするファイル名を保持
	std::string textureName_ = "particle.png";        // ▼テクスチャのパスを保持
	std::optional<ModelData> modelData_;              // ▼取得後に代入
	Material materialData_;
	std::vector<ParticleData::ParticleForGPU> instanceDataList_;

	/* emitter ------------------------------------*/
	std::vector<ParticleData::Emitter> emitters_;
	EmitType emitType_ = EmitType::Auto; // 発生タイプ（自動、手動、両方）
	bool emitPosX_ = true;
	bool emitNegX_ = true;
	bool emitPosY_ = true;
	bool emitNegY_ = true;
	bool emitPosZ_ = true;
	bool emitNegZ_ = true;

	bool isFixationAlpha_ = false;
	bool isBillboard_ = true;

	// 形状選択用
	EmitterShape currentShape_ = EmitterShape::Sphere;

	// blend
	BlendMode blendMode_ = BlendMode::ADD;
};
