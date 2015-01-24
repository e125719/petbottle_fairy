/*
 *  Frame.h
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/03.
 *  Copyright 2007 Y. Akamine. All rights reserved.
 *
 */

#pragma once 

#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;


#include "Types.h"
#include "Renderable.h"
#include "Material.h"
#include "MeshObj.h"

namespace rge {
	
	class PhBody;
	
	//!カメラ
	class Camera : public RGEObject, public Issuable
	{
	public:
		Camera() : RGEObject(),  mFov(70), mbBindFrame(true), mbUseCameraMatrix(false), mbUseCustomProj(false) {}
		virtual ~Camera() {}
		virtual void issue();
		
		OBJ_TYPE objType() { return CAMERA; }
		static Camera* upcast(IssuableRef ref) { return ref->objType() == CAMERA ?  (Camera*)ref.get() : 0; }
		
		void setFov(rgeScalar v) { mFov=v;} //!<視野角（縦方向）を設定する，角度v
		void setClip(rgeScalar near, rgeScalar far) { mNearClip = near; mFarClip = far; }
		
		void lookUp(rgeScalar ex, rgeScalar ey, rgeScalar ez,
					rgeScalar rx, rgeScalar ry, rgeScalar rz,
					rgeScalar ux, rgeScalar uy, rgeScalar uz); //!< カメラの向きを設定する:視点e,注視点r,上方ベクトルu
		
		//	void setDistortion(rgeScalar x, rgeScalar y) { mDistortion = rgeVector3(x,y,0);}
		
		void bindFrame(bool b) { mbBindFrame = b; }//!<カメラをフレームに固定する
		
		void createFromRgm(istream& is);      //!<rgeファイルからカメラ情報だけを読み込む
		void viewTransform(rgeScalar aspect); //!<OpenGLにビューマトリクスを適用する
		
		const rgeMatrix4x4& projectionMatrix() { return mProjectionMatrix; }
		const rgeMatrix4x4& cameraMatrix() { return mCameraMatrix; }
		
		void setProjectionMatrix(rgeScalar[16]); //!< 独自のプロジェクションマトリクスを設定する
		void setCameraMatrix(const rgeMatrix4x4& m) { mCameraMatrix = m; } //!<独自のカメラ行列を指定する
		void setUseCameraMatrix(bool b) { mbUseCameraMatrix = b; }         //!<独自のカメラ行列を有効にする
		void setUseProjectionMatrix(bool b=true) { mbUseCustomProj = b; } //!< 独自のプロジェクションマトリクスを有効にする
	private:
		rgeVector3 mEye;
		rgeVector3 mReference;
		rgeVector3 mUp;
		
		rgeVector3 mDistortion;
		
		rgeScalar mNearClip;
		rgeScalar mFarClip;
		rgeScalar mFov;
		
		bool mbBindFrame;
		bool mbUseCameraMatrix;
		bool mbUseCustomProj;
		
		rgeMatrix4x4 mProjectionMatrix;
		rgeMatrix4x4 mCameraMatrix;
	};
	
	typedef std::shared_ptr<Camera> CameraRef;
	typedef vector<CameraRef> CameraRefs;
	
	//!ライトの基底クラス
	class Light : public RGEObject, public Issuable
	{
	public:
		typedef enum {
			POINT_LIGHT,
			DIRECTIONAL_LIGHT,
			SPOT_LIGHT,
		} LIGHT_TYPE;
		
		Light(color3 col, rgeScalar e) : RGEObject(), mColor(col), mEnergy(e), mbCastShadow(false), mShadowIntencity(0.1) {}
		virtual ~Light() {}
		//	virtual void createFromRgm(istream& is) = 0;
		virtual void issue() = 0;
		OBJ_TYPE objType() { return LIGHT; }
		static Light* upcast(IssuableRef issue) { return issue->objType() == LIGHT ? (Light*)issue.get() : 0; }
		virtual LIGHT_TYPE lightType() = 0;
		
		void setEnergy(rgeScalar e) { mEnergy = e; }
		rgeScalar energy() { return mEnergy; }
		
		void setColor(const color3& c) { mColor = c;}
		color3 color() { return mColor; }
		
		void setRange(rgeScalar r) { mRange = r; }
		rgeScalar range() { return mRange; }
		
		void setShadowIntencity(rgeScalar i) { mShadowIntencity = i; }
		rgeScalar shadowIntencity() { return mShadowIntencity; }
		
		void setCastShadow(bool b = true) { mbCastShadow = b; }
		bool isCastShadow() { return mbCastShadow; }
		virtual rgeScalar shadowMapAngle() { return 0; }
	protected:
		color3 mColor;
		rgeScalar mEnergy;
		rgeScalar mRange;
		bool mbCastShadow;
		rgeScalar mShadowIntencity;
	};
	
