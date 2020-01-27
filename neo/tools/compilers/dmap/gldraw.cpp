/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../../../idlib/precompiled.h"
#pragma hdrstop

#include "dmap.h"

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define	WIN_SIZE	1024

void Draw_ClearWindow( void ) {

	if ( !dmapGlobals.drawflag ) {
		return;
	}

	glDrawBuffer( GL_FRONT );

	RB_SetGL2D();

	glClearColor( 0.5, 0.5, 0.5, 0 );
	glClear( GL_COLOR_BUFFER_BIT );

  GL_ProjectionMatrix.LoadIdentity();
	GL_ProjectionMatrix.Ortho( dmapGlobals.drawBounds[0][0], dmapGlobals.drawBounds[1][0],
		dmapGlobals.drawBounds[0][1], dmapGlobals.drawBounds[1][1],
		-1, 1 );
  GL_ModelViewMatrix.LoadIdentity();

	glColor3f (0,0,0);
//	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glDisable (GL_DEPTH_TEST);
//	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glFlush ();

}


//============================================================

#define	GLSERV_PORT	25001

bool	wins_init;
int			draw_socket;

void GLS_BeginScene (void)
{
	WSADATA	winsockdata;
	WORD	wVersionRequested;
	struct sockaddr_in	address;
	int		r;

	if (!wins_init)
	{
		wins_init = true;

		wVersionRequested = MAKEWORD(1, 1);

		r = WSAStartup (MAKEWORD(1, 1), &winsockdata);

		if (r)
			common->Error( "Winsock initialization failed.");

	}

	// connect a socket to the server

	draw_socket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (draw_socket == -1)
		common->Error( "draw_socket failed");

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port = GLSERV_PORT;
	r = connect (draw_socket, (struct sockaddr *)&address, sizeof(address));
	if (r == -1)
	{
		closesocket (draw_socket);
		draw_socket = 0;
	}
}

void GLS_Winding( const idWinding *w, int code )
{
	byte	buf[1024];
	int		i, j;

	if (!draw_socket)
		return;

	((int *)buf)[0] = w->GetNumPoints();
	((int *)buf)[1] = code;
	for ( i = 0; i < w->GetNumPoints(); i++ )
		for (j=0 ; j<3 ; j++)
			((float *)buf)[2+i*3+j] = (*w)[i][j];

	send (draw_socket, (const char *)buf, w->GetNumPoints() * 12 + 8, 0);
}

void GLS_Triangle( const mapTri_t *tri, int code ) {
	idWinding w;

	w.SetNumPoints( 3 );
	VectorCopy( tri->v[0].xyz, w[0] );
	VectorCopy( tri->v[1].xyz, w[1] );
	VectorCopy( tri->v[2].xyz, w[2] );
	GLS_Winding( &w, code );
}

void GLS_EndScene (void)
{
	closesocket (draw_socket);
	draw_socket = 0;
}

#endif


#ifdef __linux__
void Draw_ClearWindow() {}
void GLS_BeginScene() {}
void GLS_Winding( const idWinding *, int ) {}
void GLS_EndScene() {}
#endif