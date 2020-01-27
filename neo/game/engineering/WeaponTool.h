#pragma once

/*
===============================================================================

	Player Tool - special type of weapon

===============================================================================
*/

typedef enum
{
	TL_READY,
	TL_HOLSTERED,
	TL_RISING,
	TL_LOWERING
} toolStatus_t;

class idPlayer;
class idMoveableItem;

class admWeaponTool : public idAnimatedEntity
{
	CLASS_PROTOTYPE( admWeaponTool );

							admWeaponTool();
	virtual					~admWeaponTool();

	// Init
	void					Spawn( void );
	void					SetOwner( idPlayer *owner );
	idPlayer*				GetOwner( void );

	static void				CacheTool( const char *toolName );

	// save games
	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	// tool definition management
	void					Clear( void );
	void					GetToolDef( const char *objectname );
	bool					IsLinked( void );
	bool					IsWorldModelReady( void );

	// GUIs
	const char *			Icon( void ) const;
	void					UpdateGUI( void );

	virtual void			SetModel( const char *modelname );
	bool					GetGlobalJointTransform( bool viewModel, const jointHandle_t jointHandle, idVec3 &offset, idMat3 &axis );
	void					SetPushVelocity( const idVec3 &pushVelocity );
	bool					UpdateSkin( void );

	// State control/player interface
	void					Think( void );
	void					Raise( void );
	void					PutAway( void );
//	void					Reload( void );
	void					LowerTool( void );
	void					RaiseTool( void );
	void					HideTool( void );
	void					ShowTool( void );
	void					HideWorldModel( void );
	void					ShowWorldModel( void );
	void					OwnerDied( void );
	void					BeginUse( void );
	void					EndUse( void );
	bool					IsReady( void ) const;
	bool					IsHolstered( void ) const;
	bool					ShowCrosshair( void ) const;
	idEntity *				DropItem( const idVec3 &velocity, int activateDelay, int removeDelay, bool died );
	bool					CanDrop( void ) const;
	void					ToolStolen( void );

	// Script state management
	virtual idThread *		ConstructScriptObject( void );
	virtual void			DeconstructScriptObject( void );
	void					SetState( const char *statename, int blendFrames );
	void					UpdateScript( void );
	void					EnterCinematic( void );
	void					ExitCinematic( void );
	void					NetCatchup( void );

	// Visual presentation
	void					PresentTool( bool showViewModel );
	int						GetZoomFov( void );
	void					GetWeaponAngleOffsets( int *average, float *scale, float *max );
	void					GetWeaponTimeOffsets( float *time, float *scale );
//	bool					BloodSplat( float size );

	// Ammo
//	static ammo_t			GetAmmoNumForName( const char *ammoname );
//	static const char		*GetAmmoNameForNum( ammo_t ammonum );
//	static const char		*GetAmmoPickupNameForNum( ammo_t ammonum );
//	ammo_t					GetAmmoType( void ) const;
//	int						AmmoAvailable( void ) const;
//	int						AmmoInClip( void ) const;
//	void					ResetAmmoClip( void );
//	int						ClipSize( void ) const;
//	int						LowAmmo( void ) const;
//	int						AmmoRequired( void ) const;

	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );

	enum
	{
		EVENT_CHANGESKIN = idEntity::EVENT_MAXEVENTS,
		EVENT_MAXEVENTS
	};
	virtual bool			ClientReceiveEvent( int event, int time, const idBitMsg &msg );

	virtual void			ClientPredictionThink( void );

private:
	// script control
	idScriptBool			TOOL_ATTACK;
	idScriptBool			TOOL_NETFIRING;
	idScriptBool			TOOL_RAISE;
	idScriptBool			TOOL_LOWER;
	toolStatus_t			status;
	idThread *				thread;
	idStr					state;
	idStr					idealState;
	int						animBlendFrames;
	int						animDoneTime;
	bool					isLinked;

	// precreated projectile
//	idEntity				*projectileEnt;

	idPlayer *				owner;
	idEntityPtr<idAnimatedEntity>	worldModel;

	// hiding (for GUIs and NPCs)
	int						hideTime;
	float					hideDistance;
	int						hideStartTime;
	float					hideStart;
	float					hideEnd;
	float					hideOffset;
	bool					hide;
	bool					disabled;

	// berserk
	int						berserk;

	// these are the player render view parms, which include bobbing
	idVec3					playerViewOrigin;
	idMat3					playerViewAxis;

	// the view weapon render entity parms
	idVec3					viewToolOrigin;
	idMat3					viewToolAxis;

	// the muzzle bone's position, used for launching projectiles and trailing smoke
//	idVec3					muzzleOrigin;
//	idMat3					muzzleAxis;

	// weapon definition
	// we maintain local copies of the projectile and brass dictionaries so they
	// do not have to be copied across the DLL boundary when entities are spawned
	const idDeclEntityDef *	weaponDef;
	float					useDistance;
	idStr					icon;
//	idDict					projectileDict;
//	float					meleeDistance;
//	idStr					meleeDefName;
//	idDict					brassDict;
//	int						brassDelay;

	// view weapon gui light
	renderLight_t			guiLight;
	int						guiLightHandle;

	// muzzle flash
	renderLight_t			muzzleFlash;		// positioned on view weapon bone
	int						muzzleFlashHandle;
	renderLight_t			worldMuzzleFlash;	// positioned on world weapon bone
	int						worldMuzzleFlashHandle;

	idVec3					flashColor;
	int						muzzleFlashEnd;
	int						flashTime;
	bool					lightOn;
	bool					silent_fire;
	bool					allowDrop;

