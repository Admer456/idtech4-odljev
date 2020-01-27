#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
//#include "WeaponTool.h"

// 
// event defs
// 

const idEventDef EV_Tool_Clear(			"<clear>" );
const idEventDef EV_Tool_GetOwner(		"getOwner", NULL, 'e' );
const idEventDef EV_Tool_Next(			"nextTool" );
const idEventDef EV_Tool_ToolState(		"toolState", "sd" );
const idEventDef EV_Tool_ToolReady(		"toolReady" );
const idEventDef EV_Tool_ToolHolstered( "toolHolstered" );
const idEventDef EV_Tool_ToolRising(	"toolRising" );
const idEventDef EV_Tool_ToolLowering(	"toolLowering" );
const idEventDef EV_Tool_Use(			"toolUse", NULL, 'd' );
const idEventDef EV_Tool_Flashlight(	"flashlight", "d" );
const idEventDef EV_Tool_GetWorldModel( "getWorldModel", NULL, 'e' );
const idEventDef EV_Tool_AllowDrop(		"allowDrop", "d" );
const idEventDef EV_Tool_IsInvisible(	"isInvisible", NULL, 'f' );

//
// class def
//

CLASS_DECLARATION( idAnimatedEntity, admWeaponTool )
		EVENT( EV_Tool_Clear,			admWeaponTool::Event_Clear )
		EVENT( EV_Tool_GetOwner,		admWeaponTool::Event_GetOwner )
		EVENT( EV_Tool_Next,			admWeaponTool::Event_Next )
		EVENT( EV_Tool_ToolState,		admWeaponTool::Event_ToolState )
		EVENT( EV_Tool_ToolReady,		admWeaponTool::Event_ToolReady )
		EVENT( EV_Tool_ToolHolstered,	admWeaponTool::Event_ToolHolstered )
		EVENT( EV_Tool_ToolRising,		admWeaponTool::Event_ToolRising )
		EVENT( EV_Tool_ToolLowering,	admWeaponTool::Event_ToolLowering )
		EVENT( EV_Tool_Use,				admWeaponTool::Event_Use )
		EVENT( EV_Tool_Flashlight,		admWeaponTool::Event_Flashlight )
		EVENT( EV_Tool_GetWorldModel,	admWeaponTool::Event_GetWorldModel )
		EVENT( EV_Tool_AllowDrop,		admWeaponTool::Event_AllowDrop )
		EVENT( EV_Tool_IsInvisible,		admWeaponTool::Event_IsInvisible )
END_CLASS

admWeaponTool::admWeaponTool()
{
	owner = NULL;
	worldModel = NULL;
	weaponDef = NULL;
	thread = NULL;

	memset( &guiLight, 0, sizeof( guiLight ) );
	memset( &muzzleFlash, 0, sizeof( muzzleFlash ) );
	memset( &worldMuzzleFlash, 0, sizeof( worldMuzzleFlash ) );

	muzzleFlashEnd = 0;
	flashColor = vec3_origin;
	muzzleFlashHandle = -1;
	worldMuzzleFlashHandle = -1;
	guiLightHandle = -1;
	modelDefHandle = -1;

	berserk = 2;

	allowDrop = true;

	Clear();

	fl.networkSync = true;
}

admWeaponTool::~admWeaponTool()
{
	Clear();
	delete worldModel.GetEntity();
}

void admWeaponTool::Spawn()
{
	if ( !gameLocal.isClient )
	{
		worldModel = static_cast<idAnimatedEntity*>(gameLocal.SpawnEntityType( idAnimatedEntity::Type, NULL ));
		worldModel.GetEntity()->fl.networkSync = true;
	}

	thread = new idThread();
	thread->ManualDelete();
	thread->ManualControl();
}

void admWeaponTool::SetOwner( idPlayer *_owner )
{
	assert( !owner );
	owner = _owner;
	SetName( va( "%s_tool", owner->name.c_str() ) );

	if ( worldModel.GetEntity() )
	{
		worldModel.GetEntity()->SetName( va( "%s_tool_worldmodel", owner->name.c_str() ) );
	}
}

void admWeaponTool::CacheTool( const char *toolName )
{
	const idDeclEntityDef *weaponDef;
	const char *guiName;

	weaponDef = gameLocal.FindEntityDef( toolName, false );
	if ( !weaponDef )
	{
		return;
	}

	guiName = weaponDef->dict.GetString( "gui" );
	if ( guiName[ 0 ] )
	{
		uiManager->FindGui( guiName, true, false, true );
	}
}

void admWeaponTool::Save( idSaveGame *savefile ) const
{
	savefile->WriteInt( status );
	savefile->WriteObject( thread );
	savefile->WriteString( state );
	savefile->WriteString( idealState );
	savefile->WriteInt( animBlendFrames );
	savefile->WriteInt( animDoneTime );
	savefile->WriteBool( isLinked );

	savefile->WriteObject( owner );
	worldModel.Save( savefile );

	savefile->WriteInt( hideTime );
	savefile->WriteFloat( hideDistance );
	savefile->WriteInt( hideStartTime );
	savefile->WriteFloat( hideStart );
	savefile->WriteFloat( hideEnd );
	savefile->WriteFloat( hideOffset );
	savefile->WriteBool( hide );
	savefile->WriteBool( disabled );

	savefile->WriteInt( berserk );

	savefile->WriteVec3( playerViewOrigin );
	savefile->WriteMat3( playerViewAxis );

	savefile->WriteVec3( viewToolOrigin );
	savefile->WriteMat3( viewToolAxis );

//	savefile->WriteVec3( muzzleOrigin );
//	savefile->WriteMat3( muzzleAxis );

//	savefile->WriteVec3( pushVelocity );

	savefile->WriteString( weaponDef->GetName() );
	savefile->WriteFloat( useDistance );
//	savefile->WriteString( meleeDefName );
//	savefile->WriteInt( brassDelay );
	savefile->WriteString( icon );

	savefile->WriteInt( guiLightHandle );
	savefile->WriteRenderLight( guiLight );

	savefile->WriteInt( muzzleFlashHandle );
	savefile->WriteRenderLight( muzzleFlash );

	savefile->WriteInt( worldMuzzleFlashHandle );
	savefile->WriteRenderLight( worldMuzzleFlash );

	savefile->WriteVec3( flashColor );
	savefile->WriteInt( muzzleFlashEnd );
	savefile->WriteInt( flashTime );

	savefile->WriteBool( lightOn );
	savefile->WriteBool( silent_fire );

//	savefile->WriteInt( kick_endtime );
//	savefile->WriteInt( muzzle_kick_time );
//	savefile->WriteInt( muzzle_kick_maxtime );
//	savefile->WriteAngles( muzzle_kick_angles );
//	savefile->WriteVec3( muzzle_kick_offset );

//	savefile->WriteInt( ammoType );
//	savefile->WriteInt( ammoRequired );
//	savefile->WriteInt( clipSize );
//	savefile->WriteInt( ammoClip );
//	savefile->WriteInt( lowAmmo );
//	savefile->WriteBool( powerAmmo );

	// savegames <= 17
	savefile->WriteInt( 0 );

	savefile->WriteInt( zoomFov );

//	savefile->WriteJoint( barrelJointView );
	savefile->WriteJoint( flashJointView );
//	savefile->WriteJoint( ejectJointView );
	savefile->WriteJoint( guiLightJointView );
//	savefile->WriteJoint( ventLightJointView );

	savefile->WriteJoint( flashJointWorld );
//	savefile->WriteJoint( barrelJointWorld );
//	savefile->WriteJoint( ejectJointWorld );

//	savefile->WriteBool( hasBloodSplat );

	savefile->WriteSoundShader( sndHum );

//	savefile->WriteParticle( weaponSmoke );
//	savefile->WriteInt( weaponSmokeStartTime );
//	savefile->WriteBool( continuousSmoke );
//	savefile->WriteParticle( strikeSmoke );
//	savefile->WriteInt( strikeSmokeStartTime );
//	savefile->WriteVec3( strikePos );
//	savefile->WriteMat3( strikeAxis );
//	savefile->WriteInt( nextStrikeFx );

//	savefile->WriteBool( nozzleFx );
//	savefile->WriteInt( nozzleFxFade );

	savefile->WriteInt( lastAttack );

//	savefile->WriteInt( nozzleGlowHandle );
//	savefile->WriteRenderLight( nozzleGlow );

//	savefile->WriteVec3( nozzleGlowColor );
//	savefile->WriteMaterial( nozzleGlowShader );
//	savefile->WriteFloat( nozzleGlowRadius );

	savefile->WriteInt(   toolAngleOffsetAverages );
	savefile->WriteFloat( toolAngleOffsetScale );
	savefile->WriteFloat( toolAngleOffsetMax );
	savefile->WriteFloat( toolOffsetTime );
	savefile->WriteFloat( toolOffsetScale );

	savefile->WriteBool( allowDrop );
//	savefile->WriteObject( projectileEnt );
}

