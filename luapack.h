//
// Created by lsjlkz on 2023/5/17.
//

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define LUA_PACKAGE_META "LUA_PACK_META"

// check first is pack
#define LUA_CHECK_BUF_TYPE(L) \
	((LuaPackBuf*) luaL_checkudata(L, 1, LUA_PACKAGE_META))


// check second
#define LUA_CHECK_TYPE_CAN_PACK(L) \
	(LUA_CHECK_LAST_ARG_TYPE(L,LUA_TSTRING)||LUA_CHECK_LAST_ARG_TYPE(L,LUA_TTABLE)||LUA_CHECK_LAST_ARG_TYPE(L,LUA_TNIL)||LUA_CHECK_LAST_ARG_TYPE(L,LUA_TBOOLEAN)||LUA_CHECK_LAST_ARG_TYPE(L,LUA_TNUMBER))

#define LUA_CHECK_LAST_ARG_TYPE(L,t) \
	(lua_type(L, -1) == t)

#define SHORT_FLAG				-96
#define INT_FLAG				-97
#define LONG_FLAG				-98
#define LONG_LONG_FLAG			-99
#define NIL_FLAG				-100
#define TRUE_FLAG				-101
#define FALSE_FLAG				-102
#define TABLE_FLAG				-103
#define STRING_FLAG				-106


#define LUA_TYPE_FLAG char

#define SHORT short
#define INT int
#define LONG long
#define LONG_LONG long long
#define U_SHORT unsigned short

#define TABLE_SIZE_TYPE U_SHORT

#define MAX_STACK_DEEP			30

#define MAX_BUF_SIZE 65535

typedef struct {
	size_t write_size;
	size_t read_size;
	char buf[MAX_BUF_SIZE];
} LuaPackBuf;


LUA_API int pack_arg(lua_State* L);

LUA_API int unpack_to_arg(lua_State* L);

LUA_API int print(lua_State* L);

LUA_API int new_pack(lua_State* L);

LUA_API int obj_len(lua_State* L);

static luaL_Reg pack_functions[] = {
	{"new", new_pack},
	{"pack", pack_arg},
	{"unpack", unpack_to_arg},
	{NULL, NULL}
};



static luaL_Reg pack_methods[] = {
	{"__len", obj_len},
	{"print", print},
	{NULL, NULL}
};

LUA_API int open_pack_lib(lua_State* L) {
	luaL_newmetatable(L, "__LUA__PACK__TABLE");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, pack_functions, 0);
	return 1;
}

LUAMOD_API int open_pack_obj(lua_State* L) {
	luaL_requiref(L, "lua.pack", open_pack_lib, 0);

	luaL_newmetatable(L, LUA_PACKAGE_META);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, pack_methods, 0);
	lua_setglobal(L, LUA_PACKAGE_META);
	return 1;
}


