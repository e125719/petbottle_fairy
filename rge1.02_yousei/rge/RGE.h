/*
 *  RGE.h (Realtime Graphic Engine)
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/02.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */

#pragma once 
#include <map>
#include <string>
#include <iostream>
#include <memory>
using namespace std;


#include "Types.h"
#include "Exceptions.h"
#include "Material.h"
#include "MeshObj.h"
#include "Frame.h"
#include "Texture.h"
#include "Physics.h"
#include "Timer.h"
#include "ShadowBuffer.h"
#include "Action.h"
#include "Bone.h"

namespace rge {

    typedef map<string, MaterialRef> MaterialRefMap;
    typedef map<string, TextureRef> TextureRefMap;
    typedef map<string, MeshObjectRef> MeshObjRefMap;
    typedef map<string, CameraRef> CameraRefMap;
    typedef map<string, LightRef> LightRefMap;
    typedef map<string, FrameRef > FrameRefMap;
    typedef map<string, ActionRef> ActionRefMap;

    class PhysicsEngine;

    /*! \mainpage RGEの超簡易ドキュメント
     *  \section intro ドキュメントの構成
        上部のタブでクラスリスト，ファイルリストを切り替える事ができます
            \section blender_export Blenderからのエクスポート
            sampleModels/rge_export2_6_1_0.pyをblenderで実行する
            sceneにエクスポート関連のuiが出現するので，それを実行
 
            \section blender_import Blenderからインポート可能な情報
     - メッシュ（ポリゴンデータ）
     - 階層構造（親子関係）
     - 位置，回転，サイズ
     - F-curveアニメーション(位置，回転，サイズのみ）
     - カメラ
     - ライト（点光源，スポットライトのみ）
     - 物理特性
            - 重さ(mass)
            - staticかrigid objectか
            - 衝突判定用の形状（trimesh, convex hull, cyilinder, sphere, box）

     * \section 座標系について
    -ワールド座標系
            すべての基準となる絶対座標系. ルートフレームは，絶対座標系に置かれる
    -ローカル座標系
            メッシュなどのRenderableオブジェクトが描画される座標系
            ローカル座標 = 親のローカル座標 x ベース変換行列 x 平行移動 x 回転 x スケール
 
     *	\section bullet bullet(物理エンジン)とrgeの関係
     *	rgeは，blenderで設定された物理属性を利用してbulletオブジェクトを生成します．
 
     フレームオブジェクトのgetPhBody()を呼び出す事でbulletのbtRigidBodyオブジェクトを取得できます
        \section bullet_notice 物理シミュレーションについての注意
            bulletは，オブジェクトに一定時間速度の変化がなく，力が加わっていない場合，そのオブジェクトのシミュレーションを無効にします(スリープ）．
     　　一度無効になると，applyForceなどが無効になってしまいますので，例えば下記のコードでスリープを解除しておく必要があります.
     \code
            FrameRef f = rge.findFrame("someframe") ; //rgeは,RGEオブジェクトとする
            f->getPhBody()->setActivationState(DISABLE_DEACTIVATION);
     \endcode
     \section ph_object 物理演算におけるオブジェクトの種類
     \subsection a1 static object
            動かないがあたり判定はあるオブジェクト. 重さは0.
            blenderのlogicパネルでstaticを指定する
 
     \subsection a2 kinematic object
            外部要因（ユーザからの指示など）によって強制移動するオブジェクト．
            他のrigid objectに力を加えることはできるが，自分自身は影響を受けない．
     \code
     //例
     frame_obj->getPhBody()->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT); 
     \endcode
     \subsection a3 rigid object
            物理演算に従って動くオブジェクト.重さは0より大きい
            blenderのlogicパネルでrigid bodyを指定
     */


    //! RGEエンジンオブジェクトを定義.

    /*! rgeの全てのオブジェクトはRGEオブジェクト経由で生成され管理される．
        rgeオブジェクトが破棄されるとき，全てのオブジェクトが破棄される */
    class RGE : public ShadowMapDrawer {
    private:
        RGE();
        RGE(const RGE&); // copy protection
        
        static RGE* theInstance;
    public:
        ~RGE();

        static void deleteInstance();
        static RGE* getInstance(); //singleton
        
        void init();
        void createPE(); //!< 物理エンジンを初期化
        PhysicsEngine* getPE(); //!< 物理エンジンオブジェクトを取得
        //	MeshObjectRef createMeshObj() ; //!< 空のMeshObjectを生成

        void setBaseDir(const std::string& dir) {
            mBaseDir = dir + "/";
        }

        const std::string& baseDir() {
            return mBaseDir;
        }