//	// effects
//	bool					hasBloodSplat;
//
//	// weapon kick
//	int						kick_endtime;
//	int						muzzle_kick_time;
//	int						muzzle_kick_maxtime;
//	idAngles				muzzle_kick_angles;
//	idVec3					muzzle_kick_offset;
//
//	// ammo management
//	ammo_t					ammoType;
//	int						ammoRequired;		// amount of ammo to use each shot.  0 means weapon doesn't need ammo.
//	int						clipSize;			// 0 means no reload
//	int						ammoClip;
//	int						lowAmmo;			// if ammo in clip hits this threshold, snd_
//	bool					powerAmmo;			// true if the clip reduction is a factor of the power setting when
												// a projectile is launched
	// mp client
	bool					isFiring;

	// zoom
	int						zoomFov;			// variable zoom fov per weapon

	// joints from models
//	jointHandle_t			barrelJointView;
//	jointHandle_t			barrelJointWorld;
//	jointHandle_t			ejectJointView;
//	jointHandle_t			ejectJointWorld;
//	jointHandle_t			ventLightJointView;

	jointHandle_t			flashJointView;
	jointHandle_t			flashJointWorld;
	jointHandle_t			guiLightJointView;

	// sound
	const idSoundShader *	sndHum;

	// new style muzzle smokes
//	const idDeclParticle *	weaponSmoke;			// null if it doesn't smoke
//	int						weaponSmokeStartTime;	// set to gameLocal.time every weapon fire
//	bool					continuousSmoke;		// if smoke is continuous ( chainsaw )
//	const idDeclParticle *  strikeSmoke;			// striking something in melee
//	int						strikeSmokeStartTime;	// timing
//	idVec3					strikePos;				// position of last melee strike
//	idMat3					strikeAxis;				// axis of last melee strike
//	int						nextStrikeFx;			// used for sound and decal ( may use for strike smoke too )

	// nozzle effects
	int						lastAttack;			// last time an attack occured
//	bool					nozzleFx;			// does this use nozzle effects ( parm5 at rest, parm6 firing )
//										// this also assumes a nozzle light atm
//	int						nozzleFxFade;		// time it takes to fade between the effects
//	renderLight_t			nozzleGlow;			// nozzle light
//	int						nozzleGlowHandle;	// handle for nozzle light
//
//	idVec3					nozzleGlowColor;	// color of the nozzle glow
//	const idMaterial *		nozzleGlowShader;	// shader for glow light
//	float					nozzleGlowRadius;	// radius of glow light

	// weighting for viewmodel angles
	int						toolAngleOffsetAverages;
	float					toolAngleOffsetScale;
	float					toolAngleOffsetMax;
	float					toolOffsetTime;
	float					toolOffsetScale;

	// flashlight
	void					AlertMonsters( void );

	// Visual presentation
	void					InitWorldModel( const idDeclEntityDef *def );
	void					MuzzleRise( idVec3 &origin, idMat3 &axis );
	void					MuzzleFlashLight( void );
//	void					UpdateNozzleFx( void );
	void					UpdateFlashPosition( void );

	// script events
	void					Event_Clear( void );
	void					Event_GetOwner( void );
	void					Event_ToolState( const char *statename, int blendFrames );
	void					Event_ToolReady( void );
	void					Event_ToolHolstered( void );
	void					Event_ToolRising( void );
	void					Event_ToolLowering( void );
	virtual void			Event_Use( void );
	void					Event_PlayAnim( int channel, const char *animname );
	void					Event_PlayCycle( int channel, const char *animname );
	void					Event_AnimDone( int channel, int blendFrames );
	void					Event_SetBlendFrames( int channel, int blendFrames );
	void					Event_GetBlendFrames( int channel );
	void					Event_Next( void );
	void					Event_SetSkin( const char *skinname );
	void					Event_Flashlight( int enable );
	void					Event_GetWorldModel( void );
	void					Event_AllowDrop( int allow );
	void					Event_IsInvisible( void );
	void					Event_GetLightParm( int parmnum );
	void					Event_SetLightParm( int parmnum, float value );
	void					Event_SetLightParms( float parm0, float parm1, float parm2, float parm3 );
//	void					Event_ToolReloading( void );
//	void					Event_ToolOutOfAmmo( void );
//	void					Event_TotalAmmoCount( void );
//	void					Event_UseAmmo( int amount );
//	void					Event_AddToClip( int amount );
//	void					Event_AmmoInClip( void );
//	void					Event_AmmoAvailable( void );
//	void					Event_LaunchProjectiles( int num_projectiles, float spread, float fuseOffset, float launchPower, float dmgPower );
//	void					Event_CreateProjectile( void );
//	void					Event_AutoReload( void );
//	void					Event_NetEndReload( void );
//	void					Event_NetReload( void );
};

ID_INLINE bool admWeaponTool::IsLinked( void )
{
	return isLinked;
}

ID_INLINE bool admWeaponTool::IsWorldModelReady( void )
{
	return (worldModel.GetEntity() != NULL);
}

ID_INLINE idPlayer* admWeaponTool::GetOwner( void )
{
	return owner;
}