void admWeaponTool::Restore( idRestoreGame *savefile )
{

	savefile->ReadInt( (int &)status );
	savefile->ReadObject( reinterpret_cast<idClass *&>(thread) );
	savefile->ReadString( state );
	savefile->ReadString( idealState );
	savefile->ReadInt( animBlendFrames );
	savefile->ReadInt( animDoneTime );
	savefile->ReadBool( isLinked );

	// Re-link script fields
	TOOL_ATTACK.LinkTo( scriptObject, "WEAPON_ATTACK" );
//	TOOL_RELOAD.LinkTo( scriptObject, "WEAPON_RELOAD" );
//	TOOL_NETRELOAD.LinkTo( scriptObject, "WEAPON_NETRELOAD" );
//	TOOL_NETENDRELOAD.LinkTo( scriptObject, "WEAPON_NETENDRELOAD" );
	TOOL_NETFIRING.LinkTo( scriptObject, "WEAPON_NETFIRING" );
	TOOL_RAISE.LinkTo( scriptObject, "WEAPON_RAISEWEAPON" );
	TOOL_LOWER.LinkTo( scriptObject, "WEAPON_LOWERWEAPON" );

	savefile->ReadObject( reinterpret_cast<idClass *&>(owner) );
	worldModel.Restore( savefile );

	savefile->ReadInt( hideTime );
	savefile->ReadFloat( hideDistance );
	savefile->ReadInt( hideStartTime );
	savefile->ReadFloat( hideStart );
	savefile->ReadFloat( hideEnd );
	savefile->ReadFloat( hideOffset );
	savefile->ReadBool( hide );
	savefile->ReadBool( disabled );

	savefile->ReadInt( berserk );

	savefile->ReadVec3( playerViewOrigin );
	savefile->ReadMat3( playerViewAxis );

	savefile->ReadVec3( viewToolOrigin );
	savefile->ReadMat3( viewToolAxis );

//	savefile->ReadVec3( muzzleOrigin );
//	savefile->ReadMat3( muzzleAxis );

//	savefile->ReadVec3( pushVelocity );

/*	idStr objectname;
	savefile->ReadString( objectname );
	weaponDef = gameLocal.FindEntityDef( objectname );
	meleeDef = gameLocal.FindEntityDef( weaponDef->dict.GetString( "def_melee" ), false );

	const idDeclEntityDef *projectileDef = gameLocal.FindEntityDef( weaponDef->dict.GetString( "def_projectile" ), false );
	if ( projectileDef )
	{
		projectileDict = projectileDef->dict;
	}
	else
	{
		projectileDict.Clear();
	}

	const idDeclEntityDef *brassDef = gameLocal.FindEntityDef( weaponDef->dict.GetString( "def_ejectBrass" ), false );
	if ( brassDef )
	{
		brassDict = brassDef->dict;
	}
	else
	{
		brassDict.Clear();
	}*/

	savefile->ReadFloat( useDistance );
//	savefile->ReadString( meleeDefName );
//	savefile->ReadInt( brassDelay );
	savefile->ReadString( icon );

	savefile->ReadInt( guiLightHandle );
	savefile->ReadRenderLight( guiLight );

	savefile->ReadInt( muzzleFlashHandle );
	savefile->ReadRenderLight( muzzleFlash );

	savefile->ReadInt( worldMuzzleFlashHandle );
	savefile->ReadRenderLight( worldMuzzleFlash );

	savefile->ReadVec3( flashColor );
	savefile->ReadInt( muzzleFlashEnd );
	savefile->ReadInt( flashTime );

	savefile->ReadBool( lightOn );
	savefile->ReadBool( silent_fire );

//	savefile->ReadInt( kick_endtime );
//	savefile->ReadInt( muzzle_kick_time );
//	savefile->ReadInt( muzzle_kick_maxtime );
//	savefile->ReadAngles( muzzle_kick_angles );
//	savefile->ReadVec3( muzzle_kick_offset );

//	savefile->ReadInt( (int &)ammoType );
//	savefile->ReadInt( ammoRequired );
//	savefile->ReadInt( clipSize );
//	savefile->ReadInt( ammoClip );
//	savefile->ReadInt( lowAmmo );
//	savefile->ReadBool( powerAmmo );

	// savegame versions <= 17
	int foo;
	savefile->ReadInt( foo );

	savefile->ReadInt( zoomFov );

//	savefile->ReadJoint( barrelJointView );
	savefile->ReadJoint( flashJointView );
//	savefile->ReadJoint( ejectJointView );
	savefile->ReadJoint( guiLightJointView );
//	savefile->ReadJoint( ventLightJointView );

	savefile->ReadJoint( flashJointWorld );
//	savefile->ReadJoint( barrelJointWorld );
//	savefile->ReadJoint( ejectJointWorld );

//	savefile->ReadBool( hasBloodSplat );

	savefile->ReadSoundShader( sndHum );

//	savefile->ReadParticle( weaponSmoke );
//	savefile->ReadInt( weaponSmokeStartTime );
//	savefile->ReadBool( continuousSmoke );
//	savefile->ReadParticle( strikeSmoke );
//	savefile->ReadInt( strikeSmokeStartTime );
//	savefile->ReadVec3( strikePos );
//	savefile->ReadMat3( strikeAxis );
//	savefile->ReadInt( nextStrikeFx );

//	savefile->ReadBool( nozzleFx );
//	savefile->ReadInt( nozzleFxFade );

	savefile->ReadInt( lastAttack );

//	savefile->ReadInt( nozzleGlowHandle );
//	savefile->ReadRenderLight( nozzleGlow );

//	savefile->ReadVec3( nozzleGlowColor );
//	savefile->ReadMaterial( nozzleGlowShader );
//	savefile->ReadFloat( nozzleGlowRadius );

	savefile->ReadInt(   toolAngleOffsetAverages );
	savefile->ReadFloat( toolAngleOffsetScale );
	savefile->ReadFloat( toolAngleOffsetMax );
	savefile->ReadFloat( toolOffsetTime );
	savefile->ReadFloat( toolOffsetScale );

	savefile->ReadBool( allowDrop );
//	savefile->ReadObject( reinterpret_cast<idClass *&>(projectileEnt) );
}

void admWeaponTool::Clear( void )
{
	CancelEvents( &EV_Tool_Clear );

	DeconstructScriptObject();
	scriptObject.Free();

	TOOL_ATTACK.Unlink();
	TOOL_NETFIRING.Unlink();
	TOOL_RAISE.Unlink();
	TOOL_LOWER.Unlink();

	if ( muzzleFlashHandle != -1 )
	{
		gameRenderWorld->FreeLightDef( muzzleFlashHandle );
		muzzleFlashHandle = -1;
	}
	if ( muzzleFlashHandle != -1 )
	{
		gameRenderWorld->FreeLightDef( muzzleFlashHandle );
		muzzleFlashHandle = -1;
	}
	if ( worldMuzzleFlashHandle != -1 )
	{
		gameRenderWorld->FreeLightDef( worldMuzzleFlashHandle );
		worldMuzzleFlashHandle = -1;
	}
	if ( guiLightHandle != -1 )
	{
		gameRenderWorld->FreeLightDef( guiLightHandle );
		guiLightHandle = -1;
	}
//	if ( nozzleGlowHandle != -1 )
//	{
//		gameRenderWorld->FreeLightDef( nozzleGlowHandle );
//		nozzleGlowHandle = -1;
//	}

	memset( &renderEntity, 0, sizeof( renderEntity ) );
	renderEntity.entityNum = entityNumber;

	renderEntity.noShadow = true;
	renderEntity.noSelfShadow = true;
	renderEntity.customSkin = NULL;

	// set default shader parms
	renderEntity.shaderParms[ SHADERPARM_RED ] = 1.0f;
	renderEntity.shaderParms[ SHADERPARM_GREEN ] = 1.0f;
	renderEntity.shaderParms[ SHADERPARM_BLUE ] = 1.0f;
	renderEntity.shaderParms[ 3 ] = 1.0f;
	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = 0.0f;
	renderEntity.shaderParms[ 5 ] = 0.0f;
	renderEntity.shaderParms[ 6 ] = 0.0f;
	renderEntity.shaderParms[ 7 ] = 0.0f;

	if ( refSound.referenceSound )
	{
		refSound.referenceSound->Free( true );
	}
	memset( &refSound, 0, 56U );

	// setting diversity to 0 results in no random sound.  -1 indicates random.
	refSound.diversity = -1.0f;

	if ( owner )
	{
		// don't spatialize the weapon sounds
		refSound.listenerId = owner->GetListenerId();
	}

	// clear out the sounds from our spawnargs since we'll copy them from the weapon def
	const idKeyValue *kv = spawnArgs.MatchPrefix( "snd_" );
	while ( kv )
	{
		spawnArgs.Delete( kv->GetKey() );
		kv = spawnArgs.MatchPrefix( "snd_" );
	}

	hideTime = 300;
	hideDistance = -15.0f;
	hideStartTime = gameLocal.time - hideTime;
	hideStart = 0.0f;
	hideEnd = 0.0f;
	hideOffset = 0.0f;
	hide = false;
	disabled = false;

//	weaponSmoke = NULL;
//	weaponSmokeStartTime = 0;
//	continuousSmoke = false;
//	strikeSmoke = NULL;
//	strikeSmokeStartTime = 0;
//	strikePos.Zero();
//	strikeAxis = mat3_identity;
//	nextStrikeFx = 0;

	icon = "";

	playerViewAxis.Identity();
	playerViewOrigin.Zero();
	viewToolAxis.Identity();
	viewToolOrigin.Zero();
//	muzzleAxis.Identity();
//	muzzleOrigin.Zero();
//	pushVelocity.Zero();

	status = TL_HOLSTERED;
	state = "";
	idealState = "";
	animBlendFrames = 0;
	animDoneTime = 0;

//	projectileDict.Clear();
//	meleeDef = NULL;
//	meleeDefName = "";
//	meleeDistance = 0.0f;
//	brassDict.Clear();

	flashTime = 250;
	lightOn = false;
	silent_fire = false;

//	ammoType = 0;
//	ammoRequired = 0;
//	ammoClip = 0;
//	clipSize = 0;
//	lowAmmo = 0;
//	powerAmmo = false;

//	kick_endtime = 0;
//	muzzle_kick_time = 0;
//	muzzle_kick_maxtime = 0;
//	muzzle_kick_angles.Zero();
//	muzzle_kick_offset.Zero();

	zoomFov = 90;

//	barrelJointView = INVALID_JOINT;
	flashJointView = INVALID_JOINT;
//	ejectJointView = INVALID_JOINT;
	guiLightJointView = INVALID_JOINT;
//	ventLightJointView = INVALID_JOINT;

//	barrelJointWorld = INVALID_JOINT;
	flashJointWorld = INVALID_JOINT;
//	ejectJointWorld = INVALID_JOINT;

//	hasBloodSplat = false;
//	nozzleFx = false;
//	nozzleFxFade = 1500;
	lastAttack = 0;
//	nozzleGlowHandle = -1;
//	nozzleGlowShader = NULL;
//	nozzleGlowRadius = 10;
//	nozzleGlowColor.Zero();

	toolAngleOffsetAverages = 0;
	toolAngleOffsetScale = 0.0f;
	toolAngleOffsetMax = 0.0f;
	toolOffsetTime = 0.0f;
	toolOffsetScale = 0.0f;

	allowDrop = true;

	animator.ClearAllAnims( gameLocal.time, 0 );
	FreeModelDef();

	sndHum = NULL;

	isLinked = false;
//	projectileEnt = NULL;

	isFiring = false;
}

