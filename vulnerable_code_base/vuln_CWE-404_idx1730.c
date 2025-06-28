void AICast_ScriptParse( cast_state_t *cs ) {
	gentity_t   *ent;
	char        *pScript;
	char        *token;
	qboolean wantName;
	qboolean inScript;
	int eventNum;
	int numEventItems;
	cast_script_event_t *curEvent;
	char params[MAX_QPATH];
	cast_script_stack_action_t  *action;
	int i;
	int bracketLevel;
	qboolean buildScript;        
	if ( !level.scriptAI ) {
		return;
	}
	ent = &g_entities[cs->entityNum];
	if ( !ent->aiName ) {
		return;
	}
	buildScript = qtrue;
	pScript = level.scriptAI;
	wantName = qtrue;
	inScript = qfalse;
	COM_BeginParseSession( "AICast_ScriptParse" );
	bracketLevel = 0;
	numEventItems = 0;
	memset( cast_temp_events, 0, sizeof( cast_temp_events ) );
	while ( 1 )
	{
		token = COM_Parse( &pScript );
		if ( !token[0] ) {
			if ( !wantName ) {
				G_Error( "AICast_ScriptParse(), Error (line %d): '}' expected, end of script found.\n", COM_GetCurrentParseLine() );
			}
			break;
		}
		if ( token[0] == '}' ) {
			if ( inScript ) {
				break;
			}
			if ( wantName ) {
				G_Error( "AICast_ScriptParse(), Error (line %d): '}' found, but not expected.\n", COM_GetCurrentParseLine() );
			}
			wantName = qtrue;
		} else if ( token[0] == '{' )    {
			if ( wantName ) {
				G_Error( "AICast_ScriptParse(), Error (line %d): '{' found, NAME expected.\n", COM_GetCurrentParseLine() );
			}
		} else if ( wantName )   {
			if ( !Q_strcasecmp( ent->aiName, token ) ) {
				inScript = qtrue;
				numEventItems = 0;
			}
			wantName = qfalse;
		} else if ( inScript )   {
			if ( !Q_strcasecmp( token, "attributes" ) ) {
				AICast_CheckLevelAttributes( cs, ent, &pScript );
				continue;
			}
			eventNum = AICast_EventForString( token );
			if ( eventNum < 0 ) {
				G_Error( "AICast_ScriptParse(), Error (line %d): unknown event: %s.\n", COM_GetCurrentParseLine(), token );
			}
			if ( numEventItems >= MAX_SCRIPT_EVENTS ) {
				G_Error( "AICast_ScriptParse(), Error (line %d): MAX_SCRIPT_EVENTS reached (%d)\n", COM_GetCurrentParseLine(), MAX_SCRIPT_EVENTS );
			}
			if ( !Q_stricmp( token, "friendlysightcorpse" ) ) {
				cs->aiFlags &= ~AIFL_CORPSESIGHTING;
			}
			curEvent = &cast_temp_events[numEventItems];
			curEvent->eventNum = eventNum;
			memset( params, 0, sizeof( params ) );
			while ( ( token = COM_Parse( &pScript ) ) && ( token[0] != '{' ) )
			{
				if ( !token[0] ) {
					G_Error( "AICast_ScriptParse(), Error (line %d): '}' expected, end of script found.\n", COM_GetCurrentParseLine() );
				}
				if ( eventNum == 13 ) {    
					if ( strlen( token ) > 1 ) {
						if ( BG_IndexForString( token, animStateStr, qtrue ) < 0 ) {
							G_Error( "AICast_ScriptParse(), Error (line %d): unknown state type '%s'.\n", COM_GetCurrentParseLine(), token );
						}
					}
				}
				if ( strlen( params ) ) {  
					Q_strcat( params, sizeof( params ), " " );
				}
				Q_strcat( params, sizeof( params ), token );
			}
			if ( strlen( params ) ) {  
				curEvent->params = G_Alloc( strlen( params ) + 1 );
				Q_strncpyz( curEvent->params, params, strlen( params ) + 1 );
			}
			while ( ( token = COM_Parse( &pScript ) ) && ( token[0] != '}' ) )
			{
				if ( !token[0] ) {
					G_Error( "AICast_ScriptParse(), Error (line %d): '}' expected, end of script found.\n", COM_GetCurrentParseLine() );
				}
				action = AICast_ActionForString( cs, token );
				if ( !action ) {
					G_Error( "AICast_ScriptParse(), Error (line %d): unknown action: %s.\n", COM_GetCurrentParseLine(), token );
				}
				curEvent->stack.items[curEvent->stack.numItems].action = action;
				memset( params, 0, sizeof( params ) );
				token = COM_ParseExt( &pScript, qfalse );
				for ( i = 0; token[0]; i++ )
				{
					if ( strlen( params ) ) {  
						Q_strcat( params, sizeof( params ), " " );
					}
					if ( i == 0 ) {
						if ( !Q_stricmp( action->actionString, "playsound" ) ) {
							G_SoundIndex( token );
						}
						if (    buildScript && (
									!Q_stricmp( action->actionString, "mu_start" ) ||
									!Q_stricmp( action->actionString, "mu_play" ) ||
									!Q_stricmp( action->actionString, "mu_queue" ) ||
									!Q_stricmp( action->actionString, "startcam" ) ||
									!Q_stricmp( action->actionString, "startcamblack" ) )
								) {
							if ( strlen( token ) ) {  
								trap_SendServerCommand( cs->entityNum, va( "addToBuild %s\n", token ) );
							}
						}
						if ( !Q_stricmp( action->actionString, "giveweapon" ) ) {  
							gitem_t *weap = BG_FindItem2( token );     
							RegisterItem( weap );    
						}
						if ( !Q_stricmp( action->actionString, "changelevel" ) ) {
							Q_strncpyz( level.nextMap, token, sizeof( level.nextMap ) );
							trap_Cvar_Set( "nextmap", level.nextMap );
						}
					}
					if ( strrchr( token,' ' ) ) {  
						Q_strcat( params, sizeof( params ), "\"" );
					}
					Q_strcat( params, sizeof( params ), token );
					if ( strrchr( token,' ' ) ) {  
						Q_strcat( params, sizeof( params ), "\"" );
					}
					token = COM_ParseExt( &pScript, qfalse );
				}
				if ( strlen( params ) ) {  
					curEvent->stack.items[curEvent->stack.numItems].params = G_Alloc( strlen( params ) + 1 );
					Q_strncpyz( curEvent->stack.items[curEvent->stack.numItems].params, params, strlen( params ) + 1 );
				}
				curEvent->stack.numItems++;
				if ( curEvent->stack.numItems >= AICAST_MAX_SCRIPT_STACK_ITEMS ) {
					G_Error( "AICast_ScriptParse(): script exceeded MAX_SCRIPT_ITEMS (%d), line %d\n", AICAST_MAX_SCRIPT_STACK_ITEMS, COM_GetCurrentParseLine() );
				}
			}
			numEventItems++;
		} else     
		{
			while ( ( token = COM_Parse( &pScript ) ) )
			{
				if ( !token[0] ) {
					G_Error( "AICast_ScriptParse(), Error (line %d): '}' expected, end of script found.\n", COM_GetCurrentParseLine() );
				} else if ( token[0] == '{' ) {
					bracketLevel++;
				} else if ( token[0] == '}' ) {
					if ( !--bracketLevel ) {
						break;
					}
				}
			}
		}
	}
	if ( numEventItems > 0 ) {
		cs->castScriptEvents = G_Alloc( sizeof( cast_script_event_t ) * numEventItems );
		memcpy( cs->castScriptEvents, cast_temp_events, sizeof( cast_script_event_t ) * numEventItems );
		cs->numCastScriptEvents = numEventItems;
		cs->castScriptStatus.castScriptEventIndex = -1;
	}
}