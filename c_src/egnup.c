#include "erl_nif.h"
#include "gnuplot_i.h"

static ErlNifResourceType* egnup_RESOURCE;

typedef struct {
  gnuplot_ctrl *ctrl;
} egnup_handle;

// Prototypes
static ERL_NIF_TERM egnup_new(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM egnup_cmd(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);

static ErlNifFunc nif_funcs[] =
{
    {"new", 0, egnup_new},
    {"cmd", 2, egnup_cmd}
};

static ERL_NIF_TERM egnup_new(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    gnuplot_ctrl *ctrl = gnuplot_init();
    ctrl = NULL;

    if (ctrl == NULL) {
	return enif_make_tuple2(env, enif_make_atom(env, "error"), enif_make_atom(env, "gnuplot_init_failed"));
    }

    egnup_handle* handle = enif_alloc_resource(egnup_RESOURCE, sizeof(egnup_handle));

    handle->ctrl = ctrl;

    ERL_NIF_TERM result = enif_make_resource(env, handle);
    enif_release_resource(handle);
    return enif_make_tuple2(env, enif_make_atom(env, "ok"), result);
}

static ERL_NIF_TERM egnup_cmd(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    egnup_handle *handle;
    unsigned int cmdlen;
    char *cmdbuf;

    if (!enif_get_resource(env, argv[0], egnup_RESOURCE, (void **)&handle)) {
        return enif_make_badarg(env);
    }

    if (!enif_get_list_length(env, argv[1], &cmdlen)){
        return enif_make_badarg(env);
    }

    cmdlen++; // adjust for zero-termination
    cmdbuf = malloc(cmdlen); memset(cmdbuf, 0, cmdlen);

    if (0 == enif_get_string(env, argv[1], cmdbuf, cmdlen, ERL_NIF_LATIN1)){
        return enif_make_badarg(env);
    }

    gnuplot_cmd(handle->ctrl, cmdbuf);
    free(cmdbuf);

    return enif_make_tuple1(env, enif_make_atom(env, "ok"));
}

static void egnup_resource_cleanup(ErlNifEnv* env, void* arg)
{
    /* Delete any dynamically allocated memory stored in egnup_handle */
    egnup_handle* handle = (egnup_handle*)arg;
    gnuplot_close(handle->ctrl);
}

static int on_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    ErlNifResourceFlags flags = ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER;
    ErlNifResourceType* rt = enif_open_resource_type(env, NULL, "egnup_resource", &egnup_resource_cleanup, flags, NULL);
    if (rt == NULL)
        return -1;

    egnup_RESOURCE = rt;

    return 0;
}

ERL_NIF_INIT(egnup, nif_funcs, &on_load, NULL, NULL, NULL);