	typedef std::shared_ptr<Light> LightRef;
	typedef vector<LightRef> LightRefs;
	
	//!点光源
	class PointLight : public Light
	{
	public:
		PointLight(color3 col, rgeScalar e) : Light(col, e) {}
		
		virtual void issue();
		virtual LIGHT_TYPE lightType() { return POINT_LIGHT; }
		static PointLight* upcast(LightRef ref) { return ref->lightType() == POINT_LIGHT ?  (PointLight*)ref.get() : 0; }
	};
	
	//!平行光源（距離に応じてエネルギーが減少しない)
	class DirectionalLight : public Light
	{
	public:
		DirectionalLight(color3 col, rgeScalar e) :  Light( col, e) {}
		
		virtual void issue();
		virtual LIGHT_TYPE lightType() { return DIRECTIONAL_LIGHT; }
		static DirectionalLight* upcast(LightRef ref) { return ref->lightType() == DIRECTIONAL_LIGHT ?  (DirectionalLight*)ref.get() : 0; }
	};
	
	//!スポット光源（照射範囲が円錐状)
	class SpotLight : public Light
	{
	public:
		SpotLight(color3 col, rgeScalar e, rgeScalar cutoff, rgeScalar exp) : Light( col, e),
		mSpotCutoff(cutoff), mSpotExponent(exp) {}
		
		virtual void issue();
		virtual LIGHT_TYPE lightType() { return SPOT_LIGHT; }
		static SpotLight* upcast(LightRef ref) { return ref->lightType() == SPOT_LIGHT ?  (SpotLight*)ref.get() : 0; }
		
		rgeScalar spotCutOff()   { return mSpotCutoff; }
		rgeScalar spotExponent() { return mSpotExponent; }
		rgeScalar shadowMapAngle() { return mSpotCutoff; }
	private:
		rgeScalar mSpotCutoff;
		rgeScalar mSpotExponent;
	};
	
	class Action;
	class ValueDriver;
	typedef std::shared_ptr<Action> ActionRef;
	
	class Bone;
	typedef std::shared_ptr<Bone> BoneRef;
	typedef vector<BoneRef> BoneRefs;
	
	class Frame;
	//!アニメーション　フレームのアニメーションを制御する
	class Animation 
	{
	public:
		Animation(Frame* f) : 
		mTargetFrame(f),
		mBlendingFactor(),
		mTime(),
		mIsAnimated(true),
		mAnimSpeed(1.f)
		{}
		
		void setTarget(Frame* f) { mTargetFrame = f; }
		
		void update(rgeScalar deltaTime, rgeScalar fps);
		void setAction(ActionRef act) { mAction = act; }
		void setBlendingAction(ActionRef act) { mBlendingAction = act; }
		void setBlendingFactor(rgeScalar factor) { mBlendingFactor = factor; }
		void reward() { setTime( 0, true); } //!< アニメーションの時間を０に戻す
		void stop(bool bApplyChildren = true) ; //!< アニメーションを止める
		void start(bool bApplyChildren = true) ; //!< アニメーションを開始する
		void setLooping(bool bloop, bool bApplyChildren = true) ;//!< アニメーションをループ再生するか(bloop), 子フレームにも適用するか(bAppluChildren)
		void setTime(int t, bool bApplyChildren = true ) ; //!< アニメーションの再生時刻を設定(t), 子フレームに適用するか（bAppluChildren)
		void setSpeed(float rate) { mAnimSpeed = rate; }
		ActionRef action() { return mAction; }
		rgeScalar time() { return mTime; } //!< アニメーションの時間を取得する
	private:
		Frame* mTargetFrame;
		ActionRef mAction;
		ActionRef mBlendingAction;
		rgeScalar mBlendingFactor;
		rgeScalar mTime;
		bool mIsAnimated;
		float mAnimSpeed;	
	};
	
	//!フレーム　変換行列の階層構造を表現する
	class Frame : public RGEObject
	{
	public:
		friend class ValueDriver;
		
