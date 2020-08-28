package com.server.springboot.states;

/*
 * Copyright 2001-2020 by HireRight, Inc. All rights reserved.
 * This software is the confidential and proprietary information
 * of HireRight, Inc. Use is subject to license terms.
 */

public class TaskStateMachine
{
	static public final String NEW = "NEW";
	static public final String IN_PROGRESS = "IN PROGRESS";
	static public final String READY = "READY";
	
	public static boolean isReady(String state)
	{
		return READY.equals(state);
	}
}