void admWeaponTool::InitWorldModel( const idDeclEntityDef *def )
{
	idEntity *ent;

	ent = worldModel.GetEntity();

	assert( ent );
	assert( def );

	const char *model = def->dict.GetString( "model_world" );
	const char *attach = def->dict.GetString( "joint_attach" );

	ent->SetSkin( NULL );
	if ( model[ 0 ] && attach[ 0 ] )
	{
		ent->Show();
		ent->SetModel( model );
		if ( ent->GetAnimator()->ModelDef() )
		{
			ent->SetSkin( ent->GetAnimator()->ModelDef()->GetDefaultSkin() );
		}
		ent->GetPhysics()->SetContents( 0 );
		ent->GetPhysics()->SetClipModel( NULL, 1.0f );
		ent->BindToJoint( owner, attach, true );
		ent->GetPhysics()->SetOrigin( vec3_origin );
		ent->GetPhysics()->SetAxis( mat3_identity );

		// supress model in player views, but allow it in mirrors and remote views
		renderEntity_t *worldModelRenderEntity = ent->GetRenderEntity();
		if ( worldModelRenderEntity )
		{
			worldModelRenderEntity->suppressSurfaceInViewID = owner->entityNumber + 1;
			worldModelRenderEntity->suppressShadowInViewID = owner->entityNumber + 1;
			worldModelRenderEntity->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + owner->entityNumber;
		}
	}
	else
	{
		ent->SetModel( "" );
		ent->Hide();
	}

	flashJointWorld = ent->GetAnimator()->GetJointHandle( "flash" );
}