        bool createSceneFromRgm(istream* pis); //!< rgeファイルを読み込みシーンを生成する
        void applyPhysics(); //!< rgeファイルから読み込んだ物理特性を適用する．この関数を呼び出すまで物理エンジンに対するrgeファイル側の設定は無効
        FrameRef rootFrame(); //!< rootフレームを取得
        void setRootFrame(FrameRef root); //!< rootフレームを設定
        rgeScalar update(rgeScalar delta = -1); //!< アニメーションや物理エンジンの時間をdelta秒 進める.　省略時は自動計測．戻り値は，計測された経過時間
        void setViewport(int x, int y, int width, int height);

        void render(); //!< シーンを完全に描画する

        void castShadaw(); //!< 影を描画する
        virtual void drawLightSceneCallback(); //!< 影描画ルーチンで使用される

        ShadowMapping* shadowMapper() {
            return mShadowMapper;
        }

        void setStatus(); //!<シーン描画用ディフォルトステータスを設定
        void calcModelToWorldMatrix(); //!< 全フレームのモデル->ワールド変換行列を計算する
        void applyCameraTransform(); //!< カメラ変換を実行し，行列を記録する．
        void lighting(); //!< 照明を設定する
        void drawScene(); //!< レンダラブルオブジェクトを描画する
        void drawZOnly(); //!< Zバッファにだけ影響するレイヤーを描画

        void addShadowMap(const rgeMatrix4x4& matrix, rgeScalar fov, rgeScalar range, rgeScalar intencity = 0.1) { //!< シャドウマッピングの設定をリストに加える
            ShadowMapDesc desc;
            desc.lightMatrix = matrix;
            desc.fov = fov;
            desc.range = range;
            desc.intencity = intencity;
            mShadowMapList.push_back(desc);
        }

        void registerChildFrames(FrameRef f);
        void registerFrame(const string& name, FrameRef f); //!< フレームを名前付きリストに登録する．登録したframeはfindFrameで取得できる

        void registerLight(LightRef l) {
            mLights.insert(LightRefMap::value_type(l->getId(), l));
        }

        void registerTexture(TextureRef t) {
            //cout << "regist tex \"" << t->getId() << "\"\n";
            mTextures.insert(TextureRefMap::value_type(t->getId(), t));
        }

        void registerMaterial(MaterialRef m) {
            mMaterials.insert(MaterialRefMap::value_type(m->getId(), m));
        }

        void registerMesh(MeshObjectRef m) {
            mMeshObjs.insert(MeshObjRefMap::value_type(m->getId(), m));
        }

        void registerCamera(CameraRef c) {
            mCameras.insert(CameraRefMap::value_type(c->getId(), c));
        }

        void registerAction(ActionRef a) {
            mActions.insert(ActionRefMap::value_type(a->getId(), a));
        }


        MaterialRef findMaterial(const string& name); //!< Materialを名前で取得する
        TextureRef findTexture(const string& name); //!< Textureを名前で取得する
        MeshObjectRef findMesh(const string& name); //!< MeshObjectを名前で取得する
        CameraRef findCamera(const string& name); //!< Cameraを名前で取得する
        LightRef findLight(const string& name); //!< Lightを名前で取得する
        ActionRef findAction(const string& name); //!< Actionを名前で取得する
        FrameRef findFrame(const string& name); //!< Frameを名前で取得する
        MaterialRef makeMaterial(const string& name); //!< Materialを生成する

        //	void addCamera(CameraRef cam) { mCameras.push_back(cam); }

        void setCurrentCamera(CameraRef cam) {
            mCurrentCamera = cam;
        } //!< 現在のカメラを設定する

        Camera* currentCamera() {
            return mCurrentCamera.get();
        } //!< 現在のカメラを取得する

        void applyCameraMatrix() {
            rgeGlLoadMatrix(mCameraMatrix.getOpenGLMatrix());
        } //!< カメラの変換行列をOpenGLに設定する

        rgeMatrix4x4* cameraMatrix() {
            return &mCameraMatrix;
        }

        void setAspectRatio(rgeScalar a) {
            mAspectRatio = a;
        } //!<Windowのアスペクト比を設定する

        const color3& getAmbientColor() {
            return mAmbientColor;
        } //!< AmbientColorを設定する

        void setBackgroundColor(color3 col) {
            mBackgroundColor = col;
        } //!< 背景色を設定する

        void resetLightNum() {
            mLightNum = 0;
        } //!< 利用するライトの数を０に戻す

        int usedLight() {
            return (mLightNum < mMaxLights) ? mLightNum++ : -1;
        } //!< シーンで利用されているライトの数

        void enableLight(bool b) {
            mLightEnable = b;
        } //!< ライティングをオンにするか否か　

        bool isLightEnable() {
            return mLightEnable;
        } //!< ライティングが有効か

        void enableMaterial(bool b) {
            mMaterialEnable = b;
        } //!< マテリアルを有効にするか否か

        bool isMaterialEnable() {
            return mMaterialEnable;
        } //!< マテリアルが有効か否か

        void setRenderBlend(bool b) {
            mbRenderBlend = b;
        } //!< 現在利用できない

