#include "IScriptManager.h"


class IScriptManager;

void RegisterAllFunctions( IScriptManager* pScriptManager );

void SaveScene( IScriptState* pState );
void LoadScene( IScriptState* pState );
void Merge( IScriptState* pState );
void TestMessageBox( IScriptState* pState );
void Operation( IScriptState* pState );
void Operation3( IScriptState* pState );
void cls( IScriptState* pState );
void Exit( IScriptState* pState );
void LoadEntity( IScriptState* pState );
void DisplayEntityPosition( IScriptState* pState );
void SetEntityPos( IScriptState* pState );
void DisplayCamPos( IScriptState* pState );
void SetBkgColor( IScriptState* pState );
void DisplayBkgColor( IScriptState* pState );
void DisplayBBox( IScriptState* pState );
void DisplayEntities( IScriptState* pState );
void GetEntityID( IScriptState* pState );
void SetEntityShader( IScriptState* pState );
void YawEntity( IScriptState* pState );
void PitchEntity( IScriptState* pState );
void RollEntity( IScriptState* pState );
void AddLight( IScriptState* pState );
void AddLightw( IScriptState* pState );
void SetLightIntensity( IScriptState* pState );
void DisplayBBoxInfos( IScriptState* pState );
void ScreenCapture( IScriptState* pState );
void CreateHM( IScriptState* pState );
void LoadHM( IScriptState* pState );
void SetEntityWeight( IScriptState* pState );
void SetSceneMap( IScriptState* pState );
void ClearScene( IScriptState* pState );
void ExportBMEToAscii( IScriptState* pState );
void ExportBKEToAscii( IScriptState* pState );
void run( IScriptState* pState );
void SetCamPos( IScriptState* pState );
void SetReperePosition( IScriptState* pState );
void SetReperePos( IScriptState* pState );
void DisplayLightIntensity( IScriptState* pState );
void flist( IScriptState* pState );
void DisplaySceneChilds( IScriptState* pState );
void DisplayHM( IScriptState* pState );
void StopDisplayHM( IScriptState* pState );
void DisplayCollisionMap(IScriptState* pState);
void StopDisplayCollisionMap(IScriptState* pState);
void ExtractHM( IScriptState* pState );
void SetHMPrecision( IScriptState* pState );
void DisplayShaderName( IScriptState* pState );
void LoadShader( IScriptState* pState );
void SetAnimation( IScriptState* pState );
void PlayCurrentAnimation( IScriptState* pState );
void reset( IScriptState* pState );
void RunScript( string sFileName );
void DisplayEntitySkeletonInfos( IScriptState* pState );
void SelectBone( IScriptState* pState ); // ID entit้, ID bone
void Yaw( IScriptState* pState );
void Pitch( IScriptState* pState );
void Roll( IScriptState* pState );
void PauseAnimation( IScriptState* pState );
void DetachAnimation( IScriptState* pState );
void StopAnimation( IScriptState* pState );
void Sleep( IScriptState* pState );
void HideEntity( IScriptState* pState );
void Link( IScriptState* pState );
void SetZCollisionError( IScriptState* pState );
void SetConstantLocalTranslate( IScriptState* pState );
void NextAnimationFrame( IScriptState* pState );
void NextAnimationKey( IScriptState* pState );
void CreateMobileEntity( IScriptState* pState );
void CreateLineEntity(IScriptState* pState);
void CreateNPC( IScriptState* pState );
void Walk( IScriptState* pState );
void Stand( IScriptState* pState );
void Run( IScriptState* pState );
void DisplayNodeInfos( IScriptState* );
void StopRender( IScriptState* );
void SetAnimationSpeed( IScriptState* pState );
void SetGravity( IScriptState* pState );
void SetCurrentPerso( IScriptState* pState );
void SetCameraType( IScriptState* pState );
void LocalTranslate( IScriptState* pState );
void RunAction( IScriptState* pState );
void SetScale( IScriptState* pState );
void SetRenderType( IScriptState* pState );
void DisplayBoundingSphere( IScriptState* pState );
void DisplayBoneBoundingSphere( IScriptState* pState );
void Unlink( IScriptState* pState );
void ComputeKeysBoundingBoxes( IScriptState* pState );
void SetPreferedKeyBBox( IScriptState* pState );
void Test( IScriptState* pState );
void ChangeBase( IScriptState* pState );
void CreateBox( IScriptState* pState );
void CreateSphere( IScriptState* pState );
void CreateRepere( IScriptState* pState );
void DisplayAnimationBBox( IScriptState* pState );
void Goto( IScriptState* pState );
void SetEntityName( IScriptState* pState );
void DisplayFov( IScriptState* pState );
void SetFov( IScriptState* pState );
void print(IScriptState* pState);
void GetCameraID(IScriptState* pState);
void CreateCollisionMap(IScriptState* pState);
void SetCameraMatrix(IScriptState* pState);
void SetProjectionMatrixType(IScriptState* pState);
void LockCamera(IScriptState* pState);
void DisplayProjectionMatrix(IScriptState* pState);
void DisplayModelViewProjectionMatrix(IScriptState* pState);
void testCollisionShader(IScriptState* pState);
void ReloadShader(IScriptState* pState);
void CullFace(IScriptState* pState);
void EnableRenderCallback(IScriptState* pState);
void SendCustomUniformValue(IScriptState* pState);
void SetLineWidth(IScriptState* pState);
void DisplayGrid(IScriptState* pState);
void SetCurrentCollisionMap(IScriptState* pState);