/*
================
admWeaponTool::GetWeaponDef
================
*/
void admWeaponTool::GetToolDef( const char *objectname )
{
	const char *shader;
	const char *objectType;
	const char *vmodel;
	const char *guiName;
//	const char *projectileName;
//	const char *brassDefName;
//	const char *smokeName;
//	int			ammoAvail;

	Clear();

	if ( !objectname || !objectname[ 0 ] )
	{
		return;
	}

	assert( owner );

	weaponDef = gameLocal.FindEntityDef( objectname );

//	ammoType = GetAmmoNumForName( weaponDef->dict.GetString( "ammoType" ) );
//	ammoRequired = weaponDef->dict.GetInt( "ammoRequired" );
//	clipSize = weaponDef->dict.GetInt( "clipSize" );
//	lowAmmo = weaponDef->dict.GetInt( "lowAmmo" );

	icon = weaponDef->dict.GetString( "icon" );
	silent_fire = weaponDef->dict.GetBool( "silent_fire" );
//	powerAmmo = weaponDef->dict.GetBool( "powerAmmo" );
//
//	muzzle_kick_time = SEC2MS( weaponDef->dict.GetFloat( "muzzle_kick_time" ) );
//	muzzle_kick_maxtime = SEC2MS( weaponDef->dict.GetFloat( "muzzle_kick_maxtime" ) );
//	muzzle_kick_angles = weaponDef->dict.GetAngles( "muzzle_kick_angles" );
//	muzzle_kick_offset = weaponDef->dict.GetVector( "muzzle_kick_offset" );

	hideTime = SEC2MS( weaponDef->dict.GetFloat( "hide_time", "0.3" ) );
	hideDistance = weaponDef->dict.GetFloat( "hide_distance", "-15" );

	// muzzle smoke
/*	smokeName = weaponDef->dict.GetString( "smoke_muzzle" );
	if ( *smokeName != '\0' )
	{
		weaponSmoke = static_cast<const idDeclParticle *>(declManager->FindType( DECL_PARTICLE, smokeName ));
	}
	else
	{
		weaponSmoke = NULL;
	}
	continuousSmoke = weaponDef->dict.GetBool( "continuousSmoke" );
	weaponSmokeStartTime = (continuousSmoke) ? gameLocal.time : 0;

	smokeName = weaponDef->dict.GetString( "smoke_strike" );
	if ( *smokeName != '\0' )
	{
		strikeSmoke = static_cast<const idDeclParticle *>(declManager->FindType( DECL_PARTICLE, smokeName ));
	}
	else
	{
		strikeSmoke = NULL;
	}*/
//	strikeSmokeStartTime = 0;
//	strikePos.Zero();
//	strikeAxis = mat3_identity;
//	nextStrikeFx = 0;

	// setup gui light
	memset( &guiLight, 0, sizeof( guiLight ) );
	const char *guiLightShader = weaponDef->dict.GetString( "mtr_guiLightShader" );
	if ( *guiLightShader != '\0' )
	{
		guiLight.shader = declManager->FindMaterial( guiLightShader, false );
		guiLight.lightRadius[ 0 ] = guiLight.lightRadius[ 1 ] = guiLight.lightRadius[ 2 ] = 3;
		guiLight.pointLight = true;
	}

	// setup the view model
	vmodel = weaponDef->dict.GetString( "model_view" );
	SetModel( vmodel );

	// setup the world model
	InitWorldModel( weaponDef );

	// copy the sounds from the weapon view model def into out spawnargs
	const idKeyValue *kv = weaponDef->dict.MatchPrefix( "snd_" );
	while ( kv )
	{
		spawnArgs.Set( kv->GetKey(), kv->GetValue() );
		kv = weaponDef->dict.MatchPrefix( "snd_", kv );
	}

	// find some joints in the model for locating effects
//	barrelJointView = animator.GetJointHandle( "barrel" );
	flashJointView = animator.GetJointHandle( "flash" );
//	ejectJointView = animator.GetJointHandle( "eject" );
	guiLightJointView = animator.GetJointHandle( "guiLight" );
//	ventLightJointView = animator.GetJointHandle( "ventLight" );

	// get the projectile
//	projectileDict.Clear();

/*	projectileName = weaponDef->dict.GetString( "def_projectile" );
	if ( projectileName[ 0 ] != '\0' )
	{
		const idDeclEntityDef *projectileDef = gameLocal.FindEntityDef( projectileName, false );
		if ( !projectileDef )
		{
			gameLocal.Warning( "Unknown projectile '%s' in weapon '%s'", projectileName, objectname );
		}
		else
		{
			const char *spawnclass = projectileDef->dict.GetString( "spawnclass" );
			idTypeInfo *cls = idClass::GetClass( spawnclass );
			if ( !cls || !cls->IsType( idProjectile::Type ) )
			{
				gameLocal.Warning( "Invalid spawnclass '%s' on projectile '%s' (used by weapon '%s')", spawnclass, projectileName, objectname );
			}
			else
			{
				projectileDict = projectileDef->dict;
			}
		}
	}*/

	// set up muzzleflash render light
	const idMaterial*flashShader;
	idVec3			flashTarget;
	idVec3			flashUp;
	idVec3			flashRight;
	float			flashRadius;
	bool			flashPointLight;

	weaponDef->dict.GetString( "mtr_flashShader", "", &shader );
	flashShader = declManager->FindMaterial( shader, false );
	flashPointLight = weaponDef->dict.GetBool( "flashPointLight", "1" );
	weaponDef->dict.GetVector( "flashColor", "0 0 0", flashColor );
	flashRadius = (float)weaponDef->dict.GetInt( "flashRadius" );	// if 0, no light will spawn
	flashTime = SEC2MS( weaponDef->dict.GetFloat( "flashTime", "0.25" ) );
	flashTarget = weaponDef->dict.GetVector( "flashTarget" );
	flashUp = weaponDef->dict.GetVector( "flashUp" );
	flashRight = weaponDef->dict.GetVector( "flashRight" );

	memset( &muzzleFlash, 0, sizeof( muzzleFlash ) );
	muzzleFlash.lightId = LIGHTID_VIEW_MUZZLE_FLASH + owner->entityNumber;
	muzzleFlash.allowLightInViewID = owner->entityNumber + 1;

	// the weapon lights will only be in first person
	guiLight.allowLightInViewID = owner->entityNumber + 1;
//	nozzleGlow.allowLightInViewID = owner->entityNumber + 1;

	muzzleFlash.pointLight = flashPointLight;
	muzzleFlash.shader = flashShader;
	muzzleFlash.shaderParms[ SHADERPARM_RED ] = flashColor[ 0 ];
	muzzleFlash.shaderParms[ SHADERPARM_GREEN ] = flashColor[ 1 ];
	muzzleFlash.shaderParms[ SHADERPARM_BLUE ] = flashColor[ 2 ];
	muzzleFlash.shaderParms[ SHADERPARM_TIMESCALE ] = 1.0f;

	muzzleFlash.lightRadius[ 0 ] = flashRadius;
	muzzleFlash.lightRadius[ 1 ] = flashRadius;
	muzzleFlash.lightRadius[ 2 ] = flashRadius;

	if ( !flashPointLight )
	{
		muzzleFlash.target = flashTarget;
		muzzleFlash.up = flashUp;
		muzzleFlash.right = flashRight;
		muzzleFlash.end = flashTarget;
	}

	// the world muzzle flash is the same, just positioned differently
	worldMuzzleFlash = muzzleFlash;
	worldMuzzleFlash.suppressLightInViewID = owner->entityNumber + 1;
	worldMuzzleFlash.allowLightInViewID = 0;
	worldMuzzleFlash.lightId = LIGHTID_WORLD_MUZZLE_FLASH + owner->entityNumber;

	//-----------------------------------

//	nozzleFx = weaponDef->dict.GetBool( "nozzleFx" );
//	nozzleFxFade = weaponDef->dict.GetInt( "nozzleFxFade", "1500" );
//	nozzleGlowColor = weaponDef->dict.GetVector( "nozzleGlowColor", "1 1 1" );
//	nozzleGlowRadius = weaponDef->dict.GetFloat( "nozzleGlowRadius", "10" );
	weaponDef->dict.GetString( "mtr_nozzleGlowShader", "", &shader );
//	nozzleGlowShader = declManager->FindMaterial( shader, false );

	// get the melee damage def
	useDistance = weaponDef->dict.GetFloat( "melee_distance" );
/*	meleeDefName = weaponDef->dict.GetString( "def_melee" );
	if ( meleeDefName.Length() )
	{
		meleeDef = gameLocal.FindEntityDef( meleeDefName, false );
		if ( !meleeDef )
		{
			gameLocal.Error( "Unknown melee '%s'", meleeDefName.c_str() );
		}
	}*/

	// get the brass def
/*	brassDict.Clear();
	brassDelay = weaponDef->dict.GetInt( "ejectBrassDelay", "0" );
	brassDefName = weaponDef->dict.GetString( "def_ejectBrass" );

	if ( brassDefName[ 0 ] )
	{
		const idDeclEntityDef *brassDef = gameLocal.FindEntityDef( brassDefName, false );
		if ( !brassDef )
		{
			gameLocal.Warning( "Unknown brass '%s'", brassDefName );
		}
		else
		{
			brassDict = brassDef->dict;
		}
	}

	if ( (ammoType < 0) || (ammoType >= AMMO_NUMTYPES) )
	{
		gameLocal.Warning( "Unknown ammotype in object '%s'", objectname );
	}

	ammoClip = ammoinclip;
	if ( (ammoClip < 0) || (ammoClip > clipSize) )
	{
// first time using this weapon so have it fully loaded to start
		ammoClip = clipSize;
		ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
		if ( ammoClip > ammoAvail )
		{
			ammoClip = ammoAvail;
		}
	}*/

	renderEntity.gui[ 0 ] = NULL;
	guiName = weaponDef->dict.GetString( "gui" );
	if ( guiName[ 0 ] )
	{
		renderEntity.gui[ 0 ] = uiManager->FindGui( guiName, true, false, true );
	}

	zoomFov = weaponDef->dict.GetInt( "zoomFov", "70" );
	berserk = weaponDef->dict.GetInt( "berserk", "2" );

	toolAngleOffsetAverages = weaponDef->dict.GetInt( "toolAngleOffsetAverages", "10" );
	toolAngleOffsetScale = weaponDef->dict.GetFloat(  "toolAngleOffsetScale", "0.25" );
	toolAngleOffsetMax = weaponDef->dict.GetFloat(    "toolAngleOffsetMax", "10" );

	toolOffsetTime = weaponDef->dict.GetFloat(  "toolOffsetTime", "400" );
	toolOffsetScale = weaponDef->dict.GetFloat( "toolOffsetScale", "0.005" );

	if ( !weaponDef->dict.GetString( "tool_scriptobject", NULL, &objectType ) )
	{
		gameLocal.Error( "No 'tool_scriptobject' set on '%s'.", objectname );
	}

	// setup script object
	if ( !scriptObject.SetType( objectType ) )
	{
		gameLocal.Error( "Script object '%s' not found on tool '%s'.", objectType, objectname );
	}

	TOOL_ATTACK.LinkTo( scriptObject, "TOOL_ATTACK" );
//	TOOL_RELOAD.LinkTo( scriptObject, "WEAPON_RELOAD" );
//	TOOL_NETRELOAD.LinkTo( scriptObject, "WEAPON_NETRELOAD" );
//	TOOL_NETENDRELOAD.LinkTo( scriptObject, "WEAPON_NETENDRELOAD" );
	TOOL_NETFIRING.LinkTo( scriptObject, "TOOL_NETFIRING" );
	TOOL_RAISE.LinkTo( scriptObject, "TOOL_RAISE" );
	TOOL_LOWER.LinkTo( scriptObject, "TOOL_LOWER" );

	spawnArgs = weaponDef->dict;

	shader = spawnArgs.GetString( "snd_hum" );
	if ( shader && *shader )
	{
		sndHum = declManager->FindSound( shader );
		StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
	}

	isLinked = true;

	// call script object's constructor
	ConstructScriptObject();

	// make sure we have the correct skin
	UpdateSkin();
}

/***********************************************************************

	GUIs

***********************************************************************/

/*
================
admWeaponTool::Icon
================
*/
const char *admWeaponTool::Icon( void ) const
{
	return icon;
}