        void setClearMatrix(bool b = true){
            mbClearMatrix = b;
        }
        
        bool renderBlend() {
            return mbRenderBlend;
        } //!<現在利用できない

        u_int visibleLayer() {
            return mVisibleLayer;
        }

        void setLayerVisibility(LAYER_NUM n, bool visible) { //!<レイヤーnの可視性を設定
            if (visible)
                mVisibleLayer |= 1 << (n - 1);
            else
                mVisibleLayer &= (-1) - (1 << (n - 1));
        }

        bool getLayerVisibility(u_int layer_bit) {
            return (mVisibleLayer & layer_bit) && layer_bit != (1 << (mZMaskLayer - 1));
        }

        void setVisibleLayerMask(u_int mask) {
            mVisibleLayer = mask;
        } //!< 全レイヤーの可視性を設定

        void setActiveLayer(LAYER_NUM n) {
            mActiveLayer = n;
        } //!< 新しいフレームがおかれるレイヤーを設定

        LAYER_NUM activeLayer() {
            return mActiveLayer;
        } //!<新しいフレームが置かれるレイヤーを取得

        void setZMaskLayer(LAYER_NUM n) {
            mZMaskLayer = n;
        } //!< Zバッファにだけ影響するレイヤを設定（遮蔽はするが，描画されない）

        LAYER_NUM zmaskLayer() {
            return mZMaskLayer;
        } //!< Zバッファにだけ影響するレイヤを取得

        //! 半径radiusの球体のメッシュをもったフレームをparentの子フレームとして生成し，重さmassの物理属性を設定する
        FrameRef createSphereBody(const rgeVector3& pos, rgeScalar radius, rgeScalar mass = 1, FrameRef parent = FrameRef((Frame*) 0));
        //! 大きさがsizeの球体のメッシュをもったフレームをparentの子フレームとして生成し，重さmassの物理属性を設定する
        FrameRef createBoxBody(const rgeVector3& pos, rgeScalar size, rgeScalar mass = 1, FrameRef parent = FrameRef((Frame*) 0));
        //! ConvexHullメッシュで近似した当たり判定メッシュを設定したフレームをparentの子フレームとして生成し，重さmassの物理属性を設定する
        FrameRef createConvexHullBody(const rgeVector3& pos, MeshObjectRef mesh, rgeScalar mass = 1, FrameRef parent = FrameRef((Frame*) 0));

        LightRef createPointLight(const string& name, color3 col, rgeScalar energy) {
            LightRef l = LightRef(new PointLight(col, energy));
            l->setId(name);
            registerLight(l);
            return l;
        }

        void setFps(int fps) {
            mFps = fps;
        }

        int fps() {
            return mFps;
        }

        void setUseCallList(bool b) {
            mbUseCallList = b;
        }

        bool useCallList() {
            return mbUseCallList;
        }

        class Logger {
        public:

            Logger() {
            }

            template<typename T>
            void outlog(T& rhs) {
            }

            template<typename T>
            Logger& operator<<(T& rhs) {
                outlog(rhs);
                return *this;
            }

            Logger& operator<<(const string& rhs) {
                outlog(rhs);
                return *this;
            }

            Logger& operator<<(const char* rhs) {
                outlog(rhs);
                return *this;
            }

            Logger& operator<<(int rhs) {
                outlog(rhs);
                return *this;
            }
        };
#ifdef __RGE_VERBOSE

        std::ostream& logger() {
            return std::cout;
        }
#else

        Logger& logger() {
            return mLogger;
        }
#endif

    private:

        void clearShadowMapLishts() {
            mShadowMapList.clear();
        }

        PhysicsEngine* mpPhysicsEngine;
        ShadowMapping* mShadowMapper;

        MaterialRefMap mMaterials;
        TextureRefMap mTextures;
        MeshObjRefMap mMeshObjs;
        CameraRefMap mCameras;
        LightRefMap mLights;
        ActionRefMap mActions;

        FrameRefMap mFrames;
        //	RenderableRefs mRenderables;
        FrameRef mRootFrame;
        CameraRef mCurrentCamera;
        rgeMatrix4x4 mCameraMatrix;

        struct ShadowMapDesc {
            rgeMatrix4x4 lightMatrix;
            rgeScalar fov;
            rgeScalar range;
            rgeScalar intencity;
        };

        vector<ShadowMapDesc> mShadowMapList;

        color3 mBackgroundColor;
        rgeScalar mAspectRatio;
        color3 mAmbientColor;

        Timer mTimer;
        int mFps;

        GLint mViewport[4];

        int mMaxLights;
        int mLightNum;
        bool mLightEnable;
        bool mMaterialEnable;

        bool mbClearMatrix;
        bool mbRenderBlend;
        bool mbUseCallList;

        u_int mVisibleLayer;
        LAYER_NUM mActiveLayer;
        LAYER_NUM mZMaskLayer;

        std::string mBaseDir;

        Logger mLogger;
    };

}