		enum PHYSICSTYPE {
			NO_COLLISION, STATIC, DYNAMIC, RIGID_BODY //, SOFT_BODY, OCCLUDE, SENSOR
		};
		enum BOUNDTYPE {
			BOX, SPHERE, CYLINDER, CONE, TRIMESH, CONVEX
		};
		enum COORDSPACE {
			WORLD_SPACE, //!< ワールド座標系(カメラの座標系)を利用する
			LOCAL_SPACE  //!< ローカル座標系（親フレームの座標系）を利用する．
		};
		enum PARENT_TYPE {
			OBJECT,
			BONE_NAME,
			BONE
		};
		struct Physics {
			Physics() : enabled(false), type(NO_COLLISION),  boundBox(BOX), mass() {}
			bool enabled;
			PHYSICSTYPE type;
			BOUNDTYPE boundBox;
			rgeScalar mass;
		};
		typedef std::shared_ptr<Frame> FrameRef;
		typedef vector<FrameRef> FrameRefs;
	public:
		Frame(Frame* parent) : RGEObject(), 
		mpParent(parent), 
		mParentType(OBJECT),
		mPhBody(),
		mAnim(this),
        mbUseEulerRotation(true),
		mbUseQuat(false),
		mScales(1,1,1),
		mCoordSpace(LOCAL_SPACE), 
		mLookAt(), 
		mUp(0,1,0), 
		mbUseExtraTransform(false)
		{
			setLayer(LAYER_1, false);
		}
		
		
		void createFromRgm(istream& is);//!<RGEファイルからフレームを読み込む
		void createChildrenFromRgm(istream& is); //!<RGEから子フレームを作成する
		
		FrameRef createChild();	 //!<子フレームを作成する
		int numChildren() { return  (int)mChildren.size(); }
		FrameRef childByIndex(int i) { return mChildren[i]; }
		void removeChild(FrameRef f) ;
		void removeAllChildren() { mChildren.clear(); }
		void addChild(FrameRef f) { mChildren.push_back(f); f->mpParent = this; }
		
		const Frame* getParent() const { return mpParent; } //!< 親フレームへのコンストポインタを取得
		Frame* getParent() { return mpParent; } //!< 親フレームへのポインタを取得
		void setParentBoneName(const string& name) { mParentBoneName = name; }
		void setParentType(PARENT_TYPE t) { mParentType = t; }
		void resolveParentBone();
		
		void setPhysics(Physics p) { mPhysics = p; }
		void applyPhysics(); //!<物理演算を準備し有効にする．以後，フレームの動きは物理エンジンに支配される
		void setStatic(bool b) { mPhysics.type = STATIC; } //!<物理エンジンにおける静的オブジェクトに指定する．
		void enablePhysics(bool b) { mPhysics.enabled = true; } //!<物理演算を有効にする
		
		void addRenderable(RenderableRef rend) { mVisuals.push_back(rend); } //!<フレームに描画オブジェクトを追加する
		void addRenderable(Renderable* rend) { mVisuals.push_back(RenderableRef(rend)); } //!<フレームに描画オブジェクトを追加
		void clearRenderable() { mVisuals.clear(); } //!<フレームの描画オブジェクトをクリア
		void replaceRenderable(RenderableRef rend) { clearRenderable(); addRenderable(rend); } //!<描画オブジェクトを取り替える
		Renderable* getRenderable(size_t i=0) { //!< このフレームが保持している描画オブジェクト(renderable)を取得. iはindex,ディフォルトは0
			if(i>= mVisuals.size()) {
				return 0;
			}
			return mVisuals[i].get();
		}
		
		void addMaterial(MaterialRef mat) { mLocalIssue.push_back(mat); } //!<フレームにマテリアルを適用する
		void addLight(LightRef light) { mViewIssue.push_back(light); } //!<フレームにライトを追加	
		void addCamera(CameraRef cam) { mWorldIssue.push_back(cam); }
		void removeCamera(CameraRef cam);
		
		BoneRef createBone(Frame* base_frame = 0) ;
		BoneRef findBone(const string& name);
		void flattenBoneTree(Frame* base_frame); //!< Bonesの木構造を並列にする（すべてフレームの直接子ノードにする)
		
		void setLayer(LAYER_NUM nlayer, bool bApplyChildren = true) ;		
		void setLayerBit(u_int showLayerBit, bool  bApplyChildren = true) {
			mLayer = showLayerBit;
			
            if(bApplyChildren) {
                for(auto& c : mChildren) {
                    c->setLayerBit(showLayerBit, bApplyChildren);
                }
            }
			
		}
		
		void printChildren(int depth = 0) const; //!<子フレームをプリントする（デバッグ）
		
		void worldIssue() ; //!<ワールド座標系で実行する処理(カメラや光源の位置計算など）
		void viewIssue() ;  //!<カメラ座標系で実行する処理(光源の発光処理など)
		void render() ; //!<フレームの座標変換処理を行い従属オブジェクトを描画する
		void update(rgeScalar delta) ; //!<フレームのアニメーションを進める
		
