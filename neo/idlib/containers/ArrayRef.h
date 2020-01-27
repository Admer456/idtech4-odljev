/*
===========================================================================

fhDOOM GPL Source Code
Copyright (C) 2018 Johannes Ohlemacher

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

#pragma once

/*
===============================================================================

Non-owning reference to an array.

===============================================================================
*/
template<typename T>
class fhArrayRef {
public:
	fhArrayRef()
		: ptr( nullptr )
		, size( 0 )
	{}

	fhArrayRef( T* ptr, int size )
		: ptr( ptr )
		, size( size )
	{}

	template<typename S>
	fhArrayRef( const fhArrayRef<S>& s )
		: ptr( s.IsEmpty() ? nullptr : s.begin() )
		, size( s.Num() )
	{}

	~fhArrayRef() = default;

	bool IsNull() const { return ptr == nullptr; }
	bool IsEmpty() const { return size == 0; }
	int Num() const { return size; }

	void Set( T* ptr, int size ) {
		assert( ptr || size == 0 );
		assert( size >= 0 );
		this->ptr = ptr;
		this->size = size;
	}

	explicit operator bool() const { return !IsNull(); }

	template<typename S>
	fhArrayRef operator=(const fhArrayRef<S>& a)
	{
		this->ptr = a.IsNull() ? nullptr : a.begin();
		this->size = a.Num();
		return *this;
	}

	T& operator[]( int index ) {
		assert( !IsNull() );
		assert( index < Num() );
		assert( index >= 0 );
		return ptr[index];
	}

	const T& operator[]( int index ) const {
		assert( !IsNull() );
		assert( index < Num() );
		assert( index >= 0 );
		return ptr[index];
	}

	using iterator = T*;
	using const_iterator = const T*;
	iterator begin() { assert( ptr );  return ptr; }
	iterator end() { assert( ptr );  return ptr + size; }
	const_iterator cbegin() const { assert( ptr );  return ptr; }
	const_iterator cend() const { assert( ptr );  return ptr + size; }
	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }

private:
	T* ptr;
	int size;
};
