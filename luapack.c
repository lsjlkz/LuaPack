//
// Created by lsjlkz on 2023/5/17.
//

#include "luapack.h"
#include <stdlib.h>

static int stackDeep = 0;


#define WRITE_BUF_EMPTY(_io) ((MAX_BUF_SIZE)-(_io->write_size))
#define READ_BUF_EMPTY(_io) ((_io->write_size)-(_io->read_size))

#define WRITE_BUF_ENOUGH_ENPTY(_io, _T) ((WRITE_BUF_EMPTY(_io))>(sizeof(_T)))
#define READ_BUF_ENOUGH_ENPTY(_io, _T) ((READ_BUF_EMPTY(_io))>=(sizeof(_T)))


#define LUA_ERROR(_L, _b, _msg) \
	if(!(_b)){ \
		luaL_error(_L, _msg); \
	}

#define WRITE_BUF_CHECK_OVERFLOW(_L, _io, _Size) LUA_ERROR(_L, (WRITE_BUF_EMPTY((_io)) >= (_Size)), "err pack overflow")
#define READ_BUF_CHECK_OVERFLOW(_L, _io, _Size) LUA_ERROR(_L, (READ_BUF_EMPTY((_io)) >= (_Size)), "err unpack overflow")


#define PACK_BYTES(_io, _Val) \
	memcpy((_io->buf + _io->write_size), &_Val, (sizeof(_Val)));\
	_io->write_size += sizeof(_Val);

#define UNPACK_BYTES(L, _io, _T) \
	READ_BUF_CHECK_OVERFLOW(L, io, sizeof(_T)); \
	_T value = 0; \
	memcpy(&value, (_io->buf + _io->read_size), sizeof(_T)); \
	_io->read_size += sizeof(_T)


LUA_API int pack_arg(lua_State* L){
	stackDeep = 0;
	LuaPackBuf *io = (LuaPackBuf*)LUA_CHECK_BUF_TYPE(L);
	io->write_size = 0;
	if(!LUA_CHECK_TYPE_CAN_PACK(L)){
		return 0;
	}
	pack_arg_help(L, io, -1);
	return 1;
}

LUA_API int unpack_to_arg(lua_State* L) {
	LuaPackBuf* io = (LuaPackBuf*)LUA_CHECK_BUF_TYPE(L);
	io->read_size = 0;
	unpack_to_arg_help(L, io);
	return 1;
}

LUA_API int print(lua_State* L) {
	LuaPackBuf* io = (LuaPackBuf*)LUA_CHECK_BUF_TYPE(L);
	printf("print start...\n");
	for (int i = 0; i < io->write_size; i++) {
		printf("%d ", io->buf[i]);
	}
	printf("\nprint end...\n");
	return 1;
}


LUA_API int new_pack(lua_State* L) {
	LuaPackBuf* io = (LuaPackBuf * )lua_newuserdata(L, sizeof(LuaPackBuf));
	io->write_size = 0;

	luaL_getmetatable(L, LUA_PACKAGE_META);
	lua_setmetatable(L, -2);
	return 1;
}




LUA_API int obj_len(lua_State* L) {
	LuaPackBuf* io = (LuaPackBuf*)LUA_CHECK_BUF_TYPE(L);
	lua_pushinteger(L, io->write_size);
	return 1;
}


void pack_type(LuaPackBuf* io, char value) {
	PACK_BYTES(io, value);
}

void pack_long_long(LuaPackBuf* io, LONG_LONG value) {
	PACK_BYTES(io, value);
}

void pack_long(LuaPackBuf* io, LONG value) {
	PACK_BYTES(io, value);
}

void pack_short(LuaPackBuf* io, SHORT value) {
	PACK_BYTES(io, value);
}


void pack_int(LuaPackBuf* io, INT value) {
	PACK_BYTES(io, value);
}

void pack_string(LuaPackBuf* io, const char* s, size_t size) {
	// has done size check
	U_SHORT usize = size;

	PACK_BYTES(io, usize);
	
	memcpy(io->buf + io->write_size, s, size);
	io->write_size += size;
}

TABLE_SIZE_TYPE* pack_size(LuaPackBuf* io, TABLE_SIZE_TYPE size) {
	TABLE_SIZE_TYPE* p = io->buf + io->write_size;
	*p = size;
	io->write_size += sizeof(TABLE_SIZE_TYPE);
	return p;
}