		void applyTransformationToMatrix(); //!<ワールドへの変換行列を計算する
		
		void setTranslation(rgeVector3 p) { setTranslation(p.x,p.y,p.z);  } //!< 平行移動
		void setTranslation(rgeScalar x, rgeScalar y, rgeScalar z) { mPosition.x = x; mPosition.y = y; mPosition.z = z; }//!<平行移動
		void moveTo(rgeVector3 p) { setTranslation(p.x,p.y,p.z);  } //!< 平行移動
		void translate(rgeVector3 t) { mPosition += t; } //!< 平行移動(相対)
                void moveLocally(rgeVector3 p);
		
		void rotate(rgeScalar rx, rgeScalar ry, rgeScalar rz) { mRotations += rgeVector3(rx,ry,rz); } //!<回転(相対角）
		void setRotation(rgeScalar rx, rgeScalar ry, rgeScalar rz) { mRotations = rgeVector3(rx,ry,rz); }  //!<回転（絶対角）
		void setRotation(const rgeVector3& r) { mRotations = r; } 
        void useEulerRotation(bool b = true) { mbUseEulerRotation = b; }
		void setQuatanion(const rgeQuat& q) { mQuatanion = q; } //!<クオータニオンによる回転
		void useQuatanion(bool b = true) { mbUseQuat = b; } //!<クオータニオンベースの回転を適用する
		
		void setMatrix(const rgeMatrix4x4& m) { mTransform = m; } //!<ベース変換行列を直接指定
		rgeMatrix4x4 matrix() { return mTransform; } //!<ベース変換行列を取得
		
		
		void scale(rgeScalar sx, rgeScalar sy, rgeScalar sz) { //!<拡大する.現在の拡大率に乗算される
			mScales.x *= sx;
			mScales.y *= sy;
			mScales.z *= sz;
		}
		void setScale(const rgeVector3& s) { mScales = s; } //!<拡大を指定．現在の拡大率が上書きされる
		void setScale(rgeScalar rx, rgeScalar ry, rgeScalar rz) { mScales = rgeVector3(rx,ry,rz); } //!<拡大を指定．現在の拡大率が上書きされる
		
		void setVelocity(rgeVector3 p) { mVelocity = p; } //!<平行移動速度を指定
		void setVelocity(rgeScalar x, rgeScalar y, rgeScalar z) { mVelocity = rgeVector3(x,y,z); }
		void setAnglerVelo(rgeScalar dx, rgeScalar dy, rgeScalar dz) { mAnglerVelo = rgeVector3(dx, dy, dz); } //!<角速度を指定
		
		
		void lookAt(FrameRef target) { mLookAt = target.get(); } //!<他のフレームへz軸を向ける
		void lookAt(rgeVector3 p) ;	                     //!<指定した座標（ワールド）へz軸を向ける
		void setUpVector(rgeVector3 p) { mUp = p; }      //!<lookAtの上方ベクトルを指定する
		void setCoordSpace(COORDSPACE s) { mCoordSpace = s; } //!< render呼び出し時に適用される座標系
		
		void setRotationFromMatrix(const rgeMatrix4x4& rm) { //!<ベース変換行列の回転部だけを変更する
			mTransform.applyRotationOnly(rm);
		}
		
		rgeVector3 worldToLocal(rgeVector3 p) const ; //!<ワールド座標をローカル座標に変換する
		rgeVector3 transformLocalToWorld(rgeVector3 p) const { return p*mLocalToWorld; } //!<ローカル座標をワールド座標に変換
																						 //!ワールド変換の回転だけを適用して変換する(向きだけワールドになる）
		rgeVector3 rotateLocalToWorld(rgeVector3 p) const { 
			rgeMatrix4x4 m = mLocalToWorld;
			m[12] = m[13] = m[14] = 0.0;
			return p*m;
		}
		rgeVector3 rotateWorldToLocal(rgeVector3 worldDir) const { //!<ワールドの向きをローカルの向きへ変換
			return worldToLocal(worldDir+getWorldPos());
		}		
		rgeMatrix4x4 matrixWorld() const { return mLocalToWorld; } //!<ローカル->ワールド変換行列を取得
																   //	void bindPhysicsBody(dBody* b) { mBody = b; }
		
