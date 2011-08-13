-module(egnup).

% API
-export([start_link/0,
	 raw_command/2]).

% gen_server
-behaviour(gen_server).
-export([init/1, handle_call/3, handle_cast/2, handle_info/2, terminate/2, code_change/3]).

-record(state, { nif_ref = undefined }).

-on_load(module_init/0).

-define(nif_stub, nif_stub_error(?LINE)).

nif_stub_error(Line) ->
    erlang:nif_error({nif_not_loaded,module,?MODULE,line,Line}).

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

%% ===================================================================
%% API
%% ===================================================================

start_link() ->
    gen_server:start_link(?MODULE, [], []).

raw_command(ServerRef, CmdStr) ->
    gen_server:cast(ServerRef, {raw_cmd, CmdStr}).

%% ===================================================================
%% NIF Crap
%% ===================================================================

module_init() ->
    PrivDir = case code:priv_dir(?MODULE) of
                  {error, bad_name} ->
                      EbinDir = filename:dirname(code:which(?MODULE)),
                      AppPath = filename:dirname(EbinDir),
                      filename:join(AppPath, "priv");
                  Path ->
                      Path
              end,
    erlang:load_nif(filename:join(PrivDir, ?MODULE), 0).

new() ->
    ?nif_stub.

cmd(_Ref, _Cmd) ->
    ?nif_stub.

%% ===================================================================
%% gen_server callbacks
%% ===================================================================

init(_Args) ->
    case os:find_executable("gnuplot") of
	false ->
	    {stop, gnuplot_not_found};
	_ ->
	    case new() of
		{ok, NifRef} ->
		    State = #state{ nif_ref = NifRef },
		    {ok, State};
		{error, Reason} ->
		    {stop, Reason}
	    end
    end.

handle_call(_Request, _From, State) ->
    {stop, unimplemented, State}.

handle_cast({raw_cmd, CmdStr}, State) ->
    cmd(State#state.nif_ref, CmdStr),
    {noreply, State}.

handle_info(_Info, State) ->
    {stop, unimplemented, State}.

terminate(_Reason, _State) ->
    ok.

code_change(_OldVsn, State, _Extra) ->
    {ok, State}.

%% ===================================================================
%% EUnit tests
%% ===================================================================
-ifdef(TEST).

basic_test() ->
    {ok, Ref} = new(),
    ?assertEqual(ok, myfunction(Ref)).

-endif.
