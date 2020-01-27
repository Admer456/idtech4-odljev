#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "Electro_Base.h"
#include "Electro_Socket.h"
#include "Electro_Plug.h"

CLASS_DECLARATION( admElectroBase, admElectroPlug )
END_CLASS

void admElectroPlug::SpawnCustom()
{

}

void admElectroPlug::Think()
{
	admElectroBase::Think();
	
	if ( !GetBindMaster() &&  voltageSource.GetEntity() )
	{
		Bind( voltageSource.GetEntity(), true );
		OnUnuse( user.GetEntity() );
		return;
	}
	else if ( GetBindMaster() && !voltageSource.GetEntity() )
	{
		Unbind();
		return;
	}

	CheckForSockets();
}

void admElectroPlug::CheckForSockets()
{
	trace_t trace;
	idVec3 start, end;
	idEntity *socketEnt;

	start = GetPhysics()->GetOrigin();
	end = start + GetPhysics()->GetAxis().ToAngles().ToForward() * 8.0f;
	gameLocal.clip.TracePoint( trace, start, end, MASK_ALL, this );

	socketEnt = gameLocal.entities[ trace.c.entityNum ];

	if ( trace.fraction < 1.0f )
	{
		if ( socketEnt && socketEnt->IsType( admElectroSocket::Type ) )
		{
			voltageSource = socketEnt;
		}
	}
}

void admElectroPlug::OnUse( idPlayer *player )
{
	player->isUsing = true;
	player->isUsingDragEntity = true;
	user = player;
}

void admElectroPlug::OnUnuse( idPlayer *player )
{
	if ( user.GetEntity() )
	{
		player->isUsing = false;
		player->isUsingDragEntity = false;
		user = nullptr;
	}
}