		rgeVector3 getWorldPos() const { //!< ワールド座標系におけるオブジェクトの原点を取得
			return rgeVector3(mLocalToWorld[12], mLocalToWorld[13], mLocalToWorld[14]);
		}
		rgeVector3 getTranslation() const { //!< 平行移動を取得
			return mPosition;
			//		return rgeVector3(mTransform[12], mTransform[13], mTransform[14]);
		}
		rgeVector3 getOffsetToParent() const { //!< 親フレームの原点とこのフレームの原点の差分
			return rgeVector3() * mTransform + mPosition;
		}
		rgeVector3 rotations() const { //!< 回転を取得
			return mRotations;
		}
		rgeVector3 scales() const { //!< スケールを取得
			return mScales; 
		}
                rgeQuat getQuaterion() const { return mQuatanion; }
		//	const rgeVector3& getPos() const { return mPosition; }
		
		
		void bindByBody(PhBody* b); //!< 物理エンジンへ制御を渡す
		PhBody* getPhBody() { return mPhBody; } //!<　このフレームを制御している物理エンジンのBodyオブジェクトを取得
		void transformBody(); //!< このフレームを制御している物理エンジンのBodyオブジェクトの位置をこのフレームの現在位置にする
		
		
		std::shared_ptr<Frame> clone() { //!< 子フレームを含むフレームの完全なコピーを取得(関連オブジェクトは参照)
									//		FrameRef f = getParent()->createChild();
									//		*(f.get()) = *this;
			
			Frame* f = new Frame( getParent());
			*f = *this;
			
			f->anim()->setTarget(f);
			
			f->removeAllChildren();
			for(FrameRefs::iterator i=mChildren.begin(); i!=mChildren.end(); ++i) {
				f->addChild((*i)->clone());
			}
			
			return std::shared_ptr<Frame>(f); 
		}
		
		void setBonesVisibility(bool b) ;
		
		Animation* anim() { return &mAnim; }
		
		template <class T, class A1>
		void forEachChildren(void (T::*pFunc)(A1), A1 arg1 ) {
			for_each(mChildren.begin(), mChildren.end(), bind(pFunc, std::placeholders::_1, arg1));
		}
		
		template <class T, class A1, class A2>
		void forEachChildren(void (T::*pFunc)(A1, A2), A1 arg1, A2 arg2) {
			for_each(mChildren.begin(), mChildren.end(), bind(pFunc, std::placeholders::_1, arg1, arg2));
		}
		
		template <class T, class A1>
		void forEachAnimOfChildren(void (T::*pFunc)(A1), A1 arg1 ) {
			for(FrameRefs::iterator i=mChildren.begin(); i!=mChildren.end(); ++i)
				((*i)->anim()->*pFunc)(arg1);
		}
		
		template <class T, class A1, class A2>
		void forEachAnimOfChildren(void (T::*pFunc)(A1, A2), A1 arg1, A2 arg2) {
			for(FrameRefs::iterator i=mChildren.begin(); i!=mChildren.end(); ++i)
				((*i)->anim()->*pFunc)(arg1, arg2);
		}
		
		//以下，要修正　
		void copyAction(FrameRef dstination, bool bShared = false); //
		void shareActionWith(FrameRef f) ;
		
		//void setAction(ActionRef act) ;	
		void setAnimationBlending(FrameRef blendingFrame, float blendingFactor) ;	
		string getActionsDump( bool bApplyChildren = true);
		
	private:
		virtual void renderMe() {}
		void doTransform();
		void doExtraTransform();
		/*	void readTransformFromNode(domNode *aNode);	
		 void readGeometryFromNode(domNode* aNode);
		 */	
		 
		FrameRefs mChildren;
		Frame* mpParent;
		PARENT_TYPE mParentType;
		string mParentBoneName;
		
		Physics mPhysics;
		PhBody* mPhBody;
		
		Animation mAnim;
		u_int mLayer;
						
		rgeVector3 mPosition;
		rgeVector3 mVelocity;
		//rgeVector3 mStartRotations;
		
        bool mbUseEulerRotation;
		rgeVector3 mRotations;
		rgeVector3 mAnglerVelo;
		
		bool mbUseQuat;
		rgeQuat mQuatanion;
		
		rgeVector3 mScales;
		
		rgeMatrix4x4 mTransform;
		rgeMatrix4x4 mLocalToWorld;
		COORDSPACE mCoordSpace;
		
		Frame* mLookAt;
		rgeVector3 mReference;
		rgeVector3 mUp;

		rgeMatrix4x4 mExtraTransform;
		bool mbUseExtraTransform;
		
		RenderableRefs mVisuals;
		IssuableRefs mWorldIssue;
		IssuableRefs mViewIssue;
		IssuableRefs mLocalIssue;
		
	protected:
		BoneRefs mBones;


	};
	
	
	
	typedef std::shared_ptr<Frame> FrameRef;
	typedef vector<FrameRef> FrameRefs;
	inline FrameRef newFrame( Frame* parent) { return FrameRef(new Frame(parent)); }
	
}