void pack_number_help(lua_State* L, LuaPackBuf* io, LONG_LONG value) {

	WRITE_BUF_CHECK_OVERFLOW(L, io, sizeof(LUA_TYPE_FLAG));
	LUA_TYPE_FLAG t = 0;
	if (value >= SHRT_MIN && value <= SHRT_MAX) {
		t = SHORT_FLAG;
	}
	else if (value >= INT_MIN && value <= INT_MAX) {
		t = INT_FLAG;
	}
	else if (value >= LONG_MIN && value <= LONG_MAX) {
		t = LONG_FLAG;
	}
	else if (value >= LLONG_MIN && value <= LLONG_MAX) {
		t = LONG_LONG_FLAG;
	}
	else {
		LUA_ERROR(L, 0, "number too long");
	}
	switch (t)
	{
	case(SHORT_FLAG): {
		pack_type(io, SHORT_FLAG);
		WRITE_BUF_CHECK_OVERFLOW(L, io, sizeof(SHORT));

		pack_short(io, value);
		break;
	}
	case(INT_FLAG): {
		pack_type(io, INT_FLAG);
		WRITE_BUF_CHECK_OVERFLOW(L, io, sizeof(INT));

		pack_int(io, value);
		break;
	}
	case(LONG_FLAG): {
		pack_type(io, LONG_FLAG);
		WRITE_BUF_CHECK_OVERFLOW(L, io, sizeof(LONG));

		pack_long(io, value);
		break;
	}
	case(LONG_LONG_FLAG): {
		pack_type(io, LONG_LONG_FLAG);
		WRITE_BUF_CHECK_OVERFLOW(L, io, sizeof(LONG_LONG));
		pack_long_long(io, value);
		break;
	}
	default:
		LUA_ERROR(L, 0, "number unknown type");
	}
}


int pack_arg_help(lua_State* L, LuaPackBuf* io, int index) {
	stackDeep++;
	int top = lua_gettop(L);
	int lt = lua_type(L, index);
	switch (lt) {
	case(LUA_TNIL):
	{
		WRITE_BUF_CHECK_OVERFLOW(L, io, sizeof(LUA_TYPE_FLAG));
		pack_type(io, NIL_FLAG);
		break;
	}
	case(LUA_TBOOLEAN):
	{
		WRITE_BUF_CHECK_OVERFLOW(L, io, sizeof(LUA_TYPE_FLAG));
		int b = lua_toboolean(L, index);
		if (b) {
			pack_type(io, TRUE_FLAG);
		}
		else {
			pack_type(io, FALSE_FLAG);
		}
		break;
	}
	case(LUA_TSTRING): {
		WRITE_BUF_CHECK_OVERFLOW(L, io, sizeof(LUA_TYPE_FLAG));
		pack_type(io, STRING_FLAG);

		size_t size = 0;
		const char* s = lua_tolstring(L, index, &size);
		WRITE_BUF_CHECK_OVERFLOW(L, io, size);
		pack_string(io, s, size);
		break;
	}
	case(LUA_TNUMBER):
	{

		LONG_LONG value = lua_tointeger(L, index);
		pack_number_help(L, io, value);
		break;
	}
	case(LUA_TTABLE):
	{
		WRITE_BUF_CHECK_OVERFLOW(L, io, sizeof(LUA_TYPE_FLAG));
		pack_type(io, TABLE_FLAG);
		// table size
		int table_index = lua_absindex(L, index);
		TABLE_SIZE_TYPE table_size = 0;
		TABLE_SIZE_TYPE* p = pack_size(io, table_size);
		lua_checkstack(L, top + 2);
		// first key
		lua_pushnil(L);
		// push key and value
		while (lua_next(L, table_index)) {
			table_size += 1;
			// pack key
			pack_arg_help(L, io, -2);
			// pack value
			pack_arg_help(L, io, -1);
			// pop value
			lua_pop(L, 1);
		}
		*p = table_size;
		break;
	}default: {
		lua_pushnil(L);
		break;
	}
	stackDeep--;
	return 1;
	}
}





int unpack_to_arg_help(lua_State* L, LuaPackBuf* io) {
	int top = lua_gettop(L);
	LUA_TYPE_FLAG t = 0;
	lua_checkstack(L, top + 5);
	{
		UNPACK_BYTES(L, io, LUA_TYPE_FLAG);
		t = value;
	}
	switch (t)
	{
	case(NIL_FLAG): {
		lua_pushnil(L);
		break;
	}
	case(TRUE_FLAG): {
		lua_pushboolean(L, 1);
		break;
	}
	case(FALSE_FLAG): {
		lua_pushboolean(L, 0);
		break;
	}
	case(SHORT_FLAG): {
		UNPACK_BYTES(L, io, SHORT);
		lua_pushinteger(L, value);
		break;
	}
	case(INT_FLAG): {
		UNPACK_BYTES(L, io, INT);
		lua_pushinteger(L, value);
		break;
	}
	case(LONG_FLAG): {
		UNPACK_BYTES(L, io, LONG);
		lua_pushinteger(L, value);
		break;
	}
	case(LONG_LONG_FLAG): {
		UNPACK_BYTES(L, io, LONG_LONG);
		lua_pushinteger(L, value);
		break;
	}
	case(STRING_FLAG): {
		UNPACK_BYTES(L, io, U_SHORT);
		U_SHORT length = value;
		char* str = (char*) malloc(sizeof(char) * length);
		memcpy(str, (io->buf + io->read_size), length);
		lua_pushlstring(L, str, length);
		free(str);
		io->read_size += length;
		break;
	}
	case(TABLE_FLAG): {
		UNPACK_BYTES(L, io, TABLE_SIZE_TYPE);
		TABLE_SIZE_TYPE size = value;
		lua_newtable(L);
		int table_index = lua_gettop(L);
		for (int i = 0; i < size; i++) {
			// key
			unpack_to_arg_help(L, io);
			// value
			unpack_to_arg_help(L, io);
			lua_settable(L, table_index);
		}
		break;
	}
	default:
		lua_pushnil(L);
		break;
	}
	return 1;
}