/*
================
admWeaponTool::UpdateGUI
================
*/
void admWeaponTool::UpdateGUI( void )
{
	if ( !renderEntity.gui[ 0 ] )
	{
		return;
	}

	if ( status == TL_HOLSTERED )
	{
		return;
	}

	if ( owner->weaponGone )
	{
// dropping weapons was implemented wierd, so we have to not update the gui when it happens or we'll get a negative ammo count
		return;
	}

	if ( gameLocal.localClientNum != owner->entityNumber )
	{
// if updating the hud for a followed client
		if ( gameLocal.localClientNum >= 0 && gameLocal.entities[ gameLocal.localClientNum ] && gameLocal.entities[ gameLocal.localClientNum ]->IsType( idPlayer::Type ) )
		{
			idPlayer *p = static_cast<idPlayer *>(gameLocal.entities[ gameLocal.localClientNum ]);
			if ( !p->spectating || p->spectator != owner->entityNumber )
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
}

/***********************************************************************

	Model and muzzleflash

***********************************************************************/

/*
================
admWeaponTool::UpdateFlashPosition
================
*/
void admWeaponTool::UpdateFlashPosition( void )
{
// the flash has an explicit joint for locating it
	GetGlobalJointTransform( true, flashJointView, muzzleFlash.origin, muzzleFlash.axis );

	// if the desired point is inside or very close to a wall, back it up until it is clear
	idVec3	start = muzzleFlash.origin - playerViewAxis[ 0 ] * 16;
	idVec3	end = muzzleFlash.origin + playerViewAxis[ 0 ] * 8;
	trace_t	tr;
	gameLocal.clip.TracePoint( tr, start, end, MASK_SHOT_RENDERMODEL, owner );
	// be at least 8 units away from a solid
	muzzleFlash.origin = tr.endpos - playerViewAxis[ 0 ] * 8;

	// put the world muzzle flash on the end of the joint, no matter what
	GetGlobalJointTransform( false, flashJointWorld, worldMuzzleFlash.origin, worldMuzzleFlash.axis );
}

/*
================
admWeaponTool::MuzzleFlashLight
================
*/
void admWeaponTool::MuzzleFlashLight( void )
{

	if ( !lightOn && (!g_muzzleFlash.GetBool() || !muzzleFlash.lightRadius[ 0 ]) )
	{
		return;
	}

	if ( flashJointView == INVALID_JOINT )
	{
		return;
	}

	UpdateFlashPosition();

	muzzleFlash.smLodBias = g_muzzleFlashLightLodBias.GetInteger();

	// these will be different each fire
	muzzleFlash.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
	muzzleFlash.shaderParms[ SHADERPARM_DIVERSITY ] = renderEntity.shaderParms[ SHADERPARM_DIVERSITY ];

	worldMuzzleFlash.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
	worldMuzzleFlash.shaderParms[ SHADERPARM_DIVERSITY ] = renderEntity.shaderParms[ SHADERPARM_DIVERSITY ];

	// the light will be removed at this time
	muzzleFlashEnd = gameLocal.time + flashTime;

	if ( muzzleFlashHandle != -1 )
	{
		gameRenderWorld->UpdateLightDef( muzzleFlashHandle, &muzzleFlash );
		gameRenderWorld->UpdateLightDef( worldMuzzleFlashHandle, &worldMuzzleFlash );
	}
	else
	{
		muzzleFlashHandle = gameRenderWorld->AddLightDef( &muzzleFlash );
		worldMuzzleFlashHandle = gameRenderWorld->AddLightDef( &worldMuzzleFlash );
	}
}

/*
================
admWeaponTool::UpdateSkin
================
*/
bool admWeaponTool::UpdateSkin( void )
{
	const function_t *func;

	if ( !isLinked )
	{
		return false;
	}

	func = scriptObject.GetFunction( "UpdateSkin" );
	if ( !func )
	{
		common->Warning( "Can't find function 'UpdateSkin' in object '%s'", scriptObject.GetTypeName() );
		return false;
	}

	// use the frameCommandThread since it's safe to use outside of framecommands
	gameLocal.frameCommandThread->CallFunction( this, func, true );
	gameLocal.frameCommandThread->Execute();

	return true;
}

/*
================
admWeaponTool::SetModel
================
*/
void admWeaponTool::SetModel( const char *modelname )
{
	assert( modelname );

	if ( modelDefHandle >= 0 )
	{
		gameRenderWorld->RemoveDecals( modelDefHandle );
	}

	renderEntity.hModel = animator.SetModel( modelname );
	if ( renderEntity.hModel )
	{
		renderEntity.customSkin = animator.ModelDef()->GetDefaultSkin();
		animator.GetJoints( &renderEntity.numJoints, &renderEntity.joints );
	}
	else
	{
		renderEntity.customSkin = NULL;
		renderEntity.callback = NULL;
		renderEntity.numJoints = 0;
		renderEntity.joints = NULL;
	}

	// hide the model until an animation is played
	Hide();
}

/*
================
admWeaponTool::GetGlobalJointTransform

This returns the offset and axis of a weapon bone in world space, suitable for attaching models or lights
================
*/
bool admWeaponTool::GetGlobalJointTransform( bool viewModel, const jointHandle_t jointHandle, idVec3 &offset, idMat3 &axis )
{
	if ( viewModel )
	{
// view model
		if ( animator.GetJointTransform( jointHandle, gameLocal.time, offset, axis ) )
		{
			offset = offset * viewToolAxis + viewToolOrigin;
			axis = axis * viewToolAxis;
			return true;
		}
	}
	else
	{
	 // world model
		if ( worldModel.GetEntity() && worldModel.GetEntity()->GetAnimator()->GetJointTransform( jointHandle, gameLocal.time, offset, axis ) )
		{
			offset = worldModel.GetEntity()->GetPhysics()->GetOrigin() + offset * worldModel.GetEntity()->GetPhysics()->GetAxis();
			axis = axis * worldModel.GetEntity()->GetPhysics()->GetAxis();
			return true;
		}
	}
	offset = viewToolOrigin;
	axis =   viewToolAxis;
	return false;
}

/*
================
admWeaponTool::SetPushVelocity
================
*/
void admWeaponTool::SetPushVelocity( const idVec3 &pushVelocity )
{
//	this->pushVelocity = pushVelocity;
}


/***********************************************************************

	State control/player interface

***********************************************************************/

/*
================
admWeaponTool::Think
================
*/
void admWeaponTool::Think( void )
{
// do nothing because the present is called from the player through PresentWeapon
}

/*
================
admWeaponTool::Raise
================
*/
void admWeaponTool::Raise( void )
{
	if ( isLinked )
	{
		TOOL_RAISE = true;
	}
}

/*
================
admWeaponTool::PutAway
================
*/
void admWeaponTool::PutAway( void )
{
	if ( isLinked )
	{
		TOOL_LOWER = true;
	}
}

/*
================
admWeaponTool::LowerWeapon
================
*/
void admWeaponTool::LowerTool( void )
{
	if ( !hide )
	{
		hideStart = 0.0f;
		hideEnd = hideDistance;
		if ( gameLocal.time - hideStartTime < hideTime )
		{
			hideStartTime = gameLocal.time - (hideTime - (gameLocal.time - hideStartTime));
		}
		else
		{
			hideStartTime = gameLocal.time;
		}
		hide = true;
	}
}

/*
================
admWeaponTool::RaiseWeapon
================
*/
void admWeaponTool::RaiseTool( void )
{
	Show();

	if ( hide )
	{
		hideStart = hideDistance;
		hideEnd = 0.0f;
		if ( gameLocal.time - hideStartTime < hideTime )
		{
			hideStartTime = gameLocal.time - (hideTime - (gameLocal.time - hideStartTime));
		}
		else
		{
			hideStartTime = gameLocal.time;
		}
		hide = false;
	}
}

/*
================
admWeaponTool::HideWeapon
================
*/
void admWeaponTool::HideTool( void )
{
	Hide();
	if ( worldModel.GetEntity() )
	{
		worldModel.GetEntity()->Hide();
	}
	muzzleFlashEnd = 0;
}

/*
================
admWeaponTool::ShowWeapon
================
*/
void admWeaponTool::ShowTool( void )
{
	Show();
	if ( worldModel.GetEntity() )
	{
		worldModel.GetEntity()->Show();
	}
	if ( lightOn )
	{
		MuzzleFlashLight();
	}
}

/*
================
admWeaponTool::HideWorldModel
================
*/
void admWeaponTool::HideWorldModel( void )
{
	if ( worldModel.GetEntity() )
	{
		worldModel.GetEntity()->Hide();
	}
}

/*
================
admWeaponTool::ShowWorldModel
================
*/
void admWeaponTool::ShowWorldModel( void )
{
	if ( worldModel.GetEntity() )
	{
		worldModel.GetEntity()->Show();
	}
}

/*
================
admWeaponTool::OwnerDied
================
*/
void admWeaponTool::OwnerDied( void )
{
	if ( isLinked )
	{
		SetState( "OwnerDied", 0 );
		thread->Execute();
	}

	Hide();
	if ( worldModel.GetEntity() )
	{
		worldModel.GetEntity()->Hide();
	}

	// don't clear the weapon immediately since the owner might have killed himself by firing the weapon
	// within the current stack frame
	PostEventMS( &EV_Tool_Clear, 0 );
}

/*
================
admWeaponTool::BeginAttack
================
*/
void admWeaponTool::BeginUse( void )
{
//	if ( status != TL_OUTOFAMMO )
//	{
//		lastAttack = gameLocal.time;
//	}

	if ( !isLinked )
	{
		return;
	}

	if ( !TOOL_ATTACK )
	{
		if ( sndHum )
		{
			StopSound( SND_CHANNEL_BODY, false );
		}
	}
	TOOL_ATTACK = true;
}

/*
================
admWeaponTool::EndAttack
================
*/
void admWeaponTool::EndUse( void )
{
	if ( !TOOL_ATTACK.IsLinked() )
	{
		return;
	}
	if ( TOOL_ATTACK )
	{
		TOOL_ATTACK = false;
		if ( sndHum )
		{
			StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
		}
	}
}

/*
================
admWeaponTool::isReady
================
*/
bool admWeaponTool::IsReady( void ) const
{
	return !hide && !IsHidden() && (status == TL_READY);
}


/*
================
admWeaponTool::IsHolstered
================
*/
bool admWeaponTool::IsHolstered( void ) const
{
	return (status == TL_HOLSTERED);
}

/*
================
admWeaponTool::ShowCrosshair
================
*/
bool admWeaponTool::ShowCrosshair( void ) const
{
	return !(state == idStr( TL_RISING ) || state == idStr( TL_LOWERING ) || state == idStr( TL_HOLSTERED ));
}

/*
=====================
admWeaponTool::CanDrop
=====================
*/
bool admWeaponTool::CanDrop( void ) const
{
	if ( !weaponDef || !worldModel.GetEntity() )
	{
		return false;
	}
	const char *classname = weaponDef->dict.GetString( "def_dropItem" );
	if ( !classname[ 0 ] )
	{
		return false;
	}
	return true;
}

/*
================
admWeaponTool::WeaponStolen
================
*/
void admWeaponTool::ToolStolen( void )
{
	assert( !gameLocal.isClient );
//	if ( projectileEnt )
//	{
//		if ( isLinked )
//		{
//			SetState( "WeaponStolen", 0 );
//			thread->Execute();
//		}
//		projectileEnt = NULL;
//	}

	// set to holstered so we can switch weapons right away
	status = TL_HOLSTERED;

	HideTool();
}

/*
=====================
admWeaponTool::DropItem
=====================
*/
idEntity * admWeaponTool::DropItem( const idVec3 &velocity, int activateDelay, int removeDelay, bool died )
{
	if ( !weaponDef || !worldModel.GetEntity() )
	{
		return NULL;
	}
	if ( !allowDrop )
	{
		return NULL;
	}
	const char *classname = weaponDef->dict.GetString( "def_dropItem" );
	if ( !classname[ 0 ] )
	{
		return NULL;
	}
	StopSound( SND_CHANNEL_BODY, true );
	StopSound( SND_CHANNEL_BODY3, true );

	return idMoveableItem::DropItem( classname, worldModel.GetEntity()->GetPhysics()->GetOrigin(), worldModel.GetEntity()->GetPhysics()->GetAxis(), velocity, activateDelay, removeDelay );
}

/***********************************************************************

	Script state management

***********************************************************************/

/*
=====================
admWeaponTool::SetState
=====================
*/
void admWeaponTool::SetState( const char *statename, int blendFrames )
{
	const function_t *func;

	if ( !isLinked )
	{
		return;
	}

	func = scriptObject.GetFunction( statename );
	if ( !func )
	{
		assert( 0 );
		gameLocal.Error( "Can't find function '%s' in object '%s'", statename, scriptObject.GetTypeName() );
	}

	thread->CallFunction( this, func, true );
	state = statename;

	animBlendFrames = blendFrames;
	if ( g_debugWeapon.GetBool() )
	{
		gameLocal.Printf( "%d: weapon state : %s\n", gameLocal.time, statename );
	}

	idealState = "";
}

/***********************************************************************

	Visual presentation

***********************************************************************/

/*
================
admWeaponTool::MuzzleRise

The machinegun and chaingun will incrementally back up as they are being fired
================
*/
void admWeaponTool::MuzzleRise( idVec3 &origin, idMat3 &axis )
{
//	int			time;
//	float		amount;
//	idAngles	ang;
//	idVec3		offset;
//
//	time = kick_endtime - gameLocal.time;
//	if ( time <= 0 )
//	{
//		return;
//	}
//
//	if ( muzzle_kick_maxtime <= 0 )
//	{
//		return;
//	}
//
//	if ( time > muzzle_kick_maxtime )
//	{
//		time = muzzle_kick_maxtime;
//	}
//
//	amount = (float)time / (float)muzzle_kick_maxtime;
//	ang = muzzle_kick_angles * amount;
//	offset = muzzle_kick_offset * amount;
//
//	origin = origin - axis * offset;
//	axis = ang.ToMat3() * axis;
}

/*
================
admWeaponTool::ConstructScriptObject

Called during idEntity::Spawn.  Calls the constructor on the script object.
Can be overridden by subclasses when a thread doesn't need to be allocated.
================
*/
idThread *admWeaponTool::ConstructScriptObject( void )
{
	const function_t *constructor;

	thread->EndThread();

	// call script object's constructor
	constructor = scriptObject.GetConstructor();
	if ( !constructor )
	{
		gameLocal.Error( "Missing constructor on '%s' for tool", scriptObject.GetTypeName() );
	}

	// init the script object's data
	scriptObject.ClearObject();
	thread->CallFunction( this, constructor, true );
	thread->Execute();

	return thread;
}

/*
================
admWeaponTool::DeconstructScriptObject

Called during idEntity::~idEntity.  Calls the destructor on the script object.
Can be overridden by subclasses when a thread doesn't need to be allocated.
Not called during idGameLocal::MapShutdown.
================
*/
void admWeaponTool::DeconstructScriptObject( void )
{
	const function_t *destructor;

	if ( !thread )
	{
		return;
	}

	// don't bother calling the script object's destructor on map shutdown
	if ( gameLocal.GameState() == GAMESTATE_SHUTDOWN )
	{
		return;
	}

	thread->EndThread();

	// call script object's destructor
	destructor = scriptObject.GetDestructor();
	if ( destructor )
	{
// start a thread that will run immediately and end
		thread->CallFunction( this, destructor, true );
		thread->Execute();
		thread->EndThread();
	}

	// clear out the object's memory
	scriptObject.ClearObject();
}

/*
================
admWeaponTool::UpdateScript
================
*/
void admWeaponTool::UpdateScript( void )
{
	int	count;

	if ( !isLinked )
	{
		return;
	}

	// only update the script on new frames
	if ( !gameLocal.isNewFrame )
	{
		return;
	}

	if ( idealState.Length() )
	{
		SetState( idealState, animBlendFrames );
	}

	// update script state, which may call Event_LaunchProjectiles, among other things
	count = 10;
	while ( (thread->Execute() || idealState.Length()) && count-- )
	{
// happens for weapons with no clip (like grenades)
		if ( idealState.Length() )
		{
			SetState( idealState, animBlendFrames );
		}
	}
}

/*
================
admWeaponTool::AlertMonsters
================
*/
void admWeaponTool::AlertMonsters( void )
{
	trace_t	tr;
	idEntity *ent;
	idVec3 end = muzzleFlash.origin + muzzleFlash.axis * muzzleFlash.target;

	gameLocal.clip.TracePoint( tr, muzzleFlash.origin, end, CONTENTS_OPAQUE | MASK_SHOT_RENDERMODEL | CONTENTS_FLASHLIGHT_TRIGGER, owner );
	if ( g_debugWeapon.GetBool() )
	{
		gameRenderWorld->DebugLine( colorYellow, muzzleFlash.origin, end, 0 );
		gameRenderWorld->DebugArrow( colorGreen, muzzleFlash.origin, tr.endpos, 2, 0 );
	}

	if ( tr.fraction < 1.0f )
	{
		ent = gameLocal.GetTraceEntity( tr );
		if ( ent->IsType( idAI::Type ) )
		{
			static_cast<idAI *>(ent)->TouchedByFlashlight( owner );
		}
		else if ( ent->IsType( idTrigger::Type ) )
		{
			ent->Signal( SIG_TOUCH );
			ent->ProcessEvent( &EV_Touch, owner, &tr );
		}
	}

	// jitter the trace to try to catch cases where a trace down the center doesn't hit the monster
	end += muzzleFlash.axis * muzzleFlash.right * idMath::Sin16( MS2SEC( gameLocal.time ) * 31.34f );
	end += muzzleFlash.axis * muzzleFlash.up * idMath::Sin16( MS2SEC( gameLocal.time ) * 12.17f );
	gameLocal.clip.TracePoint( tr, muzzleFlash.origin, end, CONTENTS_OPAQUE | MASK_SHOT_RENDERMODEL | CONTENTS_FLASHLIGHT_TRIGGER, owner );
	if ( g_debugWeapon.GetBool() )
	{
		gameRenderWorld->DebugLine( colorYellow, muzzleFlash.origin, end, 0 );
		gameRenderWorld->DebugArrow( colorGreen, muzzleFlash.origin, tr.endpos, 2, 0 );
	}

	if ( tr.fraction < 1.0f )
	{
		ent = gameLocal.GetTraceEntity( tr );
		if ( ent->IsType( idAI::Type ) )
		{
			static_cast<idAI *>(ent)->TouchedByFlashlight( owner );
		}
		else if ( ent->IsType( idTrigger::Type ) )
		{
			ent->Signal( SIG_TOUCH );
			ent->ProcessEvent( &EV_Touch, owner, &tr );
		}
	}
}

/*
================
admWeaponTool::PresentWeapon
================
*/
void admWeaponTool::PresentTool( bool showViewModel )
{
	playerViewOrigin = owner->firstPersonViewOrigin;
	playerViewAxis = owner->firstPersonViewAxis;

	// calculate weapon position based on player movement bobbing
	owner->CalculateViewWeaponPos( viewToolOrigin, viewToolAxis );

	// hide offset is for dropping the gun when approaching a GUI or NPC
	// This is simpler to manage than doing the weapon put-away animation
	if ( gameLocal.time - hideStartTime < hideTime )
	{
		float frac = (float)(gameLocal.time - hideStartTime) / (float)hideTime;
		if ( hideStart < hideEnd )
		{
			frac = 1.0f - frac;
			frac = 1.0f - frac * frac;
		}
		else
		{
			frac = frac * frac;
		}
		hideOffset = hideStart + (hideEnd - hideStart) * frac;
	}
	else
	{
		hideOffset = hideEnd;
		if ( hide && disabled )
		{
			Hide();
		}
	}
	viewToolOrigin += hideOffset * viewToolAxis[ 2 ];

	// kick up based on repeat firing
	MuzzleRise( viewToolOrigin, viewToolAxis );

	// set the physics position and orientation
	GetPhysics()->SetOrigin( viewToolOrigin );
	GetPhysics()->SetAxis(   viewToolAxis );
	UpdateVisuals();

	// update the weapon script
	UpdateScript();

	UpdateGUI();

	// update animation
	UpdateAnimation();

	// only show the surface in player view
	renderEntity.allowSurfaceInViewID = owner->entityNumber + 1;

	// crunch the depth range so it never pokes into walls this breaks the machine gun gui
	renderEntity.weaponDepthHack = true;

	// present the model
	if ( showViewModel )
	{
		Present();
	}
	else
	{
		FreeModelDef();
	}

	if ( worldModel.GetEntity() && worldModel.GetEntity()->GetRenderEntity() )
	{
// deal with the third-person visible world model
// don't show shadows of the world model in first person
		if ( gameLocal.isMultiplayer || g_showPlayerShadow.GetBool() || pm_thirdPerson.GetBool() )
		{
			worldModel.GetEntity()->GetRenderEntity()->suppressShadowInViewID = 0;
		}
		else
		{
			worldModel.GetEntity()->GetRenderEntity()->suppressShadowInViewID = owner->entityNumber + 1;
			worldModel.GetEntity()->GetRenderEntity()->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + owner->entityNumber;
		}
	}

	// update the gui light
	if ( guiLight.lightRadius[ 0 ] && guiLightJointView != INVALID_JOINT )
	{
		GetGlobalJointTransform( true, guiLightJointView, guiLight.origin, guiLight.axis );

		if ( (guiLightHandle != -1) )
		{
			gameRenderWorld->UpdateLightDef( guiLightHandle, &guiLight );
		}
		else
		{
			guiLightHandle = gameRenderWorld->AddLightDef( &guiLight );
		}
	}

	if ( status != TL_READY && sndHum )
	{
		StopSound( SND_CHANNEL_BODY, false );
	}

	UpdateSound();
}

/*
================
admWeaponTool::EnterCinematic
================
*/
void admWeaponTool::EnterCinematic( void )
{
	StopSound( SND_CHANNEL_ANY, false );

	if ( isLinked )
	{
		SetState( "EnterCinematic", 0 );
		thread->Execute();

		TOOL_ATTACK = false;
		TOOL_NETFIRING = false;
		TOOL_RAISE = false;
		TOOL_LOWER = false;
	}

	disabled = true;

	LowerTool();
}

/*
================
admWeaponTool::ExitCinematic
================
*/
void admWeaponTool::ExitCinematic( void )
{
	disabled = false;

	if ( isLinked )
	{
		SetState( "ExitCinematic", 0 );
		thread->Execute();
	}

	RaiseTool();
}

/*
================
admWeaponTool::NetCatchup
================
*/
void admWeaponTool::NetCatchup( void )
{
	if ( isLinked )
	{
		SetState( "NetCatchup", 0 );
		thread->Execute();
	}
}

/*
================
admWeaponTool::GetZoomFov
================
*/
int	admWeaponTool::GetZoomFov( void )
{
	return zoomFov;
}

/*
================
admWeaponTool::GetWeaponAngleOffsets
================
*/
void admWeaponTool::GetWeaponAngleOffsets( int *average, float *scale, float *max )
{
	*average =	toolAngleOffsetAverages;
	*scale =	toolAngleOffsetScale;
	*max =		toolAngleOffsetMax;
}

/*
================
admWeaponTool::GetWeaponTimeOffsets
================
*/
void admWeaponTool::GetWeaponTimeOffsets( float *time, float *scale )
{
	*time =  toolOffsetTime;
	*scale = toolOffsetScale;
}

/*
================
admWeaponTool::WriteToSnapshot
================
*/
void admWeaponTool::WriteToSnapshot( idBitMsgDelta &msg ) const
{
	msg.WriteBits( worldModel.GetSpawnId(), 32 );
	msg.WriteBits( lightOn, 1 );
	msg.WriteBits( isFiring ? 1 : 0, 1 );
}

/*
================
admWeaponTool::ReadFromSnapshot
================
*/
void admWeaponTool::ReadFromSnapshot( const idBitMsgDelta &msg )
{
	worldModel.SetSpawnId( msg.ReadBits( 32 ) );
	bool snapLight = msg.ReadBits( 1 ) != 0;
	isFiring = msg.ReadBits( 1 ) != 0;

	// WEAPON_NETFIRING is only turned on for other clients we're predicting. not for local client
	if ( owner && gameLocal.localClientNum != owner->entityNumber && TOOL_NETFIRING.IsLinked() )
	{

// immediately go to the firing state so we don't skip fire animations
		if ( !TOOL_NETFIRING && isFiring )
		{
			idealState = "Fire";
		}

		// immediately switch back to idle
		if ( TOOL_NETFIRING && !isFiring )
		{
			idealState = "Idle";
		}

		TOOL_NETFIRING = isFiring;
	}

//	if ( snapLight != lightOn )
//	{
//		Reload();
//	}
}

/*
================
admWeaponTool::ClientReceiveEvent
================
*/
bool admWeaponTool::ClientReceiveEvent( int event, int time, const idBitMsg &msg )
{

	switch ( event )
	{
	case EVENT_CHANGESKIN: {
		int index = gameLocal.ClientRemapDecl( DECL_SKIN, msg.ReadLong() );
		renderEntity.customSkin = (index != -1) ? static_cast<const idDeclSkin *>(declManager->DeclByIndex( DECL_SKIN, index )) : NULL;
		UpdateVisuals();
		if ( worldModel.GetEntity() )
		{
			worldModel.GetEntity()->SetSkin( renderEntity.customSkin );
		}
		return true;
	}
	default: {
		return idEntity::ClientReceiveEvent( event, time, msg );
	}
	}
	return false;
}

/***********************************************************************

	Script events

***********************************************************************/

/*
===============
admWeaponTool::Event_Clear
===============
*/
void admWeaponTool::Event_Clear( void )
{
	Clear();
}

/*
===============
admWeaponTool::Event_GetOwner
===============
*/
void admWeaponTool::Event_GetOwner( void )
{
	idThread::ReturnEntity( owner );
}

/*
===============
admWeaponTool::Event_WeaponState
===============
*/
void admWeaponTool::Event_ToolState( const char *statename, int blendFrames )
{
	const function_t *func;

	func = scriptObject.GetFunction( statename );
	if ( !func )
	{
		assert( 0 );
		gameLocal.Error( "Can't find function '%s' in object '%s'", statename, scriptObject.GetTypeName() );
	}

	idealState = statename;

	if ( !idealState.Icmp( "Fire" ) )
	{
		isFiring = true;
	}
	else
	{
		isFiring = false;
	}

	animBlendFrames = blendFrames;
	thread->DoneProcessing();
}

/*
===============
admWeaponTool::Event_WeaponReady
===============
*/
void admWeaponTool::Event_ToolReady( void )
{
	status = TL_READY;
	if ( isLinked )
	{
		TOOL_RAISE = false;
	}
	if ( sndHum )
	{
		StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
	}

}

/*
===============
admWeaponTool::Event_WeaponHolstered
===============
*/
void admWeaponTool::Event_ToolHolstered( void )
{
	status = TL_HOLSTERED;
	if ( isLinked )
	{
		TOOL_LOWER = false;
	}
}

/*
===============
admWeaponTool::Event_WeaponRising
===============
*/
void admWeaponTool::Event_ToolRising( void )
{
	status = TL_RISING;
	if ( isLinked )
	{
		TOOL_LOWER = false;
	}
	owner->WeaponRisingCallback();
}

/*
===============
admWeaponTool::Event_WeaponLowering
===============
*/
void admWeaponTool::Event_ToolLowering( void )
{
	status = TL_LOWERING;
	if ( isLinked )
	{
		TOOL_RAISE = false;
	}
	owner->WeaponLoweringCallback();
}

/*
===============
admWeaponTool::Event_PlayAnim
===============
*/
void admWeaponTool::Event_PlayAnim( int channel, const char *animname )
{
	int anim;

	anim = animator.GetAnim( animname );
	if ( !anim )
	{
		gameLocal.Warning( "missing '%s' animation on '%s' (%s)", animname, name.c_str(), GetEntityDefName() );
		animator.Clear( channel, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = 0;
	}
	else
	{
		if ( !(owner && owner->GetInfluenceLevel()) )
		{
			Show();
		}
		animator.PlayAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = animator.CurrentAnim( channel )->GetEndTime();
		if ( worldModel.GetEntity() )
		{
			anim = worldModel.GetEntity()->GetAnimator()->GetAnim( animname );
			if ( anim )
			{
				worldModel.GetEntity()->GetAnimator()->PlayAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
			}
		}
	}
	animBlendFrames = 0;
	idThread::ReturnInt( 0 );
}

/*
===============
admWeaponTool::Event_PlayCycle
===============
*/
void admWeaponTool::Event_PlayCycle( int channel, const char *animname )
{
	int anim;

	anim = animator.GetAnim( animname );
	if ( !anim )
	{
		gameLocal.Warning( "missing '%s' animation on '%s' (%s)", animname, name.c_str(), GetEntityDefName() );
		animator.Clear( channel, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = 0;
	}
	else
	{
		if ( !(owner && owner->GetInfluenceLevel()) )
		{
			Show();
		}
		animator.CycleAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = animator.CurrentAnim( channel )->GetEndTime();
		if ( worldModel.GetEntity() )
		{
			anim = worldModel.GetEntity()->GetAnimator()->GetAnim( animname );
			worldModel.GetEntity()->GetAnimator()->CycleAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		}
	}
	animBlendFrames = 0;
	idThread::ReturnInt( 0 );
}

/*
===============
admWeaponTool::Event_AnimDone
===============
*/
void admWeaponTool::Event_AnimDone( int channel, int blendFrames )
{
	if ( animDoneTime - FRAME2MS( blendFrames ) <= gameLocal.time )
	{
		idThread::ReturnInt( true );
	}
	else
	{
		idThread::ReturnInt( false );
	}
}

/*
===============
admWeaponTool::Event_SetBlendFrames
===============
*/
void admWeaponTool::Event_SetBlendFrames( int channel, int blendFrames )
{
	animBlendFrames = blendFrames;
}

/*
===============
admWeaponTool::Event_GetBlendFrames
===============
*/
void admWeaponTool::Event_GetBlendFrames( int channel )
{
	idThread::ReturnInt( animBlendFrames );
}

/*
================
admWeaponTool::Event_Next
================
*/
void admWeaponTool::Event_Next( void )
{
// change to another weapon if possible
	owner->NextBestWeapon();
}

/*
================
admWeaponTool::Event_SetSkin
================
*/
void admWeaponTool::Event_SetSkin( const char *skinname )
{
	const idDeclSkin *skinDecl;

	if ( !skinname || !skinname[ 0 ] )
	{
		skinDecl = NULL;
	}
	else
	{
		skinDecl = declManager->FindSkin( skinname );
	}

	renderEntity.customSkin = skinDecl;
	UpdateVisuals();

	if ( worldModel.GetEntity() )
	{
		worldModel.GetEntity()->SetSkin( skinDecl );
	}

	if ( gameLocal.isServer )
	{
		idBitMsg	msg;
		byte		msgBuf[ MAX_EVENT_PARAM_SIZE ];

		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteLong( (skinDecl != NULL) ? gameLocal.ServerRemapDecl( -1, DECL_SKIN, skinDecl->Index() ) : -1 );
		ServerSendEvent( EVENT_CHANGESKIN, &msg, false, -1 );
	}
}

/*
================
admWeaponTool::Event_Flashlight
================
*/
void admWeaponTool::Event_Flashlight( int enable )
{
	if ( enable )
	{
		lightOn = true;
		MuzzleFlashLight();
	}
	else
	{
		lightOn = false;
		muzzleFlashEnd = 0;
	}
}

/*
================
admWeaponTool::Event_GetLightParm
================
*/
void admWeaponTool::Event_GetLightParm( int parmnum )
{
	if ( (parmnum < 0) || (parmnum >= MAX_ENTITY_SHADER_PARMS) )
	{
		gameLocal.Error( "shader parm index (%d) out of range", parmnum );
	}

	idThread::ReturnFloat( muzzleFlash.shaderParms[ parmnum ] );
}

/*
================
admWeaponTool::Event_SetLightParm
================
*/
void admWeaponTool::Event_SetLightParm( int parmnum, float value )
{
	if ( (parmnum < 0) || (parmnum >= MAX_ENTITY_SHADER_PARMS) )
	{
		gameLocal.Error( "shader parm index (%d) out of range", parmnum );
	}

	muzzleFlash.shaderParms[ parmnum ] = value;
	worldMuzzleFlash.shaderParms[ parmnum ] = value;
	UpdateVisuals();
}

/*
================
admWeaponTool::Event_SetLightParms
================
*/
void admWeaponTool::Event_SetLightParms( float parm0, float parm1, float parm2, float parm3 )
{
	muzzleFlash.shaderParms[ SHADERPARM_RED ] = parm0;
	muzzleFlash.shaderParms[ SHADERPARM_GREEN ] = parm1;
	muzzleFlash.shaderParms[ SHADERPARM_BLUE ] = parm2;
	muzzleFlash.shaderParms[ SHADERPARM_ALPHA ] = parm3;

	worldMuzzleFlash.shaderParms[ SHADERPARM_RED ] = parm0;
	worldMuzzleFlash.shaderParms[ SHADERPARM_GREEN ] = parm1;
	worldMuzzleFlash.shaderParms[ SHADERPARM_BLUE ] = parm2;
	worldMuzzleFlash.shaderParms[ SHADERPARM_ALPHA ] = parm3;

	UpdateVisuals();
}

/*
=====================
admWeaponTool::Event_Melee
=====================
*/
void admWeaponTool::Event_Use( void )
{
	idEntity	*ent;
	trace_t		tr;

/*	if ( !meleeDef )
	{
		gameLocal.Error( "No meleeDef on '%s'", weaponDef->dict.GetString( "classname" ) );
	}*/

	if ( !gameLocal.isClient )
	{
		idVec3 start = playerViewOrigin;
		idVec3 end = start + playerViewAxis[ 0 ] * (useDistance * owner->PowerUpModifier( MELEE_DISTANCE ));
		gameLocal.clip.TracePoint( tr, start, end, MASK_SHOT_RENDERMODEL, owner );
		if ( tr.fraction < 1.0f )
		{
			ent = gameLocal.GetTraceEntity( tr );
		}
		else
		{
			ent = NULL;
		}

		if ( g_debugWeapon.GetBool() )
		{
			gameRenderWorld->DebugLine( colorYellow, start, end, 100 );
			if ( ent )
			{
				gameRenderWorld->DebugBounds( colorRed, ent->GetPhysics()->GetBounds(), ent->GetPhysics()->GetOrigin(), 100 );
			}
		}

		bool hit = false;
		const char *hitSound = weaponDef->dict.GetString( "snd_miss" );

		if ( ent )
		{

//			float push = meleeDef->dict.GetFloat( "push" );
//			idVec3 impulse = -push * owner->PowerUpModifier( SPEED ) * tr.c.normal;

			if ( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) && (ent->IsType( idActor::Type ) || ent->IsType( idAFAttachment::Type )) )
			{
				idThread::ReturnInt( 0 );
				return;
			}

//			ent->ApplyImpulse( this, tr.c.id, tr.c.point, impulse );

			// weapon stealing - do this before damaging so weapons are not dropped twice
			if ( gameLocal.isMultiplayer
				 && weaponDef && weaponDef->dict.GetBool( "stealing" )
				 && ent->IsType( idPlayer::Type )
				 && !owner->PowerUpActive( BERSERK )
				 && (gameLocal.gameType != GAME_TDM || gameLocal.serverInfo.GetBool( "si_teamDamage" ) || (owner->team != static_cast<idPlayer *>(ent)->team))
				 )
			{
				owner->StealWeapon( static_cast<idPlayer *>(ent) );
			}

//			if ( ent->fl.takedamage )
//			{
//				idVec3 kickDir, globalKickDir;
//				meleeDef->dict.GetVector( "kickDir", "0 0 0", kickDir );
//				globalKickDir = muzzleAxis * kickDir;
//				ent->Damage( owner, owner, globalKickDir, meleeDefName, owner->PowerUpModifier( MELEE_DAMAGE ), tr.c.id );
//				hit = true;
//			}

			if ( weaponDef->dict.GetBool( "impact_damage_effect" ) )
			{

				if ( ent->spawnArgs.GetBool( "bleed" ) )
				{

//					hitSound = meleeDef->dict.GetString( owner->PowerUpActive( BERSERK ) ? "snd_hit_berserk" : "snd_hit" );

//					ent->AddDamageEffect( tr, impulse, meleeDef->dict.GetString( "classname" ) );

				}
				else
				{

					int type = tr.c.material->GetSurfaceType();
					if ( type == SURFTYPE_NONE )
					{
						type = GetDefaultSurfaceType();
					}

					const char *materialType = gameLocal.sufaceTypeNames[ type ];

					// start impact sound based on material type
					hitSound = weaponDef->dict.GetString( va( "snd_%s", materialType ) );
					if ( *hitSound == '\0' )
					{
						hitSound = weaponDef->dict.GetString( "snd_metal" );
					}

//					if ( gameLocal.time > nextStrikeFx )
//					{
//						const char *decal;
//						// project decal
//						decal = weaponDef->dict.GetString( "mtr_strike" );
//						if ( decal && *decal )
//						{
//							gameLocal.ProjectDecal( tr.c.point, -tr.c.normal, 8.0f, true, 6.0, decal );
//						}
//						nextStrikeFx = gameLocal.time + 200;
//					}
//					else
//					{
						hitSound = "";
//					}

//					strikeSmokeStartTime = gameLocal.time;
//					strikePos = tr.c.point;
//					strikeAxis = -tr.endAxis;
				}
			}
		}

		if ( *hitSound != '\0' )
		{
			const idSoundShader *snd = declManager->FindSound( hitSound );
			StartSoundShader( snd, SND_CHANNEL_BODY2, 0, true, NULL );
		}

		idThread::ReturnInt( hit );
		owner->WeaponFireFeedback( &weaponDef->dict );
		return;
	}

	idThread::ReturnInt( 0 );
	owner->WeaponFireFeedback( &weaponDef->dict );
}

/*
=====================
admWeaponTool::Event_GetWorldModel
=====================
*/
void admWeaponTool::Event_GetWorldModel( void )
{
	idThread::ReturnEntity( worldModel.GetEntity() );
}

/*
=====================
admWeaponTool::Event_AllowDrop
=====================
*/
void admWeaponTool::Event_AllowDrop( int allow )
{
	if ( allow )
	{
		allowDrop = true;
	}
	else
	{
		allowDrop = false;
	}
}

/*
===============
admWeaponTool::Event_IsInvisible
===============
*/
void admWeaponTool::Event_IsInvisible( void )
{
	if ( !owner )
	{
		idThread::ReturnFloat( 0 );
		return;
	}
	idThread::ReturnFloat( owner->PowerUpActive( INVISIBILITY ) ? 1 : 0 );
}

/*
===============
admWeaponTool::ClientPredictionThink
===============
*/
void admWeaponTool::ClientPredictionThink( void )
{
	UpdateAnimation();
}













