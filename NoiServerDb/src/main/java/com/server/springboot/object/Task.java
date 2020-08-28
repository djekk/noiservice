package com.server.springboot.object;

/*
 * Copyright 2001-2020 by HireRight, Inc. All rights reserved.
 * This software is the confidential and proprietary information
 * of HireRight, Inc. Use is subject to license terms.
 */

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import lombok.experimental.SuperBuilder;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.SequenceGenerator;
import javax.persistence.Table;
import java.util.Date;

@Getter
@Setter
@SuperBuilder
@AllArgsConstructor
@NoArgsConstructor
@Entity
@Table(name="task", schema = "public")
public class Task
{
	
	@Id
	@SequenceGenerator(name = "task_id_seq", sequenceName = "task_id_seq", allocationSize = 1)
	@GeneratedValue(strategy = GenerationType.SEQUENCE, generator = "task_id_seq")
	private Integer id;
	
	@Column(name="customer")
	private String customer;
	
	@Column(name="taskid")
	private Integer taskid;
	
	@Column(name="inquiry")
	private String inquiry;
	
	@Column(name="result")
	private String result;
	
	@Column(name="state")
	private String state;
	
	@Column(name="lastupdated")
	private Date lastupdated;
	
	public String getResult()
	{
		return result;
	}
	
	public void setResult(String result)
	{
		this.result = result;
	}
	
	public Integer getId()
	{
		return id;
	}
	
	public void setState(String state)
	{
		this.state = state;
	}
	
	public void setLastUpdated(Date lastupdated)
	{
		this.lastupdated = lastupdated;
	}
	
	public String getCustomer()
	{
		return customer;
	}
	
	public Integer getTaskId()
	{
		return taskid;
	}
	
	public void setId(Integer id)
	{
		this.id = id;
	}
	
	public String getState()
	{
		return state;
	}
	
	// other getters and setters...
}
