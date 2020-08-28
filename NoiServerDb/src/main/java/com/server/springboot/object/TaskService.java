package com.server.springboot.object;

/*
 * Copyright 2001-2020 by HireRight, Inc. All rights reserved.
 * This software is the confidential and proprietary information
 * of HireRight, Inc. Use is subject to license terms.
 */

import java.sql.Timestamp;
import java.util.List;

import javax.transaction.Transactional;

import com.server.springboot.states.TaskStateMachine;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
@Transactional
public class TaskService
{
	@Autowired
	private TaskRepository repo;
	
	public List<Task> listAll() {	return repo.findAll(); }
	
	public void save(Task task)
	{
		repo.save(task);
	}
	
	public Task get(Integer id)
	{
		return repo.findById(id).get();
	}
	
	
	public void delete(Integer id)
		{
		repo.deleteById(id);
	}
	
	public Task getByCustomerAndTaskId(String customer, Integer taskid)
	{
		return repo.findByCustomerAndtaskId(customer, taskid);
	}
	
	public Task getInquiryLock()
	{
		Task task = repo.getInquiryLock();
		task.setState(TaskStateMachine.IN_PROGRESS);
		task.setLastUpdated(new Timestamp(System.currentTimeMillis()));
		save(task);
		
		return task;
	}
}
